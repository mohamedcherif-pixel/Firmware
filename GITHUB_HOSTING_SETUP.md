# GitHub Hosting Setup Guide
## Using Your GitHub Repository for Firmware Hosting

**Repository:** https://github.com/mohamedcherif-pixel/Firmware  
**Last Updated:** October 18, 2025

---

## 🎯 What This Does

Instead of running a local Python server on your computer, your encrypted firmware will be hosted on **GitHub Releases** - a free, public, always-available cloud hosting solution.

**Benefits:**
- ✅ **FREE** - No costs
- ✅ **Always available** - Works 24/7
- ✅ **Works anywhere** - ESP32 can update from any WiFi network
- ✅ **HTTPS** - Secure downloads
- ✅ **No PC needed** - Your computer can be off

---

## 📋 One-Time Setup (10 minutes)

### Step 1: Install GitHub CLI

```powershell
# Install GitHub CLI
winget install GitHub.cli

# Restart your terminal, then authenticate
gh auth login
```

Follow the prompts to log into your GitHub account.

### Step 2: Verify Your Repository

Make sure your repository exists:
- Go to: https://github.com/mohamedcherif-pixel/Firmware
- If it doesn't exist, create it (can be public or private)

---

## 🚀 Deploy Your Firmware (5 minutes)

### Step 1: Build and Encrypt Your Firmware

```powershell
# Open your project in Arduino IDE
# Build: Sketch -> Export compiled Binary

# Navigate to tools folder
cd c:\Users\cheri\OneDrive\Desktop\esp32cryptcode\tools

# Encrypt the firmware
python encrypt_firmware.py encrypt ^
    -i ..\ESP32_Encrypted_Firmware\ESP32_Encrypted_Firmware.ino.esp32.bin ^
    -o firmware_encrypted.bin

# This creates:
#   - firmware_encrypted.bin (upload this)
#   - aes_key.bin (keep this safe!)
#   - encryption_key.h
#   - firmware_encrypted.bin.meta
```

### Step 2: Create version.txt

```powershell
# Create a file with your version number
echo 1 > version.txt
```

### Step 3: Deploy to GitHub

```powershell
# Run the deployment script
.\deploy_to_github.bat

# Or manually:
gh release create v1 ^
    firmware_encrypted.bin ^
    version.txt ^
    --repo mohamedcherif-pixel/Firmware ^
    --title "Firmware v1" ^
    --notes "Encrypted ESP32 firmware - Version 1"
```

**You'll see output like:**
```
========================================
  DEPLOYMENT SUCCESSFUL!
========================================

Firmware URL:
  https://github.com/mohamedcherif-pixel/Firmware/releases/download/v1/firmware_encrypted.bin

Version URL:
  https://github.com/mohamedcherif-pixel/Firmware/releases/download/v1/version.txt
```

### Step 4: Verify in Browser

Open these URLs in your browser to verify they work:
- https://github.com/mohamedcherif-pixel/Firmware/releases/download/v1/firmware_encrypted.bin
- https://github.com/mohamedcherif-pixel/Firmware/releases/download/v1/version.txt

They should download the files.

---

## 📱 Update ESP32 Code

Your ESP32 code is **already updated**! I've configured it to use:

```cpp
// In ESP32_Encrypted_Firmware.ino:
const char* OTA_FIRMWARE_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/firmware_encrypted.bin";
const char* VERSION_CHECK_URL = "https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt";
```

The `/latest/` path means it will **always get the newest version** automatically!

### Upload to ESP32

1. Open `ESP32_Encrypted_Firmware.ino` in Arduino IDE
2. Click Upload
3. Open Serial Monitor (115200 baud)
4. Watch it check for updates from GitHub!

---

## 🔄 Deploying New Versions

When you have a new firmware version:

```powershell
# 1. Update version in sketch
#    Change: #define FIRMWARE_VERSION 2

# 2. Build in Arduino IDE
#    Sketch -> Export compiled Binary

# 3. Navigate to tools
cd c:\Users\cheri\OneDrive\Desktop\esp32cryptcode\tools

# 4. Encrypt with SAME key (important!)
python encrypt_firmware.py encrypt ^
    -i ..\ESP32_Encrypted_Firmware\ESP32_Encrypted_Firmware.ino.esp32.bin ^
    -o firmware_encrypted.bin ^
    -k aes_key.bin

# 5. Update version file
echo 2 > version.txt

# 6. Deploy new version
.\deploy_to_github.bat
# This will create release "v2"

# 7. ESP32 will auto-update next time it checks!
```

---

## 📊 How It Works

```
┌──────────────────────────────────────────────────────────┐
│                                                          │
│  1. ESP32 boots and connects to WiFi                    │
│                                                          │
│  2. Checks version from GitHub:                         │
│     https://github.com/mohamedcherif-pixel/Firmware/    │
│     releases/latest/download/version.txt                │
│                                                          │
│  3. If new version available:                           │
│     Downloads encrypted firmware from:                  │
│     https://github.com/mohamedcherif-pixel/Firmware/    │
│     releases/latest/download/firmware_encrypted.bin     │
│                                                          │
│  4. Decrypts firmware using AES-256 key                 │
│                                                          │
│  5. Writes to OTA partition                             │
│                                                          │
│  6. Reboots with new firmware!                          │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

---

## ✅ Complete Workflow Example

### Initial Deployment (v1):

```powershell
# 1. Build firmware in Arduino IDE
# 2. Encrypt
cd tools
python encrypt_firmware.py encrypt -i ..\build\firmware.bin -o firmware_encrypted.bin

# 3. Create version file
echo 1 > version.txt

# 4. Deploy
.\deploy_to_github.bat

# 5. Upload ESP32 code (already configured)

# 6. ESP32 checks GitHub and installs v1
```

### Update to v2:

```powershell
# 1. Update FIRMWARE_VERSION to 2 in sketch
# 2. Build firmware in Arduino IDE
# 3. Encrypt (reusing same key)
cd tools
python encrypt_firmware.py encrypt ^
    -i ..\build\firmware.bin ^
    -o firmware_encrypted.bin ^
    -k aes_key.bin

# 4. Update version
echo 2 > version.txt

# 5. Deploy
.\deploy_to_github.bat

# 6. ESP32 auto-detects new version and updates!
```

---

## 🔍 Testing

### Test Version Check:

```powershell
# Check what version is on GitHub
curl https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt

# Should return: 1 (or your current version)
```

### Test Firmware Download:

```powershell
# Download firmware to verify it's accessible
curl -L -o test_download.bin https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/firmware_encrypted.bin

# Check file size
dir test_download.bin
```

### Test on ESP32:

1. Upload code to ESP32
2. Open Serial Monitor (115200 baud)
3. Watch output:

```
=== ESP32 Encrypted Firmware Update System ===
Firmware Version: 1

Connecting to WiFi...
✓ WiFi connected successfully
IP Address: 192.168.1.29

=== Checking for firmware updates ===
Current firmware version: 1
Checking: https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/version.txt
Server firmware version: 1
✓ Already running latest version
```

---

## 🐛 Troubleshooting

### "GitHub CLI not found"

**Solution:**
```powershell
winget install GitHub.cli
# Restart terminal
gh auth login
```

### "Permission denied" or "404 Not Found"

**Check:**
1. You're logged in: `gh auth status`
2. Repository exists: https://github.com/mohamedcherif-pixel/Firmware
3. You have write access to the repository

**Fix:**
```powershell
gh auth login
# Re-authenticate
```

### "Release already exists"

If you try to create the same version twice:

**Solution:**
```powershell
# Delete the old release
gh release delete v1 --repo mohamedcherif-pixel/Firmware --yes

# Then create it again
.\deploy_to_github.bat
```

### "ESP32 can't download from GitHub"

**Check:**
1. URLs are correct (check Serial Monitor output)
2. ESP32 has internet access
3. WiFi credentials are correct
4. Files exist on GitHub (open URLs in browser)

**Note:** GitHub may redirect URLs. The ESP32 HTTP client should handle this automatically.

---

## 🔒 Security Notes

### What's Secure:

- ✅ Firmware is encrypted with AES-256
- ✅ HTTPS download (GitHub uses HTTPS by default)
- ✅ Can't be modified without breaking encryption

### What to Keep Secret:

- 🔐 **aes_key.bin** - Never upload this to GitHub!
- 🔐 **encryption_key.h** - Don't commit to Git

### Add to .gitignore:

```
# In your .gitignore file:
aes_key.bin
encryption_key.h
*.bin
firmware_encrypted.bin
```

---

## 📈 Advantages Over Local Server

| Feature | Local Server | GitHub Releases |
|---------|--------------|-----------------|
| Cost | FREE | FREE |
| Availability | Only when PC on | 24/7 |
| Network | Same WiFi only | Any internet |
| Setup Time | 2 minutes | 10 minutes |
| Security | HTTP | HTTPS |
| Scalability | 1-10 devices | Unlimited |
| Updates | Manual | Automatic |
| Reliability | Low | Very High |

---

## 🎓 Next Steps

After successful deployment:

1. ✅ Test with version 1
2. ✅ Deploy version 2 to test updates
3. ✅ Test from different WiFi networks
4. ✅ Consider adding firmware signing for extra security
5. ✅ Monitor update success rate

---

## 📞 Quick Reference Commands

### Deploy firmware:
```powershell
cd tools
.\deploy_to_github.bat
```

### Check GitHub status:
```powershell
gh auth status
```

### List releases:
```powershell
gh release list --repo mohamedcherif-pixel/Firmware
```

### Delete a release:
```powershell
gh release delete v1 --repo mohamedcherif-pixel/Firmware --yes
```

### View release:
```powershell
gh release view v1 --repo mohamedcherif-pixel/Firmware
```

---

## ✨ Summary

**You've successfully moved from:**
```
❌ Local Python server (http://192.168.1.22:8000)
   - Only works on your WiFi
   - PC must be running
```

**To:**
```
✅ GitHub Releases (https://github.com/mohamedcherif-pixel/Firmware)
   - Works from anywhere
   - Always available
   - HTTPS secure
   - Professional solution
```

**Your firmware is now hosted at:**
- https://github.com/mohamedcherif-pixel/Firmware/releases/latest/download/firmware_encrypted.bin

---

**Questions? Issues? Check the troubleshooting section above!**

**Repository:** https://github.com/mohamedcherif-pixel/Firmware  
**Status:** ✅ Ready to deploy!
