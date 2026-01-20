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
2. Create a `platformio.ini` in the project root and add the libraries listed below.
3. Build and upload the firmware.

### Required libraries
- `ArduinoJson`
- `LiquidCrystal_I2C`
- `ModbusRtu`
- `NimBLE-Arduino`
- `arduinoWebSockets`
- `MAX6675`

### Example `platformio.ini`
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
  bblanchon/ArduinoJson
  marcoschwartz/LiquidCrystal_I2C
  smarmengol/Modbus-Master-Slave-for-Arduino
  h2zero/NimBLE-Arduino
  links2004/WebSockets
  adafruit/MAX6675 library
```

### Build / upload / monitor
```bash
pio run
pio run -t upload
pio device monitor
```
