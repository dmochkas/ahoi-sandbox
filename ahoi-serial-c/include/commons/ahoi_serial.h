#ifndef AHOI_SERIAL_H
#define AHOI_SERIAL_H

#include <stdint.h>

int open_serial_port(const char *port, int baudrate);
void send_ahoi_packet(int fd, uint8_t dst_id, uint8_t src_id, uint8_t type, const uint8_t *payload, size_t payload_len,const uint8_t *tag, size_t tag_len);
void decode_ahoi_packet(uint8_t *data,int len);

#endif