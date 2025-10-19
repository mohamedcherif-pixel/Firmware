#include "crypto_utils.h"
#include "mbedtls/aes.h"
#include "mbedtls/sha256.h"
#include "mbedtls/md.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "CRYPTO";

esp_err_t crypto_init(void)
{
    ESP_LOGI(TAG, "Crypto subsystem initialized");
    return ESP_OK;
}

esp_err_t aes_encrypt(const uint8_t *plaintext, size_t plaintext_len,
                      const uint8_t *key, const uint8_t *iv,
                      uint8_t *ciphertext, size_t *ciphertext_len)
{
    if (!plaintext || !key || !iv || !ciphertext || !ciphertext_len) {
        ESP_LOGE(TAG, "Invalid parameters for encryption");
        return ESP_ERR_INVALID_ARG;
    }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    // Set encryption key
    int ret = mbedtls_aes_setkey_enc(&aes, key, AES_KEY_SIZE * 8);
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to set encryption key: %d", ret);
        mbedtls_aes_free(&aes);
        return ESP_FAIL;
    }

    // Calculate padded length (PKCS7 padding)
    size_t padding = AES_BLOCK_SIZE - (plaintext_len % AES_BLOCK_SIZE);
    size_t padded_len = plaintext_len + padding;
    
    // Create padded buffer
    uint8_t *padded_plaintext = malloc(padded_len);
    if (!padded_plaintext) {
        ESP_LOGE(TAG, "Failed to allocate memory for padding");
        mbedtls_aes_free(&aes);
        return ESP_ERR_NO_MEM;
    }

    memcpy(padded_plaintext, plaintext, plaintext_len);
    // Apply PKCS7 padding
    for (size_t i = plaintext_len; i < padded_len; i++) {
        padded_plaintext[i] = padding;
    }

    // Copy IV for CBC mode (it will be modified)
    uint8_t iv_copy[AES_IV_SIZE];
    memcpy(iv_copy, iv, AES_IV_SIZE);

    // Encrypt using CBC mode
    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, padded_len,
                                iv_copy, padded_plaintext, ciphertext);
    
    free(padded_plaintext);
    mbedtls_aes_free(&aes);

    if (ret != 0) {
        ESP_LOGE(TAG, "Encryption failed: %d", ret);
        return ESP_FAIL;
    }

    *ciphertext_len = padded_len;
    ESP_LOGI(TAG, "Encrypted %d bytes to %d bytes", plaintext_len, padded_len);
    return ESP_OK;
}

esp_err_t aes_decrypt(const uint8_t *ciphertext, size_t ciphertext_len,
                      const uint8_t *key, const uint8_t *iv,
                      uint8_t *plaintext, size_t *plaintext_len)
{
    if (!ciphertext || !key || !iv || !plaintext || !plaintext_len) {
        ESP_LOGE(TAG, "Invalid parameters for decryption");
        return ESP_ERR_INVALID_ARG;
    }

    if (ciphertext_len % AES_BLOCK_SIZE != 0) {
        ESP_LOGE(TAG, "Invalid ciphertext length: %d", ciphertext_len);
        return ESP_ERR_INVALID_ARG;
    }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    // Set decryption key
    int ret = mbedtls_aes_setkey_dec(&aes, key, AES_KEY_SIZE * 8);
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to set decryption key: %d", ret);
        mbedtls_aes_free(&aes);
        return ESP_FAIL;
    }

    // Copy IV for CBC mode (it will be modified)
    uint8_t iv_copy[AES_IV_SIZE];
    memcpy(iv_copy, iv, AES_IV_SIZE);

    // Decrypt using CBC mode
    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ciphertext_len,
                                iv_copy, ciphertext, plaintext);
    
    mbedtls_aes_free(&aes);

    if (ret != 0) {
        ESP_LOGE(TAG, "Decryption failed: %d", ret);
        return ESP_FAIL;
    }

    // Remove PKCS7 padding
    uint8_t padding = plaintext[ciphertext_len - 1];
    if (padding > AES_BLOCK_SIZE || padding == 0) {
        ESP_LOGE(TAG, "Invalid padding: %d", padding);
        return ESP_FAIL;
    }

    // Verify padding
    for (size_t i = ciphertext_len - padding; i < ciphertext_len; i++) {
        if (plaintext[i] != padding) {
            ESP_LOGE(TAG, "Padding verification failed");
            return ESP_FAIL;
        }
    }

    *plaintext_len = ciphertext_len - padding;
    ESP_LOGI(TAG, "Decrypted %d bytes to %d bytes", ciphertext_len, *plaintext_len);
    return ESP_OK;
}

esp_err_t sha256_hash(const uint8_t *data, size_t data_len, uint8_t *hash)
{
    if (!data || !hash) {
        ESP_LOGE(TAG, "Invalid parameters for hashing");
        return ESP_ERR_INVALID_ARG;
    }

    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    
    int ret = mbedtls_sha256_starts(&sha256_ctx, 0); // 0 = SHA-256 (not SHA-224)
    if (ret != 0) {
        ESP_LOGE(TAG, "SHA256 start failed: %d", ret);
        mbedtls_sha256_free(&sha256_ctx);
        return ESP_FAIL;
    }

    ret = mbedtls_sha256_update(&sha256_ctx, data, data_len);
    if (ret != 0) {
        ESP_LOGE(TAG, "SHA256 update failed: %d", ret);
        mbedtls_sha256_free(&sha256_ctx);
        return ESP_FAIL;
    }

    ret = mbedtls_sha256_finish(&sha256_ctx, hash);
    mbedtls_sha256_free(&sha256_ctx);

    if (ret != 0) {
        ESP_LOGE(TAG, "SHA256 finish failed: %d", ret);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t sha256_verify(const uint8_t *data, size_t data_len, const uint8_t *expected_hash)
{
    uint8_t computed_hash[32];
    esp_err_t err = sha256_hash(data, data_len, computed_hash);
    if (err != ESP_OK) {
        return err;
    }

    if (memcmp(computed_hash, expected_hash, 32) != 0) {
        ESP_LOGE(TAG, "Hash verification failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Hash verification successful");
    return ESP_OK;
}
