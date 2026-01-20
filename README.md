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
