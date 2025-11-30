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

#define USER_APP_VERSION 5   // ← INCREMENT THIS WHEN YOU UPDATE YOUR CODE!

// ==========================================================================
// TFT DISPLAY CONFIGURATION (ILI9488 480x320)
// ==========================================================================
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>  // Works for ILI9488 too

// Display pins
#define TFT_CS    15
#define TFT_RST   4
#define TFT_DC    2
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_MISO  19
#define TFT_LED   32  // Backlight (optional, can connect to 3.3V)

// Touch pins (optional)
#define TOUCH_CS  21
#define TOUCH_IRQ 27

// Create display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ==========================================================================
// YOUR SETUP CODE - Runs once at startup
// ==========================================================================
void userSetup() {
    // Initialize backlight
    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH);  // Turn on backlight
    
    // Initialize display
    tft.begin();
    tft.setRotation(1);  // Landscape mode
    tft.fillScreen(ILI9341_BLACK);
    
    // Draw header
    tft.fillRect(0, 0, 320, 50, ILI9341_BLUE);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 15);
    tft.println("ESP32 OTA System v" + String(USER_APP_VERSION));
    
    // Draw info box
    tft.drawRect(10, 60, 300, 80, ILI9341_WHITE);
    tft.setCursor(20, 75);
    tft.setTextColor(ILI9341_GREEN);
    tft.println("Status: RUNNING");
    tft.setCursor(20, 100);
    tft.setTextColor(ILI9341_YELLOW);
    tft.println("OTA: Background check");
    
    // Draw footer
    tft.setTextColor(ILI9341_CYAN);
    tft.setCursor(20, 160);
    tft.println("Edit user_code.h and push");
    tft.setCursor(20, 185);
    tft.println("to GitHub to update!");
    
    Serial.println("TFT Display initialized!");
}

// ==========================================================================
// YOUR LOOP CODE - Runs continuously
// ==========================================================================
void userLoop() {
    static unsigned long lastUpdate = 0;
    static int counter = 0;
    
    // Update every second
    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();
        counter++;
        
        // Clear counter area
        tft.fillRect(10, 200, 300, 30, ILI9341_BLACK);
        
        // Draw counter
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);
        tft.setCursor(20, 205);
        tft.print("Uptime: ");
        tft.print(counter);
        tft.println(" sec");
        
        // Serial output
        Serial.printf("[USER APP v%d] Running... %d seconds\n", USER_APP_VERSION, counter);
    }
    
    delay(10);
}
