/*
 * User Application: Temperature Sensor
 * =====================================
 * This is a PURE user application - NO OTA CODE NEEDED!
 * The OTA Bootloader handles all firmware updates.
 * 
 * Simulates a temperature sensor (no actual hardware required)
 * In real use, connect a DHT11/DHT22 or DS18B20 sensor.
 */

// User App Version - increment this for each release
#define USER_APP_VERSION 3
void setup() {
    Serial.begin(115200);
    Serial.println("hello");
    delay(2000);
}
void loop() {}