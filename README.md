# ESP32 Secure OTA System

A **next-level** secure over-the-air firmware update system for ESP32. Users write standard Arduino code (`setup()` and `loop()`) in a simple file - the bootloader handles all WiFi, OTA, encryption, and updates automatically in the background.

---

## 📋 Table of Contents

1. [Overview](#-overview)
2. [Key Features](#-key-features)
3. [Architecture](#-architecture)
4. [Project Structure](#-project-structure)
5. [How It Works](#-how-it-works)
6. [Security](#-security)
7. [Initial Setup (One Time)](#-initial-setup-one-time)
8. [Daily Usage](#-daily-usage)
9. [File Details](#-file-details)
10. [Troubleshooting](#-troubleshooting)

---

## 🎯 Overview

This project solves a common problem: **How can users deploy code to ESP32 devices without dealing with OTA complexity?**

**The Solution:**
- Users write pure Arduino code in `user_code.h`
- Change version number, push to GitHub
- ESP32 automatically downloads, decrypts, verifies, and installs the new code
- Zero OTA knowledge required from the user!

**Perfect for:**
- Educational projects where students focus on application logic
- IoT deployments with remote firmware updates
- Projects requiring secure firmware distribution

---

## 🔒 Key Features

| Feature | Description |
|---------|-------------|
| **Zero OTA Code** | User writes pure Arduino - no WiFi, no HTTP, no Update library |
| **Standard Arduino** | Use familiar `void setup()` and `void loop()` |
| **Background Updates** | OTA runs on Core 0, user code runs on Core 1 (parallel!) |
| **AES-256-CBC Encryption** | Firmware encrypted during transmission |
| **RSA-2048 Signatures** | Cryptographic verification prevents tampering |
| **GitHub CI/CD** | Automatic build, encrypt, sign, and deploy on every push |
| **Auto-Update** | ESP32 checks for updates every 60 seconds |
| **Dual-Core** | User application never interrupted during update checks |

---

## 🏗 Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                            ESP32                                │
├────────────────────────────┬────────────────────────────────────┤
│         CORE 0             │            CORE 1                  │
│    (Background OTA)        │      (User Application)            │
│                            │                                    │
│  ┌──────────────────────┐  │  ┌──────────────────────────────┐  │
│  │ • Connect to WiFi    │  │  │ • Your setup() runs here     │  │
│  │ • Check version.txt  │  │  │ • Your loop() runs here      │  │
│  │ • Compare versions   │  │  │ • Full Arduino compatibility │  │
│  │ • Download firmware  │  │  │ • Use any library you want   │  │
│  │ • Verify RSA sig     │  │  │ • No OTA code needed!        │  │
│  │ • Decrypt AES-256    │  │  │                              │  │
│  │ • Install & reboot   │  │  │                              │  │
│  └──────────────────────┘  │  └──────────────────────────────┘  │
└────────────────────────────┴────────────────────────────────────┘
```

### Update Flow

```
┌──────────┐         ┌─────────────┐         ┌──────────────────┐
│   User   │         │   GitHub    │         │      ESP32       │
└────┬─────┘         └──────┬──────┘         └────────┬─────────┘
     │                      │                         │
     │ 1. Edit user_code.h  │                         │
     │    (change version)  │                         │
     │                      │                         │
     │ 2. git push          │                         │
     │ ────────────────────>│                         │
     │                      │                         │
     │                      │ 3. GitHub Actions:      │
     │                      │    • Compile firmware   │
     │                      │    • Encrypt (AES-256)  │
     │                      │    • Sign (RSA-2048)    │
     │                      │    • Create release     │
     │                      │                         │
     │                      │ 4. Check version (60s)  │
     │                      │ <────────────────────── │
     │                      │                         │
     │                      │ 5. New version found!   │
     │                      │ ──────────────────────> │
     │                      │                         │
     │                      │ 6. Download .bin & .sig │
     │                      │ ──────────────────────> │
     │                      │                         │
     │                      │         7. Verify signature
     │                      │         8. Decrypt firmware
     │                      │         9. Install to flash
     │                      │        10. Reboot!
     │                      │                         │
     │                      │                    Running new code!
```

---

## 📁 Project Structure

```
esp32cryptcode/
│
├── OTA_Bootloader/                 # Main Arduino project
│   ├── OTA_Bootloader.ino          # Bootloader (DO NOT MODIFY)
│   ├── user_code.h                 # ★ YOUR CODE GOES HERE ★
│   ├── crypto_utils.cpp            # AES-256-CBC decryption
│   ├── crypto_utils.h              # Crypto header
│   ├── rsa_verify.cpp              # RSA-2048 signature verification
│   ├── rsa_verify.h                # RSA header
│   └── rsa_public.h                # RSA public key (embedded)
│
├── .github/workflows/              # CI/CD Automation
│   ├── build_user_app.yml          # Builds user app → GitHub Release
│   └── build_bootloader.yml        # Builds bootloader for USB flash
│
├── tools/                          # Python scripts (used by CI/CD)
│   ├── encrypt_firmware.py         # AES-256-CBC encryption
│   ├── sign_firmware.py            # RSA-2048 signing
│   ├── requirements.txt            # Python dependencies
│   ├── rsa_private.pem             # RSA private key (keep secret!)
│   └── rsa_public.pem              # RSA public key
│
├── partitions_bootloader.csv       # ESP32 partition table
├── README.md                       # This file
└── .gitignore                      # Git ignore rules
```

---

## ⚙️ How It Works

### 1. User Code (`user_code.h`)

The user writes standard Arduino code:

```cpp
#define USER_APP_VERSION 1   // ← Change this to trigger update!

void setup() {
    Serial.begin(115200);
    Serial.println("Hello from my app!");
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}
```

### 2. Bootloader (`OTA_Bootloader.ino`)

The bootloader:
- Initializes WiFi connection
- Starts background OTA task on Core 0
- Calls user's `setup()` and `loop()` on Core 1
- Checks GitHub every 60 seconds for new versions
- Downloads, decrypts, verifies, and installs updates automatically

### 3. GitHub Actions (`build_user_app.yml`)

When you push to GitHub:
1. **Compile**: Arduino CLI builds the firmware
2. **Encrypt**: AES-256-CBC encryption with random IV
3. **Sign**: RSA-2048 digital signature
4. **Release**: Upload to GitHub Releases as `user_app_encrypted.bin`

### 4. ESP32 Update Process

1. **Check**: GET `version.txt` from GitHub Releases
2. **Compare**: Is server version > local version?
3. **Verify**: Download signature, verify with RSA public key
4. **Download**: Stream encrypted firmware
5. **Decrypt**: AES-256-CBC decryption on-the-fly
6. **Install**: Write to OTA partition
7. **Reboot**: Boot into new firmware

---

## 🔐 Security

### Encryption: AES-256-CBC

- **Algorithm**: AES (Advanced Encryption Standard)
- **Key Size**: 256 bits (32 bytes)
- **Mode**: CBC (Cipher Block Chaining)
- **IV**: Random 16-byte IV prepended to each encrypted file
- **Key Storage**: Hardcoded in bootloader (pre-shared key model)

```
┌──────────────────────────────────────────────────────┐
│              Encrypted Firmware Format               │
├──────────────────┬───────────────────────────────────┤
│   IV (16 bytes)  │     Encrypted Data (AES-256)     │
└──────────────────┴───────────────────────────────────┘
```

### Signature: RSA-2048

- **Algorithm**: RSA with SHA-256
- **Key Size**: 2048 bits
- **Purpose**: Verify firmware authenticity
- **Private Key**: Stored in GitHub Actions (never on device)
- **Public Key**: Embedded in ESP32 bootloader

### HTTPS Transport

- All downloads use HTTPS (TLS encrypted)
- GitHub's SSL certificates provide transport security

---

## 🚀 Initial Setup (One Time)

### Prerequisites

- Arduino IDE 2.x installed
- ESP32 board package installed (`esp32` by Espressif)
- GitHub account
- Git installed

### Step 1: Clone Repository

```bash
git clone https://github.com/mohamedcherif-pixel/Firmware.git
cd Firmware
```

### Step 2: Configure WiFi

Open `OTA_Bootloader/OTA_Bootloader.ino` and set your WiFi credentials:

```cpp
const char* DEFAULT_WIFI_SSID = "YOUR_WIFI_NAME";
const char* DEFAULT_WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

### Step 3: Configure GitHub URL

In the same file, verify the GitHub URLs point to your repository:

```cpp
const char* FIRMWARE_URL = "https://github.com/YOUR_USERNAME/YOUR_REPO/releases/latest/download/user_app_encrypted.bin";
const char* FIRMWARE_SIG_URL = "https://github.com/YOUR_USERNAME/YOUR_REPO/releases/latest/download/user_app_encrypted.bin.sig";
const char* VERSION_URL = "https://github.com/YOUR_USERNAME/YOUR_REPO/releases/latest/download/user_app_version.txt";
```

### Step 4: Flash Bootloader via USB

1. Open `OTA_Bootloader/OTA_Bootloader.ino` in Arduino IDE
2. Select Board: `ESP32 Dev Module`
3. Select Port: Your ESP32's COM port
4. Click **Upload**

### Step 5: Verify Serial Output

Open Serial Monitor (115200 baud). You should see:

```
╔═══════════════════════════════════════════════════════════╗
║     ESP32 OTA SYSTEM v2 + USER APP v1                     ║
╚═══════════════════════════════════════════════════════════╝

[OTA] Connecting to WiFi: YOUR_WIFI_NAME
[OTA] Connected! IP: 192.168.1.100
[OTA] Background checker started
[BOOT] Starting user application...

[OTA] Checking for updates...
[OTA] Server: v1 | Local: v1
[OTA] Already up to date
```

**Done!** Your ESP32 is now ready for OTA updates.

---

## 📱 Daily Usage

### Updating Your Code

1. **Edit** `OTA_Bootloader/user_code.h`:

```cpp
#define USER_APP_VERSION 2   // ← Increment version!

void setup() {
    Serial.begin(115200);
    Serial.println("Version 2 is running!");
}

void loop() {
    Serial.println("New feature!");
    delay(1000);
}
```

2. **Commit and push**:

```bash
git add .
git commit -m "Update to version 2"
git push
```

3. **Wait for GitHub Actions** (1-2 minutes)

4. **Watch ESP32 Serial Monitor**:

```
[OTA] Checking for updates...
[OTA] Server: v2 | Local: v1
[OTA] ★ NEW VERSION AVAILABLE: v2
[OTA] ✓ Signature verified
[OTA] Downloading update...
[OTA] Progress: 25%
[OTA] Progress: 50%
[OTA] Progress: 75%
[OTA] Progress: 100%
[OTA] ✓ Update installed!
[OTA] ★ REBOOTING in 3 seconds...
```

5. **ESP32 reboots with new code!**

---

## 📄 File Details

### `user_code.h` - Your Application

```cpp
/*
 * USER APPLICATION CODE
 * =====================
 * 
 * Write your normal Arduino code here!
 * Just change the version number and push to GitHub.
 * The ESP32 will automatically download your new code.
 */

#define USER_APP_VERSION 1   // ← INCREMENT THIS TO TRIGGER UPDATE!

void setup() {
    // Your setup code - runs once at startup
}

void loop() {
    // Your loop code - runs continuously
}
```

**Rules:**
- ✅ Use any Arduino library
- ✅ Use Serial, GPIO, I2C, SPI, etc.
- ✅ Standard Arduino functions work
- ❌ Don't define `USER_APP_VERSION` twice
- ❌ Don't use `setup` or `loop` as variable names

### `OTA_Bootloader.ino` - The Bootloader

**DO NOT MODIFY** unless you know what you're doing!

Key components:
- WiFi connection management
- Dual-core task management
- OTA update checking (every 60 seconds)
- Encrypted firmware download
- RSA signature verification
- AES decryption
- Firmware installation

### `crypto_utils.cpp` - AES Decryption

Handles AES-256-CBC decryption using ESP32's mbedTLS library:
- `crypto_init()` - Initialize crypto system
- `aes_decrypt_stream_init()` - Start streaming decryption
- `aes_decrypt_stream_update()` - Decrypt chunks
- `aes_decrypt_stream_free()` - Clean up

### `rsa_verify.cpp` - RSA Verification

Handles RSA-2048 signature verification:
- `rsa_verify_init()` - Load public key
- `rsa_verify_firmware_from_url()` - Download and verify signature

---

## 🔧 Troubleshooting

### ESP32 Won't Connect to WiFi

```
[OTA] Connecting to WiFi: YOUR_WIFI
[OTA] Connection failed!
```

**Solutions:**
- Check WiFi credentials in `OTA_Bootloader.ino`
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check WiFi signal strength

### Version Not Detected

```
[OTA] Server: v-1 | Local: v5
```

**Solutions:**
- Wait for GitHub Actions to complete
- Check GitHub Releases page for new release
- Verify release is marked as "Latest"

### Signature Verification Failed

```
[OTA] ✗ Signature invalid!
```

**Solutions:**
- Ensure RSA keys match (public key in ESP32 = private key on GitHub)
- Don't modify the encrypted firmware file
- Regenerate keys if corrupted

### Download Timeout

```
[OTA] Download failed!
```

**Solutions:**
- Check internet connection
- Verify GitHub URLs are correct
- Check firewall settings

### Build Failed on GitHub Actions

Check the Actions tab on GitHub for error details:
- Missing libraries → Add to workflow
- Syntax error → Fix code in `user_code.h`
- Memory overflow → Reduce code size

---

## 📊 Technical Specifications

| Specification | Value |
|---------------|-------|
| Target MCU | ESP32 (Dual-core Xtensa LX6) |
| Flash Size | 4MB minimum recommended |
| Framework | Arduino |
| ESP32 Core | 2.0.14 |
| Encryption | AES-256-CBC |
| Signature | RSA-2048 with SHA-256 |
| Update Check | Every 60 seconds |
| OTA Core | Core 0 (background) |
| User App Core | Core 1 (main) |
| WiFi | 2.4GHz 802.11 b/g/n |
| Protocol | HTTPS (TLS 1.2+) |

---

## 👤 Author

**Mohamed Cherif**
- GitHub: [@mohamedcherif-pixel](https://github.com/mohamedcherif-pixel)
- Project: [Firmware](https://github.com/mohamedcherif-pixel/Firmware)

---

## 📄 License

MIT License - See LICENSE file for details.
