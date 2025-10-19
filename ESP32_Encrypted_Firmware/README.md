# ESP32 Encrypted Firmware Update - Arduino IDE
## Project 03: Mise à jour d'un objet avec un firmware crypté

Complete implementation of secure encrypted firmware updates for ESP32 using Arduino IDE.

## Features

✅ **AES-256-CBC Encryption** - Military-grade firmware encryption  
✅ **Secure OTA Updates** - Over-the-air encrypted firmware delivery  
✅ **SHA-256 Verification** - Firmware integrity checking  
✅ **Dual Partition Support** - Safe rollback capability  
✅ **WiFi Integration** - Network-based updates  
✅ **Hardware Acceleration** - ESP32 crypto engine  

## Arduino IDE Setup

### 1. Install Arduino IDE

Download from: https://www.arduino.cc/en/software

### 2. Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "esp32" and install **ESP32 by Espressif Systems**

### 3. Configure Board Settings

1. **Tools → Board** → Select your ESP32 board (e.g., "ESP32 Dev Module")
2. **Tools → Partition Scheme** → Select **"Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)"**
   - Or use custom partition table (see below)
3. **Tools → Upload Speed** → 921600
4. **Tools → Port** → Select your COM port

### 4. Install Required Libraries

The sketch uses built-in ESP32 libraries:
- WiFi (built-in)
- HTTPClient (built-in)
- Update (built-in)
- mbedTLS (built-in)

No additional library installation needed! ✅

## Quick Start Guide

### Step 1: Upload Initial Firmware

1. Open `ESP32_Encrypted_Firmware.ino` in Arduino IDE
2. Update WiFi credentials (optional for first test):
   ```cpp
   const char* WIFI_SSID = "YOUR_WIFI_SSID";
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
   ```
3. Click **Upload** (Ctrl+U)
4. Open **Serial Monitor** (Ctrl+Shift+M) at 115200 baud
5. You should see crypto tests running successfully

### Step 2: Test Crypto Functions

After upload, the Serial Monitor will show:

```
=== ESP32 Encrypted Firmware Update System ===
Firmware Version: 1
================================================

=== Testing Crypto Functions ===
Original: Hello, ESP32 Encrypted Firmware!
[CRYPTO] Encrypted 33 bytes to 48 bytes
[CRYPTO] Decrypted 48 bytes to 33 bytes
Decryption successful: Hello, ESP32 Encrypted Firmware!
✓ Crypto test PASSED
SHA-256 hash computed successfully
✓ Hash verification PASSED
=== Crypto Tests Complete ===
```

### Step 3: Create Encrypted Firmware

1. **Build the firmware binary**:
   - In Arduino IDE: **Sketch → Export compiled Binary** (Ctrl+Alt+S)
   - Binary saved to sketch folder as: `ESP32_Encrypted_Firmware.ino.esp32.bin`

2. **Install Python tool dependencies**:
   ```bash
   cd tools
   pip install -r requirements.txt
   ```

3. **Encrypt the firmware**:
   ```bash
   python encrypt_firmware.py encrypt ^
       -i ..\ESP32_Encrypted_Firmware\ESP32_Encrypted_Firmware.ino.esp32.bin ^
       -o firmware_encrypted.bin
   ```

4. **Important files generated**:
   - `firmware_encrypted.bin` - Encrypted firmware
   - `aes_key.bin` - Encryption key (KEEP SECURE!)
   - `encryption_key.h` - C header with key values

### Step 4: Update Encryption Keys

1. Open `encryption_key.h` (generated in step 3)
2. Copy the AES_KEY and AES_IV arrays
3. Paste into `ESP32_Encrypted_Firmware.ino`:
   ```cpp
   const uint8_t aes_key[32] = {
       // Paste values from encryption_key.h
   };
   
   const uint8_t aes_iv[16] = {
       // Paste values from encryption_key.h
   };
   ```

### Step 5: Perform OTA Update

#### Option A: Local Testing (Recommended)

1. **Start HTTP server**:
   ```bash
   cd tools
   python simple_http_server.py
   ```
   Server runs on: `http://localhost:8000`

2. **Find your PC's IP address**:
   - Windows: `ipconfig` (look for IPv4 Address)
   - Example: `192.168.1.100`

3. **Update OTA URL** in sketch:
   ```cpp
   const char* OTA_FIRMWARE_URL = "http://192.168.1.100:8000/firmware_encrypted.bin";
   ```

4. **Enable WiFi** in sketch (uncomment the WiFi section in `setup()`):
   ```cpp
   // Remove /* and */ around this block:
   Serial.println("Connecting to WiFi...");
   if (wifi_connect(WIFI_SSID, WIFI_PASSWORD)) {
       // ... OTA update code ...
   }
   ```

5. **Upload and monitor**:
   - Upload the sketch
   - Watch Serial Monitor
   - ESP32 will connect to WiFi, download encrypted firmware, decrypt, and update!

#### Option B: Production Server

1. Upload `firmware_encrypted.bin` to your web server
2. Update URL in sketch
3. ESP32 will download and update from your server

## File Structure

```
ESP32_Encrypted_Firmware/
├── ESP32_Encrypted_Firmware.ino  # Main sketch
├── crypto_utils.h/cpp            # AES-256 & SHA-256
├── ota_update.h/cpp              # OTA with decryption
├── wifi_manager.h/cpp            # WiFi connection
├── partitions.csv                # Custom partition table (optional)
└── README.md                     # This file

tools/
├── encrypt_firmware.py           # Firmware encryption tool
├── simple_http_server.py         # Local OTA server
└── requirements.txt              # Python dependencies
```

## Custom Partition Table (Optional)

For better OTA support, use a custom partition table:

1. Copy `partitions.csv` to your sketch folder
2. In Arduino IDE:
   - **Tools → Partition Scheme → Custom**
   - Or manually edit `boards.txt` to add custom partition

## Serial Monitor Output Examples

### Successful OTA Update:
```
[WiFi] Connecting to: MyWiFi
[WiFi] Connected successfully!
[WiFi] IP Address: 192.168.1.50

Waiting 10 seconds before starting OTA update...
Starting OTA update...
[OTA] Starting encrypted OTA update from URL: http://192.168.1.100:8000/firmware_encrypted.bin
[OTA] Downloading encrypted firmware: 245792 bytes
[OTA] Downloaded 245792/245792 bytes
[CRYPTO] Decrypted 1024 bytes to 1008 bytes
[OTA] Written 1024/245776 bytes
...
[OTA] OTA update successful!
✓ OTA update successful! Rebooting...
```

### After Reboot:
```
=== ESP32 Encrypted Firmware Update System ===
Firmware Version: 2  ← Updated!
```

## Troubleshooting

### Compilation Errors

**Error: "mbedtls/aes.h: No such file or directory"**
- Solution: Ensure ESP32 board package is installed correctly
- Reinstall: Tools → Board → Boards Manager → ESP32

**Error: "Sketch too big"**
- Solution: Change partition scheme
- Tools → Partition Scheme → "Minimal SPIFFS" or larger

### Upload Errors

**Error: "Failed to connect to ESP32"**
- Hold BOOT button while uploading
- Check USB cable and drivers (CP210x/CH340)
- Try lower upload speed: Tools → Upload Speed → 115200

### Runtime Errors

**WiFi connection failed**
- Check SSID/password
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check WiFi signal strength

**OTA update failed**
- Verify encryption key matches
- Check firmware file exists on server
- Ensure enough free space (check partition table)
- Verify network connectivity

**Decryption failed**
- Ensure same key used for encryption and decryption
- Check IV is correct
- Verify firmware file is not corrupted

## Security Best Practices

⚠️ **IMPORTANT**: Example uses hardcoded keys for demonstration only!

### Production Recommendations:

1. **Secure Key Storage**:
   - Use ESP32 eFuse for keys
   - Implement secure element (ATECC608A)
   - Never hardcode keys in production

2. **Use HTTPS**:
   - Replace HTTP with HTTPS for OTA
   - Implement certificate pinning

3. **Firmware Signing**:
   - Add RSA/ECDSA signatures
   - Verify signature before flashing

4. **Enable Security Features**:
   - Secure Boot
   - Flash Encryption
   - Anti-rollback protection

5. **Key Management**:
   - Unique keys per device
   - Regular key rotation
   - Secure key distribution

## Python Tools

### Encrypt Firmware
```bash
python encrypt_firmware.py encrypt -i firmware.bin -o encrypted.bin
```

### Decrypt Firmware (for verification)
```bash
python encrypt_firmware.py decrypt -i encrypted.bin -o decrypted.bin -k aes_key.bin
```

### Start Local Server
```bash
python simple_http_server.py
# Server runs on http://localhost:8000
```

## Version Management

Update firmware version in sketch:
```cpp
#define FIRMWARE_VERSION 2  // Increment with each release
```

## Advanced Features

### Manual OTA Trigger

Add a button to trigger OTA:
```cpp
const int BUTTON_PIN = 0;

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("Button pressed - starting OTA...");
        ota_update_from_url(OTA_FIRMWARE_URL, aes_key);
    }
}
```

### Scheduled Updates

Check for updates periodically:
```cpp
void loop() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck >= 3600000) { // Every hour
        checkForUpdate();
        lastCheck = millis();
    }
}
```

### Rollback on Error

Automatically rollback if new firmware fails:
```cpp
void setup() {
    // ... initialization ...
    
    if (bootCount > 3) {  // Failed to boot 3 times
        ota_rollback();
    }
}
```

## Support & Resources

- **ESP32 Arduino Core**: https://github.com/espressif/arduino-esp32
- **ESP32 Documentation**: https://docs.espressif.com/projects/arduino-esp32/
- **Arduino Forum**: https://forum.arduino.cc/

## License

Educational use - provided as-is.

---

**Version**: 1.0 (Arduino IDE)  
**Last Updated**: 2025-10-05  
**Tested on**: Arduino IDE 2.x, ESP32 Dev Module
