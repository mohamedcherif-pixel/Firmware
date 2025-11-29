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
#define USER_APP_VERSION 1

// Simulated temperature range (for demo without hardware)
float baseTemperature = 22.5;  // Starting temperature in Celsius

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("\n");
    Serial.println("╔══════════════════════════════════════════════════════════╗");
    Serial.println("║          USER APPLICATION: TEMPERATURE SENSOR            ║");
    Serial.println("║          Version: " + String(USER_APP_VERSION) + "                                       ║");
    Serial.println("╚══════════════════════════════════════════════════════════╝");
    Serial.println();
    Serial.println("This is a PURE user application!");
    Serial.println("No OTA code here - the bootloader handles updates.");
    Serial.println();
    
    // In real application, initialize temperature sensor here
    // Example: dht.begin(); or sensors.begin();
    
    Serial.println("Temperature monitoring started!");
    Serial.println("(Simulated sensor - no hardware required for demo)");
    Serial.println();
}

float readTemperature() {
    // Simulated temperature reading with random variation
    // Replace with actual sensor reading in production:
    // return dht.readTemperature();
    // return sensors.getTempCByIndex(0);
    
    float variation = (random(-100, 100) / 100.0);  // -1.0 to +1.0
    return baseTemperature + variation;
}

float readHumidity() {
    // Simulated humidity reading
    // Replace with: return dht.readHumidity();
    
    return 45.0 + (random(-100, 100) / 10.0);  // 35% to 55%
}

void loop() {
    float temperature = readTemperature();
    float humidity = readHumidity();
    
    Serial.println("═══════════════════════════════════════");
    Serial.printf("  🌡️  Temperature: %.1f °C\n", temperature);
    Serial.printf("  💧 Humidity:    %.1f %%\n", humidity);
    Serial.println("═══════════════════════════════════════");
    
    // Alert if temperature is too high or too low
    if (temperature > 30.0) {
        Serial.println("  ⚠️  WARNING: High temperature!");
    } else if (temperature < 15.0) {
        Serial.println("  ⚠️  WARNING: Low temperature!");
    }
    
    Serial.println();
    
    // Read every 2 seconds
    delay(2000);
}
