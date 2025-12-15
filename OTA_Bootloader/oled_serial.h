#ifndef OLED_SERIAL_H
#define OLED_SERIAL_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1106G oled(128, 64, &Wire, -1);
#define OLED_LINES 18
#define OLED_CHARS 20
char oledBuffer[OLED_LINES][OLED_CHARS + 1];
int oledLine = 0;
char wordBuf[OLED_CHARS + 1] = "";

void oled_init() {
  Wire.begin(21, 22);
  if (oled.begin(0x3C, true)) {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextWrap(false);
    oled.setTextColor(SH110X_WHITE);
    for (int i = 0; i < OLED_LINES; i++) oledBuffer[i][0] = '\0';
  }
}

void oled_scroll() {
  for (int i = 0; i < OLED_LINES - 1; i++)
    strcpy(oledBuffer[i], oledBuffer[i + 1]);
  oledBuffer[OLED_LINES - 1][0] = '\0';
}

void oled_refresh() {
  oled.clearDisplay();
  for (int i = 0; i < OLED_LINES; i++) {
    oled.setCursor(0, i * 7+1);
    oled.print(oledBuffer[i]);
  }
  oled.display();
}

void oled_newline(char* lineBuf) {
  if (oledLine >= OLED_LINES) { oled_scroll(); oledLine = OLED_LINES - 1; }
  strcpy(oledBuffer[oledLine++], lineBuf);
  lineBuf[0] = '\0';
  oled_refresh();
}

void oled_print(const char* msg) {
  static char lineBuf[OLED_CHARS + 1] = "";
  
  for (int i = 0; msg[i]; i++) {
    char c = msg[i];
    
    if (c == '\n') {
      // Flush word buffer first
      if (strlen(wordBuf) > 0) {
        if (strlen(lineBuf) + strlen(wordBuf) > OLED_CHARS) {
          oled_newline(lineBuf);
        }
        strcat(lineBuf, wordBuf);
        wordBuf[0] = '\0';
      }
      oled_newline(lineBuf);
    }
    else if (c == ' ') {
      // Word complete - add to line if fits
      if (strlen(lineBuf) + strlen(wordBuf) + 1 > OLED_CHARS) {
        oled_newline(lineBuf);
      }
      if (strlen(wordBuf) > 0) {
        strcat(lineBuf, wordBuf);
        wordBuf[0] = '\0';
      }
      if (strlen(lineBuf) < OLED_CHARS) {
        strcat(lineBuf, " ");
      }
    }
    else {
      // Add char to word
      int len = strlen(wordBuf);
      if (len < OLED_CHARS) {
        wordBuf[len] = c;
        wordBuf[len + 1] = '\0';
      }
    }
  }
}

// Wrapper class to intercept Serial
class OledSerial : public Print {
public:
  size_t write(uint8_t c) override {
    Serial.write(c);
    char s[2] = {(char)c, '\0'};
    oled_print(s);
    return 1;
  }
};

OledSerial oledSerial;

#define OLED_LOG(x) do { Serial.print(x); oled_print(String(x).c_str()); } while(0)
#define OLED_LOGLN(x) do { Serial.println(x); oled_print((String(x) + "\n").c_str()); } while(0)

#endif
