/*
 * ESP32 Encrypted Firmware Update System
 * Project 03: Mise à jour d'un objet avec un firmware crypté
 * 
 * Main Arduino sketch for encrypted OTA updates
 */

#include <HTTPClient.h>
#include "crypto_utils.h"
#include "ota_update.h"
#include "wifi_manager.h"

// WiFi credentials - UPDATE THESE!
const char* WIFI_SSID = "TOPNET_2FB0";
const char* WIFI_PASSWORD = "3m3smnb68l";

// ============================================================================
// GITHUB RELEASES HOSTING (Public Cloud - Always Available!)
// ============================================================================
// Instead of local server, firmware is now hosted on GitHub Releases
// Repository: https://github.com/mohamedcherif-pixel/Firmware
//
// Option 1: Specific version (change v1 to match your release tag)
// const char* OTA_FIRMWARE_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/download/v1/firmware_encrypted.bin";
// const char* VERSION_CHECK_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/download/v1/version.txt";
//
// Option 2: Always get latest version (RECOMMENDED)
const char* OTA_FIRMWARE_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/firmware_encrypted.bin";
const char* VERSION_CHECK_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt";

// OLD Local server URLs (backup for testing):
// const char* OTA_FIRMWARE_URL = "http://192.168.1.22:8080/firmware_encrypted.bin";
// const char* VERSION_CHECK_URL = "http://192.168.1.22:8000/version.txt";
// ============================================================================

// AES-256 key (32 bytes) - MUST match the key used to encrypt firmware!
// This key is from aes_key.bin - DO NOT CHANGE unless you re-encrypt firmware!
const uint8_t aes_key[32] = {
    0x30, 0xee, 0x86, 0xb5, 0x2f, 0xac, 0xf1, 0xfe, 
    0x3d, 0xf8, 0x74, 0x91, 0xd0, 0xb7, 0x79, 0xc9, 
    0x7c, 0x06, 0x68, 0xe9, 0x39, 0x18, 0xef, 0x04, 
    0x80, 0x66, 0x0e, 0x97, 0x2d, 0xb2, 0x40, 0xa2
};

// IV (16 bytes) - Embedded in encrypted firmware file
// This is just a placeholder, actual IV is read from firmware file
const uint8_t aes_iv[16] = {
    0x6e, 0x75, 0xd6, 0xe9, 0x5c, 0x92, 0x5e, 0x15, 
    0xd4, 0xbd, 0x42, 0x73, 0x10, 0x11, 0xb6, 0x51
};

// Current firmware version - increment with each release
#define FIRMWARE_VERSION 3

// (VERSION_CHECK_URL defined above with OTA_FIRMWARE_URL)

// Check server for latest firmware version
int check_server_version() {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(10000); // 10 second timeout
    http.begin(VERSION_CHECK_URL);
    
    Serial.printf("[VERSION] Checking: %s\n", VERSION_CHECK_URL);
    
    int httpCode = http.GET();
    Serial.printf("[VERSION] HTTP Code: %d\n", httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("[VERSION] Response: %s\n", payload.c_str());
        
        // Clean up the version string - remove any 'v' prefix and whitespace
        payload.trim();
        if (payload.startsWith("v") || payload.startsWith("V")) {
            payload = payload.substring(1);
        }
        
        int server_version = payload.toInt();
        Serial.printf("[VERSION] Parsed version: %d\n", server_version);
        
        http.end();
        return server_version;
    } else {
        Serial.printf("[VERSION] Error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return -1; // Error
}

void testCryptoFunctions() {
    Serial.println("\n=== Testing Crypto Functions ===");
    
    const char* test_data = "Hello, ESP32 Encrypted Firmware!";
    size_t test_len = strlen(test_data);
    
    // Allocate buffers
    uint8_t* ciphertext = (uint8_t*)malloc(test_len + AES_BLOCK_SIZE);
    uint8_t* plaintext = (uint8_t*)malloc(test_len + AES_BLOCK_SIZE);
    size_t cipher_len, plain_len;
    
    // Test encryption
    Serial.print("Original: ");
    Serial.println(test_data);
    
    if (aes_encrypt((uint8_t*)test_data, test_len, aes_key, aes_iv, 
                    ciphertext, &cipher_len)) {
        Serial.print("Encryption successful, ciphertext length: ");
        Serial.println(cipher_len);
        
        // Test decryption
        if (aes_decrypt(ciphertext, cipher_len, aes_key, aes_iv, 
                       plaintext, &plain_len)) {
            plaintext[plain_len] = '\0';
            Serial.print("Decryption successful: ");
            Serial.println((char*)plaintext);
            
            if (strcmp((char*)plaintext, test_data) == 0) {
                Serial.println("✓ Crypto test PASSED");
            } else {
                Serial.println("✗ Crypto test FAILED - data mismatch");
            }
        } else {
            Serial.println("✗ Decryption failed");
        }
    } else {
        Serial.println("✗ Encryption failed");
    }
    
    // Test SHA-256
    uint8_t hash[32];
    if (sha256_hash((uint8_t*)test_data, test_len, hash)) {
        Serial.println("SHA-256 hash computed successfully");
        Serial.print("Hash: ");
        for (int i = 0; i < 32; i++) {
            Serial.printf("%02x", hash[i]);
        }
        Serial.println();
        
        // Verify hash
        if (sha256_verify((uint8_t*)test_data, test_len, hash)) {
            Serial.println("✓ Hash verification PASSED");
        } else {
            Serial.println("✗ Hash verification FAILED");
        }
    }
    
    free(ciphertext);
    free(plaintext);
    Serial.println("=== Crypto Tests Complete ===\n");
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== ESP32 Encrypted Firmware Update System ===");
    Serial.print("Firmware Version: ");
    Serial.println(FIRMWARE_VERSION);
    Serial.println("================================================\n");
    
    // Initialize subsystems
    crypto_init();
    ota_init();
    
    // Test crypto functions
    testCryptoFunctions();
    
    // Verify current firmware
    ota_verify_firmware();
    
    // Connect to WiFi (for OTA updates)
    Serial.println("Connecting to WiFi...");
    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("✓ WiFi connected successfully");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        
        // Check for firmware updates
        Serial.println("\n=== Checking for firmware updates ===");
        Serial.printf("Current firmware version: %d\n", FIRMWARE_VERSION);
        
        int server_version = check_server_version();
        if (server_version > 0) {
            Serial.printf("Server firmware version: %d\n", server_version);
            
            if (server_version > FIRMWARE_VERSION) {
                Serial.println("⚠️  New version available! Starting update...");
                delay(2000);
                
                if (ota_update_from_url(OTA_FIRMWARE_URL, aes_key)) {
                    Serial.println("✓ OTA update successful! Rebooting...");
                    delay(1000);
                    ESP.restart();
                } else {
                    Serial.println("✗ OTA update failed");
                }
            } else {
                Serial.println("✓ Already running latest version");
            }
        } else {
            Serial.println("✗ Failed to check server version");
        }
    } else {
        Serial.println("✗ WiFi connection failed");
    }
    
    Serial.println("\nSystem initialized. Running main loop...");
}

void loop() {
    // Main application loop
    static unsigned long lastPrint = 0;
    static unsigned long lastUpdateCheck = 0;
    static const unsigned long UPDATE_CHECK_INTERVAL = 5 * 60 * 1000; // Check every 5 minutes
    
    // Print status every 5 seconds
    if (millis() - lastPrint >= 5000) {
        Serial.print("Application running - Version ");
        Serial.print(FIRMWARE_VERSION);
        Serial.print(" - Uptime: ");
        Serial.print(millis() / 1000);
        Serial.println(" seconds");
        
        lastPrint = millis();
    }
    
    // Check for updates every 5 minutes
    if (millis() - lastUpdateCheck >= UPDATE_CHECK_INTERVAL) {
        Serial.println("\n=== Periodic Update Check ===");
        Serial.printf("Current firmware version: %d\n", FIRMWARE_VERSION);
        
        int server_version = check_server_version();
        if (server_version > 0) {
            Serial.printf("Server firmware version: %d\n", server_version);
            
            if (server_version > FIRMWARE_VERSION) {
                Serial.println("⚠️  New version available! Starting update...");
                delay(2000);
                
                if (ota_update_from_url(OTA_FIRMWARE_URL, aes_key)) {
                    Serial.println("✓ OTA update successful! Rebooting...");
                    delay(1000);
                    ESP.restart();
                } else {
                    Serial.println("✗ OTA update failed");
                }
            } else {
                Serial.println("✓ Already running latest version");
            }
        } else {
            Serial.println("✗ Failed to check server version");
        }
        
        lastUpdateCheck = millis();
    }
    
    // Add your application code here
    
    delay(100);
}
