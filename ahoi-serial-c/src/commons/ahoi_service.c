#include "commons/ahoi_service.h"

#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#include "ascon.h"

static uint8_t key[KEY_SIZE] = {0};
static uint8_t tag[TAG_SIZE] = {0};
static uint8_t ciphertext[MAX_PAYLOAD_SIZE] = {0};
static uint8_t nonce[NONCE_SIZE] = {0};

uint8_t seq_number = 0;

void store_key(uint8_t* new_key) {
    memcpy(key, new_key, KEY_SIZE);
}

nonce_gen_status generate_nonce(uint8_t* nonce_buf, size_t nonce_size) {
    if (nonce_size < sizeof(time_t) + sizeof(uint8_t)) {
        return NONCE_GEN_KO;
    }

    memset(nonce_buf, 0, nonce_size);

    time_t now = time(NULL);
    time_t hour_timestamp = htonl(now / SECONDS_IN_HOUR);

    memcpy(nonce_buf, &hour_timestamp, sizeof(hour_timestamp));
    memcpy(nonce_buf + sizeof(hour_timestamp), &seq_number, sizeof(seq_number));

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

    if (generate_nonce(nonce, NONCE_SIZE) != NONCE_GEN_OK) {
        fprintf(stderr, "Nonce generation failed!\n");
        return PACKET_GEN_KO;
    }

    int enc_result = ascon_aead_encrypt(
            tag, ciphertext,
            (const uint8_t*)payload, payload_size,
            header, sizeof(header),  // The same header as AD
            nonce, key
    );

    if (enc_result != 0) {
        fprintf(stderr, "Packet encryption failed!\n");
        return PACKET_GEN_KO;
    }

    // show the information
//    printf("Ciphertext (%zu bytes): ", mlen);
//    for (size_t i = 0; i < mlen; i++) printf("%02X", ciphertext[i]);
//    printf("\nTag (%d bytes): ", TAG_SIZE);
//    for (int i = 0; i < TAG_SIZE; i++) printf("%02X", tag[i]);
//    printf("\n");

    memcpy(ahoi_packet, header, HEADER_SIZE);
    memcpy(ahoi_packet->payload, ciphertext, payload_size);
    memcpy(ahoi_packet->payload + payload_size, tag, TAG_SIZE);

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

    // Escape
    for (int i = 0; i < HEADER_SIZE + ahoi_packet->pl_size; i++) {
        if (abstract_packet[i] == 0x10) {
            escaped_packet[packet_len++] = 0x10;
        }
        escaped_packet[packet_len++] = abstract_packet[i];
    }

    // Framing: DLE-ETX
    escaped_packet[packet_len++] = 0x10;
    escaped_packet[packet_len++] = 0x03;

    ssize_t bytes_written = write(fd, escaped_packet, packet_len);

    if (bytes_written < 0) {
        perror("Error writing to serial port");
        return PACKET_SEND_KO;
    } else if (bytes_written != packet_len) {
        fprintf(stderr, "Warning: Partial write (%zd of %d bytes)\n", bytes_written, packet_len);
        return PACKET_SEND_KO;
    }

    printf("Sent escaped_packet (%d bytes): ", packet_len);
    for (int i = 0; i < packet_len; i++) printf("%02X ", escaped_packet[i]);
    printf("\n");

    increment_seq_number();

    return PACKET_SEND_OK;
}