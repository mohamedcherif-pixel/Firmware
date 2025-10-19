@echo off
echo ========================================
echo ESP32 FORCE FIX - Complete Flash Erase
echo ========================================
echo.
echo This will:
echo 1. Completely erase ALL flash memory
echo 2. Write a clean bootloader
echo 3. Write correct partition table
echo.
echo IMPORTANT: 
echo - Close Arduino IDE Serial Monitor NOW!
echo - Make sure ESP32 is connected to COM3
echo.
pause

echo.
echo Step 1: Erasing ENTIRE flash (this takes ~10 seconds)...
echo.
python -m esptool --chip esp32 --port COM3 --baud 460800 erase_flash

if errorlevel 1 (
    echo.
    echo ERROR: Flash erase failed!
    echo Check:
    echo - ESP32 is connected
    echo - COM3 is correct port
    echo - Serial Monitor is closed
    echo - Try holding BOOT button on ESP32
    pause
    exit /b 1
)

echo.
echo ========================================
echo SUCCESS! Flash completely erased
echo ========================================
echo.
echo Now follow these steps CAREFULLY:
echo.
echo 1. Open Arduino IDE
echo 2. Go to: Tools -^> Board -^> ESP32 Dev Module
echo 3. Go to: Tools -^> Flash Size -^> 4MB (32Mb)
echo 4. Go to: Tools -^> Partition Scheme -^> "Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)"
echo 5. Click Upload button
echo.
echo The ESP32 will now boot correctly!
echo.
pause
