"""
Carga .env y añade WIFI_SSID, WIFI_PASS, SERVER_URL como CPPDEFINES.
PlatformIO expande ${sysenv.X} antes del pre script, por eso hay que
inyectar los valores con Import("env") y Append(CPPDEFINES).
"""
from pathlib import Path

Import("env")

def _load_env_file(path: Path) -> dict:
    defaults = {"SERVER_URL": "http://192.168.1.100:3000/api/store-json"}
    if not path.exists():
        return defaults
    for line in path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip().strip('"').strip("'")
        if key:
            defaults[key] = value
    return defaults

# .env está en la raíz del proyecto (PlatformIO siempre define PROJECT_DIR)
project_dir = Path(env["PROJECT_DIR"])
vars_dict = _load_env_file(project_dir / ".env")

# Inyectar -D WIFI_SSID=\"valor\" para que el preprocesador reciba strings literales
# CPPDEFINES pierde comillas; CPPFLAGS con escape explícito funciona
flags = []
for key in ("WIFI_SSID", "WIFI_PASS", "SERVER_URL"):
    if key in vars_dict:
        val = vars_dict[key].replace("\\", "\\\\").replace('"', '\\"')
        flags.append('-D%s=\\"%s\\"' % (key, val))

if flags:
    env.Append(CPPFLAGS=flags)
