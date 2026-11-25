# Hardware / العتاد

## What you need / ما تحتاجه

| Part | Notes |
|---|---|
| XY-SK120 (or SK120 with TTL Modbus) | Tested: model 22873, firmware 136 |
| ESP-01 or ESP-01S (ESP8266, 1 MB) | 4 MB modules work too |
| USB programmer for ESP-01 | CH340 type with a PROG/UART switch, or GPIO0 → GND while powering up |
| DC input | Any supply within the SK120 input range (e.g. 19 V laptop charger) |
| 3.3 V regulator | LDO ≥ 300 mA, e.g. AMS1117-3.3 module, with 10 µF in/out + 100 µF at the ESP-01 |

## Wiring / التوصيل

The SK120 talks **Modbus RTU over 3.3 V TTL serial**. Its communication port supplies **5 V**, so the ESP-01 is powered through a **3.3 V regulator**. Cross TX/RX:

| ESP-01 | Via | SK120 TTL port |
|---|---|---|
| TX (GPIO1) | direct | RX |
| RX (GPIO3) | direct | TX |
| GND | direct | GND |
| 3V3 + EN (CH_PD) | **3.3 V regulator** (IN ← 5 V, OUT → ESP) | 5 V |

```
SK120 5V ──► [ 3.3 V LDO ] ──┬──► ESP-01 3V3
                             └──► ESP-01 EN (CH_PD)
             10µF in/out     100µF at the ESP-01 (WiFi current peaks ~300 mA)
SK120 GND ───────────────────────► ESP-01 GND  (and LDO GND)
SK120 TX  ───────────────────────► ESP-01 RX
SK120 RX  ◄─────────────────────── ESP-01 TX
```

> ⚠️ ESP-01 pins are **3.3 V only**. Never feed the SK120's 5 V straight to the ESP-01; always go through the regulator. Check the pin labels on your SK120 board before connecting.
>
> ⚠️ منفذ الاتصال في الـ SK120 يعطي **5 فولت**، والـ ESP-01 يعمل على **3.3 فولت فقط**. وصّل الـ 5 فولت إلى **منظّم 3.3 فولت** (مثل AMS1117-3.3) ومنه إلى أطراف 3V3 و EN في الموديول، مع مكثف 100 ميكروفاراد قرب الموديول لتحمّل ذروات الواي فاي. أطراف TX و RX و GND توصل مباشرة (مع عكس TX و RX).
>
> **Two units:** power the ESP-01 from **one** SK120's 5 V only; do not join the 5 V pins of the two units. Grounds are shared. · **جهازان:** غذِّ الموديول من 5 فولت جهاز **واحد** فقط، ولا تربط أطراف 5 فولت للجهازين ببعض، والأرضي مشترك.

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
