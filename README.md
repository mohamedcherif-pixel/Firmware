# ESP32 Encrypted Firmware Update System

A secure over-the-air (OTA) firmware update system for ESP32 devices using AES-256-CBC encryption and GitHub Releases for distribution.

## 🔒 Key Features

- **Secure Firmware Updates**: AES-256-CBC encryption protects firmware during transmission
- **Automated Deployment**: GitHub Actions CI/CD pipeline for build and release
- **Cloud Distribution**: Firmware hosted on GitHub Releases
- **Periodic Updates**: ESP32 automatically checks for and applies updates
- **Integrity Verification**: Ensures firmware hasn't been tampered with
- **Rollback Protection**: Prevents installation of invalid firmware

## 📋 Project Structure

```
ESP32_Encrypted_Firmware/   # Arduino implementation
├── ESP32_Encrypted_Firmware.ino  # Main sketch
├── crypto_utils.cpp/h      # Encryption functions
├── ota_update.cpp/h        # Update process
└── wifi_manager.cpp/h      # WiFi connectivity

.github/workflows/          # CI/CD configuration
└── build.yml               # Automated build pipeline

tools/                      # Development utilities
├── encrypt_firmware.py     # Firmware encryption tool
└── simple_http_server.py   # Local testing server
```

## 🚀 How It Works

1. **Development**: Update code and increment version number
2. **Build**: GitHub Actions compiles firmware when changes are pushed
3. **Encryption**: Firmware binary is encrypted with AES-256-CBC
4. **Deployment**: Encrypted firmware is published to GitHub Releases
5. **Update Check**: ESP32 periodically checks for newer versions
6. **Download**: If available, ESP32 downloads encrypted firmware
7. **Decryption**: ESP32 uses pre-shared key to decrypt firmware
8. **Installation**: Firmware is verified and installed
9. **Verification**: After reboot, ESP32 confirms firmware is valid

## 🔧 Setup & Configuration

### ESP32 Configuration

```cpp
// WiFi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Firmware source (GitHub Releases)
const char* OTA_FIRMWARE_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/firmware_encrypted.bin";
const char* VERSION_CHECK_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt";

// Current firmware version - increment with each release
#define FIRMWARE_VERSION 1
```

### GitHub Setup

1. Fork or clone this repository
2. Update the URLs in the code to point to your GitHub repository
3. Push changes to trigger the GitHub Actions workflow
4. ESP32 devices will update from your repository's releases

## 📦 Deployment Process

### Automatic Deployment (Recommended)

1. Modify code in ESP32_Encrypted_Firmware directory
2. Increment `FIRMWARE_VERSION` in ESP32_Encrypted_Firmware.ino
3. Commit and push to GitHub
4. GitHub Actions will automatically:
   - Build the firmware
   - Encrypt it with AES-256
   - Create a new release
   - Upload the encrypted firmware
   - Update the version file

### Manual Deployment

1. Compile firmware using Arduino IDE
2. Encrypt using the encryption tool:
   ```
   cd tools
   python encrypt_firmware.py encrypt -i ../ESP32_Encrypted_Firmware/build/ESP32_Encrypted_Firmware.ino.bin -o firmware_encrypted.bin -k aes_key.bin
   ```
3. Upload `firmware_encrypted.bin` and `version.txt` to your hosting service

## 🔍 Security Considerations

- **Key Management**: The AES key is pre-shared and stored in the ESP32 firmware
- **Encryption Strength**: AES-256-CBC provides strong protection against tampering
- **Transport Security**: GitHub Releases uses HTTPS for secure downloads
- **Firmware Verification**: ESP32 validates firmware after decryption

## 📚 Documentation

- [Detailed Investigation](DETAILED_INVESTIGATION.md): Comprehensive technical analysis
- [Installation Guide](ESP32_Encrypted_Firmware/INSTALLATION.md): Setup instructions for Arduino IDE
- [GitHub Hosting Setup](GITHUB_HOSTING_SETUP.md): Configuring GitHub for firmware hosting

## 🛠️ Development Tools

- **Arduino IDE**: For ESP32 firmware development
- **Python 3.7+**: For encryption tools
- **GitHub Actions**: For CI/CD pipeline

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 📊 Technical Specifications

- **Target Hardware**: ESP32 (4MB+ flash recommended)
- **Encryption**: AES-256-CBC with 32-byte key and 16-byte IV
- **Firmware Size**: Typically 200-500KB encrypted
- **Update Time**: ~10-30 seconds depending on WiFi speed
- **Memory Usage**: Optimized for ESP32's limited RAM
- **Power Requirements**: Stable power needed during update

## 🔄 Update Process Diagram

```
┌─────────┐          ┌──────────────┐          ┌─────────────────┐
│  ESP32  │          │ GitHub       │          │ GitHub Actions  │
│  Device │          │ Releases     │          │ (CI/CD)         │
└────┬────┘          └───────┬──────┘          └────────┬────────┘
     │                       │                          │
     │                       │                          │ 1. Build firmware
     │                       │                          │
     │                       │                          │ 2. Encrypt firmware
     │                       │                          │
     │                       │ 3. Upload encrypted      │
     │                       │    firmware & version    │
     │                       │ ◄────────────────────────┘
     │                       │
     │ 4. Check version.txt  │
     │ ─────────────────────►│
     │                       │
     │ 5. If newer version:  │
     │    Download firmware  │
     │ ─────────────────────►│
     │                       │
     │ 6. Decrypt & install  │
     │                       │
     │ 7. Reboot with new    │
     │    firmware           │
     │                       │
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📞 Contact

Mohamed Cherif - [@mohamedcherif-pixel](https://github.com/mohamedcherif-pixel)

Project Link: [https://github.com/mohamedcherif-pixel/Firmware](https://github.com/mohamedcherif-pixel/Firmware)
