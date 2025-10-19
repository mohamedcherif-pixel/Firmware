# ESP32 Encrypted Firmware Update System
## Project 03: Mise à jour d'un objet avec un firmware crypté

A complete implementation of secure firmware updates for ESP32 using AES-256 encryption and OTA (Over-The-Air) updates.

## Features

- **AES-256-CBC Encryption**: Strong encryption for firmware binaries
- **Secure OTA Updates**: Over-the-air firmware updates with encryption
- **SHA-256 Verification**: Firmware integrity checking
- **Dual Partition Support**: Safe rollback capability
- **WiFi Integration**: Network-based firmware delivery
- **Hardware AES Acceleration**: Utilizes ESP32's hardware crypto engine

## Project Structure

```
esp32cryptcode/
├── main/
│   ├── main.c              # Main application entry point
│   ├── crypto_utils.c/h    # AES encryption/decryption utilities
│   ├── ota_update.c/h      # OTA update implementation
│   └── wifi_manager.c/h    # WiFi connection management
├── tools/
│   ├── encrypt_firmware.py # Firmware encryption tool
│   ├── simple_http_server.py # Local OTA server for testing
│   └── requirements.txt    # Python dependencies
├── CMakeLists.txt          # Build configuration
├── partitions.csv          # Partition table with OTA support
└── sdkconfig.defaults      # ESP-IDF configuration
```

## Prerequisites

1. **ESP-IDF**: Install ESP-IDF v4.4 or later
   - Follow: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

2. **Python 3.7+**: For encryption tools
   ```bash
   pip install -r tools/requirements.txt
   ```

3. **ESP32 Development Board**: Any ESP32 board with at least 4MB flash

## Quick Start

### 1. Build the Project

```bash
# Set up ESP-IDF environment
. $HOME/esp/esp-idf/export.sh  # Linux/Mac
# or
%userprofile%\esp\esp-idf\export.bat  # Windows

# Configure the project
idf.py menuconfig

# Build
idf.py build

# Flash to ESP32
idf.py -p COM3 flash monitor  # Replace COM3 with your port
```

### 2. Test Crypto Functions

The firmware includes built-in crypto tests that run on startup:
- AES-256 encryption/decryption
- SHA-256 hashing
- Data integrity verification

### 3. Encrypt Firmware for OTA Update

```bash
# Navigate to tools directory
cd tools

# Encrypt the firmware binary
python encrypt_firmware.py encrypt \
    -i ../build/esp32_encrypted_firmware.bin \
    -o ../build/firmware_encrypted.bin

# This generates:
# - firmware_encrypted.bin (encrypted firmware)
# - aes_key.bin (encryption key - keep secure!)
# - firmware_encrypted.bin.meta (metadata)
# - encryption_key.h (C header for development)
```

### 4. Perform OTA Update

#### Option A: Local Testing

1. Start the HTTP server:
```bash
cd tools
python simple_http_server.py
```

2. Update WiFi credentials in `main/main.c`:
```c
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

3. Update OTA URL in `main/main.c`:
```c
#define OTA_FIRMWARE_URL "http://192.168.1.100:8000/firmware_encrypted.bin"
```

4. Uncomment the WiFi initialization code in `app_main()`

5. Rebuild and flash:
```bash
idf.py build flash monitor
```

#### Option B: Production Deployment

1. Host encrypted firmware on your server
2. Configure ESP32 with server URL
3. Trigger OTA update via:
   - Scheduled updates
   - Remote command
   - User button press

## Security Features

### 1. Encryption
- **Algorithm**: AES-256-CBC
- **Key Size**: 256 bits (32 bytes)
- **IV**: Random 16-byte initialization vector
- **Padding**: PKCS7

### 2. Secure Boot (Optional)
Enable in `sdkconfig.defaults`:
```
CONFIG_SECURE_BOOT=y
CONFIG_SECURE_BOOT_V2_ENABLED=y
```

### 3. Flash Encryption (Optional)
Enable in `sdkconfig.defaults`:
```
CONFIG_SECURE_FLASH_ENC_ENABLED=y
```

### 4. Anti-Rollback
Prevents downgrade to older firmware versions:
```
CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK=y
```

## API Reference

### Crypto Utilities

```c
// Initialize crypto subsystem
esp_err_t crypto_init(void);

// Encrypt data with AES-256-CBC
esp_err_t aes_encrypt(const uint8_t *plaintext, size_t plaintext_len,
                      const uint8_t *key, const uint8_t *iv,
                      uint8_t *ciphertext, size_t *ciphertext_len);

// Decrypt data with AES-256-CBC
esp_err_t aes_decrypt(const uint8_t *ciphertext, size_t ciphertext_len,
                      const uint8_t *key, const uint8_t *iv,
                      uint8_t *plaintext, size_t *plaintext_len);

// Generate SHA-256 hash
esp_err_t sha256_hash(const uint8_t *data, size_t data_len, uint8_t *hash);
```

### OTA Updates

```c
// Initialize OTA subsystem
esp_err_t ota_init(void);

// Update from URL
esp_err_t ota_update_from_url(const char *url, const uint8_t *aes_key);

// Update from buffer
esp_err_t ota_update_from_buffer(const uint8_t *encrypted_firmware, 
                                  size_t firmware_size,
                                  const uint8_t *aes_key,
                                  const uint8_t *iv);

// Get current version
uint32_t ota_get_current_version(void);

// Rollback to previous version
esp_err_t ota_rollback(void);
```

## Partition Table

The project uses a custom partition table with dual OTA partitions:

| Name      | Type | SubType | Offset   | Size |
|-----------|------|---------|----------|------|
| nvs       | data | nvs     | 0x9000   | 24K  |
| phy_init  | data | phy     | 0xf000   | 4K   |
| factory   | app  | factory | 0x10000  | 1M   |
| ota_0     | app  | ota_0   | 0x110000 | 1M   |
| ota_1     | app  | ota_1   | 0x210000 | 1M   |
| storage   | data | fat     | 0x310000 | 1M   |

## Key Management Best Practices

⚠️ **IMPORTANT**: The example uses hardcoded keys for demonstration only!

### Production Recommendations:

1. **Secure Key Storage**:
   - Use ESP32's eFuse for key storage
   - Implement secure element (ATECC608A, etc.)
   - Use Hardware Security Module (HSM)

2. **Key Rotation**:
   - Implement periodic key rotation
   - Support multiple key versions
   - Secure key distribution mechanism

3. **Key Derivation**:
   - Use unique keys per device
   - Implement key derivation functions (KDF)
   - Never reuse IVs with the same key

4. **Access Control**:
   - Restrict key access to authorized personnel
   - Implement audit logging
   - Use multi-factor authentication

## Troubleshooting

### Build Errors

**Error**: `mbedtls/aes.h: No such file or directory`
- **Solution**: Ensure ESP-IDF is properly installed and sourced

**Error**: `Partition table does not fit in configured flash size`
- **Solution**: Adjust partition sizes in `partitions.csv`

### Runtime Errors

**Error**: `OTA update failed: ESP_ERR_OTA_VALIDATE_FAILED`
- **Solution**: Verify encryption key matches, check firmware integrity

**Error**: `WiFi connection failed`
- **Solution**: Check SSID/password, verify WiFi is in range

**Error**: `Decryption failed`
- **Solution**: Ensure correct key and IV are used, verify firmware is not corrupted

### OTA Update Issues

**Issue**: Device doesn't reboot after OTA
- **Solution**: Check partition table, verify OTA partition is valid

**Issue**: Device boots to old firmware
- **Solution**: OTA validation may have failed, check logs

## Performance

- **Encryption Speed**: ~10 MB/s (with hardware acceleration)
- **OTA Update Time**: ~30-60 seconds for 1MB firmware
- **Memory Usage**: ~50KB RAM during OTA update

## Security Considerations

1. **Transport Security**: Use HTTPS for OTA downloads in production
2. **Firmware Signing**: Add RSA/ECDSA signatures for authenticity
3. **Secure Boot**: Enable secure boot to prevent unauthorized firmware
4. **Flash Encryption**: Encrypt flash contents to protect firmware at rest
5. **Network Security**: Use WPA2/WPA3 for WiFi, implement certificate pinning

## License

This project is provided as-is for educational purposes.

## References

- [ESP-IDF OTA Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html)
- [ESP32 Security Features](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/security.html)
- [mbedTLS Documentation](https://tls.mbed.org/)

## Support

For issues and questions:
1. Check ESP-IDF documentation
2. Review ESP32 forums
3. Verify hardware connections
4. Enable debug logging: `idf.py menuconfig` → Component config → Log output

---

**Version**: 1.0  
**Last Updated**: 2025-10-05
