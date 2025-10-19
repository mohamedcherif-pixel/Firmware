# 📄 File Reference - Complete Documentation

This document provides detailed documentation for every file in the project.

---

## 📂 ESP32 Firmware Files

### `ESP32_Encrypted_Firmware.ino`

**Location:** `ESP32_Encrypted_Firmware/ESP32_Encrypted_Firmware.ino`  
**Type:** Arduino Sketch (C++)  
**Purpose:** Main application entry point

#### Configuration Constants

```cpp
// WiFi Configuration
const char* WIFI_SSID = "TOPNET_2FB0";
const char* WIFI_PASSWORD = "3m3smnb68l";

// OTA URLs
const char* OTA_FIRMWARE_URL = "http://192.168.1.22:8080/firmware_encrypted.bin";
const char* VERSION_CHECK_URL = "http://192.168.1.22:8000/version.txt";

// Firmware Version
#define FIRMWARE_VERSION 1  // Increment for each release

// AES-256 Key (32 bytes) - MUST match encryption key!
const uint8_t aes_key[32] = {
    0x30, 0xee, 0x86, 0xb5, 0x2f, 0xac, 0xf1, 0xfe,
    0x3d, 0xf8, 0x74, 0x91, 0xd0, 0xb7, 0x79, 0xc9,
    0x7c, 0x06, 0x68, 0xe9, 0x39, 0x18, 0xef, 0x04,
    0x80, 0x66, 0x0e, 0x97, 0x2d, 0xb2, 0x40, 0xa2
};

// IV Placeholder (actual IV read from firmware file)
const uint8_t aes_iv[16] = {
    0x6e, 0x75, 0xd6, 0xe9, 0x5c, 0x92, 0x5e, 0x15,
    0xd4, 0xbd, 0x42, 0x73, 0x10, 0x11, 0xb6, 0x51
};
```

#### Functions

**`setup()`**
```cpp
void setup()
```
- Initializes serial communication (115200 baud)
- Initializes crypto subsystem
- Initializes OTA subsystem
- Runs crypto function tests
- Verifies current firmware
- Connects to WiFi
- Checks for firmware updates
- Performs OTA update if new version available

**Flow:**
```
1. Serial.begin(115200)
2. crypto_init()
3. ota_init()
4. testCryptoFunctions()
5. ota_verify_firmware()
6. wifi_connect(SSID, PASSWORD)
7. check_server_version()
8. if (new_version) → ota_update_from_url()
9. Continue to loop()
```

**`loop()`**
```cpp
void loop()
```
- Main application loop
- Prints status every 5 seconds
- Shows firmware version and uptime

**`testCryptoFunctions()`**
```cpp
void testCryptoFunctions()
```
- Tests AES encryption/decryption
- Verifies data integrity
- Prints test results to serial

**`check_server_version()`**
```cpp
int check_server_version()
```
- Queries server for latest version
- Returns version number or -1 on error
- Uses HTTP GET to VERSION_CHECK_URL

**Returns:**
- `> 0`: Server version number
- `-1`: Error (connection failed, invalid response)

#### Configuration Guide

**Step 1: WiFi Credentials**
```cpp
const char* WIFI_SSID = "YourNetworkName";      // Change this
const char* WIFI_PASSWORD = "YourPassword";      // Change this
```

**Step 2: Server URLs**
```cpp
// For direct connection (production)
const char* OTA_FIRMWARE_URL = "http://192.168.1.22:8000/firmware_encrypted.bin";

// For MITM demo (testing)
const char* OTA_FIRMWARE_URL = "http://192.168.1.22:8080/firmware_encrypted.bin";
                                                        ^^^^
                                                    Port 8080 = MITM proxy
```

**Step 3: Encryption Key**
```cpp
// Copy from tools/encryption_key.h after running encrypt_firmware.py
const uint8_t aes_key[32] = {
    // Paste 32 bytes here
};
```

**Step 4: Version Number**
```cpp
#define FIRMWARE_VERSION 1  // Increment: 1 → 2 → 3 → ...
```

---

### `ota_update.cpp` / `ota_update.h`

**Location:** `ESP32_Encrypted_Firmware/ota_update.cpp`  
**Type:** C++ Implementation  
**Purpose:** OTA update functionality with streaming decryption

#### Constants

```cpp
#define BUFFER_SIZE 1024  // Chunk size for streaming
```

#### Functions

**`ota_init()`**
```cpp
void ota_init()
```
- Initializes OTA subsystem
- Prints partition information
- Shows available space

**Output:**
```
=== OTA Initialization ===
Partition scheme: default
Running partition: ota_0
Next update partition: ota_1
Available space: 1310720 bytes
```

---

**`ota_verify_firmware()`**
```cpp
void ota_verify_firmware()
```
- Verifies current firmware integrity
- Calculates SHA-256 hash
- Displays firmware information

**Output:**
```
=== Firmware Verification ===
Current partition: ota_0
Firmware size: 1030256 bytes
SHA-256: 0aad7c0013c3e7dafe43d8eef91463d2...
```

---

**`ota_update_from_url()`**
```cpp
bool ota_update_from_url(const char* url, const uint8_t* aes_key)
```

**Parameters:**
- `url`: HTTP URL to encrypted firmware
- `aes_key`: 32-byte AES-256 key

**Returns:**
- `true`: Update successful
- `false`: Update failed

**Process:**

```
PHASE 1: HTTP CONNECTION
├─ HTTPClient.begin(url)
├─ GET request
├─ Check response code (200 OK)
└─ Get content length

PHASE 2: IV EXTRACTION
├─ Read first 16 bytes from stream
├─ Store as IV
└─ Calculate encrypted_size = total - 16

PHASE 3: DECRYPTION SETUP
├─ aes_decrypt_stream_init(key, iv)
├─ Update.begin(expected_size)
├─ Allocate 1KB buffers
└─ Initialize progress tracking

PHASE 4: STREAMING DOWNLOAD & DECRYPT
Loop while data remaining:
  ├─ Read 1024 encrypted bytes
  ├─ Decrypt chunk with AES-CBC
  ├─ Remove padding (last chunk only)
  ├─ Write to flash partition
  ├─ Update progress
  └─ Check for errors

PHASE 5: FINALIZATION
├─ Free buffers
├─ Close HTTP connection
├─ Update.end() - Mark partition valid
└─ Return success/failure
```

**Error Handling:**
- HTTP connection failures
- Decryption errors
- Flash write errors
- Timeout (10 seconds no data)
- Memory allocation failures

**Progress Output:**
```
[OTA] Progress: 51200/1030256 bytes (5.0%) - Speed: 102.4 KB/s
[OTA] Progress: 102400/1030256 bytes (10.0%) - Speed: 128.0 KB/s
...
[OTA] Progress: 1030256/1030256 bytes (100.0%)
```

---

### `crypto_utils.cpp` / `crypto_utils.h`

**Location:** `ESP32_Encrypted_Firmware/crypto_utils.cpp`  
**Type:** C++ Implementation  
**Purpose:** Cryptographic utilities using mbedtls

#### Constants

```cpp
#define AES_KEY_SIZE 32      // 256 bits
#define AES_IV_SIZE 16       // 128 bits
#define AES_BLOCK_SIZE 16    // Always 16 for AES
```

#### Data Structures

```cpp
typedef struct {
    mbedtls_aes_context aes;  // mbedtls AES context
    uint8_t iv[AES_IV_SIZE];  // Initialization vector
    bool initialized;          // Context state
} aes_stream_context_t;
```

#### Functions

**`crypto_init()`**
```cpp
void crypto_init()
```
- Initializes cryptographic subsystem
- Prints library version
- Sets up mbedtls

---

**`aes_encrypt_data()`**
```cpp
bool aes_encrypt_data(const uint8_t* plaintext, size_t plaintext_len,
                      uint8_t* ciphertext, size_t* ciphertext_len,
                      const uint8_t* key, const uint8_t* iv)
```

**Parameters:**
- `plaintext`: Input data
- `plaintext_len`: Input length
- `ciphertext`: Output buffer
- `ciphertext_len`: Output length (updated)
- `key`: 32-byte AES key
- `iv`: 16-byte initialization vector

**Returns:**
- `true`: Encryption successful
- `false`: Encryption failed

**Notes:**
- Adds PKCS7 padding automatically
- Output length = input length + padding (1-16 bytes)
- Used for testing only

---

**`aes_decrypt_data()`**
```cpp
bool aes_decrypt_data(const uint8_t* ciphertext, size_t ciphertext_len,
                      uint8_t* plaintext, size_t* plaintext_len,
                      const uint8_t* key, const uint8_t* iv)
```

**Parameters:**
- `ciphertext`: Encrypted data
- `ciphertext_len`: Encrypted length (must be multiple of 16)
- `plaintext`: Output buffer
- `plaintext_len`: Output length (updated)
- `key`: 32-byte AES key
- `iv`: 16-byte initialization vector

**Returns:**
- `true`: Decryption successful
- `false`: Decryption failed

**Notes:**
- Removes PKCS7 padding automatically
- Input must be multiple of 16 bytes
- Used for testing only

---

**`aes_decrypt_stream_init()`**
```cpp
bool aes_decrypt_stream_init(aes_stream_context_t* ctx,
                             const uint8_t* key,
                             const uint8_t* iv)
```

**Parameters:**
- `ctx`: Stream context to initialize
- `key`: 32-byte AES key
- `iv`: 16-byte initialization vector

**Returns:**
- `true`: Initialization successful
- `false`: Initialization failed

**Purpose:** Initialize streaming decryption for OTA updates

---

**`aes_decrypt_stream_update()`**
```cpp
bool aes_decrypt_stream_update(aes_stream_context_t* ctx,
                               const uint8_t* ciphertext, size_t ciphertext_len,
                               uint8_t* plaintext, size_t* plaintext_len)
```

**Parameters:**
- `ctx`: Initialized stream context
- `ciphertext`: Encrypted chunk
- `ciphertext_len`: Chunk length (must be multiple of 16)
- `plaintext`: Output buffer
- `plaintext_len`: Output length (updated)

**Returns:**
- `true`: Decryption successful
- `false`: Decryption failed

**Notes:**
- Maintains CBC state across chunks
- Input must be multiple of 16 bytes (except last chunk)
- Call multiple times for streaming

---

**`aes_decrypt_stream_free()`**
```cpp
void aes_decrypt_stream_free(aes_stream_context_t* ctx)
```

**Parameters:**
- `ctx`: Stream context to free

**Purpose:** Clean up streaming decryption resources

---

### `wifi_manager.cpp` / `wifi_manager.h`

**Location:** `ESP32_Encrypted_Firmware/wifi_manager.cpp`  
**Type:** C++ Implementation  
**Purpose:** WiFi connection management

#### Functions

**`wifi_connect()`**
```cpp
bool wifi_connect(const char* ssid, const char* password)
```

**Parameters:**
- `ssid`: WiFi network name
- `password`: WiFi password

**Returns:**
- `true`: Connected successfully
- `false`: Connection failed

**Process:**
```
1. WiFi.disconnect()
2. WiFi.mode(WIFI_STA)
3. WiFi.begin(ssid, password)
4. Wait up to 10 seconds
5. Check connection status
6. Print IP address if connected
7. Return success/failure
```

**Output:**
```
Connecting to WiFi...
✓ WiFi connected successfully
IP Address: 192.168.1.29
```

---

## 🔧 Encryption Tools

### `encrypt_firmware.py`

**Location:** `tools/encrypt_firmware.py`  
**Type:** Python Script  
**Purpose:** Encrypt/decrypt ESP32 firmware binaries

#### Usage

**Encrypt (Generate New Key):**
```bash
python encrypt_firmware.py encrypt -i firmware.bin -o firmware_encrypted.bin
```

**Encrypt (Use Existing Key):**
```bash
python encrypt_firmware.py encrypt -i firmware.bin -o firmware_encrypted.bin -k aes_key.bin
```

**Decrypt:**
```bash
python encrypt_firmware.py decrypt -i firmware_encrypted.bin -o firmware_decrypted.bin -k aes_key.bin
```

#### Command-Line Arguments

| Argument | Short | Description | Required |
|----------|-------|-------------|----------|
| `encrypt` | - | Encryption mode | Yes (mode) |
| `decrypt` | - | Decryption mode | Yes (mode) |
| `--input` | `-i` | Input file path | Yes |
| `--output` | `-o` | Output file path | Yes |
| `--key` | `-k` | Key file (reuse existing) | No |

#### Encryption Process

```python
def encrypt_firmware(input_file, output_file, key_file=None):
    # 1. Read plaintext firmware
    with open(input_file, 'rb') as f:
        plaintext = f.read()
    
    # 2. Load or generate key
    if key_file and os.path.exists(key_file):
        key = load_key(key_file)
    else:
        key = os.urandom(32)  # Generate random 256-bit key
        save_key(key, 'aes_key.bin')
    
    # 3. Generate random IV
    iv = os.urandom(16)
    
    # 4. Add PKCS7 padding
    padded = add_pkcs7_padding(plaintext, 16)
    
    # 5. Encrypt with AES-256-CBC
    cipher = AES.new(key, AES.MODE_CBC, iv)
    ciphertext = cipher.encrypt(padded)
    
    # 6. Write output: [IV][Ciphertext]
    with open(output_file, 'wb') as f:
        f.write(iv)
        f.write(ciphertext)
    
    # 7. Generate metadata
    save_metadata(output_file, key, iv, plaintext, ciphertext)
    
    # 8. Generate C header
    generate_c_header(key, iv)
```

#### Output Files

**`firmware_encrypted.bin`**
- Format: `[IV: 16 bytes][Encrypted Data: N bytes]`
- Size: Original size + padding (1-16 bytes) + 16 bytes (IV)

**`aes_key.bin`**
- Format: Raw binary (32 bytes)
- **CRITICAL:** Keep secure! Losing this = can't update devices

**`firmware_encrypted.bin.meta`**
- Format: JSON
- Contains: Sizes, hashes, key, IV, timestamp

Example:
```json
{
    "original_size": 1030256,
    "encrypted_size": 1030272,
    "iv": "6e75d6e95c925e15d4bd427310...",
    "key": "30ee86b52facf1fe3df87491d0...",
    "algorithm": "AES-256-CBC",
    "timestamp": "2025-10-10T17:51:20",
    "firmware_sha256": "0aad7c0013c3e7dafe...",
    "encrypted_sha256": "54f93d992c1ff82c14..."
}
```

**`encryption_key.h`**
- Format: C header file
- Contains: Key and IV as C arrays
- Used to update ESP32 firmware

Example:
```c
#ifndef ENCRYPTION_KEY_H
#define ENCRYPTION_KEY_H

static const uint8_t AES_KEY[32] = {
    0x30, 0xee, 0x86, 0xb5, ...
};

static const uint8_t AES_IV[16] = {
    0x6e, 0x75, 0xd6, 0xe9, ...
};

#endif
```

---

### `aes_key.bin`

**Location:** `tools/aes_key.bin`  
**Type:** Binary File  
**Size:** 32 bytes  
**Purpose:** Master encryption key

**Security:**
```
✅ DO:
- Store in secure location
- Backup to encrypted storage
- Use file permissions (chmod 600)
- Use HSM in production

❌ DON'T:
- Commit to Git (.gitignore it!)
- Share via email/chat
- Store in plaintext
- Hardcode in public repos
```

**Key Rotation:**
If compromised:
1. Generate new key
2. Update ESP32 firmware with new key
3. Upload via USB (one-time)
4. Re-encrypt all future firmware
5. Devices can now OTA update with new key

---

### `version.txt`

**Location:** `tools/version.txt`  
**Type:** Text File  
**Format:** Single integer

**Example:**
```
2
```

**Usage:**
```bash
# Read version
cat version.txt

# Update version
echo 3 > version.txt
```

**Version Check Logic:**
```cpp
int server_version = check_server_version();  // Reads version.txt
int device_version = FIRMWARE_VERSION;         // Hardcoded in firmware

if (server_version > device_version) {
    // New version available → Update
} else {
    // Already latest → Skip
}
```

---

## 🎭 MITM Attack Tools

### `real_mitm_attack.py`

**Location:** `hacker_demo/real_mitm_attack.py`  
**Type:** Python Script  
**Purpose:** Man-In-The-Middle attack demonstration

**⚠️ WARNING:** Educational use only! Unauthorized interception is illegal.

#### Configuration

```python
# Proxy settings
PROXY_HOST = '0.0.0.0'        # Listen on all interfaces
PROXY_PORT = 8080             # Proxy port (ESP32 connects here)

# Target settings
TARGET_HOST = '192.168.1.22'  # Real server IP
TARGET_PORT = 8000            # Real server port

# Attack mode
MODE = 'monitor'  # Options: 'monitor', 'inject', 'block'

# Malicious firmware (for inject mode)
MALICIOUS_FIRMWARE = 'malicious_firmware.bin'
```

#### Attack Modes

**Mode 1: Monitor (Passive)**
```python
MODE = 'monitor'
```
- Intercepts and logs all traffic
- Forwards unmodified to real server
- Analyzes firmware (entropy, size, format)
- Saves copy for offline analysis
- **Use case:** Traffic analysis, learning

**Mode 2: Inject (Active - Malicious)**
```python
MODE = 'inject'
```
- Intercepts requests
- Serves malicious firmware instead
- Can inject backdoors, malware
- **Use case:** Demonstrate attack impact
- **⚠️ Extremely dangerous!**

**Mode 3: Block (Denial of Service)**
```python
MODE = 'block'
```
- Intercepts requests
- Returns HTTP 404 Not Found
- Prevents firmware updates
- **Use case:** Test error handling

#### Running the Attack

```bash
# Terminal 1: Start real server
cd tools
python -m http.server 8000

# Terminal 2: Start MITM proxy
cd hacker_demo
python real_mitm_attack.py

# Terminal 3: Monitor ESP32
# Arduino IDE → Serial Monitor
```

#### Output Example

```
🚀 MITM Proxy Server Started
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📡 Listening on: 0.0.0.0:8080
🎯 Forwarding to: 192.168.1.22:8000
⚠️  MODE: MONITOR (Passive interception)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

🎯 REAL REQUEST INTERCEPTED!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📡 Target: /firmware_encrypted.bin
📍 From: 192.168.1.29:54321
🔍 Method: GET
⏰ Time: 2025-10-10 18:00:00

📊 FIRMWARE ANALYSIS:
├─ Size: 1,030,272 bytes
├─ Entropy: 7.95/8.0
├─ Assessment: High entropy - likely encrypted ✓
└─ ESP32 Header: Not detected (encrypted)

💾 Firmware saved: intercepted_firmware_20251010_180000.bin

✅ Forwarding to real server...
✅ Response forwarded to client
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

### `network_helper.py`

**Location:** `hacker_demo/network_helper.py`  
**Type:** Python Module  
**Purpose:** Network utilities for attack demonstrations

#### Functions

**`get_local_ip()`**
```python
def get_local_ip() -> str
```
- Returns local IP address
- Used to configure proxy

**`calculate_entropy()`**
```python
def calculate_entropy(data: bytes) -> float
```
- Calculates Shannon entropy
- Range: 0.0 (no randomness) to 8.0 (perfect randomness)
- Encrypted data: ~7.9-8.0
- Plaintext: ~4.0-6.0

**`is_likely_encrypted()`**
```python
def is_likely_encrypted(data: bytes, threshold: float = 7.5) -> bool
```
- Detects if data is encrypted
- Uses entropy threshold

**`analyze_firmware()`**
```python
def analyze_firmware(data: bytes) -> dict
```
- Analyzes firmware structure
- Checks for ESP32 magic bytes (`0xE9`)
- Detects encryption
- Returns analysis report

**Example:**
```python
{
    'size': 1030272,
    'entropy': 7.95,
    'likely_encrypted': True,
    'has_esp32_header': False,
    'analysis': 'High entropy - likely encrypted'
}
```

---

**Next:** [Usage Scenarios →](03_SCENARIOS.md)
