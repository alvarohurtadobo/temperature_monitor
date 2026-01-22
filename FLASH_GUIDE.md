# Guía para subir el bin al ESP32

Esta guía explica cómo enviar el bin compilado al ESP32, cómo configurar el puerto y cómo abrir el monitor serie, usando PlatformIO o `esptool.py`.

## Requisitos previos

- Tener el bin compilado (por ejemplo con `pio run`).
- Tener el ESP32 conectado por USB.

## Ubicar el bin compilado

El bin generado por PlatformIO se encuentra en:

```
.pio/build/esp32dev/firmware.bin
```

## Opción A: Subir con PlatformIO

### 1) Identificar el puerto

Con el ESP32 conectado, lista los dispositivos:

```
pio device list
```

En macOS suelen aparecer entradas como:

- `/dev/cu.usbserial-xxxx`
- `/dev/cu.SLAB_USBtoUART` (CP210x)
- `/dev/cu.wchusbserialxxxx` (CH340)

### 2) Subir indicando el puerto

```
pio run -t upload --upload-port /dev/cu.usbserial-xxxx
```

### 3) Configurar el puerto fijo en `platformio.ini` (opcional)

```
[env:esp32dev]
upload_port = /dev/cu.usbserial-xxxx
```

Luego puedes subir con:

```
pio run -t upload
```

### 4) Monitor serie

La velocidad ya está definida en `platformio.ini`:

```
monitor_speed = 115200
```

Para abrir el monitor serie:

```
pio device monitor --port /dev/cu.usbserial-xxxx --baud 115200
```

## Opción B: Subir con `esptool.py` (sin PlatformIO)

### 1) Instalar `esptool`

```
python3 -m pip install esptool
```

### 2) Flashear el bin

```
python3 -m esptool --chip esp32 --port /dev/cu.usbserial-xxxx --baud 460800 write_flash -z 0x10000 .pio/build/esp32dev/firmware.bin
```

Notas:
- `0x10000` es el offset típico para `esp32dev` con Arduino/PlatformIO.
- Si hay fallos de carga, prueba con `--baud 115200`.

## Problemas comunes

### No aparece el puerto

- Prueba con otro cable USB (algunos son solo de carga).
- Instala el driver según el chip USB:
  - **CP210x**: Silicon Labs
  - **CH340**: WCH

### No entra en modo boot

En algunas placas debes hacerlo manualmente:

1. Mantén presionado **BOOT**.
2. Pulsa **EN/RESET** una vez.
3. Suelta **BOOT** cuando inicie la carga.

## Comandos rápidos

```
pio device list
pio run -t upload --upload-port /dev/cu.usbserial-xxxx
pio device monitor --port /dev/cu.usbserial-xxxx --baud 115200
```
