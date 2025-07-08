#include "commons/cli_helper.h"

#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -k <key_in_hex> \n", prog_name);
}

cli_parse_status parse_cli_arguments(int32_t argc, uint8_t* const * argv, uint8_t* key_buf, size_t key_size) {
    struct option long_options[] = {
            {"key", required_argument, 0, 'k'},
            {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    char *key_hex = NULL;
    while ((opt = getopt_long(argc, argv, "k:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'k':
                key_hex = optarg;
                break;
            default:
                print_usage(argv[0]);
                return CLI_PARSE_KO;
        }
    }

    if (!key_hex) {
        fprintf(stderr, "Error: Encryption key is required\n");
        print_usage(argv[0]);
        return CLI_PARSE_KO;
    }

    if (process_key(key_hex, key_buf, key_size) != 0) {
        return CLI_PARSE_KO;
    }

    return CLI_PARSE_OK;
}

int process_key(const char *hex, uint8_t *key_buffer, size_t key_size) {
    size_t hex_len = strlen(hex);

    if (hex_len < 2 || hex_len > key_size * 2) {
        fprintf(stderr, "Key must be 2-%zu hex characters\n", key_size * 2);
        return -1;
    }

    // Pad with zeros if needed
    memset(key_buffer, 0, key_size);

    // Convert what we can from the hex string
    for (size_t i = 0; i < hex_len/2 && i < key_size; i++) {
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