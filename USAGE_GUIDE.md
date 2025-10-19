# ESP32 Encrypted Firmware Update - Usage Guide

## Step-by-Step Tutorial

### Step 1: Initial Setup

1. **Install ESP-IDF** (if not already installed):
   ```bash
   # Windows (PowerShell)
   mkdir C:\esp
   cd C:\esp
   git clone --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   .\install.bat
   ```

2. **Install Python dependencies**:
   ```bash
   cd tools
   pip install -r requirements.txt
   ```

### Step 2: First Build and Flash

1. **Open ESP-IDF terminal** and navigate to project:
   ```bash
   cd C:\Users\cheri\OneDrive\Desktop\esp32cryptcode
   ```

2. **Configure project** (optional):
   ```bash
   idf.py menuconfig
   # Navigate through menus to customize settings
   # Press 'S' to save, 'Q' to quit
   ```

3. **Build the project**:
   ```bash
   idf.py build
   ```

4. **Connect ESP32** via USB and identify COM port:
   - Open Device Manager → Ports (COM & LPT)
   - Note the COM port (e.g., COM3, COM4)

5. **Flash firmware**:
   ```bash
   idf.py -p COM3 flash
   ```

6. **Monitor output**:
   ```bash
   idf.py -p COM3 monitor
   # Press Ctrl+] to exit
   ```

### Step 3: Test Crypto Functions

After flashing, you should see output like:

```
=== ESP32 Encrypted Firmware Update System ===
Firmware Version: 1
=== Testing Crypto Functions ===
Original: Hello, ESP32 Encrypted Firmware!
Encryption successful, ciphertext length: 48
Decryption successful: Hello, ESP32 Encrypted Firmware!
✓ Crypto test PASSED
SHA-256 hash computed successfully
✓ Hash verification PASSED
=== Crypto Tests Complete ===
```

### Step 4: Create Encrypted Firmware

1. **Make a change** to the firmware (e.g., update version in `main.c`):
   ```c
   #define CURRENT_FIRMWARE_VERSION 2  // Changed from 1 to 2
   ```

2. **Rebuild**:
   ```bash
   idf.py build
   ```

3. **Encrypt the new firmware**:
   ```bash
   cd tools
   python encrypt_firmware.py encrypt ^
       -i ..\build\esp32_encrypted_firmware.bin ^
       -o ..\build\firmware_encrypted.bin
   ```

   Output:
   ```
   [+] Generated new AES-256 key: aes_key.bin
   [+] Read firmware: 245760 bytes
   [+] Encrypted firmware: 245776 bytes
   [+] Encrypted firmware saved to: ..\build\firmware_encrypted.bin
   [✓] Encryption complete!
   ```

4. **Important files generated**:
   - `aes_key.bin` - Encryption key (keep secure!)
   - `firmware_encrypted.bin` - Encrypted firmware
   - `firmware_encrypted.bin.meta` - Metadata
   - `encryption_key.h` - C header with key (for development)

### Step 5: Setup OTA Update (Local Testing)

1. **Update WiFi credentials** in `main/main.c`:
   ```c
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"
   ```

2. **Find your computer's IP address**:
   ```bash
   # Windows
   ipconfig
   # Look for IPv4 Address (e.g., 192.168.1.100)
   ```

3. **Update OTA URL** in `main/main.c`:
   ```c
   #define OTA_FIRMWARE_URL "http://192.168.1.100:8000/firmware_encrypted.bin"
   ```

4. **Update AES key** in `main/main.c` with values from `encryption_key.h`:
   ```c
   static const uint8_t aes_key[32] = {
       // Copy from encryption_key.h
   };
   
   static const uint8_t aes_iv[16] = {
       // Copy from encryption_key.h
   };
   ```

5. **Uncomment WiFi code** in `main/main.c`:
   ```c
   // Remove the /* and */ around this section:
   ESP_LOGI(TAG, "Connecting to WiFi...");
   ret = wifi_init_sta(WIFI_SSID, WIFI_PASSWORD);
   if (ret == ESP_OK) {
       ESP_LOGI(TAG, "WiFi connected successfully");
       xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
   }
   ```

6. **Rebuild and flash**:
   ```bash
   idf.py build flash monitor
   ```

### Step 6: Perform OTA Update

1. **Start HTTP server** (in a new terminal):
   ```bash
   cd C:\Users\cheri\OneDrive\Desktop\esp32cryptcode\tools
   python simple_http_server.py
   ```

   Output:
   ```
   [+] HTTP Server running on port 8000
   [+] Access firmware at: http://localhost:8000/firmware_encrypted.bin
   ```

2. **Watch ESP32 monitor**:
   - ESP32 connects to WiFi
   - Waits 10 seconds
   - Downloads encrypted firmware
   - Decrypts and flashes
   - Reboots with new firmware

3. **Expected output**:
   ```
   [+] WiFi connected successfully
   [+] Starting OTA update from URL
   [+] Downloading encrypted firmware: 245792 bytes
   [+] Decrypted 1024 bytes
   [+] Written 1024/245792 bytes
   ...
   [+] OTA update successful! Rebooting...
   ```

4. **Verify new version**:
   ```
   === ESP32 Encrypted Firmware Update System ===
   Firmware Version: 2  ← Updated!
   ```

### Step 7: Test Rollback (Optional)

If the new firmware has issues:

1. **Trigger rollback** (add to code or via button):
   ```c
   ota_rollback();
   ```

2. **ESP32 will reboot** to previous working firmware

## Common Workflows

### Workflow 1: Development Testing

```bash
# 1. Make code changes
# Edit main/main.c

# 2. Build and flash directly
idf.py build flash monitor

# 3. Test functionality
```

### Workflow 2: OTA Update Testing

```bash
# 1. Make code changes and increment version
# Edit main/main.c - change CURRENT_FIRMWARE_VERSION

# 2. Build
idf.py build

# 3. Encrypt firmware
cd tools
python encrypt_firmware.py encrypt -i ..\build\esp32_encrypted_firmware.bin -o ..\build\firmware_encrypted.bin -k aes_key.bin

# 4. Start HTTP server
python simple_http_server.py

# 5. ESP32 will auto-update (if configured)
```

### Workflow 3: Production Deployment

```bash
# 1. Build release firmware
idf.py build

# 2. Encrypt with production key
python encrypt_firmware.py encrypt -i firmware.bin -o firmware_encrypted.bin -k production_key.bin

# 3. Upload to production server
# scp firmware_encrypted.bin user@server:/path/to/firmware/

# 4. Trigger OTA on devices
# Via MQTT, HTTP API, or scheduled update
```

## Verification Commands

### Check Firmware Version
```bash
idf.py -p COM3 monitor
# Look for: "Firmware Version: X"
```

### Verify Encryption
```bash
cd tools
python encrypt_firmware.py decrypt -i ..\build\firmware_encrypted.bin -o decrypted.bin -k aes_key.bin
# Compare with original: fc /b original.bin decrypted.bin
```

### Check Partition Info
```bash
idf.py partition-table
```

### Monitor OTA Progress
```bash
idf.py -p COM3 monitor
# Filter for OTA logs: grep "OTA"
```

## Troubleshooting Tips

### Issue: "Port COM3 not found"
**Solution**: 
- Check Device Manager
- Install CP210x or CH340 drivers
- Try different USB cable/port

### Issue: "Flash size mismatch"
**Solution**:
```bash
idf.py menuconfig
# Serial flasher config → Flash size → 4MB
```

### Issue: "Encryption key mismatch"
**Solution**:
- Ensure same key used for encryption and decryption
- Copy key from `encryption_key.h` to `main.c`
- Rebuild and reflash

### Issue: "OTA download fails"
**Solution**:
- Check WiFi connection
- Verify server is running: `http://192.168.1.100:8000/firmware_encrypted.bin`
- Check firewall settings
- Ensure ESP32 and PC on same network

### Issue: "Decryption fails during OTA"
**Solution**:
- Verify AES key and IV match
- Check firmware file integrity
- Ensure encryption was successful

## Next Steps

1. **Implement HTTPS**: Use `esp_https_ota` for secure downloads
2. **Add Firmware Signing**: Implement RSA/ECDSA signatures
3. **Enable Secure Boot**: Prevent unauthorized firmware
4. **Add Version Checking**: Only update if version is newer
5. **Implement Update Server**: Build REST API for firmware management
6. **Add User Interface**: Web interface or mobile app for updates

## Security Checklist

- [ ] Never commit `aes_key.bin` to version control
- [ ] Use unique keys per device in production
- [ ] Enable secure boot and flash encryption
- [ ] Use HTTPS for OTA downloads
- [ ] Implement firmware signing
- [ ] Add anti-rollback protection
- [ ] Secure key storage (eFuse, secure element)
- [ ] Regular security audits
- [ ] Encrypted communication channels
- [ ] Access control for update server

---

**Happy Coding! 🚀**
