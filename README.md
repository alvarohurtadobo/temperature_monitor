# Bluetooth Monitor

## Overview
This project monitors roasting-related temperatures with an ESP32 and shares them to a display and connected clients. It is intended as an early-stage prototype; most of the current implementation is legacy and will be replaced.

## What it does
- Reads three temperature probes (bean temp, exhaust air temp, and a third optional probe)
- Shows values on a local display
- Broadcasts data so external tools can visualize it

## Sensor connections (ESP32)
Use MAX6675 modules for the thermocouples and wire them as follows:

- **SCK/CLK** → GPIO 5
- **SO/DO** → GPIO 19
- **CS for TG probe** → GPIO 17
- **CS for TA probe** → GPIO 18
- **CS for TT probe** → GPIO 23

All probes share the same **CLK** and **DO** lines; only the **CS** line changes per probe.

## Run with PlatformIO (based on `main.cpp`)
1. Install PlatformIO (VS Code extension or CLI).
2. Use the existing `platformio.ini` in the project root.
3. Install dependencies from `platformio.ini` (command below).
4. Build and upload the firmware.

### Required libraries (from `platformio.ini`)
- `ArduinoJson` (GitHub)
- `LiquidCrystal_I2C` (GitHub)
- `Modbus-Master-Slave-for-Arduino` (GitHub)
- `NimBLE-Arduino` (GitHub)
- `arduinoWebSockets` (GitHub)
- `MAX6675-library` (GitHub)

### Example `platformio.ini`
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
  https://github.com/bblanchon/ArduinoJson.git # bblanchon/ArduinoJson
  https://github.com/marcoschwartz/LiquidCrystal_I2C.git # marcoschwartz/LiquidCrystal_I2C
  https://github.com/smarmengol/Modbus-Master-Slave-for-Arduino.git # smarmengol/Modbus-Master-Slave-for-Arduino
  https://github.com/h2zero/NimBLE-Arduino.git # h2zero/NimBLE-Arduino
  https://github.com/Links2004/arduinoWebSockets.git # links2004/WebSockets
  https://github.com/adafruit/MAX6675-library.git # adafruit/MAX6675 library
```

### Install dependencies
```bash
pio pkg install
```

### WiFi credentials (environment variables)
This project reads WiFi credentials from environment variables at build time.
You can export them directly or place them in a `.env` file (see `.env.example`):

```bash
export WIFI_SSID="your_ssid"
export WIFI_PASS="your_password"
```

### Build / upload / monitor
```bash
pio run
pio run -t upload
pio device monitor
```
