/*
 * WiFi Manager for ESP32
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

/**
 * @brief Connect to WiFi network
 * 
 * @param ssid WiFi SSID
 * @param password WiFi password
 * @return true if connected successfully
 */
bool wifi_connect(const char* ssid, const char* password);

/**
 * @brief Check if WiFi is connected
 * 
 * @return true if connected
 */
bool wifi_is_connected();

/**
 * @brief Disconnect from WiFi
 */
void wifi_disconnect();

#endif // WIFI_MANAGER_H
