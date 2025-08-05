#ifndef AHOI_SERIAL_H
#define AHOI_SERIAL_H

#include <stdint.h>

int open_serial_port(const uint8_t *port, int baudrate);

#endif