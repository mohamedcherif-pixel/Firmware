/*
 * Cryptographic Utilities for ESP32
 * AES-256-CBC encryption/decryption and SHA-256 hashing
 */

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include "mbedtls/aes.h"

// AES-256 parameters
#define AES_KEY_SIZE 32
#define AES_IV_SIZE 16
#define AES_BLOCK_SIZE 16

// Streaming decryption context
typedef struct {
    mbedtls_aes_context aes;
    uint8_t iv[AES_IV_SIZE];
    bool initialized;
} aes_stream_context_t;

// Initialize crypto subsystem
bool crypto_init();

// AES-256-CBC encryption/decryption
bool aes_encrypt(const uint8_t* plaintext, size_t plaintext_len,
                 const uint8_t* key, const uint8_t* iv,
                 uint8_t* ciphertext, size_t* ciphertext_len);

bool aes_decrypt(const uint8_t* ciphertext, size_t ciphertext_len,
                 const uint8_t* key, const uint8_t* iv,
                 uint8_t* plaintext, size_t* plaintext_len);

// Streaming decryption (for OTA)
bool aes_decrypt_stream_init(aes_stream_context_t* ctx, const uint8_t* key, const uint8_t* iv);
bool aes_decrypt_stream_update(aes_stream_context_t* ctx, const uint8_t* ciphertext, 
                               size_t ciphertext_len, uint8_t* plaintext, size_t* plaintext_len);
void aes_decrypt_stream_free(aes_stream_context_t* ctx);
// SHA-256 hashing
bool sha256_hash(const uint8_t* data, size_t data_len, uint8_t* hash);
bool sha256_verify(const uint8_t* data, size_t data_len, const uint8_t* expected_hash);
#endif // CRYPTO_UTILS_H
