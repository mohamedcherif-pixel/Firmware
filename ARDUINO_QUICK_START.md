# Arduino IDE - Quick Start Guide
## ESP32 Encrypted Firmware Update

## 🚀 5-Minute Setup

### 1. Arduino IDE Setup (2 minutes)

1. **Install Arduino IDE 2.x**
   - Download: https://www.arduino.cc/en/software
   - Install and open

2. **Add ESP32 Board Support**
   - File → Preferences
   - Additional Board Manager URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager
   - Search "esp32" → Install "ESP32 by Espressif Systems"

3. **Configure Board**
   - Tools → Board → ESP32 Arduino → ESP32 Dev Module
   - Tools → Partition Scheme → Minimal SPIFFS (1.9MB APP with OTA)
   - Tools → Upload Speed → 921600
   - Tools → Port → Select your COM port

### 2. First Upload (1 minute)

1. Open `ESP32_Encrypted_Firmware/ESP32_Encrypted_Firmware.ino`
2. Click **Upload** button (→)
3. Open Serial Monitor (Ctrl+Shift+M)
4. Set baud rate to **115200**

### 3. Verify It Works (1 minute)

You should see:
```
=== ESP32 Encrypted Firmware Update System ===
Firmware Version: 1

=== Testing Crypto Functions ===
✓ Crypto test PASSED
✓ Hash verification PASSED
```

**✅ Success!** Your ESP32 is now running encrypted firmware!

### 4. Test OTA Update (Optional - 5 minutes)

#### A. Prepare New Firmware

1. **Change version** in sketch:
   ```cpp
   #define FIRMWARE_VERSION 2  // Changed from 1
   ```

2. **Export binary**:
   - Sketch → Export compiled Binary (Ctrl+Alt+S)
   - Binary saved in sketch folder

3. **Encrypt firmware**:
   ```bash
   cd tools
   pip install pycryptodome
   python encrypt_firmware.py encrypt -i ../ESP32_Encrypted_Firmware/ESP32_Encrypted_Firmware.ino.esp32.bin -o firmware_encrypted.bin
   ```

#### B. Update Encryption Keys

1. Open generated `encryption_key.h`
2. Copy AES_KEY and AES_IV arrays
3. Paste into your `.ino` file (replace existing keys)

#### C. Enable WiFi & OTA

1. **Update WiFi credentials**:
   ```cpp
   const char* WIFI_SSID = "YourWiFiName";
   const char* WIFI_PASSWORD = "YourPassword";
   ```

2. **Find your PC's IP**:
   - Windows: Run `ipconfig` in CMD
   - Look for IPv4 Address (e.g., 192.168.1.100)

3. **Update OTA URL**:
   ```cpp
   const char* OTA_FIRMWARE_URL = "http://192.168.1.100:8000/firmware_encrypted.bin";
   ```

4. **Uncomment WiFi code** in `setup()`:
   - Remove `/*` and `*/` around WiFi connection block

#### D. Start Server & Upload

1. **Start HTTP server**:
   ```bash
   cd tools
   python simple_http_server.py
   ```

2. **Upload sketch** to ESP32

3. **Watch Serial Monitor**:
   ```
   [WiFi] Connected successfully!
   Starting OTA update...
   [OTA] OTA update successful! Rebooting...
   
   === ESP32 Encrypted Firmware Update System ===
   Firmware Version: 2  ← Updated!
   ```

**🎉 OTA Update Complete!**

## Common Issues & Fixes

### ❌ "Port not found"
- Install USB drivers: CP210x or CH340
- Check Device Manager → Ports

### ❌ "Sketch too big"
- Tools → Partition Scheme → Minimal SPIFFS (1.9MB APP with OTA)

### ❌ "Upload failed"
- Hold BOOT button while uploading
- Lower upload speed: Tools → Upload Speed → 115200

### ❌ "WiFi connection failed"
- Check SSID/password
- Use 2.4GHz WiFi (not 5GHz)
- ESP32 and PC must be on same network

### ❌ "OTA download failed"
- Check firewall settings
- Verify server is running: http://192.168.1.100:8000/firmware_encrypted.bin
- Ensure PC and ESP32 on same network

## File Locations

### Arduino Sketch
```
ESP32_Encrypted_Firmware/
├── ESP32_Encrypted_Firmware.ino  ← Main file
├── crypto_utils.cpp/h
├── ota_update.cpp/h
└── wifi_manager.cpp/h
```

### Tools
```
tools/
├── encrypt_firmware.py       ← Encrypt firmware
├── simple_http_server.py     ← Local OTA server
└── requirements.txt
```

### Generated Files
```
ESP32_Encrypted_Firmware/
├── ESP32_Encrypted_Firmware.ino.esp32.bin  ← Compiled binary
└── encryption_key.h                        ← Generated keys

tools/
├── firmware_encrypted.bin    ← Encrypted firmware
├── aes_key.bin              ← Encryption key (KEEP SECRET!)
└── firmware_encrypted.bin.meta
```

## Next Steps

1. ✅ Test crypto functions (done automatically)
2. ✅ Test OTA update locally
3. 🔒 Enable secure boot (production)
4. 🔒 Enable flash encryption (production)
5. 🌐 Deploy to production server
6. 📱 Add mobile app control

## Cheat Sheet

### Arduino IDE Shortcuts
- **Upload**: Ctrl+U
- **Verify**: Ctrl+R
- **Serial Monitor**: Ctrl+Shift+M
- **Export Binary**: Ctrl+Alt+S

### Python Commands
```bash
# Encrypt firmware
python encrypt_firmware.py encrypt -i firmware.bin -o encrypted.bin

# Decrypt (verify)
python encrypt_firmware.py decrypt -i encrypted.bin -o decrypted.bin -k aes_key.bin

# Start server
python simple_http_server.py
```

### Serial Monitor Commands
- **Baud Rate**: 115200
- **Line Ending**: Both NL & CR
- **Clear Output**: Ctrl+L

## Security Checklist

- [ ] Change default encryption keys
- [ ] Use unique keys per device (production)
- [ ] Never commit `aes_key.bin` to Git
- [ ] Use HTTPS for OTA (production)
- [ ] Enable secure boot (production)
- [ ] Enable flash encryption (production)
- [ ] Implement firmware signing (production)

## Support

**Need Help?**
1. Check `README.md` for detailed docs
2. Review Serial Monitor output
3. Check Arduino ESP32 forum
4. Verify all connections and settings

---

**Happy Coding! 🚀**

*This is a simplified guide. See README.md for complete documentation.*
