# 📚 ESP32 Encrypted OTA Update System - Complete Documentation

## 🎯 Project Overview

A complete implementation of secure Over-The-Air (OTA) firmware updates for ESP32 devices using **AES-256-CBC encryption**, including a **Man-In-The-Middle (MITM) attack demonstration** for security research and testing.

**Version:** 1.0  
**Last Updated:** 2025-10-10  
**Platform:** ESP32 + Arduino IDE  
**Language:** C++, Python

---

## ✨ Key Features

### Security Features
- ✅ **AES-256-CBC Encryption** - Military-grade firmware encryption
- ✅ **Streaming Decryption** - Memory-efficient chunk-based processing
- ✅ **PKCS7 Padding** - Standard-compliant padding scheme
- ✅ **Random IV per Firmware** - Prevents pattern analysis
- ✅ **SHA-256 Hashing** - Firmware integrity verification
- ✅ **Version Checking** - Automatic update detection

### OTA Features
- ✅ **HTTP-based Updates** - Simple firmware delivery
- ✅ **Progress Monitoring** - Real-time update status
- ✅ **Automatic Rollback** - Safe recovery from failed updates
- ✅ **Dual Partition Support** - No downtime during updates
- ✅ **WiFi Integration** - Network-based delivery

### Educational Features
- ✅ **MITM Attack Demo** - Learn about security vulnerabilities
- ✅ **Traffic Analysis** - Entropy detection and firmware analysis
- ✅ **Multiple Attack Modes** - Monitor, Inject, Block scenarios
- ✅ **Comprehensive Logging** - Detailed attack visualization

---

## 📁 Complete Project Structure

```
esp32cryptcode/
│
├── ESP32_Encrypted_Firmware/          # 🔷 Main ESP32 Firmware
│   ├── ESP32_Encrypted_Firmware.ino   # Main Arduino sketch
│   ├── ota_update.cpp                 # OTA implementation
│   ├── ota_update.h                   # OTA header
│   ├── crypto_utils.cpp               # Cryptographic functions
│   ├── crypto_utils.h                 # Crypto header
│   ├── wifi_manager.cpp               # WiFi connection handler
│   ├── wifi_manager.h                 # WiFi header
│   └── build/                         # Compiled binaries (auto-generated)
│       └── esp32.esp32.esp32/
│           └── ESP32_Encrypted_Firmware.ino.bin
│
├── tools/                             # 🔧 Encryption & Server Tools
│   ├── encrypt_firmware.py            # Firmware encryption script
│   ├── aes_key.bin                    # Master encryption key (32 bytes)
│   ├── firmware_encrypted.bin         # Encrypted firmware file
│   ├── version.txt                    # Current version number
│   ├── encryption_key.h               # C header with keys
│   └── firmware_encrypted.bin.meta    # Metadata (JSON)
│
├── hacker_demo/                       # 🎭 MITM Attack Demonstration
│   ├── real_mitm_attack.py            # MITM proxy server
│   ├── network_helper.py              # Network utilities
│   ├── BEGINNER_ATTACK_GUIDE.md       # Attack tutorial
│   └── HOW_THE_ATTACK_WORKS.md        # Technical explanation
│
├── docs/                              # 📚 Documentation
│   ├── 01_PROJECT_OVERVIEW.md         # This file
│   ├── 02_FILE_REFERENCE.md           # Detailed file documentation
│   ├── 03_SCENARIOS.md                # All usage scenarios
│   ├── 04_SECURITY_GUIDE.md           # Security best practices
│   └── 05_API_REFERENCE.md            # Function documentation
│
└── README.md                          # Quick start guide

```

---

## 🔄 System Architecture

### High-Level Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                     FIRMWARE UPDATE LIFECYCLE                    │
└─────────────────────────────────────────────────────────────────┘

1. DEVELOPMENT
   ├─ Write code in Arduino IDE
   ├─ Increment FIRMWARE_VERSION
   └─ Compile → .bin file

2. ENCRYPTION
   ├─ Run encrypt_firmware.py
   ├─ Input: firmware.bin
   ├─ Output: firmware_encrypted.bin
   └─ Uses: aes_key.bin (reusable)

3. DEPLOYMENT
   ├─ Upload firmware_encrypted.bin to server
   ├─ Update version.txt
   └─ Start HTTP server

4. DEVICE UPDATE
   ├─ ESP32 boots
   ├─ Checks version.txt
   ├─ If new version → Download encrypted firmware
   ├─ Decrypt in chunks (streaming)
   ├─ Write to OTA partition
   └─ Reboot with new firmware

5. VERIFICATION
   ├─ Bootloader validates new firmware
   ├─ If valid → Boot from new partition
   └─ If invalid → Rollback to old partition
```

---

## 🔐 Encryption Architecture

### AES-256-CBC Encryption Flow

```
PLAINTEXT FIRMWARE (1,030,256 bytes)
         │
         ├─ Add PKCS7 Padding → 1,030,272 bytes (multiple of 16)
         │
         ├─ Generate Random IV (16 bytes)
         │
         ├─ AES-256-CBC Encryption
         │  ├─ Key: 32 bytes (from aes_key.bin)
         │  ├─ IV: 16 bytes (random)
         │  └─ Mode: CBC (Cipher Block Chaining)
         │
         ▼
ENCRYPTED FIRMWARE FILE
├─ [IV: 16 bytes]
└─ [Encrypted Data: 1,030,272 bytes]
   Total: 1,030,288 bytes
```

### Decryption Flow (On ESP32)

```
DOWNLOAD ENCRYPTED FIRMWARE
         │
         ├─ Extract IV (first 16 bytes)
         │
         ├─ Initialize AES Context
         │  ├─ Key: Hardcoded in firmware
         │  └─ IV: From file
         │
         ├─ Stream Decrypt in 1KB Chunks
         │  ├─ Read 1024 encrypted bytes
         │  ├─ Decrypt with AES-CBC
         │  ├─ Write to flash partition
         │  └─ Repeat until complete
         │
         ├─ Remove PKCS7 Padding (last chunk)
         │
         ▼
PLAINTEXT FIRMWARE IN FLASH
└─ Ready to boot!
```

---

## 🌐 Network Architecture

### Normal Operation (Production)

```
┌──────────┐                    ┌──────────────┐
│  ESP32   │───── HTTP ────────▶│ Your Server  │
│192.168.1.29                    │192.168.1.22  │
│          │                     │   Port 8000  │
└──────────┘                     └──────────────┘
     │                                  │
     │ GET /version.txt                │
     │◀────────────────────────────────│
     │ Response: "2"                   │
     │                                  │
     │ GET /firmware_encrypted.bin     │
     │◀────────────────────────────────│
     │ Response: Encrypted firmware    │
     │                                  │
     └─ Decrypt & Install               │
```

### MITM Attack Scenario (Demo)

```
┌──────────┐         ┌─────────────┐         ┌──────────────┐
│  ESP32   │────────▶│ MITM Proxy  │────────▶│ Real Server  │
│192.168.1.29        │192.168.1.22 │         │192.168.1.22  │
│          │         │  Port 8080  │         │   Port 8000  │
└──────────┘         └─────────────┘         └──────────────┘
     │                      │                        │
     │ GET /firmware...     │                        │
     ├─────────────────────▶│                        │
     │                      │ 🎯 INTERCEPT!          │
     │                      │                        │
     │                      │ Forward request        │
     │                      ├───────────────────────▶│
     │                      │                        │
     │                      │◀───────────────────────│
     │                      │ Encrypted firmware     │
     │                      │                        │
     │                      │ 🔬 ANALYZE             │
     │                      │ - Calculate entropy    │
     │                      │ - Save copy            │
     │                      │ - Log metadata         │
     │                      │                        │
     │◀─────────────────────│ Forward response       │
     │ Encrypted firmware   │                        │
     │                      │                        │
     └─ Decrypt & Install   │                        │
```

---

## 🎓 Learning Objectives

This project demonstrates:

1. **Embedded Cryptography**
   - AES-256 implementation on resource-constrained devices
   - Memory-efficient streaming decryption
   - Key management in embedded systems

2. **OTA Update Mechanisms**
   - Dual partition architecture
   - Atomic updates with rollback
   - Version management

3. **Network Security**
   - Man-In-The-Middle attacks
   - Traffic analysis and entropy detection
   - Why HTTPS is critical

4. **Secure Development**
   - Defense-in-depth strategies
   - Error handling and recovery
   - Security testing methodologies

---

## ⚠️ Important Disclaimers

### Educational Purpose
This project is designed for:
- ✅ Learning about embedded security
- ✅ Understanding OTA update mechanisms
- ✅ Security research in controlled environments
- ✅ Testing your own devices

### Legal Warning
- ❌ **DO NOT** use MITM attacks on networks you don't own
- ❌ **DO NOT** intercept traffic without authorization
- ❌ **DO NOT** deploy without proper security measures
- ⚠️ Unauthorized interception is **illegal** in most jurisdictions

### Production Warning
This implementation is **NOT production-ready** without:
- ✅ HTTPS/TLS for transport security
- ✅ Firmware signing for authenticity
- ✅ Certificate pinning
- ✅ Secure key storage (HSM, secure element)
- ✅ Rollback protection
- ✅ Secure boot

---

## 📖 Documentation Index

1. **[Project Overview](01_PROJECT_OVERVIEW.md)** ← You are here
2. **[File Reference](02_FILE_REFERENCE.md)** - Detailed file documentation
3. **[Usage Scenarios](03_SCENARIOS.md)** - Step-by-step guides
4. **[Security Guide](04_SECURITY_GUIDE.md)** - Security best practices
5. **[API Reference](05_API_REFERENCE.md)** - Function documentation

---

## 🚀 Quick Start

### Minimum Requirements
- ESP32 development board (4MB flash minimum)
- Arduino IDE with ESP32 board support
- Python 3.7+ with `pycryptodome` library
- WiFi network
- USB cable for initial upload

### 5-Minute Setup

```bash
# 1. Clone/download project
cd esp32cryptcode

# 2. Install Python dependencies
pip install pycryptodome

# 3. Configure WiFi in ESP32_Encrypted_Firmware.ino
# Edit lines 13-14 with your WiFi credentials

# 4. Upload to ESP32 via Arduino IDE
# Tools → Board → ESP32 Dev Module
# Sketch → Upload

# 5. Test!
# Open Serial Monitor (115200 baud)
# Watch crypto tests and OTA check
```

**Next Steps:** See [03_SCENARIOS.md](03_SCENARIOS.md) for detailed usage scenarios.

---

## 🤝 Contributing

This is an educational project. Contributions welcome:
- Bug fixes
- Documentation improvements
- Additional security features
- Performance optimizations

---

## 📞 Support

For issues:
1. Check documentation in `docs/` folder
2. Review serial monitor output
3. Verify WiFi and server configuration
4. Test with direct HTTP server (port 8000) before MITM demo

---

**Next:** [File Reference →](02_FILE_REFERENCE.md)
