#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

#include "ascon.h"
#include "commons/ahoi_serial.h"
#include "commons/commons.h"

// Key and nonce
static uint8_t key[KEY_SIZE] = {0}; 
static uint8_t sequence_number = 0; // Will be set in send_ahoi_packet


int main(int argc, char *argv[]) {
    const char *port = "/dev/ttyUSB0"; // It's neccesary let that here 
    char *message = NULL;
    int opt;
    int option_index = 0;
    char *key_hex = NULL;

    struct option long_options[] = {
        {"key", required_argument, 0, 'k'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "k:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'k':
                key_hex = optarg;
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (!key_hex) {
        fprintf(stderr, "Error: Encryption key is required\n");
        print_usage(argv[0]);
        return 1;
    }

    if (process_key(key_hex, key, KEY_SIZE) != 0) {
        return 1;
    }

    int baudrate = B115200;
    int fd = open_serial_port(port, baudrate);

    if (fd == -1) {
        fprintf(stderr, "Error opening serial port\n");
        return 1;
    }

    char plaintext[256];
    if (message) {
        strncpy(plaintext, message, sizeof(plaintext));
        plaintext[sizeof(plaintext)-1] = '\0';
    } else {
        printf("Enter the word to send: ");
        if (fgets(plaintext, sizeof(plaintext), stdin) == NULL) {
            fprintf(stderr, "Error reading input\n");
            close(fd);
            return 1;
        }
        plaintext[strcspn(plaintext, "\n")] = '\0';
    }

    size_t mlen = strlen(plaintext);

    // Create Associated Data (AD) (0x58 ID sender and 0x56 ID receiver)
    uint8_t header[HEADER_SIZE] = {0x58, 0x56, 0x00, 0x00, sequence_number, (uint8_t)(mlen + TAG_SIZE)};
    
    // Buffers for cipher
    uint8_t ciphertext[256];
    uint8_t tag[TAG_SIZE];

    //Generate nonce from timestamp and sequence number
    time_t now = time(NULL);
    time_t hour_timestamp = (now / 3600) * 3600; // Round to hours
    
    uint8_t nonce[NONCE_SIZE] = {0};
    memcpy(nonce, &hour_timestamp, sizeof(hour_timestamp));
    nonce[sizeof(hour_timestamp)] = sequence_number;

    printf("Using nonce: ");
    for (int i = 0; i < NONCE_SIZE; i++) printf("%02X", nonce[i]);
    printf("\n");
    
    int enc_result = ascon_aead_encrypt(
        tag, ciphertext,
        (const uint8_t*)plaintext, mlen,
        header, sizeof(header),  // The same header as AD
        nonce, key
    );

    if (enc_result != 0) {
        fprintf(stderr, "Encryption failed!\n");
        close(fd);
        return 1;
    }

    // show the information
    printf("Ciphertext (%zu bytes): ", mlen);
    for (size_t i = 0; i < mlen; i++) printf("%02X", ciphertext[i]);
    printf("\nTag (%d bytes): ", TAG_SIZE);
    for (int i = 0; i < TAG_SIZE; i++) printf("%02X", tag[i]);
    printf("\n");

    // send ciphertext + tag
    send_ahoi_packet(fd,header, ciphertext, mlen, tag, TAG_SIZE);
    
    close(fd);
    return 0;
}

void send_ahoi_packet(int fd, const uint8_t *header,
                     const uint8_t *payload, size_t payload_len,
                     const uint8_t *tag, size_t tag_len) {

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