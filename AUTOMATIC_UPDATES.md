# ✨ Automatic Firmware Updates with GitHub Actions

**Goal:** Update code from anywhere → Push to GitHub → ESP32 automatically detects and installs new firmware

---

## 🎯 How It Works Now

```
You (anywhere in the world):
├─ Edit ESP32_Encrypted_Firmware.ino
├─ Push to GitHub
│
GitHub Actions CI/CD (automatic):
├─ Builds firmware
├─ Encrypts with AES-256
├─ Creates release v2
├─ Uploads to GitHub Releases
│
ESP32 (checks every 5 minutes):
├─ Detects new version
├─ Downloads encrypted firmware
├─ Decrypts & validates
├─ Installs to OTA partition
├─ Reboots automatically ✓
│
Result: NO BUTTON PRESS NEEDED! 🎉
```

---

## 📋 Setup Instructions

### Step 1: Initialize Git Repository

```powershell
cd "c:\Users\cheri\OneDrive\Desktop\esp32cryptcode"

# Initialize git if not already done
git init
git add .
git commit -m "Initial commit with encrypted OTA"

# Add remote (replace with your repo if different)
git remote add origin https://github.com/mohamedcherif-pixel/Firmware.git

# Push to GitHub
git branch -M main
git push -u origin main
```

### Step 2: Verify GitHub Actions Workflow

Check that the workflow file exists:
- File: `.github/workflows/build.yml`
- Location: Root of your repository

**Push this file to GitHub:**

```powershell
git add .github/workflows/build.yml
git commit -m "Add GitHub Actions CI/CD pipeline"
git push
```

### Step 3: First Automated Build

Make a small change to trigger the workflow:

```powershell
# Edit the file and make a small change (e.g., add a comment)
# Then commit and push

git add ESP32_Encrypted_Firmware/ESP32_Encrypted_Firmware.ino
git commit -m "Test: Add comment to trigger CI/CD"
git push
```

**Check the build:**
1. Go to: https://github.com/mohamedcherif-pixel/Firmware/actions
2. You should see the workflow running
3. Wait for it to complete (usually 2-3 minutes)
4. Check Releases: https://github.com/mohamedcherif-pixel/Firmware/releases

---

## 🚀 Complete Workflow Example

### Scenario: You want to update your firmware

#### Step 1: Edit Code Locally

Edit: `ESP32_Encrypted_Firmware/ESP32_Encrypted_Firmware.ino`

```cpp
#define FIRMWARE_VERSION 2  // Change from 1 to 2

// Your code changes here...
Serial.println("Updated firmware v2!");
```

#### Step 2: Commit & Push to GitHub

```powershell
cd "c:\Users\cheri\OneDrive\Desktop\esp32cryptcode"

git add .
git commit -m "v2: Add new feature XYZ"
git push
```

#### Step 3: GitHub Actions Automatically:
- ✅ Detects push
- ✅ Builds firmware
- ✅ Encrypts it
- ✅ Creates release v2
- ✅ Uploads to GitHub Releases

#### Step 4: ESP32 Automatically:
- 🔍 Checks GitHub (every 5 minutes)
- 📥 Detects version 2 available
- ⬇️ Downloads encrypted firmware
- 🔐 Decrypts with AES key
- 💾 Writes to OTA partition
- 🔄 Reboots with new code

**Result:** New firmware running without pressing button! 🎉

---

## 📊 Update Check Frequency

**Current setting: Every 5 minutes**

To change, edit this line in the `loop()` function:

```cpp
static const unsigned long UPDATE_CHECK_INTERVAL = 5 * 60 * 1000; // 5 minutes

// Change to:
// 1 minute:
// static const unsigned long UPDATE_CHECK_INTERVAL = 1 * 60 * 1000;

// 10 minutes:
// static const unsigned long UPDATE_CHECK_INTERVAL = 10 * 60 * 1000;

// 30 seconds (for testing):
// static const unsigned long UPDATE_CHECK_INTERVAL = 30 * 1000;
```

Then compile and upload to ESP32.

---

## 🔍 Monitor Serial Output

Watch Serial Monitor at 115200 baud to see:

```
=== Periodic Update Check ===
Current firmware version: 1
[VERSION] Checking: https://github.com/...
[VERSION] HTTP Code: 200
[VERSION] Response: 2
Server firmware version: 2
⚠️  New version available! Starting update...
[OTA] Starting encrypted OTA update from URL...
[OTA] Streaming encrypted firmware: 1234567 bytes
[OTA] IV extracted: ...
[OTA] Progress: 50KB (4.5%) - Speed: 120 KB/s
...
[OTA] Total decrypted: 1000000 bytes
[OTA] OTA update successful!
✓ OTA update successful! Rebooting...

=== ESP32 Encrypted Firmware Update System ===
Firmware Version: 2  ← NEW VERSION!
```

---

## ⚙️ What GitHub Actions Does

The `.github/workflows/build.yml` file:

1. **Triggered by:**
   - Push to `main` or `master` branch
   - Only if files in `ESP32_Encrypted_Firmware/` changed
   - Or manually via "Run workflow" button

2. **Automatically:**
   - Sets up Ubuntu environment
   - Installs Arduino CLI
   - Installs ESP32 core
   - Compiles your code
   - Extracts version number
   - Encrypts firmware
   - Creates GitHub Release
   - Uploads files

3. **No secrets needed:**
   - Uses `GITHUB_TOKEN` (automatic)
   - No manual authentication

---

## 🎯 Test the Automation

### Quick Test (30 seconds):

1. Change update interval to **30 seconds** in loop():
```cpp
static const unsigned long UPDATE_CHECK_INTERVAL = 30 * 1000;
```

2. Upload to ESP32

3. Make a small code change and push:
```powershell
git add .
git commit -m "v2: Test quick update"
git push
```

4. Watch GitHub Actions build (1-2 min)

5. Watch Serial Monitor - ESP32 should detect and update within 30 seconds!

---

## ✅ Checklist

Before everything works:

- [ ] Code is in GitHub repository
- [ ] `.github/workflows/build.yml` is in repo
- [ ] GitHub Actions workflow is enabled (check Actions tab)
- [ ] `aes_key.bin` is in `tools/` folder locally
- [ ] Version number in sketch matches released version
- [ ] ESP32 code uploaded with new loop() that checks every 5 minutes
- [ ] ESP32 is connected to WiFi with internet access

---

## 🚨 Troubleshooting

### "Workflow not running"

Check:
1. Go to: https://github.com/mohamedcherif-pixel/Firmware/actions
2. Click on failed workflow
3. See error details
4. Common issues:
   - Arduino CLI not installed
   - Wrong board type
   - Missing Python dependencies

### "ESP32 not detecting updates"

Check Serial Monitor:
```
✗ Failed to check server version
```

Reasons:
1. WiFi not connected
2. GitHub URL wrong
3. No internet on ESP32
4. Redirects not followed

**Fix:** URL must use `setFollowRedirects()` (already added)

### "Version file downloaded but shows wrong number"

Make sure:
1. Version in sketch matches release version
2. `version.txt` has just the number: `2`
3. No extra spaces or newlines

---

## 📈 Monitor Deployments

After setup, watch:

1. **GitHub Actions:** https://github.com/mohamedcherif-pixel/Firmware/actions
   - Shows build progress
   - Shows errors if any
   - Shows release created

2. **Releases Page:** https://github.com/mohamedcherif-pixel/Firmware/releases
   - Shows all available versions
   - Shows download counts

3. **Serial Monitor:**
   - Shows when ESP32 checks
   - Shows when update happens
   - Shows any errors

---

## 🎉 You're All Set!

From now on:
- Edit code locally
- Push to GitHub
- ESP32 automatically updates within 5 minutes
- No manual steps!
- Works from anywhere in the world!

---

**Documentation Version:** 1.0  
**Date:** October 19, 2025  
**Repository:** https://github.com/mohamedcherif-pixel/Firmware
