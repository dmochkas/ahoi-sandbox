#ifndef AHOI_SERIAL_H
#define AHOI_SERIAL_H

#include <stdint.h>

int open_serial_port(const uint8_t *port, int baudrate);
void decode_ahoi_packet(uint8_t *data,int len);

#endif