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

#define USER_APP_VERSION 2   // ← INCREMENT THIS WHEN YOU UPDATE YOUR CODE!

// ==========================================================================
// YOUR SETUP CODE - Runs once at startup
// ==========================================================================
void userSetup() {
    // Example: Initialize LED
    pinMode(2, OUTPUT);
    
    Serial.println("Hello from user code v2!");
}

// ==========================================================================
// YOUR LOOP CODE - Runs continuously
// ==========================================================================
void userLoop() {
    // Example: Blink LED
    digitalWrite(2, HIGH);
    delay(500);
    digitalWrite(2, LOW);
    delay(500);
    
    Serial.println("hello...");
}
