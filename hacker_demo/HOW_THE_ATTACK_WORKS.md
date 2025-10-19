# How the MITM Attack Works - Technical Explanation

## 🎯 Attack Overview

This document explains **exactly** how the Man-in-the-Middle (MITM) attack works at a technical level.

## 📡 Normal Firmware Update Flow

### Without Attack:

```
Step 1: ESP32 DNS Lookup
ESP32: "What's the IP of update.server.com?"
DNS: "It's 192.168.1.100"

Step 2: TCP Connection
ESP32 → 192.168.1.100:8000
[SYN] → [SYN-ACK] → [ACK]
Connection established

Step 3: HTTP Request
ESP32 → Server:
GET /firmware.bin HTTP/1.1
Host: 192.168.1.100:8000
User-Agent: ESP32HTTPClient

Step 4: HTTP Response
Server → ESP32:
HTTP/1.1 200 OK
Content-Type: application/octet-stream
Content-Length: 245792

[Binary firmware data...]

Step 5: Firmware Installation
ESP32:
- Receives firmware
- Writes to OTA partition
- Validates
- Reboots
```

**Security**: Direct connection, but HTTP is unencrypted

## 🔴 MITM Attack Flow

### With Attack (What We Do):

```
Step 1: Network Positioning
Attacker: Runs proxy on 192.168.1.100:8080
ESP32: Configured to use 192.168.1.100:8080 (not 8000!)

Step 2: ESP32 Connects to Attacker
ESP32 → Attacker's Proxy (192.168.1.100:8080)
[SYN] → [SYN-ACK] → [ACK]
Connection established with ATTACKER, not real server!

Step 3: ESP32 Sends Request (to attacker)
ESP32 → Attacker:
GET /firmware.bin HTTP/1.1
Host: 192.168.1.100:8080  ← Goes to attacker!
User-Agent: ESP32HTTPClient

Step 4: Attacker Intercepts
Attacker receives request:
- Logs everything
- Sees all headers
- Knows what ESP32 wants

Step 5: Attacker Fetches Real Firmware
Attacker → Real Server (192.168.1.100:8000):
GET /firmware.bin HTTP/1.1
Host: 192.168.1.100:8000

Real Server → Attacker:
HTTP/1.1 200 OK
[Binary firmware data...]

Step 6: Attacker Analyzes
Attacker:
- Saves firmware copy
- Calculates hash
- Checks if encrypted
- Decides what to do

Step 7: Attacker Responds to ESP32
Option A (Monitor): Send original
Option B (Inject): Send modified
Option C (Block): Send error

Step 8: ESP32 Receives
ESP32 receives firmware from attacker
ESP32 thinks it came from real server!
```

**Problem**: ESP32 has no way to know it's talking to attacker

## 🔍 Technical Details

### 1. Why ESP32 Connects to Attacker

```cpp
// In ESP32 code:
const char* OTA_FIRMWARE_URL = "http://192.168.1.100:8080/firmware.bin";
                                                        ^^^^
                                                        Port 8080 = Attacker
                                                        Port 8000 = Real server
```

**Key Point**: ESP32 trusts whatever URL you give it!

### 2. How Attacker Intercepts

```python
# Python MITM Proxy
class MITMProxy(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        # ESP32 sends request here
        request_path = self.path  # e.g., "/firmware.bin"
        
        # Attacker fetches from real server
        real_url = f"http://{REAL_SERVER}:8000{request_path}"
        firmware = urllib.request.urlopen(real_url).read()
        
        # Attacker has the firmware!
        # Can view, modify, or block it
        
        # Send to ESP32
        self.wfile.write(firmware)  # Original or modified
```

**Key Point**: Proxy sits in the middle, sees everything

### 3. Why HTTP is Vulnerable

```
HTTP Request (Plaintext):
┌─────────────────────────────────────┐
│ GET /firmware.bin HTTP/1.1          │
│ Host: 192.168.1.100:8080            │ ← All visible!
│ User-Agent: ESP32HTTPClient         │
│                                     │
│ [Firmware data in plaintext]        │ ← Can be read/modified!
└─────────────────────────────────────┘

HTTPS Request (Encrypted):
┌─────────────────────────────────────┐
│ [Encrypted TLS handshake]           │ ← Cannot read
│ [Encrypted HTTP headers]            │ ← Cannot read
│ [Encrypted firmware data]           │ ← Cannot read/modify
└─────────────────────────────────────┘
```

**Key Point**: HTTP = plaintext, HTTPS = encrypted

### 4. How Firmware Encryption Protects

```
Unencrypted Firmware:
┌─────────────────────────────────────┐
│ 0x00: E9 03 00 00 00 00 00 00       │ ← ESP32 header
│ 0x08: 40 08 05 B4 ...               │ ← Entry point
│ 0x10: [Readable code]               │ ← Can analyze
│ 0x20: "WiFi password: 12345"        │ ← Secrets exposed!
└─────────────────────────────────────┘
Attacker can:
✓ Read code
✓ Find vulnerabilities
✓ Extract secrets
✓ Modify and re-send

Encrypted Firmware:
┌─────────────────────────────────────┐
│ 0x00: A7 F3 9B 2C 8E 1D 4F 6A       │ ← Random-looking
│ 0x08: 3C 7E 9F 1A B5 D2 8C 4E       │ ← Cannot read
│ 0x10: [Encrypted data]              │ ← Meaningless
│ 0x20: 6F 2D 8A 3B C9 E4 7F 1C       │ ← No secrets visible
└─────────────────────────────────────┘
Attacker can:
✗ Cannot read code
✗ Cannot find vulnerabilities
✗ Cannot extract secrets
✓ Can intercept (but useless)
✗ Cannot modify (breaks encryption)
```

**Key Point**: Encryption makes intercepted data useless

### 5. Attack Detection by Entropy

```python
def calculate_entropy(data):
    """
    Shannon entropy: measures randomness
    0.0 = all same byte (e.g., all zeros)
    8.0 = perfectly random (encrypted data)
    """
    entropy = 0
    for byte_value in range(256):
        probability = data.count(byte_value) / len(data)
        if probability > 0:
            entropy += -probability * log2(probability)
    return entropy

# Results:
# Unencrypted firmware: 4.5 - 6.5 (structured data)
# Encrypted firmware: 7.5 - 8.0 (random-looking)
# Compressed firmware: 6.0 - 7.5 (somewhat random)
```

**Key Point**: High entropy = likely encrypted

## 🛡️ Defense Mechanisms

### 1. Firmware Encryption (Your Project)

```cpp
// Encryption protects integrity
if (!aes_decrypt(encrypted_data, key, iv, decrypted_data)) {
    // Decryption failed = firmware was tampered
    reject_firmware();
}
```

**Protection**: Modified firmware won't decrypt correctly

### 2. HTTPS/TLS (Transport Layer)

```
With HTTPS:
ESP32 → [TLS Encrypted Tunnel] → Server

Attacker sees:
- Encrypted handshake
- Encrypted HTTP headers
- Encrypted firmware data
- Cannot read or modify anything
```

**Protection**: Prevents interception at network level

### 3. Firmware Signing (Authentication)

```cpp
// Digital signature verifies authenticity
if (!verify_signature(firmware, signature, public_key)) {
    // Signature invalid = not from trusted source
    reject_firmware();
}
```

**Protection**: Ensures firmware came from trusted source

### 4. Certificate Pinning (Advanced)

```cpp
// Only accept specific certificate
if (server_cert != expected_cert) {
    // Wrong certificate = MITM attack!
    reject_connection();
}
```

**Protection**: Prevents fake certificates

## 🔬 Attack Variations

### Variation 1: ARP Spoofing (More Advanced)

```
Normal:
ESP32 asks: "Who has 192.168.1.100?"
Router: "I do! MAC: AA:BB:CC:DD:EE:FF"
ESP32 → Router → Server

With ARP Spoofing:
ESP32 asks: "Who has 192.168.1.100?"
Attacker: "I do! MAC: 11:22:33:44:55:66" (lying!)
ESP32 → Attacker → Server
```

**More dangerous**: ESP32 doesn't need to be reconfigured

### Variation 2: DNS Spoofing

```
Normal:
ESP32: "What's IP of update.server.com?"
DNS: "192.168.1.100"

With DNS Spoofing:
ESP32: "What's IP of update.server.com?"
Attacker's DNS: "192.168.1.200" (attacker's IP!)
```

**More dangerous**: Works with domain names

### Variation 3: WiFi Access Point

```
Attacker creates fake WiFi:
SSID: "Company_WiFi" (looks legitimate)
ESP32 connects automatically
All traffic goes through attacker
```

**Most dangerous**: Complete network control

## 📊 Attack Success Matrix

| Defense Layer | Monitor | Inject | Block |
|---------------|---------|--------|-------|
| None | ✅ Success | ✅ Success | ✅ Success |
| Encryption | ⚠️ Intercept only | ❌ Fails | ✅ Success |
| HTTPS | ❌ Fails | ❌ Fails | ⚠️ Partial |
| Signing | ⚠️ Intercept only | ❌ Fails | ✅ Success |
| All Three | ❌ Fails | ❌ Fails | ⚠️ DoS only |

## 🎓 Key Takeaways

1. **HTTP is transparent** - Everything visible to MITM
2. **Encryption protects data** - Cannot read/modify
3. **HTTPS protects transport** - Cannot intercept
4. **Signing proves authenticity** - Cannot fake
5. **Defense in depth** - Multiple layers needed

## 🔗 Real-World Examples

### Case Study 1: Smart Light Bulb Hack
- Unencrypted firmware updates
- MITM attack on home WiFi
- Attacker gains control of lights
- Can spy via microphone (if present)

### Case Study 2: Industrial Sensor Compromise
- HTTP firmware updates
- Corporate network MITM
- Modified firmware exfiltrates data
- Production secrets stolen

### Case Study 3: Medical Device Attack
- Unencrypted updates over WiFi
- Hospital network compromise
- Firmware modified to give wrong readings
- Patient safety at risk

**All prevented by**: Encryption + HTTPS + Signing

## 📚 Further Reading

- **RFC 2818**: HTTP Over TLS (HTTPS)
- **RFC 5246**: TLS Protocol
- **NIST SP 800-57**: Key Management
- **OWASP**: IoT Security Top 10

---

**Document Version**: 1.0  
**Technical Level**: Intermediate  
**Prerequisites**: Basic networking knowledge
