@echo off
echo ========================================
echo ESP32 Flash Erase and Recovery Tool
echo ========================================
echo.
echo This will completely erase your ESP32 flash
echo and fix the partition table issue.
echo.
echo IMPORTANT: Close Arduino IDE Serial Monitor first!
echo.
pause

echo.
echo Step 1: Erasing entire flash...
echo.
python -m esptool --chip esp32 --port COM3 erase_flash

echo.
echo ========================================
echo Flash erased successfully!
echo ========================================
echo.
echo Next steps:
echo 1. Open Arduino IDE
echo 2. Tools -^> Partition Scheme -^> "Minimal SPIFFS (1.9MB APP with OTA)"
echo 3. Upload your sketch
echo.
pause
