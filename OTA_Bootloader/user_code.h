#define USER_APP_VERSION 23
#include <TFT_eSPI.h>
#include "myImage.h"

TFT_eSPI tft = TFT_eSPI();

#define TFT_BL 4

void setup() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Display the downloaded image
  tft.setSwapBytes(true); // Swap bytes for correct endianness
  tft.pushImage(0, 0, 240, 240, myImage);
}

void loop() {}
