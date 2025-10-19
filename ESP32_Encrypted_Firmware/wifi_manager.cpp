/*
 * WiFi Manager Implementation
 */

#include "wifi_manager.h"

#define WIFI_TIMEOUT_MS 20000
#define WIFI_RETRY_DELAY_MS 500

bool wifi_connect(const char* ssid, const char* password) {
    Serial.print("[WiFi] Connecting to: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    unsigned long startAttemptTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
        Serial.print(".");
        delay(WIFI_RETRY_DELAY_MS);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n[WiFi] Connection failed!");
        return false;
    }
    
    Serial.println("\n[WiFi] Connected successfully!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WiFi] Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    return true;
}

bool wifi_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

void wifi_disconnect() {
    WiFi.disconnect();
    Serial.println("[WiFi] Disconnected");
}
