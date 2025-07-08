#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

#include "commons/ahoi_serial.h"

int open_serial_port(const char *port, int baudrate) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Error opening serial port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

int process_key(const char *hex, uint8_t *key_buffer, size_t buffer_size) {
    size_t hex_len = strlen(hex);
    
    if (hex_len < 2 || hex_len > buffer_size*2) {
        fprintf(stderr, "Key must be 2-%d hex characters\n", buffer_size*2);
        return -1;
    }

    // Pad with zeros if needed
    memset(key_buffer, 0, buffer_size);
    
    // Convert what we can from the hex string
    for (size_t i = 0; i < hex_len/2 && i < buffer_size; i++) {
        if (!isxdigit(hex[i*2]) || !isxdigit(hex[i*2+1])) {
            fprintf(stderr, "Invalid hex characters in key\n");
            return -1;
        }
        sscanf(hex + i*2, "%2hhx", &key_buffer[i]);
    }
    
    // Handle odd-length hex string
    if (hex_len % 2 != 0) {
        if (!isxdigit(hex[hex_len-1])) {
            fprintf(stderr, "Invalid hex character in key\n");
            return -1;
        }
        sscanf(hex + hex_len-1, "%1hhx", &key_buffer[hex_len/2]);
    }

    return 0;
}

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -k <key_in_hex> \n", prog_name);
}
