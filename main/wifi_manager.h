#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"

/**
 * @brief Initialize WiFi in station mode
 * 
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_init_sta(const char *ssid, const char *password);

/**
 * @brief Check if WiFi is connected
 * 
 * @return true if connected, false otherwise
 */
bool wifi_is_connected(void);

/**
 * @brief Disconnect WiFi
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_disconnect(void);

#endif // WIFI_MANAGER_H
