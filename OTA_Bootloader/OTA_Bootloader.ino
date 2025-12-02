#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <Preferences.h>
#include "crypto_utils.h"
#include "rsa_verify.h"
#define setup user_setup
#define loop user_loop
#include "user_code.h"
#undef setup
#undef loop

// ============================================================================
// CONFIGURATION - DO NOT MODIFY BELOW THIS LINE
// ============================================================================

const char* DEFAULT_WIFI_SSID = "iPhone";
const char* DEFAULT_WIFI_PASSWORD = "zied20244";

const char* FIRMWARE_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/user_app_encrypted.bin";
const char* FIRMWARE_SIG_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/user_app_encrypted.bin.sig";
const char* VERSION_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/user_app_version.txt";

const uint8_t aes_key[32] = {
    0x30, 0xee, 0x86, 0xb5, 0x2f, 0xac, 0xf1, 0xfe, 
    0x3d, 0xf8, 0x74, 0x91, 0xd0, 0xb7, 0x79, 0xc9, 
    0x7c, 0x06, 0x68, 0xe9, 0x39, 0x18, 0xef, 0x04, 
    0x80, 0x66, 0x0e, 0x97, 0x2d, 0xb2, 0x40, 0xa2
};

#define BOOTLOADER_VERSION 2
#define UPDATE_CHECK_INTERVAL_MS 5000
#define WIFI_TIMEOUT_MS 15000

// ============================================================================
// GLOBALS
// ============================================================================

Preferences preferences;
String wifi_ssid;
String wifi_password;
int stored_version = 0;

TaskHandle_t otaTaskHandle = NULL;
SemaphoreHandle_t wifiMutex;
volatile bool updateInProgress = false;

// THREAD-SAFE PRINTING
void otaPrint(const char* msg) {
    Serial.print(msg);
}
void otaPrintln(const char* msg) {
    Serial.println(msg);
}
void otaPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.print(buffer);
    va_end(args);
}
// WIFI FUNCTIONS
void loadSettings() {
    preferences.begin("ota", false);
    wifi_ssid = preferences.getString("ssid", DEFAULT_WIFI_SSID);
    wifi_password = preferences.getString("pass", DEFAULT_WIFI_PASSWORD);
    stored_version = preferences.getInt("version", 0);
    preferences.end();
}
void saveVersion(int version) {
    preferences.begin("ota", false);
    preferences.putInt("version", version);
    preferences.end();
    stored_version = version;
}
bool connectWiFi() {
    if (xSemaphoreTake(wifiMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
        return false;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        xSemaphoreGive(wifiMutex);
        return true;
    }
    
    otaPrintf("[OTA] Connecting to WiFi: %s\n", wifi_ssid.c_str());
    
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
        delay(500);
    }
    
    bool connected = (WiFi.status() == WL_CONNECTED);
    if (connected) {
        otaPrintf("[OTA] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    }
    
    xSemaphoreGive(wifiMutex);
    return connected;
}
// VERSION CHECK
int getServerVersion() {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(5000);
    
    String url = String(VERSION_URL) + "?t=" + String(millis());
    http.begin(url.c_str());
    
    int httpCode = http.GET();
    int version = -1;
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        payload.trim();
        if (payload.startsWith("v") || payload.startsWith("V")) {
            payload = payload.substring(1);
        }
        version = payload.toInt();
    }
    
    http.end();
    return version;
}
// SIGNATURE VERIFICATION
bool verifySignature() {
    if (!rsa_verify_init()) return false;
    return rsa_verify_firmware_from_url(FIRMWARE_URL, FIRMWARE_SIG_URL);
}
// OTA UPDATE
bool downloadAndInstall() {
    updateInProgress = true;
    otaPrintln("[OTA] Downloading update...");
    
    const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
    if (!partition) {
        updateInProgress = false;
        return false;
    }
    
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(30000);
    http.begin(FIRMWARE_URL);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        updateInProgress = false;
        return false;
    }
    
    int contentLength = http.getSize();
    WiFiClient* stream = http.getStreamPtr();
    
    // Read IV
    uint8_t iv[16];
    int iv_read = 0;
    while (iv_read < 16 && http.connected()) {
        if (stream->available()) iv[iv_read++] = stream->read();
        delay(1);
    }
    
    if (iv_read != 16) {
        http.end();
        updateInProgress = false;
        return false;
    }
    
    size_t encrypted_size = contentLength - 16;
    
    if (!Update.begin(encrypted_size, U_FLASH, -1, LOW)) {
        http.end();
        updateInProgress = false;
        return false;
    }
    
    aes_stream_context_t ctx;
    if (!aes_decrypt_stream_init(&ctx, aes_key, iv)) {
        Update.abort();
        http.end();
        updateInProgress = false;
        return false;
    }
    
    uint8_t* enc_buf = (uint8_t*)malloc(1024);
    uint8_t* dec_buf = (uint8_t*)malloc(1024);
    
    if (!enc_buf || !dec_buf) {
        if (enc_buf) free(enc_buf);
        if (dec_buf) free(dec_buf);
        aes_decrypt_stream_free(&ctx);
        Update.abort();
        http.end();
        updateInProgress = false;
        return false;
    }
    
    size_t offset = 0;
    bool success = true;
    
    while (offset < encrypted_size && http.connected() && success) {
        size_t to_read = min((size_t)1024, encrypted_size - offset);
        to_read = (to_read / 16) * 16;
        if (to_read == 0) to_read = encrypted_size - offset;
        
        size_t read_len = 0;
        unsigned long read_start = millis();
        while (read_len < to_read && http.connected()) {
            if (stream->available()) {
                size_t can_read = min((size_t)stream->available(), to_read - read_len);
                read_len += stream->readBytes(enc_buf + read_len, can_read);
                read_start = millis();
            }
            if (millis() - read_start > 10000) {
                success = false;
                break;
            }
            delay(1);
        }
        
        if (!success || read_len == 0) break;
        
        size_t dec_len = 0;
        if (!aes_decrypt_stream_update(&ctx, enc_buf, read_len, dec_buf, &dec_len)) {
            success = false;
            break;
        }
        
        // Remove padding on last chunk
        if (offset + read_len >= encrypted_size) {
            uint8_t padding = dec_buf[dec_len - 1];
            if (padding > 0 && padding <= 16) dec_len -= padding;
        }
        
        if (Update.write(dec_buf, dec_len) != dec_len) {
            success = false;
            break;
        }
        
        offset += read_len;
        
        if (offset % 51200 == 0) {
            otaPrintf("[OTA] Progress: %.0f%%\n", (offset * 100.0) / encrypted_size);
        }
        
        vTaskDelay(1);
    }
    
    free(enc_buf);
    free(dec_buf);
    aes_decrypt_stream_free(&ctx);
    http.end();
    
    if (!success) {
        Update.abort();
        updateInProgress = false;
        return false;
    }
    
    if (!Update.end(true)) {
        updateInProgress = false;
        return false;
    }
    
    otaPrintln("[OTA] ✓ Update installed!");
    updateInProgress = false;
    return true;
}
// BACKGROUND OTA TASK (Core 0)
void otaTask(void* parameter) {
    otaPrintln("[OTA] Background checker started");
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    while (true) {
        if (!updateInProgress) {
            otaPrintln("\n[OTA] Checking for updates...");
            
            if (WiFi.status() != WL_CONNECTED) {
                connectWiFi();
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                int serverVersion = getServerVersion();
                otaPrintf("[OTA] Server: v%d | Local: v%d\n", serverVersion, USER_APP_VERSION);
                
                if (serverVersion > 0 && serverVersion > USER_APP_VERSION) {
                    otaPrintf("[OTA] ★ NEW VERSION AVAILABLE: v%d\n", serverVersion);
                    
                    if (verifySignature()) {
                        otaPrintln("[OTA] ✓ Signature verified");
                        
                        if (downloadAndInstall()) {
                            saveVersion(serverVersion);
                            otaPrintln("[OTA] ★ REBOOTING in 3 seconds...");
                            vTaskDelay(pdMS_TO_TICKS(3000));
                            ESP.restart();
                        }
                    } else {
                        otaPrintln("[OTA] ✗ Signature invalid!");
                    }
                } else if (serverVersion > 0) {
                    otaPrintln("[OTA] Already up to date");
                }
            }
            
            otaPrintf("[OTA] Next check in %d seconds\n", UPDATE_CHECK_INTERVAL_MS / 1000);
        }
        
        vTaskDelay(pdMS_TO_TICKS(UPDATE_CHECK_INTERVAL_MS));
    }
}
// BOOTLOADER SETUP
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n");
    Serial.println("╔═══════════════════════════════════════════════════════════╗");
    Serial.println("║     ESP32 OTA SYSTEM v" + String(BOOTLOADER_VERSION) + " + USER APP v" + String(USER_APP_VERSION) + "                 ║");
    Serial.println("╚═══════════════════════════════════════════════════════════╝");
    Serial.println();
    
    // Initialize OTA system
    crypto_init();
    rsa_verify_init();
    wifiMutex = xSemaphoreCreateMutex();
    loadSettings();
    
    // Save current version
    if (USER_APP_VERSION > stored_version) {
        saveVersion(USER_APP_VERSION);
    }
    
    // Connect WiFi
    connectWiFi();
    
    // Start OTA task on Core 0 (background)
    xTaskCreatePinnedToCore(otaTask, "OTA", 8192, NULL, 1, &otaTaskHandle, 0);
    
    Serial.println("[BOOT] Starting user application...\n");
    
    // Run USER's setup() function (renamed via macro)
    user_setup();
}
// BOOTLOADER LOOP - Calls user's loop() (renamed via macro)
void loop() {
    user_loop();
}
