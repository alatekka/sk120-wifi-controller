# Changelog / سجل التغييرات

## v3.2.1 — 2026-11-25
- **Live updates over WebSocket** (`ws://<host>/ws`, ~6 readings/s), automatic fallback to HTTP polling.
- Estimated input current and adapter load % with a warning near the limit.
- **تحديث لحظي عبر WebSocket** (حوالي 6 قراءات في الثانية)، مع رجوع تلقائي للطريقة القديمة.
- تيار دخل تقديري ونسبة الحمل على المحوّل مع تنبيه.

## v3.1.x — 2026-11-24
- Soft start, auto-off timer, daily schedule (NTP), step program (10 steps), battery charger profiles.
- Telegram alerts, MQTT + Home Assistant discovery, 30-minute history chart with tooltip.
- Faster Modbus polling at 115200 baud with automatic baud detection; gzip OTA.
- تشغيل ناعم، مؤقت، جدول يومي، برنامج خطوات، شاحن بطاريات، تيليجرام و MQTT، سجل 30 دقيقة.

## v3.0.x — 2026-11-24
- Full SK120 register support: protections, memory groups M0–M9, CP / MPPT / battery cut-off, system settings.
- Arabic / English interface (RTL), WiFi icon on the SK120 screen, OTA updates, mDNS `sk120.local`.
- دعم كامل للجهاز، واجهة عربية وإنجليزية، علامة الواي فاي على الشاشة، تحديث عبر الواي فاي.

## v2 — 2025-12
- Simple dashboard: V/I, output on/off, lock, presets, CSV log.
