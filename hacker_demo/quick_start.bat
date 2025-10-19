@echo off
color 0A
title ESP32 MITM Attack - Quick Start

echo.
echo ========================================
echo ESP32 MITM Attack - Quick Start
echo ========================================
echo.
echo This will help you set up the attack quickly!
echo.
pause

:MENU
cls
echo.
echo ========================================
echo ESP32 MITM Attack Menu
echo ========================================
echo.
echo 1. Check Network Configuration
echo 2. Start Firmware Server (Port 8000)
echo 3. Start MITM Attack Proxy (Port 8080)
echo 4. View Attack Logs
echo 5. Open Beginner's Guide
echo 6. Exit
echo.
set /p choice="Select option (1-6): "

if "%choice%"=="1" goto CHECK_NETWORK
if "%choice%"=="2" goto START_SERVER
if "%choice%"=="3" goto START_MITM
if "%choice%"=="4" goto VIEW_LOGS
if "%choice%"=="5" goto OPEN_GUIDE
if "%choice%"=="6" goto EXIT
goto MENU

:CHECK_NETWORK
cls
echo.
echo ========================================
echo Network Configuration Check
echo ========================================
echo.
echo Your Computer's IP Address:
echo.
ipconfig | findstr /i "IPv4"
echo.
echo Make sure:
echo - ESP32 is on the same network
echo - Note your IPv4 Address above
echo - You'll need this for the attack
echo.
pause
goto MENU

:START_SERVER
cls
echo.
echo ========================================
echo Starting Firmware Server
echo ========================================
echo.
echo Server will run on port 8000
echo Keep this window open!
echo.
echo Press Ctrl+C to stop
echo.
cd ..\tools
python simple_http_server.py
pause
goto MENU

:START_MITM
cls
echo.
echo ========================================
echo Starting MITM Attack Proxy
echo ========================================
echo.
echo This will intercept ESP32 firmware requests
echo Keep this window open!
echo.
python real_mitm_attack.py
pause
goto MENU

:VIEW_LOGS
cls
echo.
echo ========================================
echo Attack Logs
echo ========================================
echo.
if exist real_attack_log.txt (
    type real_attack_log.txt
    echo.
    echo.
    echo Log file: real_attack_log.txt
) else (
    echo No attack logs found yet.
    echo Run an attack first!
)
echo.
pause
goto MENU

:OPEN_GUIDE
cls
echo.
echo Opening Beginner's Guide...
start BEGINNER_ATTACK_GUIDE.md
goto MENU

:EXIT
cls
echo.
echo Thanks for using ESP32 MITM Attack Tool!
echo Stay ethical! Only test on your own devices.
echo.
pause
exit
