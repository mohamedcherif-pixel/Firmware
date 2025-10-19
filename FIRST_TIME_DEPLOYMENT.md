# First-Time GitHub Deployment Guide

**Repository:** https://github.com/mohamedcherif-pixel/Firmware  
**Date:** October 18, 2025

---

## 🚀 Quick Start (5 minutes)

### Step 1: Prepare Firmware Files

Open PowerShell and navigate to your project:

```powershell
cd "c:\Users\cheri\OneDrive\Desktop\esp32cryptcode\tools"
```

### Step 2: Build & Encrypt Firmware

**In Arduino IDE:**
1. Open `ESP32_Encrypted_Firmware.ino`
2. Click **Sketch → Export Compiled Binary**
3. Wait for compilation to complete
4. File saved as: `ESP32_Encrypted_Firmware.ino.esp32.bin`

**Encrypt the firmware:**

```powershell
python encrypt_firmware.py encrypt `
    -i "..\ESP32_Encrypted_Firmware\ESP32_Encrypted_Firmware.ino.esp32.bin" `
    -o firmware_encrypted.bin
```

**Output files created:**
- ✅ `firmware_encrypted.bin` - Encrypted firmware (upload this!)
- ✅ `aes_key.bin` - Encryption key (keep secret!)
- ✅ `encryption_key.h` - Key header file
- ✅ `firmware_encrypted.bin.meta` - Metadata

### Step 3: Create Version File

```powershell
echo "1" > version.txt
```

### Step 4: Authenticate with GitHub CLI

```powershell
gh auth login
```

**Follow the prompts:**
1. Choose: **GitHub.com**
2. Choose: **HTTPS**
3. Choose: **Y** (to authenticate with your browser)
4. Click the link and authorize
5. Return to terminal

**Verify authentication:**

```powershell
gh auth status
```

You should see:
```
Logged in to github.com as mohamedcherif-pixel
```

### Step 5: Deploy to GitHub

```powershell
.\deploy_to_github.bat
```

**What the script does:**
1. ✅ Checks for `firmware_encrypted.bin` and `version.txt`
2. ✅ Creates a new GitHub release named `v1`
3. ✅ Uploads both files to the release
4. ✅ Makes them downloadable

---

## ✅ Verify Deployment

### Check Release in Browser

Open: https://github.com/mohamedcherif-pixel/Firmware/releases

You should see:
- **Release v1** with tag `v1`
- Files: `firmware_encrypted.bin` and `version.txt`

### Test Download URLs

```powershell
# Test firmware URL
curl -I https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/firmware_encrypted.bin

# Test version URL
curl https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt
```

Both should return **HTTP 200** ✅

---

## 📱 Update ESP32 Code

Your ESP32 code is **already configured** for GitHub! Check:

```cpp
// In ESP32_Encrypted_Firmware.ino around line 27-28:
const char* OTA_FIRMWARE_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/firmware_encrypted.bin";
const char* VERSION_CHECK_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt";
```

### Upload to ESP32

1. Open `ESP32_Encrypted_Firmware.ino` in Arduino IDE
2. Select: **Tools → Board → ESP32 Dev Module**
3. Select: **Tools → Port → COM3** (or your port)
4. Click **Upload** (or Ctrl+U)
5. Wait for upload to complete
6. Open **Serial Monitor** (Ctrl+Shift+M) at **115200 baud**

---

## 🔍 Test OTA Update

### Monitor Serial Output

When ESP32 boots, you should see:

```
=== ESP32 Encrypted Firmware Update System ===
Firmware Version: 1
================================================

Connecting to WiFi...
✓ WiFi connected successfully
IP Address: 192.168.1.29

=== Checking for firmware updates ===
Current firmware version: 1
Checking: https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt
Server firmware version: 1
✓ Already running latest version
```

**This means:**
- ✅ ESP32 connected to WiFi
- ✅ Downloaded version file from GitHub
- ✅ Compared versions
- ✅ Already on latest (no update needed)

---

## 🔄 Deploy Version 2

When you want to release a new firmware version:

### Step 1: Update Firmware Version

Edit `ESP32_Encrypted_Firmware.ino` line 47:

```cpp
#define FIRMWARE_VERSION 2  // Change from 1 to 2
```

### Step 2: Build & Encrypt

```powershell
cd tools

# Build in Arduino IDE first (Sketch → Export Compiled Binary)

# Then encrypt:
python encrypt_firmware.py encrypt `
    -i "..\ESP32_Encrypted_Firmware\ESP32_Encrypted_Firmware.ino.esp32.bin" `
    -o firmware_encrypted.bin `
    -k aes_key.bin
```

⚠️ **IMPORTANT:** Use same `-k aes_key.bin` key! Otherwise ESP32 won't decrypt!

### Step 3: Update Version File

```powershell
echo "2" > version.txt
```

### Step 4: Deploy

```powershell
.\deploy_to_github.bat
```

Creates release `v2` automatically!

### Step 5: Upload New Code to ESP32

1. Update `#define FIRMWARE_VERSION 2` in sketch
2. Upload to ESP32 (Ctrl+U)
3. Serial monitor shows:

```
Current firmware version: 2
Checking: https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt
Server firmware version: 2
✓ Already running latest version
```

---

## 📊 Complete Workflow Example

```
Day 1: Initial Release
├─ Build firmware v1
├─ Encrypt → firmware_encrypted.bin
├─ Create version.txt = "1"
├─ Deploy v1 to GitHub
└─ Upload ESP32 code (FIRMWARE_VERSION = 1)

Day 5: Bug Fix Release
├─ Update ESP32 code with fixes
├─ Change FIRMWARE_VERSION = 2
├─ Build firmware v2
├─ Encrypt → firmware_encrypted.bin
├─ Update version.txt = "2"
├─ Deploy v2 to GitHub
└─ Upload ESP32 code (FIRMWARE_VERSION = 2)
    → ESP32 detects new version 2 on GitHub
    → Auto-downloads and updates!
```

---

## 🐛 Troubleshooting

### Error: "GitHub CLI not found"

```powershell
winget install GitHub.cli
```

Then restart PowerShell and try again.

### Error: "Permission denied" on deploy script

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

Then retry:

```powershell
.\deploy_to_github.bat
```

### Error: "Release already exists"

If you try to deploy same version twice:

```powershell
# Delete old release
gh release delete v1 --repo mohamedcherif-pixel/Firmware --yes

# Deploy again
.\deploy_to_github.bat
```

### Error: "Could not authenticate"

```powershell
# Re-authenticate
gh auth logout
gh auth login
```

### ESP32 shows "Failed to check server version"

**Check:**
1. Is WiFi connected? (Serial output shows IP?)
2. Is URL correct? (Check line 27-28 in sketch)
3. Are files on GitHub? (Check https://github.com/mohamedcherif-pixel/Firmware/releases)
4. Does ESP32 have internet? (Can it ping 8.8.8.8?)

**Test manually:**

```powershell
# Check if URL works
curl -v https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt

# Should download a file with just "1" or "2"
```

---

## 🔐 Security Checklist

**Do:**
- ✅ Keep `aes_key.bin` in `.gitignore`
- ✅ Use same AES key for all versions
- ✅ Keep WiFi credentials in sketch only
- ✅ Use HTTPS URLs only
- ✅ Verify firmware after download

**Don't:**
- ❌ Commit `aes_key.bin` to GitHub
- ❌ Use different AES key for each version
- ❌ Share AES key with anyone
- ❌ Use HTTP URLs (always use HTTPS)
- ❌ Trust unverified downloads

---

## 📈 Next Steps

After successful first deployment:

1. **Test from different networks** - Verify OTA works outside your WiFi
2. **Add firmware signing** - Extra security layer
3. **Create release notes** - Document changes on GitHub
4. **Setup CI/CD pipeline** - Automate builds
5. **Add rollback mechanism** - Safer updates

---

## 📞 Quick Reference

### Important Paths

```
Project:  c:\Users\cheri\OneDrive\Desktop\esp32cryptcode
Tools:    ..\tools
Firmware: ..\ESP32_Encrypted_Firmware
Keys:     ..\tools\aes_key.bin
```

### Important Commands

```powershell
# Authenticate
gh auth login

# Deploy
.\deploy_to_github.bat

# List releases
gh release list --repo mohamedcherif-pixel/Firmware

# View latest release
gh release view latest --repo mohamedcherif-pixel/Firmware

# Delete release
gh release delete v1 --repo mohamedcherif-pixel/Firmware --yes
```

### Important URLs

```
Repository: https://github.com/mohamedcherif-pixel/Firmware
Releases:   https://github.com/mohamedcherif-pixel/Firmware/releases
Latest:     https://github.com/mohamedcherif-pixel/Firmware/releases/latest
```

---

## ✨ Success Indicators

You've successfully deployed when:

1. ✅ Release appears on GitHub Releases page
2. ✅ Files downloadable from browser
3. ✅ ESP32 connects to WiFi
4. ✅ Serial monitor shows successful version check
5. ✅ Can see firmware download starting

**Congratulations! 🎉 Your firmware is now deployed!**

---

**Document Version:** 1.0  
**Last Updated:** October 18, 2025  
**Repository:** https://github.com/mohamedcherif-pixel/Firmware
