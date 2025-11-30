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

#define USER_APP_VERSION 7   // ← INCREMENT THIS WHEN YOU UPDATE YOUR CODE!

// ==========================================================================
// TFT DISPLAY CONFIGURATION (ILI9488 480x320)
// For 3.5" TFT with ILI9488 controller
// ==========================================================================
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Hardware SPI pins for ESP32
#define TFT_CS    15
#define TFT_RST   4
#define TFT_DC    2

// Create display object using hardware SPI
// Note: ILI9488 is similar to ILI9341 but 480x320
// The Adafruit_ILI9341 library works but is limited to 320x240
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ==========================================================================
// YOUR SETUP CODE - Runs once at startup
// ==========================================================================
void userSetup() {
    Serial.println("Initializing TFT display...");
    
    // Initialize SPI with correct pins for ESP32
    SPI.begin(18, 19, 23, 15);  // SCK, MISO, MOSI, SS
    
    // Initialize display
    tft.begin(40000000);  // 40MHz SPI speed
    delay(100);
    
    tft.setRotation(1);  // Landscape mode
    
    // Fill screen with black first
    tft.fillScreen(ILI9341_BLACK);
    delay(100);
    
    // Draw a simple test pattern
    Serial.println("Drawing test pattern...");
    
    // Red rectangle at top
    tft.fillRect(0, 0, 320, 60, ILI9341_RED);
    
    // Green rectangle in middle  
    tft.fillRect(0, 80, 320, 60, ILI9341_GREEN);
    
    // Blue rectangle at bottom
    tft.fillRect(0, 160, 320, 60, ILI9341_BLUE);
    
    // White text
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.setCursor(50, 20);
    tft.print("ESP32 OTA v");
    tft.print(USER_APP_VERSION);
    
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(80, 100);
    tft.print("WORKING!");
    
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(60, 180);
    tft.print("TFT TEST OK");
    
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
        
        // Update counter on screen
        tft.fillRect(0, 220, 320, 20, ILI9341_BLACK);
        tft.setTextColor(ILI9341_YELLOW);
        tft.setTextSize(2);
        tft.setCursor(20, 220);
        tft.print("Uptime: ");
        tft.print(counter);
        tft.print(" sec");
        
        // Serial output
        Serial.printf("[v%d] Running... %d sec\n", USER_APP_VERSION, counter);
    }
    
    delay(10);
}
