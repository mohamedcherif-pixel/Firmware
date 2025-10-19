/*
 * OTA Update Implementation
 */

#include "ota_update.h"
#include "crypto_utils.h"
#include <Update.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_ota_ops.h>

#define BUFFER_SIZE 1024

bool ota_init() {
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            Serial.println("[OTA] First boot after OTA update, validating...");
            esp_ota_mark_app_valid_cancel_rollback();
        }
    }

    Serial.println("[OTA] OTA subsystem initialized");
    Serial.print("[OTA] Running partition: ");
    Serial.println(running->label);
    
    return true;
}

uint32_t ota_get_current_version() {
    // This should match FIRMWARE_VERSION in main sketch
    return 1;
}

bool ota_verify_firmware() {
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_app_desc_t app_desc;
    
    if (esp_ota_get_partition_description(running, &app_desc) == ESP_OK) {
        Serial.print("[OTA] Current firmware: ");
        Serial.print(app_desc.project_name);
        Serial.print(" ");
        Serial.println(app_desc.version);
        return true;
    }
    
    return false;
}

bool ota_rollback() {
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_VALID || ota_state == ESP_OTA_IMG_UNDEFINED) {
            Serial.println("[OTA] Initiating rollback to previous firmware");
            esp_ota_mark_app_invalid_rollback_and_reboot();
            return true;
        }
    }
    
    Serial.println("[OTA] Rollback not possible");
    return false;
}

bool ota_update_from_buffer(const uint8_t* encrypted_firmware, 
                            size_t firmware_size,
                            const uint8_t* aes_key,
                            const uint8_t* iv) {
    
    Serial.println("[OTA] Starting encrypted OTA update from buffer");
    Serial.printf("[OTA] Encrypted firmware size: %d bytes\n", firmware_size);

    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Serial.println("[OTA] Failed to begin update");
        return false;
    }

    // Decrypt and write firmware in chunks
    size_t offset = 0;
    uint8_t* decrypt_buffer = (uint8_t*)malloc(BUFFER_SIZE + AES_BLOCK_SIZE);
    if (!decrypt_buffer) {
        Serial.println("[OTA] Failed to allocate decryption buffer");
        Update.abort();
        return false;
    }

    while (offset < firmware_size) {
        size_t chunk_size = (firmware_size - offset) > BUFFER_SIZE ? BUFFER_SIZE : (firmware_size - offset);
        
        // Ensure chunk is aligned to AES block size
        if (chunk_size % AES_BLOCK_SIZE != 0 && offset + chunk_size < firmware_size) {
            chunk_size = (chunk_size / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
        }

        size_t decrypted_len = 0;
        if (!aes_decrypt(encrypted_firmware + offset, chunk_size,
                        aes_key, iv, decrypt_buffer, &decrypted_len)) {
            Serial.printf("[OTA] Decryption failed at offset %d\n", offset);
            free(decrypt_buffer);
            Update.abort();
            return false;
        }

        // Write decrypted chunk
        if (Update.write(decrypt_buffer, decrypted_len) != decrypted_len) {
            Serial.println("[OTA] Write failed");
            free(decrypt_buffer);
            Update.abort();
            return false;
        }

        offset += chunk_size;
        Serial.printf("[OTA] Written %d/%d bytes\n", offset, firmware_size);
    }

    free(decrypt_buffer);

    // Finalize update
    if (!Update.end(true)) {
        Serial.println("[OTA] Update end failed");
        return false;
    }

    Serial.println("[OTA] OTA update successful!");
    return true;
}

bool ota_update_from_url(const char* url, const uint8_t* aes_key) {
    Serial.print("[OTA] Starting encrypted OTA update from URL: ");
    Serial.println(url);

    HTTPClient http;
    http.begin(url);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[OTA] HTTP GET failed: %d\n", httpCode);
        http.end();
        return false;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
        Serial.println("[OTA] Invalid content length");
        http.end();
        return false;
    }

    Serial.printf("[OTA] Streaming encrypted firmware: %d bytes\n", contentLength);

    // Read IV from first 16 bytes
    WiFiClient* stream = http.getStreamPtr();
    uint8_t iv[AES_IV_SIZE];
    
    int iv_read = 0;
    while (iv_read < AES_IV_SIZE && http.connected()) {
        if (stream->available()) {
            iv[iv_read++] = stream->read();
        }
        delay(1);
    }
    
    if (iv_read != AES_IV_SIZE) {
        Serial.println("[OTA] Failed to read IV");
        http.end();
        return false;
    }
    
    Serial.print("[OTA] IV extracted: ");
    for (int i = 0; i < AES_IV_SIZE; i++) {
        Serial.printf("%02x", iv[i]);
    }
    Serial.println();
    Serial.println("[OTA] Starting decryption...");
    Serial.printf("[OTA] Stream connected: %s\n", http.connected() ? "YES" : "NO");
    Serial.printf("[OTA] Bytes available: %d\n", stream->available());

    // Calculate encrypted firmware size (total - IV)
    size_t encrypted_size = contentLength - AES_IV_SIZE;
    
    // Begin OTA update - specify we're writing plain firmware (not encrypted)
    // Calculate expected firmware size (encrypted size minus padding, approximately)
    size_t expected_firmware_size = encrypted_size - AES_BLOCK_SIZE; // Approximate
    
    if (!Update.begin(expected_firmware_size, U_FLASH, -1, LOW)) {
        Serial.printf("[OTA] Failed to begin update: %s\n", Update.errorString());
        http.end();
        return false;
    }
    
    // Disable MD5 check since we're doing our own encryption verification
    Update.setMD5("");
    
    Serial.printf("[OTA] Update partition ready, expecting ~%d bytes\n", expected_firmware_size);

    // Initialize streaming decryption
    aes_stream_context_t decrypt_ctx;
    if (!aes_decrypt_stream_init(&decrypt_ctx, aes_key, iv)) {
        Serial.println("[OTA] Failed to initialize decryption");
        Update.abort();
        http.end();
        return false;
    }

    // Stream, decrypt, and write in chunks
    size_t total_written = 0;
    
    // Allocate buffers (must be multiple of AES block size)
    const size_t CHUNK_SIZE = (BUFFER_SIZE / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    uint8_t* encrypt_buffer = (uint8_t*)malloc(CHUNK_SIZE);
    uint8_t* decrypt_buffer = (uint8_t*)malloc(CHUNK_SIZE);
    
    if (!encrypt_buffer || !decrypt_buffer) {
        Serial.println("[OTA] Failed to allocate buffers");
        if (encrypt_buffer) free(encrypt_buffer);
        if (decrypt_buffer) free(decrypt_buffer);
        aes_decrypt_stream_free(&decrypt_ctx);
        Update.abort();
        http.end();
        return false;
    }

    size_t offset = 0;
    bool success = true;
    size_t last_chunk_size = 0;
    
    unsigned long last_progress = millis();
    
    while (offset < encrypted_size && http.connected()) {
        // Calculate chunk size (must be multiple of AES_BLOCK_SIZE)
        size_t to_read = min(CHUNK_SIZE, encrypted_size - offset);
        to_read = (to_read / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
        
        if (to_read == 0 && offset < encrypted_size) {
            // Last chunk, read remaining bytes
            to_read = encrypted_size - offset;
        }
        
        // Read encrypted chunk with timeout
        size_t read_len = 0;
        unsigned long read_start = millis();
        
        while (read_len < to_read && http.connected()) {
            size_t available = stream->available();
            if (available) {
                size_t can_read = min(available, to_read - read_len);
                read_len += stream->readBytes(encrypt_buffer + read_len, can_read);
                read_start = millis(); // Reset timeout on successful read
            }
            
            // Timeout after 10 seconds of no data
            if (millis() - read_start > 10000) {
                Serial.printf("[OTA] Timeout waiting for data at offset %d\n", offset);
                success = false;
                break;
            }
            
            delay(1);
        }
        
        if (!success || read_len == 0) break;
        
        // Decrypt chunk using streaming context
        size_t decrypted_len = 0;
        if (!aes_decrypt_stream_update(&decrypt_ctx, encrypt_buffer, read_len, 
                                       decrypt_buffer, &decrypted_len)) {
            Serial.printf("[OTA] Decryption failed at offset %d\n", offset);
            success = false;
            break;
        }
        
        // For last chunk, remove PKCS7 padding
        if (offset + read_len >= encrypted_size) {
            uint8_t padding = decrypt_buffer[decrypted_len - 1];
            Serial.printf("[OTA] Last chunk - padding byte: %d\n", padding);
            if (padding > 0 && padding <= AES_BLOCK_SIZE) {
                decrypted_len -= padding;
                last_chunk_size = decrypted_len;
                Serial.printf("[OTA] After padding removal: %d bytes\n", decrypted_len);
            }
        }
        
        // Debug: Show first 16 bytes of first chunk
        if (total_written == 0 && decrypted_len > 0) {
            Serial.print("[OTA] First 16 bytes: ");
            for (int i = 0; i < min(16, (int)decrypted_len); i++) {
                Serial.printf("%02x ", decrypt_buffer[i]);
            }
            Serial.println();
        }
        
        // Write decrypted chunk
        Serial.printf("[OTA] Writing %d bytes at offset %d\n", decrypted_len, total_written);
        size_t written = Update.write(decrypt_buffer, decrypted_len);
        if (written != decrypted_len) {
            Serial.printf("[OTA] Write failed: wrote %d of %d bytes\n", written, decrypted_len);
            Serial.printf("[OTA] Update error: %s\n", Update.errorString());
            Serial.printf("[OTA] Has error: %d\n", Update.hasError());
            success = false;
            break;
        }
        
        offset += read_len;
        total_written += decrypted_len;
        
        // Show progress every 50KB or at completion
        if (offset % 51200 == 0 || offset >= encrypted_size) {
            Serial.printf("[OTA] Progress: %d/%d bytes (%.1f%%) - Speed: %.1f KB/s\n", 
                         offset, encrypted_size, (offset * 100.0) / encrypted_size,
                         (offset / 1024.0) / ((millis() - last_progress) / 1000.0));
        }
    }
    
    Serial.printf("[OTA] Download complete: %d bytes in %.1f seconds\n", 
                 offset, (millis() - last_progress) / 1000.0);

    free(encrypt_buffer);
    free(decrypt_buffer);
    aes_decrypt_stream_free(&decrypt_ctx);
    http.end();

    if (!success) {
        Update.abort();
        return false;
    }
    
    Serial.printf("[OTA] Total decrypted: %d bytes\n", total_written);

    // Finalize update
    if (!Update.end(true)) {
        Serial.println("[OTA] Update end failed");
        return false;
    }

    Serial.println("[OTA] OTA update successful!");
    return true;
}
