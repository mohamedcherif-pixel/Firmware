#include "ota_update.h"
#include "crypto_utils.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_app_format.h"
#include <string.h>

static const char *TAG = "OTA";

#define BUFFSIZE 1024
#define HASH_LEN 32

// Current firmware version (should be incremented with each release)
#define CURRENT_FIRMWARE_VERSION 1

esp_err_t ota_init(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            ESP_LOGI(TAG, "First boot after OTA update, validating...");
            esp_ota_mark_app_valid_cancel_rollback();
        }
    }

    ESP_LOGI(TAG, "OTA subsystem initialized. Current version: %d", CURRENT_FIRMWARE_VERSION);
    ESP_LOGI(TAG, "Running partition: %s", running->label);
    
    return ESP_OK;
}

uint32_t ota_get_current_version(void)
{
    return CURRENT_FIRMWARE_VERSION;
}

esp_err_t ota_verify_firmware(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t app_desc;
    
    if (esp_ota_get_partition_description(running, &app_desc) == ESP_OK) {
        ESP_LOGI(TAG, "Current firmware: %s %s", app_desc.project_name, app_desc.version);
        return ESP_OK;
    }
    
    return ESP_FAIL;
}

esp_err_t ota_rollback(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_VALID || ota_state == ESP_OTA_IMG_UNDEFINED) {
            ESP_LOGI(TAG, "Initiating rollback to previous firmware");
            esp_ota_mark_app_invalid_rollback_and_reboot();
            return ESP_OK;
        }
    }
    
    ESP_LOGE(TAG, "Rollback not possible");
    return ESP_FAIL;
}

esp_err_t ota_update_from_buffer(const uint8_t *encrypted_firmware, 
                                  size_t firmware_size,
                                  const uint8_t *aes_key,
                                  const uint8_t *iv)
{
    esp_err_t err;
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;

    ESP_LOGI(TAG, "Starting encrypted OTA update from buffer");
    ESP_LOGI(TAG, "Encrypted firmware size: %d bytes", firmware_size);

    // Get next OTA partition
    const esp_partition_t *running = esp_ota_get_running_partition();
    update_partition = esp_ota_get_next_update_partition(NULL);
    
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "No OTA partition found");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Writing to partition: %s at offset 0x%lx", 
             update_partition->label, update_partition->address);

    // Begin OTA update
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(err));
        return err;
    }

    // Decrypt and write firmware in chunks
    size_t offset = 0;
    uint8_t *decrypt_buffer = malloc(BUFFSIZE + AES_BLOCK_SIZE);
    if (!decrypt_buffer) {
        ESP_LOGE(TAG, "Failed to allocate decryption buffer");
        esp_ota_abort(update_handle);
        return ESP_ERR_NO_MEM;
    }

    while (offset < firmware_size) {
        size_t chunk_size = (firmware_size - offset) > BUFFSIZE ? BUFFSIZE : (firmware_size - offset);
        
        // Ensure chunk is aligned to AES block size
        if (chunk_size % AES_BLOCK_SIZE != 0 && offset + chunk_size < firmware_size) {
            chunk_size = (chunk_size / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
        }

        size_t decrypted_len = 0;
        err = aes_decrypt(encrypted_firmware + offset, chunk_size,
                         aes_key, iv, decrypt_buffer, &decrypted_len);
        
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Decryption failed at offset %d", offset);
            free(decrypt_buffer);
            esp_ota_abort(update_handle);
            return err;
        }

        // Write decrypted chunk to OTA partition
        err = esp_ota_write(update_handle, decrypt_buffer, decrypted_len);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_write failed: %s", esp_err_to_name(err));
            free(decrypt_buffer);
            esp_ota_abort(update_handle);
            return err;
        }

        offset += chunk_size;
        ESP_LOGI(TAG, "Written %d/%d bytes", offset, firmware_size);
    }

    free(decrypt_buffer);

    // Finalize OTA update
    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(err));
        return err;
    }

    // Set boot partition
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "OTA update successful! Rebooting...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();

    return ESP_OK;
}

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

esp_err_t ota_update_from_url(const char *url, const uint8_t *aes_key)
{
    ESP_LOGI(TAG, "Starting encrypted OTA update from URL: %s", url);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .keep_alive_enable = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    int content_length = esp_http_client_fetch_headers(client);
    if (content_length <= 0) {
        ESP_LOGE(TAG, "Invalid content length: %d", content_length);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Downloading encrypted firmware: %d bytes", content_length);

    // Allocate buffer for encrypted firmware
    uint8_t *encrypted_firmware = malloc(content_length);
    if (!encrypted_firmware) {
        ESP_LOGE(TAG, "Failed to allocate memory for firmware");
        esp_http_client_cleanup(client);
        return ESP_ERR_NO_MEM;
    }

    // Download encrypted firmware
    int total_read = 0;
    while (total_read < content_length) {
        int read_len = esp_http_client_read(client, (char *)(encrypted_firmware + total_read), 
                                            content_length - total_read);
        if (read_len < 0) {
            ESP_LOGE(TAG, "Error reading HTTP stream");
            free(encrypted_firmware);
            esp_http_client_cleanup(client);
            return ESP_FAIL;
        }
        total_read += read_len;
        ESP_LOGI(TAG, "Downloaded %d/%d bytes", total_read, content_length);
    }

    esp_http_client_cleanup(client);

    // Extract IV from first 16 bytes (or use provided IV)
    uint8_t iv[AES_IV_SIZE];
    memcpy(iv, encrypted_firmware, AES_IV_SIZE);

    // Decrypt and flash firmware
    err = ota_update_from_buffer(encrypted_firmware + AES_IV_SIZE, 
                                 content_length - AES_IV_SIZE,
                                 aes_key, iv);

    free(encrypted_firmware);
    return err;
}
