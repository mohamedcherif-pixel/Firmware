+# Complete Beginner's Guide to Real MITM Attack
## Step-by-Step Tutorial for Testing ESP32 Firmware Security

⚠️ **IMPORTANT**: Only use on YOUR OWN devices and network!

## 🎯 What You'll Learn

By the end of this guide, you'll understand:
1. How real MITM attacks work
2. How to intercept actual ESP32 firmware
3. Why encryption is critical
4. How to test your security

## 📋 Prerequisites

### What You Need:
- ✅ Your ESP32 board
- ✅ Your computer (Windows)
- ✅ WiFi network (your home network)
- ✅ Python installed
- ✅ Arduino IDE with your project

### Skills Required:
- 🟢 **Beginner Level** - I'll explain everything!
- No hacking experience needed
- Basic computer skills only

## 🔍 Understanding the Attack

### The Setup:

```
Normal (Secure):
ESP32 ──────────────> Firmware Server
        (direct)

MITM Attack (What we'll do):
ESP32 ──> MITM Proxy ──> Firmware Server
          (attacker)
          intercepts here!
```

### What Happens:

1. **ESP32 requests firmware** from server
2. **Request goes through attacker's proxy** (your computer)
3. **Attacker intercepts** the firmware
4. **Attacker can**:
   - View the firmware (if unencrypted)
   - Modify it (if unencrypted)
   - Block it
5. **ESP32 receives** modified/original firmware

## 🚀 Step-by-Step Attack Tutorial

### Phase 1: Setup (10 minutes)

#### Step 1.1: Find Your Computer's IP Address

**Windows**:
1. Press `Win + R`
2. Type `cmd` and press Enter
3. Type `ipconfig` and press Enter
4. Look for "IPv4 Address" under your WiFi adapter
5. Example: `192.168.1.100`
6. **Write this down!** You'll need it.

```
Example output:
Wireless LAN adapter Wi-Fi:
   IPv4 Address. . . . . . . . . . . : 192.168.1.100  ← THIS ONE!
```

#### Step 1.2: Check ESP32 Connection

1. Open Arduino IDE
2. Upload your ESP32 firmware
3. Open Serial Monitor (Ctrl+Shift+M)
4. Check ESP32 connects to WiFi
5. Note the ESP32's IP address (it will print it)

Example output:
```
[WiFi] Connected successfully!
[WiFi] IP Address: 192.168.1.50  ← ESP32's IP
```

#### Step 1.3: Verify Network Setup

Make sure:
- ✅ Computer and ESP32 on **same WiFi network**
- ✅ You know your computer's IP
- ✅ You know ESP32's IP
- ✅ Both can ping each other

**Test ping** (optional):
```bash
# From your computer
ping 192.168.1.50  # ESP32's IP
```

### Phase 2: Start the Attack (5 minutes)

#### Step 2.1: Start Real Firmware Server

**Terminal 1** (keep this open):
```bash
cd C:\Users\cheri\OneDrive\Desktop\esp32cryptcode\tools
python simple_http_server.py
```

You should see:
```
[+] HTTP Server running on port 8000
[+] Access firmware at: http://localhost:8000/firmware_encrypted.bin
```

**Don't close this terminal!**

#### Step 2.2: Start MITM Attack Proxy

**Terminal 2** (new terminal):
```bash
cd C:\Users\cheri\OneDrive\Desktop\esp32cryptcode\hacker_demo
python real_mitm_attack.py
```

**Interactive setup**:
```
Enter firmware server IP [192.168.1.100]: 
→ Press Enter (or type your PC's IP)

Select attack mode:
  1 - Monitor (passive - just intercept and log)
  2 - Inject (active - modify firmware)
  3 - Block (DoS - prevent updates)
Choice [1]: 
→ Type 1 and press Enter (start with monitor mode)
```

You should see:
```
✓ MITM PROXY ACTIVE ON PORT 8080
📡 Waiting for ESP32 connections...
```

**Don't close this terminal!**

#### Step 2.3: Configure ESP32 to Use MITM Proxy

**In Arduino IDE**, open your sketch and change:

```cpp
// OLD (direct connection):
const char* OTA_FIRMWARE_URL = "http://192.168.1.100:8000/firmware_encrypted.bin";

// NEW (through MITM proxy):
const char* OTA_FIRMWARE_URL = "http://192.168.1.100:8080/firmware_encrypted.bin";
//                                                    ^^^^ Changed port to 8080!
```

**Important**: Use YOUR computer's IP, not mine!

#### Step 2.4: Enable OTA Update in ESP32

In your sketch, **uncomment** the WiFi OTA code:

```cpp
void setup() {
    // ... existing code ...
    
    // UNCOMMENT THIS SECTION:
    Serial.println("Connecting to WiFi...");
    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("✓ WiFi connected successfully");
        
        // Trigger OTA update
        Serial.println("\nStarting OTA update...");
        if (ota_update_from_url(OTA_FIRMWARE_URL, aes_key)) {
            Serial.println("✓ OTA update successful! Rebooting...");
            delay(1000);
            ESP.restart();
        } else {
            Serial.println("✗ OTA update failed");
        }
    }
}
```

#### Step 2.5: Upload and Watch the Attack!

1. **Upload** the modified sketch to ESP32
2. **Open Serial Monitor** (115200 baud)
3. **Watch both terminals**:
   - Terminal 1 (server): Shows firmware request
   - Terminal 2 (MITM): Shows interception!
4. **Watch Serial Monitor**: ESP32 downloading firmware

### Phase 3: See the Attack Happen! (Real-time)

#### What You'll See in Terminal 2 (MITM Proxy):

```
════════════════════════════════════════════════════════════════════
🎯 REAL REQUEST INTERCEPTED!
════════════════════════════════════════════════════════════════════
📡 Target: /firmware_encrypted.bin
📍 From: 192.168.1.50:54321
🖥️  User-Agent: ESP32HTTPClient

⚠️  FIRMWARE DOWNLOAD DETECTED!

──────────────────────────────────────────────────────────────────
🔍 INTERCEPTING REAL FIRMWARE
──────────────────────────────────────────────────────────────────
📥 Fetching from real server: http://192.168.1.100:8000/firmware_encrypted.bin
⏳ Downloading...
✓ Downloaded 245792 bytes

──────────────────────────────────────────────────────────────────
🔬 FIRMWARE ANALYSIS
──────────────────────────────────────────────────────────────────
📊 Size: 245,792 bytes (240.03 KB)
🔐 SHA-256: a3f5e8b2c9d1...
📈 Entropy: 7.89/8.0
⚠️  HIGH ENTROPY - Firmware appears ENCRYPTED
⚠️  Cannot modify without decryption key
💾 Saved to: intercepted_real_firmware_1728393847.bin

📊 MODE: MONITOR (Passive)
✓ Forwarding original firmware to ESP32
ℹ️  ESP32 will receive unmodified firmware
ℹ️  Attacker has complete copy for analysis

✓ ATTACK SUCCESSFUL - Firmware intercepted and logged
```

#### What You'll See in Serial Monitor (ESP32):

```
[WiFi] Connected successfully!
[WiFi] IP Address: 192.168.1.50

Starting OTA update...
[OTA] Starting encrypted OTA update from URL: http://192.168.1.100:8080/firmware_encrypted.bin
[OTA] Downloading encrypted firmware: 245792 bytes
[OTA] Downloaded 245792/245792 bytes
[CRYPTO] Decrypted 1024 bytes to 1008 bytes
[OTA] Written 1024/245776 bytes
...
[OTA] OTA update successful!
✓ OTA update successful! Rebooting...
```

**🎉 CONGRATULATIONS! You just performed a real MITM attack!**

## 🔬 Understanding What Happened

### Attack Flow Breakdown:

```
1. ESP32 sends request:
   GET /firmware_encrypted.bin HTTP/1.1
   Host: 192.168.1.100:8080  ← Goes to MITM proxy, not real server!

2. MITM Proxy intercepts:
   - Sees the request
   - Logs everything
   - Fetches real firmware from server

3. MITM Proxy analyzes:
   - Calculates size and hash
   - Checks entropy (encryption detection)
   - Saves copy to disk

4. MITM Proxy decides:
   - Monitor mode: Forward original
   - Inject mode: Modify firmware
   - Block mode: Return error

5. ESP32 receives:
   - Gets firmware (original or modified)
   - Attempts to decrypt
   - Installs if valid
```

### Why This Works:

1. **ESP32 trusts the IP address** you gave it
2. **HTTP has no encryption** at transport level
3. **MITM proxy is transparent** - ESP32 doesn't know
4. **Attacker sees everything** in plaintext HTTP

### Why Encryption Saves You:

1. **Firmware is encrypted** with AES-256
2. **Attacker intercepts** encrypted data
3. **Attacker cannot decrypt** without key
4. **Attacker cannot modify** - would break encryption
5. **ESP32 rejects** any tampered firmware

## 🎭 Testing Different Attack Modes

### Test 1: Monitor Mode (What we just did)

**Purpose**: Passive interception
**Result**: Firmware intercepted but not modified
**ESP32**: Updates successfully
**Attacker**: Has complete firmware copy

### Test 2: Inject Mode (Active Attack)

**Change attack mode**:
1. Stop MITM proxy (Ctrl+C in Terminal 2)
2. Run again: `python real_mitm_attack.py`
3. Choose option `2` (Inject)

**What happens**:
```
💉 MODE: INJECT (Active Attack)
⚠️  ATTEMPTING TO INJECT MALICIOUS CODE
✗ ATTACK FAILED - Firmware is encrypted
ℹ️  Forwarding original (cannot modify encrypted firmware)
```

**With encrypted firmware**: Attack fails! ✅
**With unencrypted firmware**: Attack succeeds! ❌

### Test 3: Block Mode (Denial of Service)

**Change attack mode**:
1. Stop MITM proxy
2. Run again, choose option `3` (Block)

**What happens**:
```
🚫 MODE: BLOCK (Denial of Service)
🚫 BLOCKING FIRMWARE DOWNLOAD
✓ ATTACK SUCCESSFUL - Firmware download blocked
```

**ESP32 Serial Monitor**:
```
[OTA] HTTP GET failed: 404
✗ OTA update failed
```

**Result**: ESP32 cannot update (but not damaged)

## 📊 Attack Results Comparison

### With ENCRYPTED Firmware (Your Project):

| Attack Type | Attacker Can | ESP32 Status | Security |
|-------------|--------------|--------------|----------|
| Monitor | Intercept | ✅ Updates OK | 🟢 SAFE |
| Inject | Intercept only | ✅ Updates OK | 🟢 SAFE |
| Block | Block updates | ⚠️ No update | 🟡 DoS only |

**Verdict**: ✅ **SECURE** - Encryption protects firmware integrity

### With UNENCRYPTED Firmware (Vulnerable):

| Attack Type | Attacker Can | ESP32 Status | Security |
|-------------|--------------|--------------|----------|
| Monitor | Intercept & analyze | ✅ Updates | 🔴 EXPOSED |
| Inject | Modify firmware | ❌ Compromised | 🔴 CRITICAL |
| Block | Block updates | ⚠️ No update | 🔴 DoS |

**Verdict**: ❌ **VULNERABLE** - No protection against attacks

## 🔍 Analyzing Intercepted Firmware

### Files Created by Attack:

```
hacker_demo/
├── intercepted_real_firmware_1728393847.bin  ← Real firmware copy
├── real_attack_log.txt                       ← Attack log
└── malicious_firmware_1728393847.bin         ← Modified version (if inject mode)
```

### Examine Intercepted Firmware:

**Check file size**:
```bash
dir intercepted_real_firmware_*.bin
```

**Calculate hash**:
```bash
certutil -hashfile intercepted_real_firmware_1728393847.bin SHA256
```

**Compare with original**:
```bash
# Should match if encryption worked
certutil -hashfile ..\build\firmware_encrypted.bin SHA256
```

### View Attack Log:

```bash
type real_attack_log.txt
```

You'll see:
- All intercepted requests
- Firmware analysis
- Attack decisions
- Timestamps

## 🎓 Learning Exercises

### Exercise 1: Test Encryption Protection

**Goal**: Prove encryption works

**Steps**:
1. Run attack in INJECT mode
2. Watch it fail to modify encrypted firmware
3. Check ESP32 still updates successfully

**Expected Result**: Attack fails, ESP32 protected

### Exercise 2: Test Unencrypted Vulnerability

**Goal**: See what happens without encryption

**Steps**:
1. Create unencrypted firmware (don't encrypt it)
2. Upload to server
3. Run attack in INJECT mode
4. Watch successful modification

**Expected Result**: Attack succeeds, firmware modified

**⚠️ Don't actually install modified firmware - just demonstrate!**

### Exercise 3: Network Analysis

**Goal**: Understand network traffic

**Steps**:
1. Run attack in MONITOR mode
2. Use Wireshark to capture traffic
3. See HTTP requests in plaintext
4. Compare with HTTPS (encrypted transport)

**Expected Result**: Learn why HTTPS is important

## 🛡️ Security Lessons Learned

### What This Demo Teaches:

1. ✅ **HTTP is insecure** - Everything visible to MITM
2. ✅ **Encryption is essential** - Protects firmware integrity
3. ✅ **HTTPS adds transport security** - Prevents interception
4. ✅ **Defense in depth** - Multiple security layers needed
5. ✅ **Testing is important** - Verify security actually works

### Real-World Implications:

- 🏠 **Smart home devices** - Can be compromised via WiFi
- 🏭 **Industrial IoT** - Critical infrastructure at risk
- 🚗 **Connected cars** - Safety-critical systems vulnerable
- 🏥 **Medical devices** - Patient safety concerns

### How to Protect:

1. ✅ **Always encrypt firmware** (you're doing this!)
2. ✅ **Use HTTPS** for downloads
3. ✅ **Implement firmware signing**
4. ✅ **Enable secure boot**
5. ✅ **Use WPA2/WPA3** for WiFi
6. ✅ **Regular security audits**

## 🚨 Troubleshooting

### Problem: "Cannot reach firmware server"

**Solution**:
- Check server is running (Terminal 1)
- Verify IP address is correct
- Check firewall isn't blocking port 8000
- Ping server from ESP32

### Problem: "ESP32 not connecting to proxy"

**Solution**:
- Verify OTA URL uses port 8080
- Check ESP32 and PC on same network
- Confirm PC's IP address
- Check firewall isn't blocking port 8080

### Problem: "Firmware download fails"

**Solution**:
- Check firmware file exists
- Verify file path in server
- Check file permissions
- Try direct connection first (bypass MITM)

### Problem: "Attack shows no data"

**Solution**:
- Ensure ESP32 actually requests firmware
- Check OTA code is uncommented
- Verify WiFi connection works
- Check Serial Monitor for errors

## 📝 Attack Checklist

Before starting attack:
- [ ] Firmware server running (port 8000)
- [ ] MITM proxy running (port 8080)
- [ ] Know your PC's IP address
- [ ] ESP32 connected to WiFi
- [ ] OTA URL points to MITM proxy
- [ ] Serial Monitor open
- [ ] Both terminals visible

During attack:
- [ ] Watch Terminal 2 for interception
- [ ] Watch Serial Monitor for ESP32 status
- [ ] Note attack mode being used
- [ ] Check files being created

After attack:
- [ ] Review intercepted firmware
- [ ] Check attack log
- [ ] Verify ESP32 still works
- [ ] Analyze results

## 🎯 Next Steps

### Beginner → Intermediate:

1. **Add HTTPS** to prevent transport interception
2. **Implement firmware signing** for authenticity
3. **Enable secure boot** for boot-time verification
4. **Add anti-rollback** to prevent downgrade attacks

### Advanced Topics:

1. **Certificate pinning** - Prevent fake certificates
2. **Hardware security modules** - Secure key storage
3. **Secure element integration** - ATECC608A
4. **Over-the-air key rotation** - Dynamic key updates

## 📚 Additional Resources

- ESP32 Security Features: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/
- OWASP IoT Top 10: https://owasp.org/www-project-internet-of-things/
- Network Security Basics: https://www.cloudflare.com/learning/security/

## ⚠️ Legal & Ethical Notice

**IMPORTANT**:
- ✅ Only attack YOUR OWN devices
- ✅ Only on YOUR OWN network
- ✅ For educational purposes only
- ❌ Never attack others' devices
- ❌ Never use on public networks
- ❌ Never use for malicious purposes

**Unauthorized access to computer systems is illegal!**

---

**Guide Version**: 1.0  
**Last Updated**: 2025-10-08  
**Difficulty**: Beginner-Friendly  
**Time Required**: 30 minutes  
**Success Rate**: 100% (if followed correctly)

**Questions?** Review the troubleshooting section or check the attack log for details.

**🎉 Congratulations on learning real cybersecurity testing!**
