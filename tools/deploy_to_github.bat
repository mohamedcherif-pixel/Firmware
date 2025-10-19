@echo off
REM Simple GitHub Releases Deployment Script
REM Uploads encrypted firmware to GitHub Releases

echo.
echo ============================================================
echo    Deploy Firmware to GitHub Releases
echo ============================================================
echo.

REM Check if firmware exists
if not exist "firmware_encrypted.bin" (
    echo [ERROR] firmware_encrypted.bin not found!
    echo.
    echo Please run encrypt_firmware.py first:
    echo   python encrypt_firmware.py encrypt -i ../build/firmware.bin -o firmware_encrypted.bin
    echo.
    pause
    exit /b 1
)

if not exist "version.txt" (
    echo [ERROR] version.txt not found!
    echo Creating version.txt with version 1...
    echo 1 > version.txt
)

REM Read version
set /p VERSION=<version.txt
echo [INFO] Current firmware version: %VERSION%
echo.

REM GitHub repository
set REPO=mohamedcherif-pixel/Firmware
set TAG=v%VERSION%

echo [INFO] Deploying to GitHub repository: %REPO%
echo [INFO] Release tag: %TAG%
echo.

REM Check if GitHub CLI is installed
where gh >nul 2>nul
if errorlevel 1 (
    echo [ERROR] GitHub CLI not found!
    echo.
    echo Please install GitHub CLI:
    echo   winget install GitHub.cli
    echo.
    echo Then authenticate:
    echo   gh auth login
    echo.
    pause
    exit /b 1
)

echo [INFO] Creating release %TAG%...
echo.

REM Create release and upload files
gh release create %TAG% ^
    firmware_encrypted.bin ^
    version.txt ^
    --repo %REPO% ^
    --title "Firmware v%VERSION%" ^
    --notes "Encrypted ESP32 firmware - Version %VERSION%"

if errorlevel 1 (
    echo.
    echo [ERROR] Release creation failed!
    echo.
    echo Make sure:
    echo   1. You're authenticated: gh auth login
    echo   2. Repository exists: https://github.com/%REPO%
    echo   3. You have write access to the repository
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo   DEPLOYMENT SUCCESSFUL!
echo ========================================
echo.
echo Firmware URL:
echo   https://github.com/%REPO%/releases/download/%TAG%/firmware_encrypted.bin
echo.
echo Version URL:
echo   https://github.com/%REPO%/releases/download/%TAG%/version.txt
echo.
echo Update your ESP32 code with:
echo   const char* OTA_FIRMWARE_URL = "https://github.com/%REPO%/releases/download/%TAG%/firmware_encrypted.bin";
echo   const char* VERSION_CHECK_URL = "https://github.com/%REPO%/releases/download/%TAG%/version.txt";
echo.
echo Or use "latest" to always get the newest version:
echo   const char* OTA_FIRMWARE_URL = "https://github.com/%REPO%/releases/latest/download/firmware_encrypted.bin";
echo   const char* VERSION_CHECK_URL = "https://github.com/%REPO%/releases/latest/download/version.txt";
echo.
pause
