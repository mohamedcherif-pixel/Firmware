#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "crypto_utils.h"
#include "ota_update.h"
#include "wifi_manager.h"

static const char *TAG = "MAIN";

// Example AES-256 key (32 bytes) - In production, this should be securely stored
static const uint8_t aes_key[32] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

// Example IV (16 bytes)
static const uint8_t aes_iv[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

// WiFi credentials - Update these with your network details
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// OTA firmware URL - Update with your server URL
#define OTA_FIRMWARE_URL "http://192.168.1.100:8000/firmware_encrypted.bin"

void test_crypto_functions(void)
{
    ESP_LOGI(TAG, "=== Testing Crypto Functions ===");
    
    const char *test_data = "Hello, ESP32 Encrypted Firmware!";
    size_t test_len = strlen(test_data);
    
    // Allocate buffers
    uint8_t *ciphertext = malloc(test_len + AES_BLOCK_SIZE);
    uint8_t *plaintext = malloc(test_len + AES_BLOCK_SIZE);
    size_t cipher_len, plain_len;
    
    // Test encryption
    ESP_LOGI(TAG, "Original: %s", test_data);
    esp_err_t err = aes_encrypt((uint8_t *)test_data, test_len, aes_key, aes_iv, 
                                ciphertext, &cipher_len);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Encryption successful, ciphertext length: %d", cipher_len);
        
        // Test decryption
        err = aes_decrypt(ciphertext, cipher_len, aes_key, aes_iv, 
                         plaintext, &plain_len);
        if (err == ESP_OK) {
            plaintext[plain_len] = '\0';
            ESP_LOGI(TAG, "Decryption successful: %s", plaintext);
            
            if (strcmp((char *)plaintext, test_data) == 0) {
                ESP_LOGI(TAG, "✓ Crypto test PASSED");
            } else {
                ESP_LOGE(TAG, "✗ Crypto test FAILED - data mismatch");
            }
        } else {
            ESP_LOGE(TAG, "✗ Decryption failed");
        }
    } else {
        ESP_LOGE(TAG, "✗ Encryption failed");
    }
    
    // Test SHA-256
    uint8_t hash[32];
    err = sha256_hash((uint8_t *)test_data, test_len, hash);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "SHA-256 hash computed successfully");
        ESP_LOG_BUFFER_HEX("Hash", hash, 32);
        
        // Verify hash
        err = sha256_verify((uint8_t *)test_data, test_len, hash);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "✓ Hash verification PASSED");
        } else {
            ESP_LOGE(TAG, "✗ Hash verification FAILED");
        }
    }
    
    free(ciphertext);
    free(plaintext);
    ESP_LOGI(TAG, "=== Crypto Tests Complete ===\n");
}

void ota_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Waiting 10 seconds before starting OTA update...");
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    
    ESP_LOGI(TAG, "Starting OTA update from URL: %s", OTA_FIRMWARE_URL);
    
    esp_err_t err = ota_update_from_url(OTA_FIRMWARE_URL, aes_key);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "OTA update failed: %s", esp_err_to_name(err));
    }
    
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP32 Encrypted Firmware Update System ===");
    ESP_LOGI(TAG, "Firmware Version: %d", ota_get_current_version());
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize subsystems
    ESP_ERROR_CHECK(crypto_init());
    ESP_ERROR_CHECK(ota_init());
    
    // Test crypto functions
    test_crypto_functions();
    
    // Verify current firmware
    ota_verify_firmware();
    
    // Initialize WiFi (optional - uncomment and configure for OTA updates)
    /*
    ESP_LOGI(TAG, "Connecting to WiFi...");
    ret = wifi_init_sta(WIFI_SSID, WIFI_PASSWORD);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "WiFi connected successfully");
        
        // Start OTA update task
        xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
    } else {
        ESP_LOGE(TAG, "WiFi connection failed");
    }
    */
    
    ESP_LOGI(TAG, "System initialized. Running main loop...");
    
    // Main application loop
    while (1) {
        ESP_LOGI(TAG, "Application running - Version %d", ota_get_current_version());
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
