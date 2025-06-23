
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ascon.h"
#include "api.h"

// TODO: define Ascon input variables here (e.g secret key)

int crypto_aead_encrypt(
    unsigned char *c, unsigned long long *clen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *nsec,
    const unsigned char *npub,
    const unsigned char *k);

int crypto_aead_decrypt(
    unsigned char *m, unsigned long long *mlen,
    unsigned char *nsec,
    const unsigned char *c, unsigned long long clen,
    const unsigned char *ad, unsigned long long adlen,
    const unsigned char *npub,
    const unsigned char *k);

int main() {

    // TODO: Read the input from the console

    unsigned char key[CRYPTO_KEYBYTES] = {0};
    unsigned char nonce[CRYPTO_NPUBBYTES] = {0};
    unsigned char ad[] = "associated data";  // Optional
    const unsigned char *nsec = NULL;

    // User input
    char input[256];
    printf("Enter the plaintext to encrypt: ");
    fgets(input, sizeof(input), stdin);
    size_t mlen = strcspn(input, "\n");  // Remove newline character

    // Output buffers
    unsigned char ciphertext[512];
    unsigned long long clen;

    unsigned char decrypted[256];
    unsigned long long decrypted_len;

    // TODO: Use encrypt function to encrypt the user input
    // ascon_aead_encrypt();

     int enc_result = crypto_aead_encrypt(
        ciphertext, &clen,
        (unsigned char *)input, mlen,
        ad, strlen((char *)ad),
        nsec, nonce, key
    );

    if (enc_result != 0) {
        printf("Encryption failed!\n");
        return 1;
    }

    printf("\nCiphertext (hex): ");
    for (unsigned long long i = 0; i < clen; i++) {
        printf("%02X", ciphertext[i]);
    }
    printf("\n");

    // TODO: Print the result

    // TODO: Decrypt the encrypted ciphertext and print it
    int dec_result = crypto_aead_decrypt(
        decrypted, &decrypted_len,
        NULL,
        ciphertext, clen,
        ad, strlen((char *)ad),
        nonce, key
    );

    if (dec_result != 0) {
        printf("Decryption failed!\n");
        return 1;
    }

    printf("Decrypted text: %.*s\n", (int)decrypted_len, decrypted);
    
    return EXIT_SUCCESS;
}
