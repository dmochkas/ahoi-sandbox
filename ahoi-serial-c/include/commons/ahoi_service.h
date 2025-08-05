#ifndef AHOI_SERVICE_H
#define AHOI_SERVICE_H

#include <stddef.h>
#include <stdint.h>

#include "commons/common_defs.h"

#define SECONDS_IN_HOUR 3600
#define RECV_BUF_SIZE 512

extern uint8_t seq_number;

typedef struct {
    uint8_t src;
    uint8_t dst;
    uint8_t type;
    uint8_t flags;
    uint8_t seq;
    uint8_t pl_size;
    uint8_t* payload;
} ahoi_packet_t;

typedef enum {
    NONCE_GEN_OK,
    NONCE_GEN_KO
} nonce_gen_status;

typedef enum {
    PACKET_GEN_OK,
    PACKET_GEN_KO
} packet_gen_status;

typedef enum {
    PACKET_SEND_OK,
    PACKET_SEND_KO
} packet_send_status;

typedef enum {
    PACKET_RECV_OK,
    PACKET_RECV_KO
} packet_rcv_status;

typedef enum {
    PACKET_DECODE_OK,
    PACKET_DECODE_KO
} packet_decode_status;

void store_key(uint8_t* new_key);

void print_packet(const ahoi_packet_t* ahoi_packet);

nonce_gen_status generate_nonce(uint8_t seq, uint8_t* buf, size_t nonce_size);

packet_gen_status generate_secure_ahoi_packet(uint8_t src, uint8_t dst,
                                              uint8_t type, uint8_t flags,
                                              const uint8_t* payload, size_t payload_size, ahoi_packet_t* ahoi_packet);

packet_send_status send_ahoi_packet(int fd, const ahoi_packet_t* ahoi_packet);

packet_rcv_status receive_ahoi_packet(int fd, void (*cb)(const ahoi_packet_t*));

packet_decode_status decode_ahoi_packet(const uint8_t *data, size_t len, ahoi_packet_t* ahoi_packet);

#endif