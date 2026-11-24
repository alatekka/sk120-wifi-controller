"""Save a snapshot of the SK120 + WiFi module settings to backups/device-<date>.json.

Usage:  python tools/backup_device.py [host]      (default host: sk120.local)
Needs firmware v3.1+ on the ESP module. Saved secrets (Telegram token, MQTT password)
are masked by the module and are NOT included, and the home WiFi name is left out.
"""
import datetime
import json
import os
import sys
import urllib.request

HOST = sys.argv[1] if len(sys.argv) > 1 else "sk120.local"


def get(path):
    with urllib.request.urlopen(f"http://{HOST}{path}", timeout=10) as r:
        return json.loads(r.read().decode())


def main():
    snap = {
        "taken": datetime.datetime.now().isoformat(timespec="seconds"),
        "host": HOST,
        "info": get("/info"),
        "opts": get("/opts"),
        "registers_0x00_0x34": get("/r?a=0&n=36") + get("/r?a=36&n=17"),
        "memory_groups": {f"M{n}": get(f"/mem?n={n}") for n in range(10)},
    }
    for k in ("heap", "up", "mbOk", "mbErr", "rssi"):
        snap["info"].pop(k, None)              # volatile values
    snap["info"].pop("staSsid", None)          # private: home WiFi name is not stored in backups
    out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "backups")
    os.makedirs(out_dir, exist_ok=True)
    path = os.path.join(out_dir, f"device-{datetime.date.today().isoformat()}.json")
    with open(path, "w", encoding="utf-8") as f:
        json.dump(snap, f, indent=1, ensure_ascii=False)
    print("saved", os.path.normpath(path))


if __name__ == "__main__":
    main()
