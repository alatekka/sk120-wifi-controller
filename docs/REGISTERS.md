# XY-SK120 Modbus registers

Holding registers, function codes 03 / 06 / 16, slave ID 1. Values marked ✅ were verified on real hardware
(SK120 model 22873, firmware 136).

| Addr | Name | Scale / unit | Notes |
|---|---|---|---|
| 0x00 | V-SET | ×100 V | ✅ |
| 0x01 | I-SET | ×1000 A | ✅ |
| 0x02 | VOUT | ×100 V | ✅ |
| 0x03 | IOUT | ×1000 A | ✅ |
| 0x04 | POWER | ×100 W | ✅ |
| 0x05 | UIN | ×100 V | ✅ input voltage |
| 0x06 / 0x07 | Ah lo / hi | mAh | ✅ verified with a DC motor load; cumulative |
| 0x08 / 0x09 | Wh lo / hi | mWh | ✅ |
| 0x0A–0x0C | Output time | h / m / s | ✅ |
| 0x0D | T-IN | ×10 °C | ✅ |
| 0x0E | T-EX | ×10 °C | ✅ `8888` = no sensor |
| 0x0F | LOCK | 0 / 1 | ✅ |
| 0x10 | PROTECT | code | ✅ 0 OK, 1 OVP, 2 OCP, 3 OPP, 4 LVP, 5 OAH, 6 OHP, 7 OTP, 8 OEP, 9 OWH, 10 ICP, 11 ETP. Write 0 to clear |
| 0x11 | CV/CC | 0 CV, 1 CC | ✅ |
| 0x12 | ONOFF | 0 / 1 | ✅ |
| 0x13 | F-C | 0 °C, 1 °F | |
| 0x14 | B-LED | 0–5 | ✅ backlight |
| 0x15 | SLEEP | minutes | |
| 0x16 | MODEL | | ✅ 22873 |
| 0x17 | VERSION | | ✅ 136 |
| 0x18 | SLAVE-ADD | | |
| 0x19 | BAUD code | 0 9600, 1 14400, 2 19200, 3 38400, 4 56000, 5 57600, 6 115200, 7 2400, 8 4800 | ✅ written value is stored, applied only after power cycle |
| 0x1A / 0x1B | Temp offsets | ×10, signed | |
| 0x1C | BUZZER | 0 / 1 | |
| 0x1D | EXTRACT-M | 0–9 | ✅ **active memory group**; write N to recall group N |
| 0x1F | MPPT-SW | 0 / 1 | |
| 0x20 | MPPT-K | ×100 | ✅ default 80 (0.80) |
| 0x21 | BAT-FUL | ×1000 A | charge cut-off current |
| 0x22 | CP-SW | 0 / 1 | constant power |
| 0x23 | CP | ×10 W | |
| 0x30–0x34 | WiFi host | `0x3B3A, 2, 4, ipHi, ipLo` | ✅ shows the WiFi icon on the SK120 screen; zeros hide it |
| 0x50 + 0x10·n | Memory group n (n = 0–9) | 14 registers | ✅ V, I, LVP, OVP, OCP, OPP(×10 W), OHP h, OHP m, OAH lo/hi (mAh), OWH lo/hi, OTP, POn |
| 0x0200–0x0214 | Clock / weather | | ❌ rejected (exception 2) on this model; SK120X only |

**Not available:** there is no input-current register (0x00–0x34 scanned under load). The UI estimates input
current as output power ÷ (Vin × 0.9).
