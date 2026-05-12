# GXAirCom — Heltec Wireless Tracker V1.2 Port

> ⚠️ **BETA v1.0** — Tested on **Heltec Wireless Tracker V1.2 only**.  
> If you encounter issues or test on other hardware versions, please open an issue.

This fork adapts the official [GXAirCom firmware](https://github.com/gereic/GXAirCom) for the **Heltec Wireless Tracker V1.2** (ESP32-S3 + SX1262 LoRa + UC6580 GPS + ST7735 0.96" 160×80 LCD).

---

## ✅ Tested & Working

| Feature | Status |
|---|---|
| LoRa SX1262 — FANET TX/RX | ✅ Working |
| GPS UC6580 — satellite fix + NMEA | ✅ Working (cold start up to 25 min) |
| WiFi Access Point + web interface 192.168.4.1 | ✅ Working |
| BLE — compatible with XCGuide, XCSoar | ✅ Working |
| ST7735 0.96" 160×80 display | ✅ Working |
| 3 display pages (short press USER button) | ✅ Working |
| Deep sleep power off (long press USER button) | ✅ Working |
| Board & display in web settings dropdowns | ✅ Working |
| FANET packet reception on ground station | ✅ Verified |

## ⚠️ Not Tested / Known Issues

| Feature | Status |
|---|---|
| Barometer (BME280/BMP280) | ❌ Not tested |
| Wind sensor (anemometer) | ❌ Not tested |
| Variometer (MPU6050) | ❌ Not tested |
| GSM/4G module | ❌ Not tested |
| Battery percentage | ⚠️ Voltage displayed, percentage needs calibration |
| OTA firmware update | ❌ Not tested |

> **Note:** This firmware was compiled and tested exclusively on a **Heltec Wireless Tracker V1.2**. It has not been tested on V1.0 or V1.1. Community reports suggest the hardware is identical — if you test it, please share your results by opening an issue.

---

## 🔧 Technical Changes

### 1. New board type (`src/enums.h`)
```cpp
HELTEC_WIRELESS_TRACKER = 12,
TFT_ST7789 = 4,  // display enum (used internally)
```

### 2. Board detection & configuration (`src/main.cpp`)

Board is automatically forced at compile time via:
```cpp
#ifdef WIRELESS_TRACKER
  setting.boardType = HELTEC_WIRELESS_TRACKER;
#endif
```

Full hardware configuration in `case eBoard::HELTEC_WIRELESS_TRACKER`:

| GPIO | Function | Notes |
|---|---|---|
| 3 | Vext power enable | Powers GPS UC6580 + TFT display (HIGH = ON) |
| 33 | GPS UART RX | Receives NMEA from UC6580 GNSS_TX |
| 34 | GPS UART TX | Sends commands to UC6580 GNSS_RX |
| 35 | GPS RST | Active LOW, must be released HIGH at boot |
| 21 | TFT backlight | ⚠️ **NOT GPIO18** — GPIO18 is the LoRa TX LED |
| 36 | PinExtPower | |
| 37 | PinADCCtrl | Battery ADC enable |
| 1 | PinADCVoltage | Battery ADC input |
| 45 | I2C SDA | For barometer (not tested) |
| 46 | I2C SCL | For barometer (not tested) |

### 3. LoRa SX1262 — XTAL vs TCXO fix (`lib/FANETLORA/radio/LoRa.cpp`)

Previous Heltec boards (V3/V4) use a TCXO. The Wireless Tracker V1.2 uses a **32MHz XTAL** on XTA/XTB. After investigation and field testing, the existing TCXO code (`SetDIO3AsTcxoCtrl`) is **kept as-is** — removing it caused crashes on the first TX. The chip behaves correctly with TCXO mode enabled even when using a crystal.

### 4. GPS UC6580 — baud rate & power sequence

- Default baud rate on Wireless Tracker: **115200 baud** (not 9600)
- GPIO3 (Vext) must be set HIGH **before** any GPS communication
- GPIO35 (GNSS_RST) must be pulsed LOW then released HIGH at boot
- UC6580 cold start time: up to **25 minutes** — this is normal for this chip
- The baudrate scan (`checkGPSBaudrates()`) is skipped for this board — GPS is initialized directly at 115200

### 5. ST7735 display — custom bit-bang driver

No external library required. A minimal SPI bit-bang driver is embedded directly in `main.cpp`:

```
TFT_CS  = GPIO38    TFT_DC   = GPIO40
TFT_MOSI = GPIO42   TFT_SCK  = GPIO41
TFT_RST  = GPIO39   TFT_BL   = GPIO21
```

**ST7735 initialization:**
```
MADCTL  = 0x68  (MX|MV|BGR — landscape rotation=1)
XSTART  = 1, YSTART = 26  (MINI160x80 required offsets)
INVON   = required on this display
SPI speed: no delayMicroseconds — ~5MHz on ESP32-S3 @ 240MHz
```

> ⚠️ **Critical:** GPIO18 is **NOT** the display backlight — it is the LoRa TX LED.  
> The actual TFT backlight is on **GPIO21**.

### 6. Web interface — `website.h` regenerated

`src/web/website.h` was regenerated using `generate_website.py` (Python-only, no Node.js required) to add:
- Board type **12 → "HELTEC Wireless Tracker V1.2"** in the board dropdown
- Display type **4 → "ST7735 0.96" 160×80 (Wireless Tracker)"** in the display dropdown

### 7. `setting.displayType`

Set to `NO_DISPLAY (0)` in the settings for web interface compatibility. The ST7735 display is managed independently by the `taskTFT` FreeRTOS task and does not rely on GXAirCom's OLED subsystem.

---

## 🚀 Build & Flash

### Requirements
- VS Code + PlatformIO extension
- **Windows**: Node.js is **not required** (minify_www.py is disabled in `platformio.ini`)

### Flash
```bash
pio run -e Wireless_Tracker -t upload
```

### Bootloader mode (ESP32-S3)
1. Hold **USER** button
2. Plug in USB
3. Release after 2–3 seconds

### Regenerate web interface (after editing `src/web/orig/`)
```bash
python generate_website.py
```
This replaces `minify_www.py` and requires no Node.js.

---

## 📱 Display

| Action | Result |
|---|---|
| Short press USER button | Cycle through 3 pages |
| Long press USER button (~1s) | Power off (deep sleep) |

**Page 0 — Flight dashboard:**  
Large altitude (left) + large speed (right), GPS coordinates, FANET count, battery

**Page 1 — FANET neighbors:**  
Total count + list of up to 5 nearby aircraft

**Page 2 — System info:**  
Firmware version, Device ID, free heap, WiFi state, battery voltage

---

## 📋 Changelog & Roadmap

### v1.0-beta (current)
- Initial port to Heltec Wireless Tracker V1.2
- UC6580 GPS support at 115200 baud
- Custom ST7735 bit-bang display driver
- 3-page TFT display with button navigation
- Power off via long press
- Board and display type added to web settings

### v1.1 — Planned
- [ ] Correct battery percentage calibration
- [ ] Test barometer (BME280)
- [ ] Test on Wireless Tracker V1.0 / V1.1
- [ ] Hardware SPI display (pending arduino-esp32 3.x compatibility)

### v2.0 — Ideas
- [ ] Runtime board auto-detection
- [ ] Variometer via GNSS altitude differential
- [ ] Improved display layout with graphical battery bar

---

## 🤝 Contributing & Support

**Tested on:** Heltec Wireless Tracker **V1.2** only  
**Probably compatible:** V1.0, V1.1 (same pinout per community reports — unconfirmed)

If you encounter issues, test on a different hardware version, or successfully use a non-tested module (barometer, wind, GSM):
→ **Open an issue** on this repo specifying your hardware version

Pull requests are welcome, especially for untested modules.

---

## 📄 License

Based on [GXAirCom](https://github.com/gereic/GXAirCom) by gereic.  
Wireless Tracker modifications released under the same license terms.
