#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>

#include "ascon.h"
#include "commons/ahoi_serial.h"
//receiver
#define KEY_SIZE 16
#define NONCE_SIZE 16
#define TAG_SIZE 16

// The same as in the sender
static uint8_t key[KEY_SIZE] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
}; 
static uint8_t nonce[NONCE_SIZE] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
};  
static uint8_t decrypted[256];  // Buffer for the cipher

void decode_ahoi_packet(uint8_t *data, int len) {
    if (len < 6) {
        printf("Packet too short\n");
        return;
    }

    uint8_t header[6];
    memcpy(header, data, 6);
    int total_len = header[5];
    
    int ciphertext_len = total_len - TAG_SIZE;
    
    // Verify
    if (ciphertext_len <= 0 || ciphertext_len > sizeof(decrypted) - 1 || 
        6 + ciphertext_len + TAG_SIZE > len) {
        printf("Invalid lengths: total=%d, cipher=%d, tag=%d, received=%d\n",
              total_len, ciphertext_len, TAG_SIZE, len);
        return;
    }

    uint8_t *ciphertext = data + 6;
    uint8_t *tag = data + 6 + ciphertext_len;

    // AD same as the sender
    uint8_t ad_header[6] = {0x58, 0x56, 0x00, 0x00, 0x00, 0x00}; 

    printf("=== DEBUG ===\n");
    printf("Ciphertext (%d): ", ciphertext_len);
    for(int i=0; i<ciphertext_len; i++) printf("%02X", ciphertext[i]);
    printf("\nTag: ");
    for(int i=0; i<TAG_SIZE; i++) printf("%02X", tag[i]);
    printf("\nAD: ");
    for(int i=0; i<6; i++) printf("%02X", ad_header[i]);
    printf("\n=============\n");

    int dec_result = ascon_aead_decrypt(
        decrypted,
        tag, ciphertext, ciphertext_len,
        ad_header, sizeof(ad_header),
        nonce, key
    );

    if (dec_result == 0) {
        decrypted[ciphertext_len] = '\0';
        printf("Decrypted: %s\n", decrypted);
    } else {
        printf("DECRYPTION FAILED!\n");
    }
}

int main() {
    const char *port = "/dev/ttyUSB0";
    int fd = open_serial_port(port, B115200);
    if (fd == -1) {
        fprintf(stderr, "Error opening serial port\n");
        return 1;
    }

    printf("Waiting for AHOI packets (using ASCON decryption)...\n");
    printf("Using key: ");
    for (int i = 0; i < KEY_SIZE; i++) printf("%02X", key[i]);
    printf("\n");

    uint8_t buffer[512];
    int buf_pos = 0;
    int in_packet = 0;

    while (1) {
        uint8_t byte;
        if (read(fd, &byte, 1) != 1) continue;

        if (!in_packet && byte == 0x10) {
            if (read(fd, &byte, 1) == 1 && byte == 0x02) {
                in_packet = 1;
                buf_pos = 0;
            }
        } else if (in_packet) {
            if (byte == 0x10) {
                if (read(fd, &byte, 1) == 1) {
                    if (byte == 0x03) {
                        
                        decode_ahoi_packet(buffer, buf_pos);
                        in_packet = 0;
                    } else if (byte == 0x10) {
                        buffer[buf_pos++] = 0x10;
                    }
                }
            } else {
                buffer[buf_pos++] = byte;
            }
        }
    }

    close(fd);
    return 0;
}