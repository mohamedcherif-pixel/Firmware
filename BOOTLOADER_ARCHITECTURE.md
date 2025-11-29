# ESP32 Bootloader-Based OTA System
## Next-Level Architecture: Separate Bootloader + User Applications

This is the **advanced architecture** that separates the OTA update mechanism from user applications, allowing you to deploy **any code** without including OTA logic.

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                     ESP32 FLASH (4MB)                           │
├─────────────────────────────────────────────────────────────────┤
│ 0x10000  │ OTA BOOTLOADER (1MB)                                 │
│          │ ├─ WiFi Connection                                   │
│          │ ├─ Version Checking                                  │
│          │ ├─ AES-256 Decryption                                │
│          │ ├─ RSA Signature Verification                        │
│          │ └─ Flash Writing & Boot Management                   │
├─────────────────────────────────────────────────────────────────┤
│ 0x110000 │ USER APPLICATION (1.5MB)                             │
│          │ ├─ Your pure code only!                              │
│          │ ├─ No OTA logic needed                               │
│          │ └─ Just business functionality                       │
├─────────────────────────────────────────────────────────────────┤
│ 0x290000 │ USER APPLICATION BACKUP (1.5MB)                      │
│          │ └─ Automatic rollback partition                      │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔄 Boot Flow

```
ESP32 Powers On
      │
      ▼
┌─────────────────────────────┐
│   OTA BOOTLOADER STARTS     │
│   (Always runs first)       │
└─────────────────────────────┘
      │
      ▼
┌─────────────────────────────┐
│ 1. Connect to WiFi          │
│ 2. Check version on server  │
│ 3. Compare with installed   │
└─────────────────────────────┘
      │
      ├─── New version? ───────────────┐
      │         │                       │
      │        YES                      NO
      │         │                       │
      │         ▼                       │
      │  ┌──────────────────────┐       │
      │  │ Download encrypted   │       │
      │  │ user application     │       │
      │  │ from GitHub Releases │       │
      │  └──────────────────────┘       │
      │         │                       │
      │         ▼                       │
      │  ┌──────────────────────┐       │
      │  │ Verify RSA signature │       │
      │  │ Decrypt with AES-256 │       │
      │  │ Write to user_app    │       │
      │  └──────────────────────┘       │
      │         │                       │
      └─────────┴───────────────────────┘
                │
                ▼
┌─────────────────────────────────────────┐
│   BOOT USER APPLICATION                  │
│   (Your pure code runs here!)            │
└─────────────────────────────────────────┘
```

---

## 📁 Project Structure

```
esp32cryptcode/
│
├── OTA_Bootloader/                    # 🔧 Bootloader (Flash once via USB)
│   ├── OTA_Bootloader.ino            # Main bootloader code
│   ├── crypto_utils.cpp/h            # AES-256 encryption
│   ├── rsa_verify.cpp/h              # RSA signature verification
│   └── rsa_public.h                  # Public key for verification
│
├── User_Application/                  # 📱 User Apps (Deploy via OTA)
│   ├── User_App_Blink/               # Example: LED Blink
│   │   └── User_App_Blink.ino        # Pure blink code, NO OTA!
│   ├── User_App_Temperature/         # Example: Temp Sensor
│   │   └── User_App_Temperature.ino  # Pure sensor code, NO OTA!
│   └── User_App_WiFiScanner/         # Example: WiFi Scanner
│       └── User_App_WiFiScanner.ino  # Pure WiFi code, NO OTA!
│
├── partitions_bootloader.csv         # 📋 New partition layout
│
├── .github/workflows/
│   ├── build_bootloader.yml          # Build bootloader (for USB flash)
│   └── build_user_app.yml            # Build & encrypt user apps
│
└── tools/                            # 🔐 Encryption tools
    ├── encrypt_firmware.py
    └── sign_firmware.py
```

---

## 🚀 Quick Start

### Step 1: Flash the Bootloader (One Time Only!)

1. Open `OTA_Bootloader/OTA_Bootloader.ino` in Arduino IDE
2. Configure WiFi credentials:
   ```cpp
   const char* DEFAULT_WIFI_SSID = "YourWiFi";
   const char* DEFAULT_WIFI_PASSWORD = "YourPassword";
   ```
3. Select partition scheme: **Custom** (use `partitions_bootloader.csv`)
4. Upload via USB
5. **Done!** The bootloader is now installed forever.

### Step 2: Deploy User Applications (Via OTA!)

1. Create your application in `User_Application/`
2. **No OTA code needed!** Just your pure logic:
   ```cpp
   #define USER_APP_VERSION 1
   
   void setup() {
       Serial.begin(115200);
       Serial.println("Hello from my app!");
   }
   
   void loop() {
       // Your code here - NO OTA needed!
   }
   ```
3. Push to GitHub
4. GitHub Actions encrypts and uploads to Releases
5. ESP32 bootloader automatically downloads and installs!

---

## 📝 Creating a New User Application

### Minimal User App Template

```cpp
/*
 * My Custom User Application
 * NO OTA CODE NEEDED - The bootloader handles everything!
 */

#define USER_APP_VERSION 1  // Increment for each release

void setup() {
    Serial.begin(115200);
    Serial.println("My Application Started!");
    
    // Your initialization code here
}

void loop() {
    // Your application code here
    
    delay(1000);
}
```

### That's it! Compare to old system:

**OLD (with OTA in every app):** ~400 lines of OTA code + your app
**NEW (bootloader system):** Only your app code!

---

## 🔐 Security Features

| Feature | Description |
|---------|-------------|
| **AES-256-CBC** | User app encrypted before transmission |
| **RSA-2048** | Digital signature for authenticity |
| **SHA-256** | Integrity verification |
| **Automatic Rollback** | Failed updates don't brick device |

---

## 📊 Partition Layout

| Name | Type | Offset | Size | Purpose |
|------|------|--------|------|---------|
| nvs | data | 0x9000 | 20KB | WiFi credentials, settings |
| otadata | data | 0xE000 | 8KB | Boot partition selection |
| ota_boot | app | 0x10000 | 1MB | **OTA Bootloader (fixed)** |
| user_app | app | 0x110000 | 1.5MB | **User Application (updates here)** |
| user_app_b | app | 0x290000 | 1.5MB | Backup for rollback |

---

## 🔄 GitHub Actions Workflows

### `build_bootloader.yml`
- Triggers on changes to `OTA_Bootloader/`
- Builds bootloader binary
- Creates release with `bootloader-vX` tag

### `build_user_app.yml`
- Triggers on changes to `User_Application/`
- Builds user application
- Encrypts with AES-256
- Signs with RSA-2048
- Creates release with `vX` tag

---

## 🆚 Old vs New Architecture

| Aspect | Old System | New Bootloader System |
|--------|------------|----------------------|
| User app code | Includes OTA (~400 lines) | Pure application only |
| Update mechanism | Built into every app | Separate bootloader |
| App size | Large (app + OTA) | Small (app only) |
| Deployment | Must include OTA | Just push your code |
| Flexibility | OTA tied to app | Any app can be deployed |
| Professional level | Educational | Industry standard |

---

## 🏭 This is How Real Products Work!

- **Sonoff/Tasmota**: Bootloader + swappable firmware
- **ESPHome**: OTA managed separately from user config
- **AWS IoT**: FreeRTOS bootloader + application separation
- **Commercial IoT**: All use this pattern!

---

## 📞 Support

If you have issues:
1. Check Serial Monitor at 115200 baud
2. Verify WiFi credentials in bootloader
3. Ensure GitHub Releases contain the encrypted files
4. Check RSA key consistency
