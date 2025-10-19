/*
 * OTA Update with Encryption Support
 */

#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <Arduino.h>

/**
 * @brief Initialize OTA update subsystem
 * 
 * @return true on success
 */
bool ota_init();

/**
 * @brief Start encrypted OTA update from URL
 * 
 * @param url URL of encrypted firmware binary
 * @param aes_key AES-256 decryption key (32 bytes)
 * @return true on success
 */
bool ota_update_from_url(const char* url, const uint8_t* aes_key);

/**
 * @brief Start encrypted OTA update from buffer
 * 
 * @param encrypted_firmware Encrypted firmware data
 * @param firmware_size Size of encrypted firmware
 * @param aes_key AES-256 decryption key (32 bytes)
 * @param iv Initialization vector (16 bytes)
 * @return true on success
 */
bool ota_update_from_buffer(const uint8_t* encrypted_firmware, 
                            size_t firmware_size,
                            const uint8_t* aes_key,
                            const uint8_t* iv);

/**
 * @brief Get current firmware version
 * 
 * @return Current firmware version
 */
uint32_t ota_get_current_version();

/**
 * @brief Verify firmware integrity
 * 
 * @return true if firmware is valid
 */
bool ota_verify_firmware();

/**
 * @brief Rollback to previous firmware version
 * 
 * @return true on success
 */
bool ota_rollback();

#endif // OTA_UPDATE_H
