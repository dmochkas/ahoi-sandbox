#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>

#include "ascon.h"
#include "commons/ahoi_serial.h"
#include "commons/commons.h"

// Clave and nonce
static uint8_t key[KEY_SIZE] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
}; 
static uint8_t nonce[NONCE_SIZE] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
}; //example, use one for each message

int main() {
    const char *port = SENDER_SERIAL_PORT;
    int baudrate = B115200;
    int fd = open_serial_port(port, baudrate);

    if (fd == -1) {
        fprintf(stderr, "Error opening serial port\n");
        return 1;
    }

    char plaintext[256];
    printf("Enter the word to send: ");
    if (fgets(plaintext, sizeof(plaintext), stdin) == NULL) {
        fprintf(stderr, "Error reading input\n");
        close(fd);
        return 1;
    }
    plaintext[strcspn(plaintext, "\n")] = '\0'; // delete newline
    size_t mlen = strlen(plaintext);

    // Create Associated Data (AD)
    uint8_t ad_header[HEADER_SIZE] = {0x56, 0x58, 0x00, 0x00, 0x00, 0x00};
    size_t adlen = sizeof(ad_header);

    // Buffers for cipher
    uint8_t ciphertext[256];
    uint8_t tag[TAG_SIZE];

    // encrypt the message
    int enc_result = ascon_aead_encrypt(
        tag, ciphertext,
        (const uint8_t*)plaintext, mlen,
        ad_header, adlen,
        nonce, key
    );

    if (enc_result != 0) {
        fprintf(stderr, "Encryption failed!\n");
        close(fd);
        return 1;
    }

    // show the information
    printf("Original: %s\n", plaintext);
    printf("Ciphertext (%zu bytes): ", mlen);
    for (size_t i = 0; i < mlen; i++) printf("%02X", ciphertext[i]);
    printf("\nTag (%d bytes): ", TAG_SIZE);
    for (int i = 0; i < TAG_SIZE; i++) printf("%02X", tag[i]);
    printf("\n");

    // send ciphertext + tag
    send_ahoi_packet(fd, 0x56, 0x58, 0x00, ciphertext, mlen, tag, TAG_SIZE);
    
    close(fd);
    return 0;
}

void send_ahoi_packet(int fd, uint8_t dst_id, uint8_t src_id, uint8_t type, 
                     const uint8_t *payload, size_t payload_len,
                     const uint8_t *tag, size_t tag_len) {
    static uint8_t sequence_number =0;
    uint8_t header[HEADER_SIZE] = {src_id, dst_id, type, 0x00, sequence_number, (uint8_t)(payload_len + tag_len)};
    uint8_t packet[512];
    int packet_len = 0;

    // Framing: DLE-STX
    packet[packet_len++] = 0x10;
    packet[packet_len++] = 0x02;

    // Escape header
    for (int i = 0; i < HEADER_SIZE; i++) {
        if (header[i] == 0x10) packet[packet_len++] = 0x10;
        packet[packet_len++] = header[i];
    }

    // Escape payload (ciphertext)
    for (size_t i = 0; i < payload_len; i++) {
        if (payload[i] == 0x10) packet[packet_len++] = 0x10;
        packet[packet_len++] = payload[i];
    }

    // Escape tag
    for (size_t i = 0; i < tag_len; i++) {
        if (tag[i] == 0x10) packet[packet_len++] = 0x10;
        packet[packet_len++] = tag[i];
    }

    // Framing: DLE-ETX
    packet[packet_len++] = 0x10;
    packet[packet_len++] = 0x03;

    ssize_t bytes_written = write(fd, packet, packet_len);
    if (bytes_written < 0) {
        perror("Error writing to serial port");
        return;
    } else if (bytes_written != packet_len) {
        fprintf(stderr, "Warning: Partial write (%zd of %d bytes)\n", bytes_written, packet_len);
    }

    printf("Sent packet (%d bytes): ", packet_len);
    for (int i = 0; i < packet_len; i++) printf("%02X ", packet[i]);
    printf("\n");
    sequence_number = (sequence_number +1) % 256;
 }