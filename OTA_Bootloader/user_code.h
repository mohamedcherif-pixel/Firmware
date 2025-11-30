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

#define USER_APP_VERSION 9   // ← INCREMENT THIS WHEN YOU UPDATE YOUR CODE!

// ==========================================================================
// TFT DISPLAY - ILI9488 480x320 using TFT_eSPI
// User_Setup.h must be configured in the library!
// ==========================================================================
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// ==========================================================================
// YOUR SETUP CODE - Runs once at startup
// ==========================================================================
void userSetup() {
    Serial.println("=================================");
    Serial.println("ILI9488 TFT Test v9 - TFT_eSPI");
    Serial.println("=================================");
    
    tft.init();
    tft.setRotation(1);  // Landscape
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    
    // Title
    tft.drawString("ESP32 OTA System", 20, 20, 4);
    tft.drawString("Version " + String(USER_APP_VERSION), 20, 60, 4);
    
    // Draw shapes
    tft.drawRect(10, 100, 200, 80, TFT_RED);
    tft.fillCircle(300, 140, 40, TFT_BLUE);
    tft.drawLine(0, 200, 480, 200, TFT_GREEN);
    
    // Status
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("OTA: Background checking", 20, 220, 2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Edit user_code.h to update!", 20, 250, 2);
    
    Serial.println("TFT Display initialized!");
    Serial.println("=================================");
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
        
        // Clear and update counter area
        tft.fillRect(0, 280, 480, 40, TFT_BLACK);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawString("Uptime: " + String(counter) + " sec", 20, 290, 4);
        
        Serial.printf("[v%d] Running... %d sec\n", USER_APP_VERSION, counter);
    }
    
    delay(10);
}
