#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "../include/ahoi_serial.h"

void decode_ahoi_packet(uint8_t *data, int len) {
    if (len < 6) return;

    uint8_t header[6];
    memcpy(header, data, 6);
    int payload_len = header[5];

    printf("Received from ID=%d: ", header[0]);
    for (int i = 6; i < 6 + payload_len; i++) {
        putchar(data[i]);
    }
    printf("\n");

    if (len >= 6 + payload_len + 6) {
        uint8_t *footer = data + 6 + payload_len;
        printf("Footer: ");
        for (int i = 0; i < 6; i++) printf("%02X ", footer[i]);
        printf("\n  - Power: %d%%, RSSI: %d%%, Bit errors: %d\n", footer[0], footer[1], footer[2]);
    }
}

int main() {
    const char *port = "/dev/ttyUSB0";
    int fd = open_serial_port(port, B115200);
    if (fd == -1) return 1;

    uint8_t buffer[512];
    int buf_pos = 0;
    int in_packet = 0;

    printf("Waiting for AHOI packets...\n");
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
