#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include "esp_err.h"
#include <stdint.h>

// Firmware metadata structure
typedef struct {
    uint32_t version;
    uint32_t size;
    uint8_t hash[32];        // SHA-256 hash of encrypted firmware
    uint8_t iv[16];          // AES IV for decryption
    uint8_t signature[64];   // Optional: RSA signature for additional verification
} firmware_metadata_t;

/**
 * @brief Initialize OTA update subsystem
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ota_init(void);

/**
 * @brief Start encrypted OTA update from URL
 * 
 * @param url URL of encrypted firmware binary
 * @param aes_key AES-256 decryption key (32 bytes)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ota_update_from_url(const char *url, const uint8_t *aes_key);

/**
 * @brief Start encrypted OTA update from buffer
 * 
 * @param encrypted_firmware Encrypted firmware data
 * @param firmware_size Size of encrypted firmware
 * @param aes_key AES-256 decryption key (32 bytes)
 * @param iv Initialization vector (16 bytes)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ota_update_from_buffer(const uint8_t *encrypted_firmware, 
                                  size_t firmware_size,
                                  const uint8_t *aes_key,
                                  const uint8_t *iv);

/**
 * @brief Get current firmware version
 * 
 * @return uint32_t Current firmware version
 */
uint32_t ota_get_current_version(void);

/**
 * @brief Verify firmware integrity
 * 
 * @return esp_err_t ESP_OK if firmware is valid
 */
esp_err_t ota_verify_firmware(void);

/**
 * @brief Rollback to previous firmware version
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ota_rollback(void);

#endif // OTA_UPDATE_H
