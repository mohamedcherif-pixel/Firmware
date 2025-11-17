/*
 * WiFi Manager Implementation
 */

#include "wifi_manager.h"

#define WIFI_TIMEOUT_MS 20000
#define WIFI_RETRY_DELAY_MS 500

bool wifi_connect(const char* ssid, const char* password, unsigned long timeout_ms) {
    Serial.print("[WiFi] Connecting to: ");
    Serial.println(ssid);

    // Disconnect any previous connection
    WiFi.disconnect(true);  // true = turn off WiFi radio
    delay(100);

    // Set to station mode
    WiFi.mode(WIFI_STA);

    // Configure for better compatibility with iPhone hotspots
    WiFi.setAutoReconnect(true);

    // Begin connection
    WiFi.begin(ssid, password);

    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && 
            millis() - startAttemptTime < timeout_ms) {
        Serial.print(".");
        delay(WIFI_RETRY_DELAY_MS);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n[WiFi] Connection failed!");
        Serial.print("[WiFi] Status: ");
        Serial.println(WiFi.status());
        Serial.println("[WiFi] Possible causes:");
        Serial.println("  * Incorrect SSID or password");
        Serial.println("  * Network not in range");
        Serial.println("  * Network congestion");
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
