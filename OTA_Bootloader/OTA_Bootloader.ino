/*
 * ESP32 OTA Bootloader
 * ====================
 * This bootloader handles encrypted firmware updates and loads user applications.
 * 
 * The bootloader:
 * 1. Connects to WiFi
 * 2. Checks for new user application firmware
 * 3. Downloads, decrypts, and verifies firmware
 * 4. Writes to user_app partition
 * 5. Boots the user application
 * 
 * User applications DO NOT need any OTA code - this bootloader handles everything!
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <Preferences.h>
#include "crypto_utils.h"
#include "rsa_verify.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

// WiFi credentials stored in NVS, but defaults here for first boot
const char* DEFAULT_WIFI_SSID = "TOPNET_2FB0";
const char* DEFAULT_WIFI_PASSWORD = "3m3smnb68l";

// GitHub Releases URLs for user application firmware
const char* USER_APP_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/user_app_encrypted.bin";
const char* USER_APP_SIG_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/user_app_encrypted.bin.sig";
const char* USER_APP_VERSION_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/user_app_version.txt";

// AES-256 key (32 bytes) - MUST match the key used to encrypt firmware
const uint8_t aes_key[32] = {
    0x30, 0xee, 0x86, 0xb5, 0x2f, 0xac, 0xf1, 0xfe, 
    0x3d, 0xf8, 0x74, 0x91, 0xd0, 0xb7, 0x79, 0xc9, 
    0x7c, 0x06, 0x68, 0xe9, 0x39, 0x18, 0xef, 0x04, 
    0x80, 0x66, 0x0e, 0x97, 0x2d, 0xb2, 0x40, 0xa2
};

// Bootloader version
#define BOOTLOADER_VERSION 1

// Update check timeout
#define WIFI_TIMEOUT_MS 15000
#define UPDATE_CHECK_TIMEOUT_MS 5000

// ============================================================================
// GLOBALS
// ============================================================================

Preferences preferences;
String wifi_ssid;
String wifi_password;
int current_user_app_version = 0;

// ============================================================================
// WIFI FUNCTIONS
// ============================================================================

bool loadWiFiCredentials() {
    preferences.begin("bootloader", false);
    wifi_ssid = preferences.getString("wifi_ssid", DEFAULT_WIFI_SSID);
    wifi_password = preferences.getString("wifi_pass", DEFAULT_WIFI_PASSWORD);
    current_user_app_version = preferences.getInt("app_version", 0);
    preferences.end();
    
    Serial.printf("[BOOT] Loaded WiFi SSID: %s\n", wifi_ssid.c_str());
    Serial.printf("[BOOT] Current user app version: %d\n", current_user_app_version);
    return true;
}

void saveUserAppVersion(int version) {
    preferences.begin("bootloader", false);
    preferences.putInt("app_version", version);
    preferences.putBool("valid_app", true);  // Mark that we have a valid user app
    preferences.end();
    current_user_app_version = version;
    Serial.printf("[BOOT] Saved user app version: %d\n", version);
}

bool connectWiFi() {
    Serial.printf("[WIFI] Connecting to: %s\n", wifi_ssid.c_str());
    
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT_MS) {
        Serial.print(".");
        delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.printf("[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }
    
    Serial.println();
    Serial.println("[WIFI] Connection failed!");
    return false;
}

// ============================================================================
// VERSION CHECK
// ============================================================================

int checkServerVersion() {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(UPDATE_CHECK_TIMEOUT_MS);
    
    // Cache-busting
    String url = String(USER_APP_VERSION_URL) + "?t=" + String(millis());
    
    Serial.printf("[VERSION] Checking: %s\n", USER_APP_VERSION_URL);
    http.begin(url.c_str());
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        payload.trim();
        if (payload.startsWith("v") || payload.startsWith("V")) {
            payload = payload.substring(1);
        }
        int version = payload.toInt();
        Serial.printf("[VERSION] Server version: %d\n", version);
        http.end();
        return version;
    }
    
    Serial.printf("[VERSION] Check failed: %d\n", httpCode);
    http.end();
    return -1;
}

// ============================================================================
// SIGNATURE VERIFICATION
// ============================================================================

bool verifyFirmwareSignature() {
    Serial.println("[RSA] Verifying user app signature...");
    
    if (!rsa_verify_init()) {
        Serial.println("[RSA] Failed to initialize RSA");
        return false;
    }
    
    if (rsa_verify_firmware_from_url(USER_APP_URL, USER_APP_SIG_URL)) {
        Serial.println("[RSA] ✓ Signature valid!");
        return true;
    }
    
    Serial.println("[RSA] ✗ Signature verification failed!");
    return false;
}

// ============================================================================
// OTA UPDATE TO USER PARTITION
// ============================================================================

bool updateUserApplication() {
    Serial.println("[OTA] Starting user application update...");
    
    // Find the user_app partition (ota_1 - second partition, bootloader runs from ota_0/factory)
    const esp_partition_t* user_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    
    // Fallback: try to get the "next" OTA partition
    if (!user_partition) {
        Serial.println("[OTA] ota_1 not found, trying next update partition...");
        user_partition = esp_ota_get_next_update_partition(NULL);
    }
    
    if (!user_partition) {
        Serial.println("[OTA] User app partition not found!");
        return false;
    }
    
    Serial.printf("[OTA] Target partition: %s (0x%x, %d bytes)\n", 
                  user_partition->label, user_partition->address, user_partition->size);
    
    // Download and decrypt firmware
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(30000);
    http.begin(USER_APP_URL);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[OTA] Download failed: %d\n", httpCode);
        http.end();
        return false;
    }
    
    int contentLength = http.getSize();
    Serial.printf("[OTA] Downloading %d bytes...\n", contentLength);
    
    WiFiClient* stream = http.getStreamPtr();
    
    // Read IV (first 16 bytes)
    uint8_t iv[16];
    int iv_read = 0;
    while (iv_read < 16 && http.connected()) {
        if (stream->available()) {
            iv[iv_read++] = stream->read();
        }
        delay(1);
    }
    
    if (iv_read != 16) {
        Serial.println("[OTA] Failed to read IV");
        http.end();
        return false;
    }
    
    Serial.print("[OTA] IV: ");
    for (int i = 0; i < 16; i++) Serial.printf("%02x", iv[i]);
    Serial.println();
    
    // Calculate encrypted size
    size_t encrypted_size = contentLength - 16;
    size_t expected_size = encrypted_size - 16; // Approximate (minus padding)
    
    // Begin update to user partition
    if (!Update.begin(expected_size, U_FLASH, -1, LOW)) {
        Serial.printf("[OTA] Update.begin failed: %s\n", Update.errorString());
        http.end();
        return false;
    }
    
    // Initialize streaming decryption
    aes_stream_context_t decrypt_ctx;
    if (!aes_decrypt_stream_init(&decrypt_ctx, aes_key, iv)) {
        Serial.println("[OTA] Decrypt init failed");
        Update.abort();
        http.end();
        return false;
    }
    
    // Allocate buffers
    const size_t CHUNK_SIZE = 1024;
    uint8_t* enc_buffer = (uint8_t*)malloc(CHUNK_SIZE);
    uint8_t* dec_buffer = (uint8_t*)malloc(CHUNK_SIZE);
    
    if (!enc_buffer || !dec_buffer) {
        Serial.println("[OTA] Buffer allocation failed");
        if (enc_buffer) free(enc_buffer);
        if (dec_buffer) free(dec_buffer);
        aes_decrypt_stream_free(&decrypt_ctx);
        Update.abort();
        http.end();
        return false;
    }
    
    // Stream, decrypt, and write
    size_t offset = 0;
    size_t total_written = 0;
    bool success = true;
    
    while (offset < encrypted_size && http.connected()) {
        size_t to_read = min((size_t)CHUNK_SIZE, encrypted_size - offset);
        to_read = (to_read / 16) * 16; // Align to AES block
        
        if (to_read == 0 && offset < encrypted_size) {
            to_read = encrypted_size - offset;
        }
        
        // Read encrypted chunk
        size_t read_len = 0;
        unsigned long read_start = millis();
        while (read_len < to_read && http.connected()) {
            size_t avail = stream->available();
            if (avail) {
                size_t can_read = min(avail, to_read - read_len);
                read_len += stream->readBytes(enc_buffer + read_len, can_read);
                read_start = millis();
            }
            if (millis() - read_start > 10000) {
                Serial.println("[OTA] Read timeout");
                success = false;
                break;
            }
            delay(1);
        }
        
        if (!success || read_len == 0) break;
        
        // Decrypt
        size_t dec_len = 0;
        if (!aes_decrypt_stream_update(&decrypt_ctx, enc_buffer, read_len, dec_buffer, &dec_len)) {
            Serial.println("[OTA] Decrypt failed");
            success = false;
            break;
        }
        
        // Remove padding on last chunk
        if (offset + read_len >= encrypted_size) {
            uint8_t padding = dec_buffer[dec_len - 1];
            if (padding > 0 && padding <= 16) {
                dec_len -= padding;
            }
        }
        
        // Write to flash
        if (Update.write(dec_buffer, dec_len) != dec_len) {
            Serial.printf("[OTA] Write failed: %s\n", Update.errorString());
            success = false;
            break;
        }
        
        offset += read_len;
        total_written += dec_len;
        
        // Progress
        if (offset % 51200 == 0 || offset >= encrypted_size) {
            Serial.printf("[OTA] Progress: %d/%d (%.1f%%)\n", 
                         offset, encrypted_size, (offset * 100.0) / encrypted_size);
        }
    }
    
    // Cleanup
    free(enc_buffer);
    free(dec_buffer);
    aes_decrypt_stream_free(&decrypt_ctx);
    http.end();
    
    if (!success) {
        Update.abort();
        return false;
    }
    
    // Finalize
    if (!Update.end(true)) {
        Serial.printf("[OTA] Update.end failed: %s\n", Update.errorString());
        return false;
    }
    
    Serial.printf("[OTA] ✓ User app updated! %d bytes written\n", total_written);
    return true;
}

// ============================================================================
// BOOT USER APPLICATION
// ============================================================================

// Magic marker to identify valid user apps installed by this bootloader
#define USER_APP_MAGIC_CHECK "USER_APP_INSTALLED"

bool isValidUserApp() {
    // Check if we have saved a valid user app version (meaning we installed one)
    preferences.begin("bootloader", true);  // Read-only
    int saved_version = preferences.getInt("app_version", 0);
    bool has_valid_app = preferences.getBool("valid_app", false);
    preferences.end();
    
    return (saved_version > 0 && has_valid_app);
}

void markUserAppValid() {
    preferences.begin("bootloader", false);
    preferences.putBool("valid_app", true);
    preferences.end();
}

void bootUserApplication() {
    Serial.println("\n[BOOT] Booting user application...");
    
    // First check: Did WE install a user app?
    if (!isValidUserApp()) {
        Serial.println("[BOOT] No user app installed by this bootloader!");
        Serial.println("[BOOT] Waiting for first firmware upload via GitHub...");
        Serial.println("[BOOT] Push code to User_Application/ folder to trigger deployment");
        return;
    }
    
    // Find user_app partition (ota_1 - the second app partition)
    const esp_partition_t* user_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    
    // Fallback to ota_0 if ota_1 doesn't exist (different partition schemes)
    if (!user_partition) {
        Serial.println("[BOOT] Trying alternate partition...");
        user_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
    }
    
    if (!user_partition) {
        Serial.println("[BOOT] User app partition not found!");
        Serial.println("[BOOT] Staying in bootloader mode...");
        return;
    }
    
    // Check if partition has valid app
    esp_app_desc_t app_desc;
    if (esp_ota_get_partition_description(user_partition, &app_desc) != ESP_OK) {
        Serial.println("[BOOT] No valid user app in partition!");
        Serial.println("[BOOT] Waiting for first firmware upload...");
        return;
    }
    
    // Don't boot if it's the same as bootloader (prevent loop!)
    if (strstr(app_desc.project_name, "OTA_Bootloader") != NULL ||
        strstr(app_desc.project_name, "arduino-lib-builder") != NULL) {
        Serial.println("[BOOT] Detected bootloader or default firmware in user partition");
        Serial.println("[BOOT] Waiting for real user app upload...");
        return;
    }
    
    Serial.printf("[BOOT] Found user app: %s v%s\n", app_desc.project_name, app_desc.version);
    Serial.println("[BOOT] Jumping to user application in 2 seconds...\n");
    delay(2000);
    
    // Set boot partition and restart
    esp_ota_set_boot_partition(user_partition);
    ESP.restart();
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n");
    Serial.println("╔══════════════════════════════════════════════════════════╗");
    Serial.println("║          ESP32 OTA BOOTLOADER v" + String(BOOTLOADER_VERSION) + "                        ║");
    Serial.println("║   Secure Encrypted Firmware Update System                ║");
    Serial.println("╚══════════════════════════════════════════════════════════╝");
    Serial.println();
    
    // Initialize crypto
    crypto_init();
    
    // Initialize RSA verification
    if (!rsa_verify_init()) {
        Serial.println("[BOOT] Warning: RSA init failed");
    }
    
    // Load settings from NVS
    loadWiFiCredentials();
    
    // Try to connect to WiFi
    if (connectWiFi()) {
        // Check for updates
        Serial.println("\n[BOOT] Checking for user app updates...");
        
        int server_version = checkServerVersion();
        if (server_version > 0 && server_version > current_user_app_version) {
            Serial.printf("[BOOT] New version available: %d (current: %d)\n", 
                         server_version, current_user_app_version);
            
            // Verify signature first
            if (verifyFirmwareSignature()) {
                // Download and install
                if (updateUserApplication()) {
                    saveUserAppVersion(server_version);
                    Serial.println("[BOOT] ✓ Update successful!");
                } else {
                    Serial.println("[BOOT] ✗ Update failed!");
                }
            } else {
                Serial.println("[BOOT] ✗ Signature invalid - update rejected!");
            }
        } else if (server_version > 0) {
            Serial.println("[BOOT] User app is up to date");
        } else {
            Serial.println("[BOOT] Could not check version (offline mode)");
        }
        
        WiFi.disconnect();
    } else {
        Serial.println("[BOOT] WiFi failed - booting existing user app");
    }
    
    // Boot user application
    bootUserApplication();
    
    // If we get here, no user app exists
    Serial.println("\n[BOOT] No user application installed!");
    Serial.println("[BOOT] Entering bootloader maintenance mode...");
    Serial.println("[BOOT] Upload a user application via GitHub Releases");
}

// ============================================================================
// LOOP - Maintenance Mode (only runs if no user app)
// ============================================================================

void loop() {
    static unsigned long lastCheck = 0;
    
    // Check for updates every 30 seconds in maintenance mode
    if (millis() - lastCheck >= 30000) {
        Serial.println("\n[MAINT] Checking for user application...");
        
        if (WiFi.status() != WL_CONNECTED) {
            connectWiFi();
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            int server_version = checkServerVersion();
            if (server_version > 0) {
                Serial.printf("[MAINT] Found version %d on server\n", server_version);
                
                if (verifyFirmwareSignature()) {
                    if (updateUserApplication()) {
                        saveUserAppVersion(server_version);
                        Serial.println("[MAINT] ✓ User app installed! Rebooting...");
                        delay(1000);
                        ESP.restart();
                    }
                }
            }
        }
        
        lastCheck = millis();
    }
    
    delay(100);
}
