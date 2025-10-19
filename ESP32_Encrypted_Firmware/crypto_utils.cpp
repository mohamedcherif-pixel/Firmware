/*
 * Cryptographic Utilities Implementation
 */

#include "crypto_utils.h"
#include "mbedtls/aes.h"
#include "mbedtls/sha256.h"
#include <string.h>

bool crypto_init() {
    Serial.println("[CRYPTO] Crypto subsystem initialized");
    return true;
}

bool aes_encrypt(const uint8_t* plaintext, size_t plaintext_len,
                 const uint8_t* key, const uint8_t* iv,
                 uint8_t* ciphertext, size_t* ciphertext_len) {
    
    if (!plaintext || !key || !iv || !ciphertext || !ciphertext_len) {
        Serial.println("[CRYPTO] Invalid parameters for encryption");
        return false;
    }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    // Set encryption key
    int ret = mbedtls_aes_setkey_enc(&aes, key, AES_KEY_SIZE * 8);
    if (ret != 0) {
        Serial.printf("[CRYPTO] Failed to set encryption key: %d\n", ret);
        mbedtls_aes_free(&aes);
        return false;
    }

    // Calculate padded length (PKCS7 padding)
    size_t padding = AES_BLOCK_SIZE - (plaintext_len % AES_BLOCK_SIZE);
    size_t padded_len = plaintext_len + padding;
    
    // Create padded buffer
    uint8_t* padded_plaintext = (uint8_t*)malloc(padded_len);
    if (!padded_plaintext) {
        Serial.println("[CRYPTO] Failed to allocate memory for padding");
        mbedtls_aes_free(&aes);
        return false;
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
        Serial.printf("[CRYPTO] Encryption failed: %d\n", ret);
        return false;
    }

    *ciphertext_len = padded_len;
    Serial.printf("[CRYPTO] Encrypted %d bytes to %d bytes\n", plaintext_len, padded_len);
    return true;
}

bool aes_decrypt(const uint8_t* ciphertext, size_t ciphertext_len,
                 const uint8_t* key, const uint8_t* iv,
                 uint8_t* plaintext, size_t* plaintext_len) {
    
    if (!ciphertext || !key || !iv || !plaintext || !plaintext_len) {
        Serial.println("[CRYPTO] Invalid parameters for decryption");
        return false;
    }

    if (ciphertext_len % AES_BLOCK_SIZE != 0) {
        Serial.printf("[CRYPTO] Invalid ciphertext length: %d\n", ciphertext_len);
        return false;
    }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    // Set decryption key
    int ret = mbedtls_aes_setkey_dec(&aes, key, AES_KEY_SIZE * 8);
    if (ret != 0) {
        Serial.printf("[CRYPTO] Failed to set decryption key: %d\n", ret);
        mbedtls_aes_free(&aes);
        return false;
    }

    // Copy IV for CBC mode (it will be modified)
    uint8_t iv_copy[AES_IV_SIZE];
    memcpy(iv_copy, iv, AES_IV_SIZE);

    // Decrypt using CBC mode
    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ciphertext_len,
                                iv_copy, ciphertext, plaintext);
    
    mbedtls_aes_free(&aes);

    if (ret != 0) {
        Serial.printf("[CRYPTO] Decryption failed: %d\n", ret);
        return false;
    }

    // Remove PKCS7 padding
    uint8_t padding = plaintext[ciphertext_len - 1];
    if (padding > AES_BLOCK_SIZE || padding == 0) {
        Serial.printf("[CRYPTO] Invalid padding: %d\n", padding);
        return false;
    }

    // Verify padding
    for (size_t i = ciphertext_len - padding; i < ciphertext_len; i++) {
        if (plaintext[i] != padding) {
            Serial.println("[CRYPTO] Padding verification failed");
            return false;
        }
    }

    *plaintext_len = ciphertext_len - padding;
    Serial.printf("[CRYPTO] Decrypted %d bytes to %d bytes\n", ciphertext_len, *plaintext_len);
    return true;
}

bool sha256_hash(const uint8_t* data, size_t data_len, uint8_t* hash) {
    if (!data || !hash) {
        Serial.println("[CRYPTO] Invalid parameters for hashing");
        return false;
    }

    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    
    int ret = mbedtls_sha256_starts(&sha256_ctx, 0); // 0 = SHA-256 (not SHA-224)
    if (ret != 0) {
        Serial.printf("[CRYPTO] SHA256 start failed: %d\n", ret);
        mbedtls_sha256_free(&sha256_ctx);
        return false;
    }

    ret = mbedtls_sha256_update(&sha256_ctx, data, data_len);
    if (ret != 0) {
        Serial.printf("[CRYPTO] SHA256 update failed: %d\n", ret);
        mbedtls_sha256_free(&sha256_ctx);
        return false;
    }

    ret = mbedtls_sha256_finish(&sha256_ctx, hash);
    mbedtls_sha256_free(&sha256_ctx);

    if (ret != 0) {
        Serial.printf("[CRYPTO] SHA256 finish failed: %d\n", ret);
        return false;
    }

    return true;
}

bool sha256_verify(const uint8_t* data, size_t data_len, const uint8_t* expected_hash) {
    uint8_t computed_hash[32];
    if (!sha256_hash(data, data_len, computed_hash)) {
        return false;
    }

    if (memcmp(computed_hash, expected_hash, 32) != 0) {
        Serial.println("[CRYPTO] Hash verification failed");
        return false;
    }

    Serial.println("[CRYPTO] Hash verification successful");
    return true;
}

// Streaming decryption for OTA updates
bool aes_decrypt_stream_init(aes_stream_context_t* ctx, const uint8_t* key, const uint8_t* iv) {
    if (!ctx || !key || !iv) {
        return false;
    }
    
    mbedtls_aes_init(&ctx->aes);
    
    int ret = mbedtls_aes_setkey_dec(&ctx->aes, key, AES_KEY_SIZE * 8);
    if (ret != 0) {
        mbedtls_aes_free(&ctx->aes);
        return false;
    }
    
    memcpy(ctx->iv, iv, AES_IV_SIZE);
    ctx->initialized = true;
    return true;
}

bool aes_decrypt_stream_update(aes_stream_context_t* ctx, const uint8_t* ciphertext, 
                               size_t ciphertext_len, uint8_t* plaintext, size_t* plaintext_len) {
    if (!ctx || !ctx->initialized || !ciphertext || !plaintext || !plaintext_len) {
        return false;
    }
    
    // Must be multiple of block size
    if (ciphertext_len % AES_BLOCK_SIZE != 0) {
        return false;
    }
    
    int ret = mbedtls_aes_crypt_cbc(&ctx->aes, MBEDTLS_AES_DECRYPT, ciphertext_len,
                                    ctx->iv, ciphertext, plaintext);
    
    if (ret != 0) {
        return false;
    }
    
    *plaintext_len = ciphertext_len;
    return true;
}

void aes_decrypt_stream_free(aes_stream_context_t* ctx) {
    if (ctx && ctx->initialized) {
        mbedtls_aes_free(&ctx->aes);
        ctx->initialized = false;
    }
}
