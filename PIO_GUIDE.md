# PlatformIO: Instalación y uso

Guía breve para instalar PlatformIO y usar este proyecto sin depender del Arduino IDE.

## Opciones de instalación

### Opción A: Extensión de VS Code / Cursor
1. Instala la extensión **PlatformIO IDE**.
2. Abre el proyecto en Cursor.
3. La extensión instalará automáticamente PlatformIO Core.

### Opción B: CLI (recomendada si usas terminal)
**macOS (Homebrew):**
```bash
brew install platformio
```

**Python (pipx):**
```bash
pipx install platformio
```

**Python (pip):**
```bash
python3 -m pip install -U platformio
```

Verifica la instalación:
```bash
pio --version
```

---

## Uso en este proyecto

1. Instalar dependencias:
```bash
pio pkg install
```

2. Compilar:
```bash
pio run
```

3. Subir al ESP32:
```bash
pio run -t upload
```

4. Monitor serie:
```bash
pio device monitor
```

---

## Variables de entorno (WiFi)
Puedes exportarlas o usar `.env`:
```bash
export WIFI_SSID="your_ssid"
export WIFI_PASS="your_password"
```

Si prefieres `.env`, copia el ejemplo:
```bash
cp .env.example .env
```
