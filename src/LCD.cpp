#ifdef WIRELESS_TRACKER



#include "LCD.h"
#include "FanetLora.h"
#include "esp_sleep.h"

extern struct SettingsData setting;
extern struct statusData status;
extern FanetLora fanet;
extern volatile uint8_t g_tft_page;

LCD::LCD() {}

bool LCD::begin(int8_t cs, int8_t dc, int8_t rst, int8_t clk, int8_t din, int8_t bl) {
  pinBL = bl;
  pinMode(pinBL, OUTPUT);
  digitalWrite(pinBL, HIGH);

  spi = new SPIClass(HSPI);
  spi->begin(clk, -1, din, -1);

  display = new Adafruit_ST7735(spi, cs, dc, rst);
  display->setSPISpeed(40000000);
  display->initR(INITR_MINI160x80_PLUGIN); // colstart=26 rowstart=1 → xstart=1 ystart=26; BGR on rot 1; INVON in init
  display->setRotation(1);
  display->setTextWrap(false);

  // Splash screen
  display->fillScreen(ST77XX_BLACK);
  display->fillRect(0, 25, 160, 30, ST77XX_BLUE);
  display->setTextSize(1);
  display->setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  display->setCursor(20, 28);
  display->print("GXAirCom");
  display->setTextColor(ST77XX_CYAN, ST77XX_BLUE);
  display->setCursor(10, 40);
  display->print("Wireless Tracker");
  delay(2000);
  display->fillScreen(ST77XX_BLACK);

  return true;
}

void LCD::end() {
  display->fillScreen(ST77XX_BLACK);
  display->fillRect(30, 30, 100, 20, ST77XX_RED);
  display->setTextColor(ST77XX_WHITE, ST77XX_RED);
  display->setTextSize(1);
  display->setCursor(35, 37);
  display->print("Power Off");
  delay(1000);
  display->fillScreen(ST77XX_BLACK);
  digitalWrite(pinBL, LOW);
}

void LCD::run() {
  bool pageChanged = (g_tft_page != lastPage);
  if (!pageChanged && millis() - tLastRefresh < 1000) return;

  if (pageChanged && g_tft_page != 0) display->fillScreen(ST77XX_BLACK);
  tLastRefresh = millis();
  lastPage = g_tft_page;

  if      (g_tft_page == 0) drawPage0();
  else if (g_tft_page == 1) drawPage1();
  else if (g_tft_page == 2) drawPage2();
}

// ---- Page 0: Flight dashboard ----
void LCD::drawPage0() {
  char buf[32];

  // Header — blue
  display->fillRect(0, 0, 160, 12, ST77XX_BLUE);
  display->setTextSize(1);
  display->setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  snprintf(buf, sizeof(buf), "GXAirCom %s", fanet.getMyDevId().c_str());
  display->setCursor(2, 2);
  display->print(buf);
  snprintf(buf, sizeof(buf), "%02d", status.gps.NumSat);
  display->setTextColor(status.gps.NumSat > 3 ? ST77XX_GREEN : ST77XX_RED, ST77XX_BLUE);
  display->setCursor(145, 2);
  display->print(buf);

  // Vertical separator
  display->fillRect(78, 12, 4, 40, 0x2104);

  // Left column — Altitude
  const uint16_t colBg = 0x0841;
  display->fillRect(0, 12, 78, 40, colBg);
  display->setTextSize(1);
  display->setTextColor(0x8410, colBg);
  display->setCursor(2, 14);
  display->print("ALT");
  snprintf(buf, sizeof(buf), "%d", (int)status.gps.alt);
  display->setTextSize(2);
  display->setTextColor(ST77XX_WHITE, colBg);
  display->setCursor(38 - (int16_t)(strlen(buf) * 12) / 2, 22);
  display->print(buf);
  display->setTextSize(1);
  display->setTextColor(0x8410, colBg);
  display->setCursor(60, 44);
  display->print("m");

  // Right column — Ground speed
  display->fillRect(82, 12, 78, 40, colBg);
  display->setTextSize(1);
  display->setTextColor(0x8410, colBg);
  display->setCursor(84, 14);
  display->print("GS");
  snprintf(buf, sizeof(buf), "%.0f", status.gps.speed);
  display->setTextSize(2);
  display->setTextColor(ST77XX_CYAN, colBg);
  display->setCursor(121 - (int16_t)(strlen(buf) * 12) / 2, 22);
  display->print(buf);
  display->setTextSize(1);
  display->setTextColor(0x8410, colBg);
  display->setCursor(84, 44);
  display->print("km/h");

  // GPS coordinates
  display->fillRect(0, 52, 160, 10, ST77XX_BLACK);
  display->setTextSize(1);
  if (status.gps.bHasGPS && status.gps.NumSat > 0) {
    snprintf(buf, sizeof(buf), "%.4fN  %.4fE", status.gps.Lat, status.gps.Lon);
    display->setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  } else {
    snprintf(buf, sizeof(buf), "GPS: no fix       ");
    display->setTextColor(ST77XX_RED, ST77XX_BLACK);
  }
  display->setCursor(2, 53);
  display->print(buf);

  // Footer — FANET count | time | battery
  display->fillRect(0, 63, 160, 10, ST77XX_BLACK);
  char vbuf[12], fbuf[12], bbuf[16];
  snprintf(fbuf, sizeof(fbuf), "F:%d", (int)fanet.getNeighboursCount());
  snprintf(vbuf, sizeof(vbuf), "  -- ");
  if (strlen(status.gps.Time) == 6)
    snprintf(vbuf, sizeof(vbuf), "%c%c:%c%c",
      status.gps.Time[0], status.gps.Time[1],
      status.gps.Time[2], status.gps.Time[3]);
  snprintf(bbuf, sizeof(bbuf), "%02d%%", status.battery.percent);
  display->setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  display->setCursor(2, 64);
  display->print(fbuf);
  display->setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  display->setCursor(50, 64);
  display->print(vbuf);
  display->setTextColor(ST77XX_RED, ST77XX_BLACK);
  display->setCursor(95, 64);
  display->print(bbuf);
}

// ---- Page 1: FANET neighbours ----
void LCD::drawPage1() {
  char buf[32];
  display->fillRect(0, 0, 160, 11, ST77XX_GREEN);
  display->setTextSize(1);
  display->setTextColor(ST77XX_BLACK, ST77XX_GREEN);
  display->setCursor(2, 2);
  display->print("FANET Neighbours  ");

  uint8_t cnt = fanet.getNeighboursCount();
  snprintf(buf, sizeof(buf), "Total: %d         ", cnt);
  display->fillRect(0, 12, 160, 10, ST77XX_BLACK);
  display->setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  display->setCursor(2, 13);
  display->print(buf);

  for (int n = 0; n < cnt; n++) {
    display->fillRect(0, 24 + n*11, 160, 10, ST77XX_BLACK);
    int16_t idx = fanet.getNextNeighbor(n);
    if (idx >= 0) {
      const String& name = fanet.neighbours[idx].name.substring(0, 16);
      const float alt = fanet.neighbours[idx].altitude;
      snprintf(buf, sizeof(buf), "%s ... alt %4dm", name.length() > 0 ? name.c_str() : "Unknown", (int)alt);
      display->setTextColor(ST77XX_CYAN, ST77XX_BLACK);
      display->setCursor(2, 25 + n*11);
      display->print(buf);
    }
  }
}

// ---- Page 2: System info ----
void LCD::drawPage2() {
  char buf[32];
  display->fillRect(0, 0, 160, 11, ST77XX_YELLOW);
  display->setTextSize(1);
  display->setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
  display->setCursor(2, 2);
  display->print("System         ");

  display->fillRect(0, 12, 160, 58, ST77XX_BLACK);
  snprintf(buf, sizeof(buf), "FW: " VERSION);
  display->setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  display->setCursor(2, 13);
  display->print(buf);

  snprintf(buf, sizeof(buf), "ID: %s", fanet.getMyDevId().c_str());
  display->setTextColor(ST77XX_CYAN, ST77XX_BLACK);
  display->setCursor(2, 23);
  display->print(buf);

  snprintf(buf, sizeof(buf), "Heap:%dkB  WiFi:%s",
    (int)(ESP.getFreeHeap() / 1024), status.wifiAP.state == CONNECTED ? "ON" : "OFF");
  display->setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  display->setCursor(2, 33);
  display->print(buf);

  snprintf(buf, sizeof(buf), "Batt:%dmV %02d%%",
    (int)status.battery.voltage, status.battery.percent);
  display->setTextColor(ST77XX_RED, ST77XX_BLACK);
  display->setCursor(2, 43);
  display->print(buf);

  // Diagnostic: both raw GPS alt and geoid undulation — if Geo≈50m then
  // the GPS reports WGS84 ellipsoidal height; true MSL = Alt - Geo.
  snprintf(buf, sizeof(buf), "Alt:%dm Geo:%dm",
    (int)status.gps.alt, (int)status.gps.geoidAlt);
  display->setTextColor(0xC618, ST77XX_BLACK); // light grey
  display->setCursor(2, 53);
  display->print(buf);
}

#endif // WIRELESS_TRACKER
