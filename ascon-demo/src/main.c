#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ascon.h"  // Make sure this is the 7-parameter version

#define MAX_INPUT_SIZE 128
#define KEY_SIZE 16
#define NONCE_SIZE 16
#define TAG_SIZE 16

char input[MAX_INPUT_SIZE];
const char* associated_data = "ASCON_AD";

uint8_t key[KEY_SIZE] = {0x00};
uint8_t nonce[NONCE_SIZE] = {0x00};
uint8_t tag[TAG_SIZE];

uint8_t ciphertext[MAX_INPUT_SIZE];
uint8_t decrypted[MAX_INPUT_SIZE];

void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    
    printf("Enter your plaintext message: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
    fprintf(stderr, "Error reading input.\n");
    return 1;
    }
    input[strcspn(input, "\n")] = '\0'; // remove newline character

    size_t mlen = strlen(input);
    size_t adlen = strlen(associated_data);

    // Show Key, Nonce, AD
    print_hex("Key", key, KEY_SIZE);
    print_hex("Nonce", nonce, NONCE_SIZE);
    printf("Associated Data (AD): %s\n", associated_data);

    // Encryption
    int enc_result = ascon_aead_encrypt(
        tag, ciphertext,
        (const uint8_t*)input, mlen,
        (const uint8_t*)associated_data, adlen,
        nonce, key
    );

    if (enc_result != 0) {
        printf("Encryption failed!\n");
        return 1;
    }

    print_hex("Ciphertext", ciphertext, mlen);
    print_hex("Tag", tag, TAG_SIZE);

    // Decryption
    int dec_result = ascon_aead_decrypt(
        decrypted,
        tag, ciphertext, mlen,
        (const uint8_t*)associated_data, adlen,
        nonce, key
    );

    if (dec_result == 0) {
        decrypted[mlen] = '\0'; // null-terminate the decrypted message
        printf("Decrypted Message: %s\n", decrypted);
    } else {
        printf("Decryption failed!\n");
    }

    return 0;
}
