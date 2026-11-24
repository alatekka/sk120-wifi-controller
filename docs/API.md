# API

Everything the web page does is available to your own scripts. Base URL: `http://sk120.local` (or the device IP).
Raw register values are integers: volts ×100, amps ×1000, power ×100 (see [REGISTERS.md](REGISTERS.md)).

## Live data

| Method | Path | Description |
|---|---|---|
| WebSocket | `ws://<host>/ws` (also `ws://<host>:81/`) | Pushes the `/status` JSON after every device read (~6/s at 115200 baud) |
| GET | `/status` | Current state: `ok`, `age` (ms), `r` (registers 0x00–0x23), job fields (`ramp`, `timer`, `tOff`, `prog`, `progLeft`, `chg`, `chgMah`, `time`, …) |
| GET | `/log.json?sec=600&pts=300` | History (up to 1800 s): `{"step":2,"v":[...],"i":[...]}` raw values, oldest first |
| GET | `/log.csv` | Last 30 min as CSV |
| GET | `/info` | Module info: firmware, WiFi, baud, Modbus counters, heap |

## Control

| Path | Description |
|---|---|
| `/set?v=12.00&i=1.000` | Set voltage and/or current limit |
| `/power?on=1[&soft=0]` | Output on (with soft start unless `soft=0`) / `on=0` off |
| `/lock?on=1` | Lock / unlock the front panel |
| `/recall?n=3` | Load memory group M3 into the active settings |
| `/mem?n=3` | Read memory group M3 (14 raw registers) |
| `/wm?a=0x80&v=500,1000,...` | Write several registers (memory groups, settings) |
| `/w?a=0x14&v=3` | Write one register (writable registers only) |
| `/r?a=0x00&n=16` | Read raw registers |
| `/timer?sec=600` | Turn output off after N seconds (`0` cancels) |
| `/prog?run=1` / `run=0` | Start / stop the saved step program |
| `/charge?run=1&cap=2000` | Start a battery charge job (after writing V/I/cut-off) |
| `/opts` | Read module options; `/opts?soft=2000&tz=180&...` changes them |
| `/notify-test` | Send a Telegram test message |
| `/baud?c=6` | Write SK120 baud code (applied by the SK120 after a power cycle) |
| `/update` | OTA firmware upload (Basic auth: `admin` / AP password) |

Example:

```bash
curl "http://sk120.local/set?v=5.00&i=0.500"
curl "http://sk120.local/power?on=1"
curl -s http://sk120.local/status | jq '.r[2]/100'   # output voltage
```

## MQTT

Enable it in **Network → MQTT**. With prefix `sk120`:

| Topic | Direction | Payload |
|---|---|---|
| `sk120/state` | out (every 2 s) | `{"v":12.00,"i":0.100,"p":1.20,"vin":19.1,"vs":12,"is":1,"t":31.5,"wh":0.01,"ah":0.001,"on":"ON","mode":"CV","prot":"OK","lock":0}` |
| `sk120/status` | out (retained) | `online` / `offline` |
| `sk120/event` | out | Alerts (protection, timer, charge done, …) |
| `sk120/set/voltage` | in | `12.5` |
| `sk120/set/current` | in | `0.8` |
| `sk120/set/output` | in | `ON` / `OFF` |

Home Assistant discovers the device automatically (sensors, two numbers and an output switch).
