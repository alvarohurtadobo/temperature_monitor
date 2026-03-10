# Solar (Monitor CAYAPA)

`src/solar.cpp` es la versión PlatformIO de `legacy/solar.ino`. Lee el ADC, controla LEDs según el rango de energía, muestra valores en un LCD I2C y envía JSON al servidor cada 5 segundos.

## Configuración previa

1. **Variables de entorno** – Crea `.env` (o copia `.env.example`):

```bash
cp .env.example .env
```

Edita `.env`:

```
WIFI_SSID=tu_red
WIFI_PASS=tu_contraseña
SERVER_URL=http://192.168.1.XXX:3000/api/store-json
```

2. **Pines** (igual que en el .ino original):

| Función       | GPIO |
|--------------|------|
| ADC (lectura)| 36   |
| LED negativo | 32   |
| LED 0-10     | 33   |
| LED 10-20    | 26   |

3. **LCD I2C** – Dirección por defecto `0x27`. Si tu LCD usa `0x3F`, cambia en `src/solar.cpp` la línea:

```cpp
static LiquidCrystal_I2C lcd(0x27, 16, 2);
```

## Compilar

```bash
pio run -e solar
```

El bin se genera en `.pio/build/solar/firmware.bin`.

## Subir al ESP32

1. Conecta el ESP32 por USB.
2. Lista puertos:

```bash
pio device list
```

3. Sube indicando el puerto:

```bash
pio run -e solar -t upload --upload-port /dev/cu.usbserial-xxxx
```

O define el puerto en `platformio.ini` dentro de `[env:solar]`:

```ini
upload_port = /dev/cu.usbserial-xxxx
```

4. Monitor serie (115200 baud):

```bash
pio device monitor -e solar --port /dev/cu.usbserial-xxxx --baud 115200
```

## Resumen de comandos

```bash
# Compilar
pio run -e solar

# Subir (reemplaza el puerto por el tuyo)
pio run -e solar -t upload --upload-port /dev/cu.usbserial-xxxx

# Monitor
pio device monitor -e solar -b 115200
```

## Alternar entre Solar y main

- **Solar**: `pio run -e solar`  
- **Main (bluetooth_monitor)**: `pio run` o `pio run -e esp32dev`
