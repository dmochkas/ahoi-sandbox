#include "commons/ahoi_service.h"

#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#include "ascon.h"

static uint8_t key[KEY_SIZE] = {0};
static uint8_t nonce_buf[NONCE_SIZE] = {0};
static uint8_t ciphertext_buf[MAX_PAYLOAD_SIZE] = {0};
static uint8_t tag_buf[TAG_SIZE] = {0};

static uint8_t recv_buf[RECV_BUF_SIZE] = {0};
static uint8_t payload_buf[MAX_PAYLOAD_SIZE] = {0};
static ahoi_packet_t staging_packet = {
    .payload = payload_buf
};

uint8_t seq_number = 0;

void store_key(uint8_t* new_key) {
    memcpy(key, new_key, KEY_SIZE);
}

void print_packet(const ahoi_packet_t *ahoi_packet) {
    if (ahoi_packet == NULL) {
        printf("ahoi_packet is NULL\n");
        return;
    }

    printf("Ahoi Packet:\n");
    printf("  Source:      %u\n", ahoi_packet->src);
    printf("  Destination: %u\n", ahoi_packet->dst);
    printf("  Type:        %u\n", ahoi_packet->type);
    printf("  Flags:       %u\n", ahoi_packet->flags);
    printf("  Sequence:    %u\n", ahoi_packet->seq);
    printf("  PL Size:     %u\n", ahoi_packet->pl_size);

    if (ahoi_packet->pl_size > 0 && ahoi_packet->payload != NULL) {
        printf("  Payload:     ");
        for (int i = 0; i < ahoi_packet->pl_size; i++) {
            printf("%02x ", ahoi_packet->payload[i]);
        }
        printf("\n");
    }
}

nonce_gen_status generate_nonce(const uint8_t seq, uint8_t* buf, const size_t nonce_size) {
    if (nonce_size < sizeof(time_t) + sizeof(uint8_t)) {
        return NONCE_GEN_KO;
    }

    memset(buf, 0, nonce_size);

    const time_t now = time(NULL);
    const time_t hour_timestamp = htonl(now / SECONDS_IN_HOUR);

    memcpy(buf, &hour_timestamp, sizeof(hour_timestamp));
    memcpy(buf + sizeof(hour_timestamp), &seq, sizeof(seq));

    return NONCE_GEN_OK;
}

packet_gen_status generate_secure_ahoi_packet(const uint8_t src, const uint8_t dst,
                                              const uint8_t type, const uint8_t flags,
                                              const uint8_t* payload, const size_t payload_size, ahoi_packet_t* ahoi_packet) {
    if (payload_size > MAX_PAYLOAD_SIZE) {
        return PACKET_GEN_KO;
    }

    const uint8_t header[HEADER_SIZE] = {
            src, dst,
            type, flags,
            seq_number, (uint8_t)(payload_size + TAG_SIZE)
    };

    if (generate_nonce(seq_number, nonce_buf, NONCE_SIZE) != NONCE_GEN_OK) {
        fprintf(stderr, "Nonce generation failed!\n");
        return PACKET_GEN_KO;
    }

    const int enc_result = ascon_aead_encrypt(
            tag_buf, ciphertext_buf,
            (const uint8_t*)payload, payload_size,
            header, sizeof(header),  // The same header as AD
            nonce_buf, key
    );

    if (enc_result != 0) {
        fprintf(stderr, "Packet encryption failed!\n");
        return PACKET_GEN_KO;
    }

    memcpy(ahoi_packet, header, HEADER_SIZE);
    memcpy(ahoi_packet->payload, ciphertext_buf, payload_size);
    memcpy(ahoi_packet->payload + payload_size, tag_buf, TAG_SIZE);

    return PACKET_GEN_OK;
}

void increment_seq_number() {
    seq_number = (seq_number +1) % 256;
}

packet_send_status send_ahoi_packet(int fd, const ahoi_packet_t* ahoi_packet) {
    const uint8_t* abstract_packet = (const uint8_t*) ahoi_packet;
    uint8_t escaped_packet[512];
    int packet_len = 0;

    // Framing: DLE-STX
    escaped_packet[packet_len++] = 0x10;
    escaped_packet[packet_len++] = 0x02;

    // Escape header
    for (int i = 0; i < HEADER_SIZE; i++) {
        if (abstract_packet[i] == 0x10) {
            escaped_packet[packet_len++] = 0x10;
        }
        escaped_packet[packet_len++] = abstract_packet[i];
    }

    // Escape payload
    for (int i = 0; i < ahoi_packet->pl_size; i++) {
        if (ahoi_packet->payload[i] == 0x10) {
            escaped_packet[packet_len++] = 0x10;
        }
        escaped_packet[packet_len++] = ahoi_packet->payload[i];
    }

    // Framing: DLE-ETX
    escaped_packet[packet_len++] = 0x10;
    escaped_packet[packet_len++] = 0x03;

    ssize_t bytes_written = write(fd, escaped_packet, packet_len);

    if (bytes_written < 0) {
        fprintf(stderr, "Error writing to serial port");
        return PACKET_SEND_KO;
    } else if (bytes_written != packet_len) {
        fprintf(stderr, "Warning: Partial write (%zd of %d bytes)\n", bytes_written, packet_len);
        return PACKET_SEND_KO;
    }

    increment_seq_number();

    return PACKET_SEND_OK;
}

packet_rcv_status receive_ahoi_packet(const int fd, void (*cb)(const ahoi_packet_t*)) {
    int buf_pos = 0;
    int in_packet = 0;
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
                        decode_ahoi_packet(recv_buf, buf_pos, &staging_packet);
                        cb(&staging_packet);
                        in_packet = 0;
                    } else if (byte == 0x10) {
                        recv_buf[buf_pos++] = 0x10;
                    }
                }
            } else {
                recv_buf[buf_pos++] = byte;
            }
        }
    }
}

packet_decode_status decode_ahoi_packet(const uint8_t *data, const size_t len, ahoi_packet_t* ahoi_packet) {
    if (len < HEADER_SIZE) {
        fprintf(stderr,"Packet too short\n");
        return PACKET_DECODE_KO;
    }

    const uint8_t *header = data;
    const uint8_t total_len = header[5];
    const uint8_t ciphertext_len = total_len - TAG_SIZE;

    if (HEADER_SIZE + total_len > len) {
        fprintf(stderr,"Invalid lengths: total=%d, cipher=%d, tag=%d, received=%ld\n",
              total_len, ciphertext_len, TAG_SIZE, len);
        return PACKET_DECODE_KO;
    }

    const uint8_t seq = header[4];
    const uint8_t *ciphertext = data + HEADER_SIZE;
    const uint8_t *tag = data + HEADER_SIZE + ciphertext_len;

    generate_nonce(seq, nonce_buf, NONCE_SIZE);

    const int dec_result = ascon_aead_decrypt(
        ahoi_packet->payload,
        tag, ciphertext, ciphertext_len,
        header, HEADER_SIZE,
        nonce_buf, key
    );

    if (dec_result != 0) {
        fprintf(stderr,"Decryption failed.\n");
        return PACKET_DECODE_KO;
    }

    seq_number = seq;
    return PACKET_DECODE_OK;
}
