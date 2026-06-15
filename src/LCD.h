#ifndef __LCD_H__
#define __LCD_H__

#ifdef WIRELESS_TRACKER

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "main.h"


// Backlight auto-dim: full → dim after BL_DIM_MS, off after BL_OFF_MS
static constexpr uint32_t BL_DIM_MS  = 30000;  // 30 s to dim
static constexpr uint32_t BL_OFF_MS  = 60000;  // 60 s to turn off
static constexpr uint8_t  BL_DIM_VAL = 60;     // ~24% duty cycle when dimmed
static constexpr uint8_t  BL_LEDC_CH = 1;      // LEDC channel (0 = buzzer)

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
  void setBacklight(uint8_t brightness);

  Adafruit_ST7735 *display = nullptr;
  SPIClass *spi = nullptr;
  int8_t  pinBL = -1;
  uint8_t lastPage = 255;
  uint32_t tLastRefresh = 0;
  uint8_t  blCurrent = 0;    // starts at 0 so first setBacklight(255) in begin() always calls ledcWrite
};

#endif // WIRELESS_TRACKER
#endif // __LCD_H__
