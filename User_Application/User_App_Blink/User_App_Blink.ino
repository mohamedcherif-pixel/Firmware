/*
 * User Application: LED Blink
 * ===========================
 * This is a PURE user application - NO OTA CODE NEEDED!
 * The OTA Bootloader handles all firmware updates.
 * 
 * Just write your application logic and upload via GitHub Releases.
 */

// User App Version - increment this for each release
#define USER_APP_VERSION 1

// LED pin (built-in LED on most ESP32 boards)
#define LED_PIN 2

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("\n");
    Serial.println("╔══════════════════════════════════════════════════════════╗");
    Serial.println("║          USER APPLICATION: LED BLINK                     ║");
    Serial.println("║          Version: " + String(USER_APP_VERSION) + "                                       ║");
    Serial.println("╚══════════════════════════════════════════════════════════╝");
    Serial.println();
    Serial.println("This is a PURE user application!");
    Serial.println("No OTA code here - the bootloader handles updates.");
    Serial.println();
    
    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("LED Blink application started!");
    Serial.printf("Using GPIO %d for LED\n", LED_PIN);
}

void loop() {
    // Blink LED
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");
    delay(500);
    
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
    delay(500);
}
