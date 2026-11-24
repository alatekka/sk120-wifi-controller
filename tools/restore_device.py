"""Restore SK120 settings from a snapshot made by backup_device.py.

Usage:  python tools/restore_device.py backups/device-YYYY-MM-DD.json [host]

Restores: memory groups M0..M9 (V/I + protections), device settings (temp unit, backlight,
sleep, temp offsets, buzzer, MPPT, battery cut-off, CP) and module options (soft start,
time zone, schedule, step program, MQTT host/port/user/prefix, Telegram on/off + chat id).
Does NOT restore: output on/off, lock, Telegram token, MQTT password (re-enter them in the page).
"""
import json
import sys
import urllib.parse
import urllib.request

if len(sys.argv) < 2:
    sys.exit(__doc__)
snap = json.load(open(sys.argv[1], encoding="utf-8"))
HOST = sys.argv[2] if len(sys.argv) > 2 else snap.get("host", "sk120.local")


def call(path):
    with urllib.request.urlopen(f"http://{HOST}{path}", timeout=10) as r:
        return r.read().decode()


def main():
    print(f"Restoring {sys.argv[1]} (taken {snap['taken']}) to {HOST}")
    if input("Type YES to continue: ").strip() != "YES":
        sys.exit("cancelled")

    for name, vals in snap["memory_groups"].items():
        n = int(name[1:])
        call(f"/wm?a={0x50 + n * 16}&v=" + ",".join(map(str, vals)))
        print("  ", name, "ok")

    regs = snap["registers_0x00_0x34"]
    for a in (0x13, 0x14, 0x15, 0x1A, 0x1B, 0x1C, 0x1F, 0x20, 0x21, 0x22, 0x23):
        call(f"/w?a={a}&v={regs[a]}")
    call(f"/recall?n={regs[0x1D]}")            # re-select the active group
    print("   device settings ok, active group M%d" % regs[0x1D])

    o = snap["opts"]
    q = {
        "soft": o["soft"], "tz": o["tz"], "clock": o["clock"],
        "schedEn": o["schedEn"], "schedOn": o["schedOn"], "schedOff": o["schedOff"],
        "tgEn": o["tgEn"], "tgChat": o["tgChat"], "tgToken": "********",
        "mqEn": o["mqEn"], "mqHost": o["mqHost"], "mqPort": o["mqPort"],
        "mqUser": o["mqUser"], "mqPass": "********", "mqBase": o["mqBase"],
        "repeat": o["repeat"],
        "steps": ";".join(",".join(map(str, s)) for s in o["steps"]),
    }
    if not o["steps"]:
        q.pop("steps"); q.pop("repeat")
    call("/opts?" + urllib.parse.urlencode(q))
    print("   module options ok")
    print("Done. Re-enter the Telegram token / MQTT password in the page if you use them.")


if __name__ == "__main__":
    main()
