/*
 * User Application: WiFi Scanner
 * ===============================
 * This is a PURE user application - NO OTA CODE NEEDED!
 * The OTA Bootloader handles all firmware updates.
 * 
 * Scans and displays nearby WiFi networks with signal strength.
 */

#include <WiFi.h>

// User App Version - increment this for each release
#define USER_APP_VERSION 1

// Scan interval in milliseconds
#define SCAN_INTERVAL 10000

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("\n");
    Serial.println("╔══════════════════════════════════════════════════════════╗");
    Serial.println("║          USER APPLICATION: WIFI SCANNER                  ║");
    Serial.println("║          Version: " + String(USER_APP_VERSION) + "                                       ║");
    Serial.println("╚══════════════════════════════════════════════════════════╝");
    Serial.println();
    Serial.println("This is a PURE user application!");
    Serial.println("No OTA code here - the bootloader handles updates.");
    Serial.println();
    
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    Serial.println("WiFi Scanner initialized!");
    Serial.printf("Scanning every %d seconds...\n", SCAN_INTERVAL / 1000);
    Serial.println();
}

String getEncryptionType(wifi_auth_mode_t authMode) {
    switch (authMode) {
        case WIFI_AUTH_OPEN:            return "Open";
        case WIFI_AUTH_WEP:             return "WEP";
        case WIFI_AUTH_WPA_PSK:         return "WPA";
        case WIFI_AUTH_WPA2_PSK:        return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Enterprise";
        case WIFI_AUTH_WPA3_PSK:        return "WPA3";
        default:                        return "Unknown";
    }
}

String getSignalBars(int rssi) {
    if (rssi >= -50) return "████";      // Excellent
    if (rssi >= -60) return "███░";      // Good
    if (rssi >= -70) return "██░░";      // Fair
    if (rssi >= -80) return "█░░░";      // Weak
    return "░░░░";                        // Very Weak
}

void scanNetworks() {
    Serial.println("🔍 Scanning WiFi networks...\n");
    
    int networksFound = WiFi.scanNetworks();
    
    if (networksFound == 0) {
        Serial.println("No networks found!");
    } else {
        Serial.printf("Found %d networks:\n\n", networksFound);
        Serial.println("╔════╦══════════════════════════════╦════════╦══════════════╦════════════╗");
        Serial.println("║ #  ║ SSID                         ║ RSSI   ║ Signal       ║ Security   ║");
        Serial.println("╠════╬══════════════════════════════╬════════╬══════════════╬════════════╣");
        
        for (int i = 0; i < networksFound; i++) {
            String ssid = WiFi.SSID(i);
            int rssi = WiFi.RSSI(i);
            String encryption = getEncryptionType(WiFi.encryptionType(i));
            String signalBars = getSignalBars(rssi);
            
            // Truncate SSID if too long
            if (ssid.length() > 28) {
                ssid = ssid.substring(0, 25) + "...";
            }
            
            // Pad SSID to 28 characters
            while (ssid.length() < 28) ssid += " ";
            
            // Pad encryption to 10 characters
            while (encryption.length() < 10) encryption += " ";
            
            Serial.printf("║ %2d ║ %s ║ %4d   ║ %s       ║ %s ║\n", 
                         i + 1, ssid.c_str(), rssi, signalBars.c_str(), encryption.c_str());
        }
        
        Serial.println("╚════╩══════════════════════════════╩════════╩══════════════╩════════════╝");
    }
    
    Serial.println();
    
    // Clean up
    WiFi.scanDelete();
}

void loop() {
    scanNetworks();
    
    Serial.printf("Next scan in %d seconds...\n\n", SCAN_INTERVAL / 1000);
    delay(SCAN_INTERVAL);
}
