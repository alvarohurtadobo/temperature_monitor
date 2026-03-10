from pathlib import Path
import os


def _load_env_file(path: Path) -> None:
    # Defaults (pueden ser sobrescritos por .env)
    if "SERVER_URL" not in os.environ:
        os.environ["SERVER_URL"] = "http://192.168.1.100:3000/api/store-json"

    if not path.exists():
        return

    for line in path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip().strip('"').strip("'")
        if key:
            os.environ[key] = value


_load_env_file(Path(".env"))
