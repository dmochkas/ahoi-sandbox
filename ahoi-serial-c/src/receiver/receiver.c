#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include "commons/serial_utils.h"
#include "commons/cli_helper.h"
#include "commons/common_defs.h"
#include "commons/ahoi_service.h"


void handle_ahoi_packet(const ahoi_packet_t* packet) {
    print_packet(packet);
}

int main(int argc, char *argv[]) {
    uint8_t key_arg[KEY_SIZE];
    if (parse_cli_arguments(argc, argv, key_arg, KEY_SIZE) == CLI_PARSE_KO) {
        fprintf(stderr, "Error parsing cli arguments\n");
        return EXIT_FAILURE;
    }

    store_key(key_arg);

    const char *port = RECEIVER_SERIAL_PORT;
    int fd = open_serial_port(port, B115200);

    if (fd == -1) {
        fprintf(stderr, "Error opening serial port\n");
        return 1;
    }

    printf("Waiting for AHOI packets (using ASCON decryption)...\n");
    receive_ahoi_packet(fd, handle_ahoi_packet);

    close(fd);
    return 0;
}