#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "commons/ahoi_serial.h"

int main() {
    const char *port = SENDER_SERIAL_PORT;
    int baudrate = B115200;
    int fd = open_serial_port(port, baudrate);

    if (fd == -1) return 1;

    char payload[256];
    printf("Enter the word to send: ");
    fgets(payload, sizeof(payload), stdin);
    payload[strcspn(payload, "\n")] = 0;  // Remove newline

    send_ahoi_packet(fd, 0x56, 0x58, 0x00, payload);
    close(fd);
    return 0;
}

void send_ahoi_packet(int fd, uint8_t dst_id, uint8_t src_id, uint8_t type, const char *payload) {
    uint8_t header[6] = {src_id, dst_id, type, 0x00, 0x00, (uint8_t)strlen(payload)};
    uint8_t packet[512];
    int packet_len = 0;

    // Framing: DLE-STX
    packet[packet_len++] = 0x10;
    packet[packet_len++] = 0x02;

    // Escapar header + payload
    for (int i = 0; i < 6; i++) {
        if (header[i] == 0x10) packet[packet_len++] = 0x10;
        packet[packet_len++] = header[i];
    }
    for (int i = 0; payload[i] != '\0'; i++) {
        if (payload[i] == 0x10) packet[packet_len++] = 0x10;
        packet[packet_len++] = payload[i];
    }

    // Framing: DLE-ETX
    packet[packet_len++] = 0x10;
    packet[packet_len++] = 0x03;

    write(fd, packet, packet_len);

    printf("Sent: ");
    for (int i = 0; i < packet_len; i++) printf("%02X ", packet[i]);
    printf("\n");
}