# Manual: Eliminar dependencia de Arduino IDE

Este manual describe cómo eliminar por completo la dependencia del Arduino IDE y trabajar de forma independiente usando PlatformIO o ESP-IDF.

## Objetivo
- Compilar y subir firmware sin Arduino IDE.
- Gestionar librerías desde el proyecto (no globales).
- Opcional: eliminar también el framework Arduino (migración a ESP-IDF).

---

## Opción A — PlatformIO + Arduino (sin IDE Arduino)

### A.1 Estructura mínima
- `platformio.ini`
- `src/main.cpp` (o `main.cpp` si lo mantienes en root)

### A.2 Dependencias gestionadas por PlatformIO
Usa `lib_deps` con URLs de GitHub en `platformio.ini`.  
Esto evita el Library Manager de Arduino IDE.

### A.3 Compilar y subir
```bash
pio run
pio run -t upload
pio device monitor
```

### A.4 Limpieza total del IDE
1. Desinstala Arduino IDE (opcional).
2. Borra librerías globales de Arduino:
   - macOS: `~/Documents/Arduino/libraries`

✅ Resultado: **sin dependencia del Arduino IDE**.

---

## Opción B — ESP-IDF puro (sin framework Arduino)

Esta opción elimina completamente Arduino (IDE + framework), pero requiere más trabajo.

### B.1 Instalar ESP-IDF
1. Instala ESP-IDF en tu sistema.
2. Inicializa el entorno (export de variables de IDF).

### B.2 Estructura base del proyecto
```
project/
  main/
  CMakeLists.txt
  sdkconfig
```

### B.3 Reemplazos principales de APIs Arduino
- `Arduino.h` → `freertos`, `driver`, `esp_*`
- `delay()` → `vTaskDelay()`
- `millis()` → `esp_timer_get_time() / 1000`
- `WiFi.h` → `esp_wifi`
- `WebSocketsServer` → componente IDF o librería externa
- `NimBLE` → `nimble` de IDF
- `LiquidCrystal_I2C` → driver I2C propio/externo
- `MAX6675` → driver SPI propio/externo
- `ModbusRtu` → stack Modbus IDF (externo)

### B.4 Dependencias como componentes
1. Coloca librerías como submódulos en `components/`.
2. O usa `idf.py add-dependency` cuando aplique.

✅ Resultado: **sin IDE Arduino y sin framework Arduino**.

---

## Recomendación
Si solo quieres dejar el IDE, usa **Opción A**.  
Si necesitas eliminar Arduino por completo, planifica una migración por módulos con **Opción B**.
