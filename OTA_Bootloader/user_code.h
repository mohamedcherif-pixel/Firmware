/*
 * ╔════════════════════════════════════════════════════════════════════╗
 * ║                      USER APPLICATION CODE                         ║
 * ╠════════════════════════════════════════════════════════════════════╣
 * ║  This is YOUR code! Edit userSetup() and userLoop() below.         ║
 * ║                                                                    ║
 * ║  TO UPDATE:                                                        ║
 * ║  1. Change USER_APP_VERSION to a higher number                     ║
 * ║  2. Edit your code in userSetup() and userLoop()                   ║
 * ║  3. Push to GitHub                                                 ║
 * ║  4. ESP32 will automatically download and run your new code!       ║
 * ║                                                                    ║
 * ║  You do NOT need to add any OTA, WiFi, or encryption code.         ║
 * ║  Just write normal Arduino code!                                   ║
 * ╚════════════════════════════════════════════════════════════════════╝
 */

#define USER_APP_VERSION 11   // ← INCREMENT THIS WHEN YOU UPDATE YOUR CODE!

// ==========================================================================
// TFT DISPLAY - ILI9488 480x320 using TFT_eSPI
// Split screen: LEFT = User Code | RIGHT = OTA Status
// ==========================================================================
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// Screen layout (480x320 landscape)
#define SCREEN_W 480
#define SCREEN_H 320
#define DIVIDER_X 240  // Split in middle

// Colors
#define COLOR_BG       TFT_BLACK
#define COLOR_USER_BG  0x0010  // Dark blue
#define COLOR_OTA_BG   0x1000  // Dark green
#define COLOR_DIVIDER  TFT_WHITE
#define COLOR_TITLE    TFT_WHITE
#define COLOR_TEXT     TFT_CYAN
#define COLOR_VALUE    TFT_YELLOW
#define COLOR_OK       TFT_GREEN
#define COLOR_WAIT     TFT_ORANGE

// Variables for OTA status display
String otaStatus = "Initializing...";
String otaVersion = "Checking...";
unsigned long lastOtaCheck = 0;

// Draw the static UI layout
void drawLayout() {
    // Clear screen
    tft.fillScreen(COLOR_BG);
    
    // LEFT SIDE - User Application (dark blue background)
    tft.fillRect(0, 0, DIVIDER_X - 2, SCREEN_H, COLOR_USER_BG);
    
    // RIGHT SIDE - OTA Status (dark green background)
    tft.fillRect(DIVIDER_X + 2, 0, SCREEN_W - DIVIDER_X - 2, SCREEN_H, COLOR_OTA_BG);
    
    // Divider line
    tft.drawFastVLine(DIVIDER_X - 1, 0, SCREEN_H, COLOR_DIVIDER);
    tft.drawFastVLine(DIVIDER_X, 0, SCREEN_H, COLOR_DIVIDER);
    tft.drawFastVLine(DIVIDER_X + 1, 0, SCREEN_H, COLOR_DIVIDER);
    
    // LEFT HEADER - User Code
    tft.fillRect(0, 0, DIVIDER_X - 2, 35, TFT_BLUE);
    tft.setTextColor(COLOR_TITLE, TFT_BLUE);
    tft.drawString("USER CODE", 50, 8, 4);
    
    // RIGHT HEADER - OTA Status
    tft.fillRect(DIVIDER_X + 2, 0, SCREEN_W - DIVIDER_X - 2, 35, TFT_DARKGREEN);
    tft.setTextColor(COLOR_TITLE, TFT_DARKGREEN);
    tft.drawString("OTA STATUS", 290, 8, 4);
    
    // LEFT - Version label
    tft.setTextColor(COLOR_TEXT, COLOR_USER_BG);
    tft.drawString("Version:", 15, 50, 2);
    tft.setTextColor(COLOR_VALUE, COLOR_USER_BG);
    tft.drawString(String(USER_APP_VERSION), 100, 50, 2);
    
    // LEFT - Status label
    tft.setTextColor(COLOR_TEXT, COLOR_USER_BG);
    tft.drawString("Status:", 15, 80, 2);
    
    // LEFT - Uptime label
    tft.setTextColor(COLOR_TEXT, COLOR_USER_BG);
    tft.drawString("Uptime:", 15, 140, 2);
    
    // RIGHT - WiFi label
    tft.setTextColor(COLOR_TEXT, COLOR_OTA_BG);
    tft.drawString("WiFi:", 255, 50, 2);
    
    // RIGHT - Server Ver label
    tft.setTextColor(COLOR_TEXT, COLOR_OTA_BG);
    tft.drawString("Server:", 255, 80, 2);
    
    // RIGHT - Status label
    tft.setTextColor(COLOR_TEXT, COLOR_OTA_BG);
    tft.drawString("Status:", 255, 110, 2);
    
    // RIGHT - Last Check label
    tft.setTextColor(COLOR_TEXT, COLOR_OTA_BG);
    tft.drawString("Checked:", 255, 140, 2);
    
    // Footer
    tft.setTextColor(TFT_DARKGREY, COLOR_BG);
    tft.drawString("ESP32 OTA System - github.com/mohamedcherif-pixel", 60, 300, 2);
}

// Update User Code section
void updateUserSection(int uptime) {
    // Clear value areas
    tft.fillRect(15, 100, 200, 30, COLOR_USER_BG);
    tft.fillRect(15, 160, 200, 30, COLOR_USER_BG);
    
    // Status
    tft.setTextColor(COLOR_OK, COLOR_USER_BG);
    tft.drawString("RUNNING", 100, 80, 2);
    
    // Uptime
    tft.setTextColor(COLOR_VALUE, COLOR_USER_BG);
    int mins = uptime / 60;
    int secs = uptime % 60;
    String uptimeStr = String(mins) + "m " + String(secs) + "s";
    tft.drawString(uptimeStr, 100, 140, 2);
}

// Update OTA section (call this from OTA task if needed)
void updateOtaSection(bool wifiConnected, String serverVer, String status) {
    // Clear value areas
    tft.fillRect(320, 50, 150, 20, COLOR_OTA_BG);
    tft.fillRect(320, 80, 150, 20, COLOR_OTA_BG);
    tft.fillRect(255, 130, 200, 20, COLOR_OTA_BG);
    tft.fillRect(320, 140, 150, 20, COLOR_OTA_BG);
    
    // WiFi status
    if (wifiConnected) {
        tft.setTextColor(COLOR_OK, COLOR_OTA_BG);
        tft.drawString("Connected", 320, 50, 2);
    } else {
        tft.setTextColor(TFT_RED, COLOR_OTA_BG);
        tft.drawString("Disconnected", 320, 50, 2);
    }
    
    // Server version
    tft.setTextColor(COLOR_VALUE, COLOR_OTA_BG);
    tft.drawString("v" + serverVer, 320, 80, 2);
    
    // Status
    tft.setTextColor(COLOR_OK, COLOR_OTA_BG);
    tft.drawString(status, 320, 110, 2);
    
    // Last check time
    tft.setTextColor(COLOR_TEXT, COLOR_OTA_BG);
    tft.drawString("Just now", 320, 140, 2);
}

// ==========================================================================
// YOUR SETUP CODE - Runs once at startup
// ==========================================================================
void userSetup() {
    Serial.println("=================================");
    Serial.println("ESP32 OTA Display v10");
    Serial.println("Split Screen: User | OTA");
    Serial.println("=================================");
    
    tft.init();
    tft.setRotation(1);  // Landscape 480x320
    
    drawLayout();
    
    // Initial OTA section (will be updated by OTA task)
    updateOtaSection(false, "?", "Starting...");
    
    Serial.println("TFT Display initialized!");
}

// ==========================================================================
// YOUR LOOP CODE - Runs continuously
// ==========================================================================
void userLoop() {
    static unsigned long lastUpdate = 0;
    static int counter = 0;
    
    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();
        counter++;
        
        // Update user section
        updateUserSection(counter);
        
        // Simulate OTA status update (in real system, OTA task updates this)
        bool wifiOk = (WiFi.status() == WL_CONNECTED);
        updateOtaSection(wifiOk, String(USER_APP_VERSION), wifiOk ? "Up to date" : "Waiting...");
        
        Serial.printf("[v%d] Uptime: %d sec\n", USER_APP_VERSION, counter);
    }
    
    delay(10);
}
