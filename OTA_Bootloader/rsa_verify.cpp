/*
 * RSA Digital Signature Verification Implementation
 */

#include "rsa_verify.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include "rsa_public.h"

static mbedtls_pk_context pk_ctx;
static bool rsa_initialized = false;

bool rsa_verify_init() {
    if (rsa_initialized) {
        return true;
    }
    
    mbedtls_pk_init(&pk_ctx);
    
    int ret = mbedtls_pk_parse_public_key(&pk_ctx, rsa_public_key, rsa_public_key_len);
    if (ret != 0) {
        Serial.printf("[RSA] Failed to parse public key: -0x%04x\n", -ret);
        mbedtls_pk_free(&pk_ctx);
        return false;
    }
    
    if (!mbedtls_pk_can_do(&pk_ctx, MBEDTLS_PK_RSA)) {
        Serial.println("[RSA] Key is not an RSA key");
        mbedtls_pk_free(&pk_ctx);
        return false;
    }
    
    rsa_initialized = true;
    Serial.println("[RSA] RSA verification initialized");
    return true;
}

void rsa_verify_cleanup() {
    if (rsa_initialized) {
        mbedtls_pk_free(&pk_ctx);
        rsa_initialized = false;
    }
}

bool rsa_verify_firmware(const uint8_t* firmware_data, size_t firmware_len,
                        const uint8_t* signature_data, size_t signature_len) {
    if (!rsa_initialized) {
        Serial.println("[RSA] RSA not initialized");
        return false;
    }
    
    unsigned char hash[32];
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0);
    mbedtls_sha256_update(&sha256_ctx, firmware_data, firmware_len);
    mbedtls_sha256_finish(&sha256_ctx, hash);
    mbedtls_sha256_free(&sha256_ctx);
    
    int ret = mbedtls_pk_verify(&pk_ctx, MBEDTLS_MD_SHA256, hash, 32, signature_data, signature_len);
    
    if (ret == 0) {
        Serial.println("[RSA] ✓ Signature valid");
        return true;
    } else {
        Serial.printf("[RSA] ✗ Signature invalid: -0x%04x\n", -ret);
        return false;
    }
}

static bool download_file(const char* url, uint8_t** buffer, size_t* buffer_len) {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(20000);
    
    if (!http.begin(url)) {
        return false;
    }
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        return false;
    }
    
    size_t content_len = http.getSize();
    if (content_len == 0) {
        http.end();
        return false;
    }
    
    *buffer = (uint8_t*)malloc(content_len);
    if (*buffer == NULL) {
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
        return true;
    } else {
        free(*buffer);
        *buffer = NULL;
        return false;
    }
}

bool rsa_verify_firmware_from_url(const char* firmware_url, const char* signature_url) {
    if (!rsa_verify_init()) {
        return false;
    }
    
    // Download signature
    uint8_t* signature_data = NULL;
    size_t signature_len = 0;
    if (!download_file(signature_url, &signature_data, &signature_len)) {
        Serial.println("[RSA] Failed to download signature");
        return false;
    }
    
    // Stream firmware and calculate hash
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(30000);
    
    if (!http.begin(firmware_url)) {
        free(signature_data);
        return false;
    }
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        free(signature_data);
        return false;
    }
    
    size_t content_len = http.getSize();
    
    // Calculate hash while streaming
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0);
    
    WiFiClient* stream = http.getStreamPtr();
    size_t downloaded = 0;
    uint8_t* buffer = (uint8_t*)malloc(4096);
    if (!buffer) {
        http.end();
        free(signature_data);
        return false;
    }
    
    while (http.connected() && downloaded < content_len) {
        size_t available = stream->available();
        if (available) {
            size_t to_read = min((size_t)available, min((size_t)4096, content_len - downloaded));
            int read = stream->readBytes(buffer, to_read);
            if (read > 0) {
                mbedtls_sha256_update(&sha256_ctx, buffer, read);
                downloaded += read;
            }
        }
        delay(1);
    }
    
    http.end();
    free(buffer);
    
    unsigned char hash[32];
    mbedtls_sha256_finish(&sha256_ctx, hash);
    mbedtls_sha256_free(&sha256_ctx);
    
    if (downloaded != content_len) {
        free(signature_data);
        return false;
    }
    
    // Verify signature
    int ret = mbedtls_pk_verify(&pk_ctx, MBEDTLS_MD_SHA256, hash, 32, signature_data, signature_len);
    
    free(signature_data);
    
    return (ret == 0);
}
