// TFT_eSPI User_Setup.h for ILI9488 480x320 Display
// This file is copied to the library folder during GitHub Actions build

#define USER_SETUP_INFO "ILI9488_ESP32_OTA"

// ==================== DRIVER ====================
#define ILI9488_DRIVER

// ==================== ESP32 PINS ====================
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4

// Backlight control (not used, connected to 3.3V)
#define TFT_BL   -1

// ==================== FONTS ====================
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

// ==================== SPI SETTINGS ====================
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
