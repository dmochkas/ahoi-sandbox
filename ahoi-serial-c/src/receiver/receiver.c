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


static uint8_t key[KEY_SIZE]= {0}; 
static uint8_t decrypted[256];  // Buffer for the cipher

void decode_ahoi_packet(uint8_t *data, int len) {
    if (len < HEADER_SIZE) {
        printf("Packet too short\n");
        return;
    }

    uint8_t *header = data;
    uint8_t total_len = header[5];
    uint8_t ciphertext_len = total_len - TAG_SIZE;
    
    // Verify
    if (ciphertext_len <= 0 || ciphertext_len > sizeof(decrypted) - 1 || 
        HEADER_SIZE + ciphertext_len + TAG_SIZE > len) {
        printf("Invalid lengths: total=%d, cipher=%d, tag=%d, received=%d\n",
              total_len, ciphertext_len, TAG_SIZE, len);
        return;
    }

    uint8_t *ciphertext = data + HEADER_SIZE;
    uint8_t *tag = data + HEADER_SIZE + ciphertext_len;
 

    time_t now = time(NULL);
    time_t hour_timestamp = (now / 3600) * 3600; // Round to hours
    uint8_t sequence_number = header[4]; // Get sequence number from header
    uint8_t nonce[NONCE_SIZE] = {0};
    memcpy(nonce, &hour_timestamp, sizeof(hour_timestamp));
    nonce[sizeof(hour_timestamp)] = sequence_number;

    printf("=== DEBUG ===\n");
    printf("Nonce: ");
    for(int i=0; i<NONCE_SIZE; i++) printf("%02X", nonce[i]);
    printf("\nAD Header: ");
    for(int i=0; i<HEADER_SIZE; i++) printf("%02X", header[i]);
    printf("\nCiphertext (%d): ", ciphertext_len);
    for(int i=0; i<ciphertext_len; i++) printf("%02X", ciphertext[i]);
    printf("\nTag: ");
    for(int i=0; i<TAG_SIZE; i++) printf("%02X", tag[i]);
    printf("\n=============\n");

    int dec_result = ascon_aead_decrypt(
        decrypted,
        tag, ciphertext, ciphertext_len,
        header, HEADER_SIZE,
        nonce, key
    );

    if (dec_result == 0) {
        decrypted[ciphertext_len] = '\0';
        printf("Decrypted: %s\n", decrypted);
    } else {
        printf("DECRYPTION FAILED!\n");
    }
}

int main(int argc, char *argv[]) {
    const char *port = "/dev/cu.usbserial-1410";
    int fd = open_serial_port(port, B115200);
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

    printf("Using key: ");
    for (int i = 0; i < KEY_SIZE; i++) printf("%02X", key[i]);
    printf("\n");

    if (fd == -1) {
        fprintf(stderr, "Error opening serial port\n");
        return 1;
    }

    printf("Waiting for AHOI packets (using ASCON decryption)...\n");

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