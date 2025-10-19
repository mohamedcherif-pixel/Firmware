# Installation Guide - Arduino IDE
## ESP32 Encrypted Firmware Update System

## Prerequisites

- **Arduino IDE 1.8.x or 2.x** (Download: https://www.arduino.cc/en/software)
- **ESP32 Development Board** (any ESP32 with 4MB+ flash)
- **USB Cable** (data cable, not charge-only)
- **Python 3.7+** (for encryption tools)

## Step-by-Step Installation

### 1. Install Arduino IDE

1. Download Arduino IDE from https://www.arduino.cc/en/software
2. Install and launch Arduino IDE
3. Close Arduino IDE (we'll configure it next)

### 2. Install ESP32 Board Support

#### Method 1: Using Board Manager (Recommended)

1. **Open Arduino IDE**

2. **Add ESP32 Board Manager URL**:
   - Go to **File → Preferences** (or Arduino IDE → Settings on Mac)
   - Find "Additional Board Manager URLs"
   - Add this URL:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - If there are already URLs, separate with comma or click the icon to add on new line
   - Click **OK**

3. **Install ESP32 Board Package**:
   - Go to **Tools → Board → Boards Manager**
   - Search for "esp32"
   - Find "ESP32 by Espressif Systems"
   - Click **Install** (this may take several minutes)
   - Wait for installation to complete
   - Click **Close**

#### Method 2: Manual Installation

1. Download ESP32 Arduino core from: https://github.com/espressif/arduino-esp32
2. Extract to Arduino hardware folder:
   - Windows: `C:\Users\<Username>\Documents\Arduino\hardware\espressif\esp32`
   - Mac: `~/Documents/Arduino/hardware/espressif/esp32`
   - Linux: `~/Arduino/hardware/espressif/esp32`
3. Run `get.exe` (Windows) or `get.py` (Mac/Linux) in the esp32 folder
4. Restart Arduino IDE

### 3. Install USB Drivers

Your ESP32 board uses one of these USB-to-Serial chips:

#### CP210x Driver (Most Common)
- Download: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- Install and restart computer

#### CH340 Driver (Alternative)
- Download: http://www.wch.cn/downloads/CH341SER_EXE.html
- Install and restart computer

**Verify Installation**:
- Connect ESP32 via USB
- Open Device Manager (Windows) or System Information (Mac)
- Look for COM port (e.g., COM3, COM4)

### 4. Configure Arduino IDE for ESP32

1. **Select Board**:
   - Tools → Board → ESP32 Arduino → **ESP32 Dev Module**
   - (Or select your specific board model if available)

2. **Configure Settings**:
   - **Upload Speed**: 921600 (or 115200 if upload fails)
   - **CPU Frequency**: 240MHz (WiFi/BT)
   - **Flash Frequency**: 80MHz
   - **Flash Mode**: QIO
   - **Flash Size**: 4MB (or match your board)
   - **Partition Scheme**: **Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)**
   - **Core Debug Level**: None (or Info for debugging)
   - **Port**: Select your COM port

### 5. Install Python Tools

1. **Install Python 3.7+**:
   - Download: https://www.python.org/downloads/
   - During installation, check "Add Python to PATH"
   - Verify: Open CMD/Terminal and type `python --version`

2. **Install Python Dependencies**:
   ```bash
   cd C:\Users\cheri\OneDrive\Desktop\esp32cryptcode\tools
   pip install -r requirements.txt
   ```

   Or manually:
   ```bash
   pip install pycryptodome
   ```

### 6. Open the Project

1. **Navigate to sketch folder**:
   ```
   C:\Users\cheri\OneDrive\Desktop\esp32cryptcode\ESP32_Encrypted_Firmware\
   ```

2. **Open main sketch**:
   - Double-click `ESP32_Encrypted_Firmware.ino`
   - Arduino IDE will open with all files in tabs

3. **Verify all files are loaded**:
   - ESP32_Encrypted_Firmware.ino (main)
   - crypto_utils.h
   - crypto_utils.cpp
   - ota_update.h
   - ota_update.cpp
   - wifi_manager.h
   - wifi_manager.cpp

### 7. First Compilation Test

1. Click **Verify** button (✓) or press Ctrl+R
2. Wait for compilation (may take 1-2 minutes first time)
3. Check for "Done compiling" message
4. If errors occur, see Troubleshooting section below

### 8. First Upload

1. **Connect ESP32** via USB
2. **Select correct COM port**: Tools → Port → COMx
3. Click **Upload** button (→) or press Ctrl+U
4. Wait for upload to complete
5. If upload fails, try:
   - Hold BOOT button on ESP32 while uploading
   - Lower upload speed: Tools → Upload Speed → 115200
   - Try different USB cable/port

### 9. Test Serial Monitor

1. Open Serial Monitor: Tools → Serial Monitor (or Ctrl+Shift+M)
2. Set baud rate to **115200**
3. Set line ending to **Both NL & CR**
4. Press RESET button on ESP32
5. You should see:
   ```
   === ESP32 Encrypted Firmware Update System ===
   Firmware Version: 1
   ✓ Crypto test PASSED
   ✓ Hash verification PASSED
   ```

**✅ Installation Complete!**

## Troubleshooting

### Issue: "Board not found" or "Port not available"

**Solutions**:
1. Install USB drivers (CP210x or CH340)
2. Try different USB port
3. Use different USB cable (must be data cable)
4. Check Device Manager for COM port
5. Restart Arduino IDE
6. Restart computer

### Issue: "Compilation error: mbedtls not found"

**Solutions**:
1. Verify ESP32 board package is installed
2. Reinstall ESP32 board package:
   - Tools → Board → Boards Manager
   - Uninstall ESP32
   - Reinstall ESP32
3. Restart Arduino IDE

### Issue: "Sketch too big"

**Solutions**:
1. Change partition scheme:
   - Tools → Partition Scheme → **Minimal SPIFFS (1.9MB APP with OTA)**
2. Or use **No OTA (2MB APP/2MB SPIFFS)** for initial testing
3. Disable debug output:
   - Tools → Core Debug Level → None

### Issue: "Upload failed" or "Timed out waiting for packet header"

**Solutions**:
1. **Hold BOOT button** on ESP32 during upload
2. Lower upload speed: Tools → Upload Speed → 115200
3. Press RESET button before upload
4. Try different USB port/cable
5. Close Serial Monitor before uploading
6. Disable antivirus temporarily

### Issue: "Python not found" or "pip not recognized"

**Solutions**:
1. Reinstall Python with "Add to PATH" checked
2. Restart CMD/Terminal
3. Use full path: `C:\Python39\python.exe -m pip install pycryptodome`

### Issue: "WiFi connection failed"

**Solutions**:
1. Check SSID and password are correct
2. Use 2.4GHz WiFi (ESP32 doesn't support 5GHz)
3. Move ESP32 closer to router
4. Check WiFi signal strength in Serial Monitor

### Issue: "OTA update failed"

**Solutions**:
1. Verify partition scheme supports OTA
2. Check encryption key matches
3. Ensure firmware file exists on server
4. Verify network connectivity
5. Check firewall settings

## Verification Checklist

- [ ] Arduino IDE installed
- [ ] ESP32 board package installed
- [ ] USB drivers installed
- [ ] Board appears in Tools → Port
- [ ] Sketch compiles without errors
- [ ] Upload successful
- [ ] Serial Monitor shows output
- [ ] Crypto tests pass
- [ ] Python installed
- [ ] pycryptodome installed

## Directory Structure

```
esp32cryptcode/
├── ESP32_Encrypted_Firmware/        ← Arduino sketch folder
│   ├── ESP32_Encrypted_Firmware.ino ← Main sketch (open this)
│   ├── crypto_utils.h/cpp
│   ├── ota_update.h/cpp
│   ├── wifi_manager.h/cpp
│   ├── partitions.csv
│   └── README.md
│
├── tools/                           ← Python tools
│   ├── encrypt_firmware.py
│   ├── simple_http_server.py
│   └── requirements.txt
│
├── ARDUINO_QUICK_START.md          ← Quick reference
└── INSTALLATION.md                  ← This file
```

## Next Steps

After successful installation:

1. ✅ **Test crypto functions** (automatic on boot)
2. 📝 **Read ARDUINO_QUICK_START.md** for usage guide
3. 🔐 **Create encrypted firmware** using Python tools
4. 📡 **Test OTA update** locally
5. 🚀 **Deploy to production**

## Additional Resources

- **ESP32 Arduino Core**: https://github.com/espressif/arduino-esp32
- **Arduino ESP32 Docs**: https://docs.espressif.com/projects/arduino-esp32/
- **ESP32 Forum**: https://www.esp32.com/
- **Arduino Forum**: https://forum.arduino.cc/

## Support

If you encounter issues:

1. Check Serial Monitor output for error messages
2. Review troubleshooting section above
3. Verify all installation steps completed
4. Check ESP32 board connections
5. Try with a simple blink sketch first

---

**Installation Guide Version**: 1.0  
**Last Updated**: 2025-10-05  
**Tested On**: Arduino IDE 2.2.1, ESP32 Dev Module, Windows 10/11
