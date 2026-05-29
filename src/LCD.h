#ifndef __LCD_H__
#define __LCD_H__

#ifdef WIRELESS_TRACKER

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "main.h"


class LCD {
public:
  LCD();
  bool begin(int8_t cs, int8_t dc, int8_t rst, int8_t clk, int8_t din, int8_t bl);
  void end();
  void run();

private:
  void drawPage0();
  void drawPage1();
  void drawPage2();

  Adafruit_ST7735 *display = nullptr;
  SPIClass *spi = nullptr;
  int8_t pinBL = -1;
  uint8_t lastPage = 255;
  uint32_t tLastRefresh = 0;
};

#endif // WIRELESS_TRACKER
#endif // __LCD_H__
