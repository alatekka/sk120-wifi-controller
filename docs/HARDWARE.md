# Hardware / العتاد

## What you need / ما تحتاجه

| Part | Notes |
|---|---|
| XY-SK120 (or SK120 with TTL Modbus) | Tested: model 22873, firmware 136 |
| ESP-01 or ESP-01S (ESP8266, 1 MB) | 4 MB modules work too |
| USB programmer for ESP-01 | CH340 type with a PROG/UART switch, or GPIO0 → GND while powering up |
| DC input | Any supply within the SK120 input range (e.g. 19 V laptop charger) |

## Wiring / التوصيل

The SK120 talks **Modbus RTU over 3.3 V TTL serial**. Cross TX/RX:

| ESP-01 | SK120 TTL port |
|---|---|
| TX (GPIO1) | RX |
| RX (GPIO3) | TX |
| GND | GND |
| 3V3 + EN (CH_PD) | 3.3 V (≥ 250 mA) |

> ⚠️ ESP-01 pins are **3.3 V only**. Check the pin labels on your SK120 board before connecting; do not feed 5 V to the ESP-01.
>
> ⚠️ أطراف ESP-01 تعمل على **3.3 فولت فقط**. تأكد من أسماء الأطراف على لوحة الـ SK120 قبل التوصيل، ولا توصل 5 فولت للموديول.

## SK120 menu / قائمة الـ SK120

| Item | Value |
|---|---|
| `Add` (Modbus address) | `1` |
| `bRE` (baud) | `6` = 115200 (recommended) or `8` = 4800 |

The SK120 applies a new baud only **after a power cycle**. The firmware tries the saved baud first and then scans all rates, so you never have to set it on the ESP side.

لا يطبّق الـ SK120 السرعة الجديدة إلا **بعد إطفائه وتشغيله**. السوفت وير يكتشف السرعة تلقائياً.

## Flashing the ESP-01 / برمجة الـ ESP-01

1. Put the ESP-01 on the USB programmer in **PROG** mode (or hold GPIO0 to GND while plugging in).
2. Flash from [alatekka.com](https://alatekka.com/project.html?p=sk120-wifi) (Chrome / Edge), or with Arduino IDE / `esptool.py`:
   ```
   esptool.py --chip esp8266 --baud 460800 write_flash -fm dout 0x0 sk120-wifi-esp01.bin
   ```
3. Switch back to **UART** mode (GPIO0 free), put the module in the SK120 and power on.
4. Later updates: open `http://sk120.local/update` and upload `sk120-wifi-esp01.bin.gz` (user `admin`, password = AP password).
