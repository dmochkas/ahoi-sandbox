#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include "commons/serial_utils.h"
#include "commons/common_defs.h"
#include "commons/cli_helper.h"
#include "commons/ahoi_service.h"

static uint8_t payload_buf[MAX_PAYLOAD_SIZE];

int main(int argc, char *argv[]) {
    uint8_t key_arg[KEY_SIZE];
    if (parse_cli_arguments(argc, argv, key_arg, KEY_SIZE) == CLI_PARSE_KO) {
        fprintf(stderr, "Error parsing cli arguments\n");
        return EXIT_FAILURE;
    }

    store_key(key_arg);

    const char *port = SENDER_SERIAL_PORT;
    int baudrate = B115200;
    int fd = open_serial_port(port, baudrate);

    if (fd == -1) {
        fprintf(stderr, "Error opening serial port\n");
        return EXIT_FAILURE;
    }

    char plaintext[MAX_PAYLOAD_SIZE];
    printf("Enter the word to send: ");
    if (fgets(plaintext, sizeof(plaintext), stdin) == NULL) {
        fprintf(stderr, "Error reading input\n");
        close(fd);
        return EXIT_FAILURE;
    }
    plaintext[strcspn(plaintext, "\n")] = '\0';
    size_t pl_len = strlen(plaintext);

    ahoi_packet_t ahoi_packet = {0};
    ahoi_packet.payload = payload_buf;

    if (generate_secure_ahoi_packet(0x58, 0x56, 0x00, 0x00, plaintext, pl_len, &ahoi_packet) != PACKET_GEN_OK) {
        fprintf(stderr, "Error generating ahoi packet\n");
        close(fd);
        return EXIT_FAILURE;
    }

    send_ahoi_packet(fd, &ahoi_packet);
    
    close(fd);
    return EXIT_SUCCESS;
}