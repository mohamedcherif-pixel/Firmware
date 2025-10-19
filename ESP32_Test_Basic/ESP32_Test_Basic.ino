/*
 * ESP32 Basic Test Sketch
 * Use this to verify your ESP32 is working before trying OTA
 */

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("ESP32 Basic Test - SUCCESS!");
  Serial.println("========================================");
  Serial.println("If you see this message, your ESP32 is working correctly!");
  Serial.println("Partition table is OK!");
  Serial.println("========================================\n");
}

void loop() {
  static unsigned long lastPrint = 0;
  
  if (millis() - lastPrint >= 2000) {
    Serial.print("Running... Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    lastPrint = millis();
  }
}
