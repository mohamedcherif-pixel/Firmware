/*
 * RSA Digital Signature Verification Implementation
 * Uses mbedTLS for RSA-SHA256 signature verification
 */

#include "rsa_verify.h"
#include <HTTPClient.h>
#include <WiFi.h>

// RSA public key (this will be replaced by rsa_public.h content)
// For now, we'll include a placeholder - you'll need to copy the actual key from rsa_public.h
#include "rsa_public.h"

static mbedtls_pk_context pk_ctx;
static bool rsa_initialized = false;

bool rsa_verify_init() {
    if (rsa_initialized) {
        return true;
    }
    
    // Initialize mbedTLS PK context
    mbedtls_pk_init(&pk_ctx);
    
    // Parse public key
    int ret = mbedtls_pk_parse_public_key(&pk_ctx, rsa_public_key, rsa_public_key_len);
    if (ret != 0) {
        Serial.printf("[RSA] Failed to parse public key: -0x%04x\n", -ret);
        mbedtls_pk_free(&pk_ctx);
        return false;
    }
    
    // Verify it's an RSA key
    if (!mbedtls_pk_can_do(&pk_ctx, MBEDTLS_PK_RSA)) {
        Serial.println("[RSA] Key is not an RSA key");
        mbedtls_pk_free(&pk_ctx);
        return false;
    }
    
    rsa_initialized = true;
    Serial.println("[RSA] RSA verification initialized successfully");
    return true;
}

void rsa_verify_cleanup() {
    if (rsa_initialized) {
        mbedtls_pk_free(&pk_ctx);
        rsa_initialized = false;
        Serial.println("[RSA] RSA verification cleaned up");
    }
}

bool rsa_verify_firmware(const uint8_t* firmware_data, size_t firmware_len,
                        const uint8_t* signature_data, size_t signature_len) {
    if (!rsa_initialized) {
        Serial.println("[RSA] RSA not initialized");
        return false;
    }
    
    // Calculate SHA-256 hash of firmware
    unsigned char hash[32];
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0); // 0 = SHA-256
    mbedtls_sha256_update(&sha256_ctx, firmware_data, firmware_len);
    mbedtls_sha256_finish(&sha256_ctx, hash);
    mbedtls_sha256_free(&sha256_ctx);
    
    // Verify signature using PKCS#1 v1.5
    int ret = mbedtls_pk_verify(&pk_ctx, MBEDTLS_MD_SHA256, hash, 32, signature_data, signature_len);
    
    if (ret == 0) {
        Serial.println("[RSA] ✓ Signature verification successful");
        return true;
    } else {
        Serial.printf("[RSA] ✗ Signature verification failed: -0x%04x\n", -ret);
        return false;
    }
}

bool download_file(const char* url, uint8_t** buffer, size_t* buffer_len) {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(20000);
    
    Serial.printf("[RSA] Downloading: %s\n", url);
    
    if (!http.begin(url)) {
        Serial.println("[RSA] Failed to begin HTTP request");
        return false;
    }
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[RSA] HTTP GET failed: %d\n", httpCode);
        http.end();
        return false;
    }
    
    size_t content_len = http.getSize();
    if (content_len == 0) {
        Serial.println("[RSA] Content length is 0");
        http.end();
        return false;
    }
    
    *buffer = (uint8_t*)malloc(content_len);
    if (*buffer == NULL) {
        Serial.println("[RSA] Failed to allocate memory");
        http.end();
        return false;
    }
    
    WiFiClient* stream = http.getStreamPtr();
    size_t downloaded = 0;
    
    while (http.connected() && downloaded < content_len) {
        size_t available = stream->available();
        if (available) {
            size_t to_read = min(available, content_len - downloaded);
            int read = stream->readBytes((*buffer) + downloaded, to_read);
            if (read > 0) {
                downloaded += read;
            }
        }
        delay(1);
    }
    
    http.end();
    
    if (downloaded == content_len) {
        *buffer_len = downloaded;
        Serial.printf("[RSA] ✓ Downloaded %zu bytes\n", downloaded);
        return true;
    } else {
        Serial.printf("[RSA] Download incomplete: %zu/%zu bytes\n", downloaded, content_len);
        free(*buffer);
        *buffer = NULL;
        return false;
    }
}

bool rsa_verify_firmware_from_url(const char* firmware_url, const char* signature_url) {
    if (!rsa_verify_init()) {
        return false;
    }
    
    // Download firmware
    uint8_t* firmware_data = NULL;
    size_t firmware_len = 0;
    if (!download_file(firmware_url, &firmware_data, &firmware_len)) {
        Serial.println("[RSA] Failed to download firmware");
        return false;
    }
    
    // Download signature
    uint8_t* signature_data = NULL;
    size_t signature_len = 0;
    if (!download_file(signature_url, &signature_data, &signature_len)) {
        Serial.println("[RSA] Failed to download signature");
        free(firmware_data);
        return false;
    }
    
    // Verify signature
    bool result = rsa_verify_firmware(firmware_data, firmware_len, signature_data, signature_len);
    
    // Cleanup
    free(firmware_data);
    free(signature_data);
    
    return result;
}
