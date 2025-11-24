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
#include "rsa_verify.h"

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
const char* OTA_SIGNATURE_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/firmware_encrypted.bin.sig";
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
#define FIRMWARE_VERSION 18

// Update check interval (milliseconds). Change here to affect the periodic check.
// Set to 10 seconds to perform version checks more frequently for testing.
const unsigned long UPDATE_CHECK_INTERVAL_MS = 10UL * 1000UL; // 10 seconds

// (VERSION_CHECK_URL defined above with OTA_FIRMWARE_URL)

// Check server for latest firmware version
int check_server_version() {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    // Keep timeout shorter than the interval so that slow network responses don't block
    // subsequent checks. Use 5 seconds here.
    http.setTimeout(5000); // 5 second timeout
    
    // Add cache-busting query parameter to prevent GitHub CDN caching
    String url = String(VERSION_CHECK_URL) + "?t=" + String(millis());
    http.begin(url.c_str());
    
    Serial.printf("[VERSION] Checking: %s\n", VERSION_CHECK_URL);
    unsigned long start_ms = millis();
    Serial.printf("[VERSION] Start check at: %lu ms\n", start_ms);
    
    Serial.print("[WIFI] WiFi Status: ");
    Serial.println(WiFi.status());
    Serial.print("[WIFI] Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("[WIFI] WiFi Mode: ");
    Serial.println(WiFi.getMode());
    Serial.print("[WIFI] WiFi Channel: ");
    Serial.println(WiFi.channel());
    Serial.print("[WIFI] WiFi BSSID: ");
    Serial.println(WiFi.BSSIDstr());
    Serial.print("[WIFI] WiFi Subnet Mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("[WIFI] WiFi Gateway IP: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("[WIFI] WiFi DNS IP: ");
    Serial.println(WiFi.dnsIP());
    
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
        unsigned long elapsed = millis() - start_ms;
        Serial.printf("[VERSION] Check completed in %lu ms\n", elapsed);
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
    
    // Initialize RSA verification
    if (!rsa_verify_init()) {
        Serial.println("✗ RSA verification initialization failed");
    } else {
        Serial.println("✓ RSA verification initialized");
    }
    
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
                Serial.println("⚠️  New version available! Verifying signature...");
                delay(2000);
                
                // Verify firmware signature before downloading
                Serial.println("[RSA] Verifying firmware signature...");
                if (rsa_verify_firmware_from_url(OTA_FIRMWARE_URL, OTA_SIGNATURE_URL)) {
                    Serial.println("[RSA] ✓ Signature verification passed");
                    Serial.println("Starting OTA update...");
                    
                    if (ota_update_from_url(OTA_FIRMWARE_URL, aes_key)) {
                        Serial.println("✓ OTA update successful! Rebooting...");
                        delay(1000);
                        ESP.restart();
                    } else {
                        Serial.println("✗ OTA update failed");
                    }
                } else {
                    Serial.println("[RSA] ✗ Signature verification failed - Update rejected!");
                    Serial.println("Security: Firmware may be tampered or corrupted");
                }
            } else {
                Serial.println("✓ Already running latest version");
            }
        } else {
            Serial.println("✗ Failed to check server version");
        }
        
        Serial.println("\nSystem initialized. Running main loop...");
        Serial.printf("[UPDATE] Periodic update check interval: %lu ms\n", UPDATE_CHECK_INTERVAL_MS);
    } else {
        Serial.println("✗ WiFi connection failed");
    }
    
    Serial.println("\nSystem initialized. Running main loop...");
    Serial.printf("[UPDATE] Periodic update check interval: %lu ms\n", UPDATE_CHECK_INTERVAL_MS);
}

void loop() {
    // Main application loop
    static unsigned long lastPrint = 0;
    static unsigned long lastUpdateCheck = 0;
    // Use the global interval defined above so we only edit in one place.
    // Keep a `static` in the loop so it persists across iterations.
    static const unsigned long UPDATE_CHECK_INTERVAL = UPDATE_CHECK_INTERVAL_MS;
    
    // Print status every 5 seconds
    if (millis() - lastPrint >= 5000) {
        Serial.print("Application running - Version ");
        Serial.print(FIRMWARE_VERSION);
        Serial.print(" - Uptime: ");
        Serial.print(millis() / 1000);
        Serial.println(" seconds");
        
        lastPrint = millis();
    }
    
    // Check for updates at the interval defined by UPDATE_CHECK_INTERVAL_MS
    if (millis() - lastUpdateCheck >= UPDATE_CHECK_INTERVAL) {
        Serial.println("\n=== Periodic Update Check ===");
        Serial.printf("Current firmware version: %d\n", FIRMWARE_VERSION);
        Serial.printf("[UPDATE] Checking now (millis=%lu)\n", millis());
        // Ensure WiFi is connected; if not, try to reconnect and skip this check if reconnection fails
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WIFI] WiFi disconnected, attempting quick reconnection (3s)...");
            // Use a short timeout to avoid blocking the periodic check for too long
            if (!wifi_connect(WIFI_SSID, WIFI_PASSWORD, 3000UL)) {
                Serial.println("[WIFI] Reconnection failed; skipping this update check");
                lastUpdateCheck = millis();
                // Skip this cycle; give time to reconnect
            } else {
                Serial.println("[WIFI] Reconnected successfully");
            }
        }
        
        int server_version = check_server_version();
        if (server_version > 0) {
            Serial.printf("Server firmware version: %d\n", server_version);
            
            if (server_version > FIRMWARE_VERSION) {
                Serial.println("⚠️  New version available! Verifying signature...");
                delay(2000);
                
                // Verify firmware signature before downloading
                Serial.println("[RSA] Verifying firmware signature...");
                if (rsa_verify_firmware_from_url(OTA_FIRMWARE_URL, OTA_SIGNATURE_URL)) {
                    Serial.println("[RSA] ✓ Signature verification passed");
                    Serial.println("Starting OTA update...");
                    
                    if (ota_update_from_url(OTA_FIRMWARE_URL, aes_key)) {
                        Serial.println("✓ OTA update successful! Rebooting...");
                        delay(1000);
                        ESP.restart();
                    } else {
                        Serial.println("✗ OTA update failed");
                    }
                } else {
                    Serial.println("[RSA] ✗ Signature verification failed - Update rejected!");
                    Serial.println("Security: Firmware may be tampered or corrupted");
                }
            } else {
                Serial.println("✓ Already running latest version");
            }
        } else {
            Serial.println("✗ Failed to check server version");
        }
        
        unsigned long check_end_ms = millis();
        Serial.printf("[UPDATE] Next check scheduled at: %lu (interval %lu ms)\n", check_end_ms + UPDATE_CHECK_INTERVAL, UPDATE_CHECK_INTERVAL);
        lastUpdateCheck = check_end_ms;
    }
    
    // Add your application code here
    
    delay(100);
}
