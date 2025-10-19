#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

// AES-256 key size (32 bytes)
#define AES_KEY_SIZE 32
// AES block size (16 bytes)
#define AES_BLOCK_SIZE 16
// IV size for AES-CBC (16 bytes)
#define AES_IV_SIZE 16

/**
 * @brief Initialize crypto subsystem
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t crypto_init(void);

/**
 * @brief Encrypt data using AES-256-CBC
 * 
 * @param plaintext Input plaintext data
 * @param plaintext_len Length of plaintext
 * @param key AES-256 key (32 bytes)
 * @param iv Initialization vector (16 bytes)
 * @param ciphertext Output buffer for encrypted data
 * @param ciphertext_len Pointer to store length of ciphertext
 * @return esp_err_t ESP_OK on success
 */
esp_err_t aes_encrypt(const uint8_t *plaintext, size_t plaintext_len,
                      const uint8_t *key, const uint8_t *iv,
                      uint8_t *ciphertext, size_t *ciphertext_len);

/**
 * @brief Decrypt data using AES-256-CBC
 * 
 * @param ciphertext Input encrypted data
 * @param ciphertext_len Length of ciphertext
 * @param key AES-256 key (32 bytes)
 * @param iv Initialization vector (16 bytes)
 * @param plaintext Output buffer for decrypted data
 * @param plaintext_len Pointer to store length of plaintext
 * @return esp_err_t ESP_OK on success
 */
esp_err_t aes_decrypt(const uint8_t *ciphertext, size_t ciphertext_len,
                      const uint8_t *key, const uint8_t *iv,
                      uint8_t *plaintext, size_t *plaintext_len);

/**
 * @brief Generate SHA-256 hash of data
 * 
 * @param data Input data
 * @param data_len Length of data
 * @param hash Output buffer for hash (32 bytes)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sha256_hash(const uint8_t *data, size_t data_len, uint8_t *hash);

/**
 * @brief Verify SHA-256 hash
 * 
 * @param data Input data
 * @param data_len Length of data
 * @param expected_hash Expected hash value (32 bytes)
 * @return esp_err_t ESP_OK if hash matches
 */
esp_err_t sha256_verify(const uint8_t *data, size_t data_len, const uint8_t *expected_hash);

#endif // CRYPTO_UTILS_H
