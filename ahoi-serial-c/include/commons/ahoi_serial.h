#ifndef AHOI_SERIAL_H
#define AHOI_SERIAL_H

#include <stdint.h>
#include <string.h>

int open_serial_port(const char *port, int baudrate);
void send_ahoi_packet(int fd, const uint8_t *header,
                     const uint8_t *payload, size_t payload_len,
                     const uint8_t *tag, size_t tag_len);
void decode_ahoi_packet(uint8_t *data,int len);
int process_key(const char *hex, uint8_t *key_buffer, size_t buffer_size);
void print_usage(const char *prog_name);
#endif