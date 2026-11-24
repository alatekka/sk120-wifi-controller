/*
  XY-SK120 WiFi Controller — Full System Firmware (v3)
  Board : ESP8266 ESP-01 / ESP-01S (Generic ESP8266 Module, 1MB flash, FS: none / OTA ~502KB)
  Link  : ESP TX/RX  <->  SK120 Modbus RTU (TTL), default 4800 8N1, slave ID 1

  Features
    - Live: V/I/P out, V in, CV/CC, protection status, temps, Ah / Wh / run time
    - Set V / I, Output ON/OFF, front-panel lock
    - Protections (active group M0): LVP OVP OCP OPP OHP OAH OWH OTP POn
    - Memory groups M0..M9: read / edit / save / recall
    - Modes: Constant Power (CP), MPPT, battery-full cut-off
    - System: backlight, sleep, buzzer, °C/°F, temp offsets, Modbus baud
    - Raw register read/write tool, CSV log, OTA update (/update), mDNS (http://sk120.local)
    - WiFi AP + STA, scan / connect; v2 saved WiFi credentials are kept

  Register map (holding regs, FC 03 / 06 / 16) — XY-SK120 / XY6020 family
    0x00 V-SET  x100     0x01 I-SET  x1000    0x02 VOUT x100     0x03 IOUT x1000
    0x04 POWER  x100     0x05 UIN    x100     0x06/07 Ah (mAh, lo/hi)
    0x08/09 Wh (mWh lo/hi)  0x0A/0B/0C out time h/m/s
    0x0D T-IN x10        0x0E T-EX x10        0x0F LOCK          0x10 PROTECT
    0x11 CV/CC           0x12 ONOFF           0x13 F-C           0x14 B-LED
    0x15 SLEEP           0x16 MODEL           0x17 VERSION       0x18 SLAVE-ADD
    0x19 BAUD code       0x1A T-IN-OFS        0x1B T-EX-OFS      0x1C BUZZER
    0x1D EXTRACT-M       0x1E DEVICE          0x1F MPPT-SW       0x20 MPPT-K x100
    0x21 BAT-FUL x1000   0x22 CP-SW           0x23 CP x10
    0x50 + 0x10*n  memory group n (14 regs):
         V I LVP OVP OCP OPP OHP_H OHP_M OAH_L OAH_H OWH_L OWH_H OTP INI
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#define FW_VERSION "3.2.1"

struct LogEntry { uint16_t v, i; };   // here so Arduino's auto-prototypes can see it

// Implemented in features.h (included near the end of this file)
void appendFeatureStatus(String &j);
bool outputOn(bool allowSoft);
void cancelRamp(bool restoreTarget);
void progStop(bool turnOff, const char *why);
extern bool rampActive;

// ======================= DEFAULTS ========================
const char*    AP_SSID_DEFAULT   = "SK120-AP";
const char*    AP_PASS_DEFAULT   = "12345678";
const char*    HOST_DEFAULT      = "sk120";
const uint8_t  MODBUS_ID_DEFAULT = 1;
const uint32_t MODBUS_BAUD_DEFAULT = 4800;

// SK120 baud-rate codes (register 0x19)
const uint32_t BAUD_TABLE[] = {9600, 14400, 19200, 38400, 56000, 57600, 115200, 2400, 4800};
const uint8_t  BAUD_COUNT   = sizeof(BAUD_TABLE) / sizeof(BAUD_TABLE[0]);

// ======================= CONFIG ==========================
// Offset 0: identical to v2 layout so saved home-WiFi credentials survive the upgrade.
struct Config {
  uint32_t magic;
  char staSsid[32];
  char staPass[64];
};
const uint32_t CFG_MAGIC = 0xABCD1234;

// Offset 128: v3 extension
struct ConfigExt {
  uint32_t magic;
  uint32_t baud;
  uint8_t  modbusId;
  char apSsid[32];
  char apPass[64];
  char host[24];
};
const uint32_t EXT_MAGIC  = 0x5A120003;
const int      EXT_OFFSET = 128;

Config    cfg;
ConfigExt ext;

void saveConfig() {
  EEPROM.put(0, cfg);
  EEPROM.put(EXT_OFFSET, ext);
  EEPROM.commit();
}

void loadConfig() {
  EEPROM.begin(1024);   // 0: WiFi (v2 layout), 128: v3 ext, 512: v3.1 options
  EEPROM.get(0, cfg);
  EEPROM.get(EXT_OFFSET, ext);
  bool dirty = false;
  if (cfg.magic != CFG_MAGIC) {
    memset(&cfg, 0, sizeof(cfg));
    cfg.magic = CFG_MAGIC;
    dirty = true;
  }
  if (ext.magic != EXT_MAGIC) {
    memset(&ext, 0, sizeof(ext));
    ext.magic    = EXT_MAGIC;
    ext.baud     = MODBUS_BAUD_DEFAULT;
    ext.modbusId = MODBUS_ID_DEFAULT;
    strncpy(ext.apSsid, AP_SSID_DEFAULT, sizeof(ext.apSsid) - 1);
    strncpy(ext.apPass, AP_PASS_DEFAULT, sizeof(ext.apPass) - 1);
    strncpy(ext.host,   HOST_DEFAULT,    sizeof(ext.host) - 1);
    dirty = true;
  }
  cfg.staSsid[31] = 0; cfg.staPass[63] = 0;
  ext.apSsid[31] = 0;  ext.apPass[63] = 0; ext.host[23] = 0;
  if (ext.modbusId == 0 || ext.modbusId > 247) { ext.modbusId = MODBUS_ID_DEFAULT; dirty = true; }
  if (dirty) saveConfig();
}

// ======================= MODBUS ==========================
uint32_t mbBaud;
uint32_t mbOkCount = 0, mbErrCount = 0;
uint8_t  mbLastExc = 0;   // last Modbus exception code

uint16_t modbusCRC(const uint8_t *buf, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t pos = 0; pos < len; pos++) {
    crc ^= buf[pos];
    for (uint8_t i = 0; i < 8; i++) crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
  }
  return crc;
}

void mbBegin(uint32_t baud) {
  mbBaud = baud;
  Serial.flush();
  Serial.end();
  Serial.begin(baud, SERIAL_8N1);
  Serial.setRxBufferSize(256);
  delay(20);
}

// Sends tx (CRC appended here), waits for exactly `expect` bytes (or a 5-byte exception).
bool mbXferOnce(uint8_t *tx, uint8_t txLen, uint8_t *rx, uint8_t expect) {
  uint16_t crc = modbusCRC(tx, txLen);
  tx[txLen]     = crc & 0xFF;
  tx[txLen + 1] = crc >> 8;

  while (Serial.available()) Serial.read();
  Serial.write(tx, txLen + 2);
  Serial.flush();

  uint32_t timeoutMs = 80 + ((uint32_t)expect * 11000UL) / mbBaud;
  uint32_t start = millis();
  uint8_t idx = 0;
  while (millis() - start < timeoutMs) {
    while (Serial.available() && idx < expect) rx[idx++] = Serial.read();
    if (idx >= expect) break;
    if (idx >= 5 && (rx[1] & 0x80)) break;
    yield();
  }
  if (idx >= 5 && (rx[1] & 0x80)) {
    if (modbusCRC(rx, 3) == (uint16_t)(rx[3] | (rx[4] << 8))) mbLastExc = rx[2];
    return false;
  }
  if (idx < expect) return false;
  if (modbusCRC(rx, expect - 2) != (uint16_t)(rx[expect - 2] | (rx[expect - 1] << 8))) return false;
  if (rx[0] != ext.modbusId || rx[1] != tx[1]) return false;
  return true;
}

bool mbXfer(uint8_t *tx, uint8_t txLen, uint8_t *rx, uint8_t expect) {
  for (uint8_t attempt = 0; attempt < 2; attempt++) {
    if (mbXferOnce(tx, txLen, rx, expect)) { mbOkCount++; return true; }
    delay(5);
  }
  mbErrCount++;
  return false;
}

bool mbRead(uint16_t addr, uint8_t count, uint16_t *dest) {
  if (count == 0 || count > 60) return false;
  uint8_t tx[8] = { ext.modbusId, 0x03, (uint8_t)(addr >> 8), (uint8_t)addr, 0, count };
  uint8_t rx[5 + 2 * 60];
  uint8_t expect = 5 + count * 2;
  if (!mbXfer(tx, 6, rx, expect)) return false;
  if (rx[2] != count * 2) return false;
  for (uint8_t i = 0; i < count; i++) dest[i] = (rx[3 + i * 2] << 8) | rx[4 + i * 2];
  return true;
}

bool mbWrite(uint16_t addr, uint16_t value) {
  uint8_t tx[8] = { ext.modbusId, 0x06, (uint8_t)(addr >> 8), (uint8_t)addr, (uint8_t)(value >> 8), (uint8_t)value };
  uint8_t rx[8];
  if (!mbXfer(tx, 6, rx, 8)) return false;
  return memcmp(rx, tx, 6) == 0;
}

bool mbWriteMulti(uint16_t addr, uint8_t count, const uint16_t *vals) {
  if (count == 0 || count > 30) return false;
  uint8_t tx[9 + 2 * 30];
  tx[0] = ext.modbusId; tx[1] = 0x10;
  tx[2] = addr >> 8;    tx[3] = addr & 0xFF;
  tx[4] = 0;            tx[5] = count;
  tx[6] = count * 2;
  for (uint8_t i = 0; i < count; i++) { tx[7 + i * 2] = vals[i] >> 8; tx[8 + i * 2] = vals[i] & 0xFF; }
  uint8_t rx[8];
  if (!mbXfer(tx, 7 + count * 2, rx, 8)) return false;
  return memcmp(rx, tx, 6) == 0;
}

// ======================= LIVE CACHE ======================
const uint8_t REG_FAST = 0x13;          // 0x00..0x12 polled fast
const uint8_t REG_TOTAL = 0x24;         // 0x00..0x23
uint16_t R[REG_TOTAL];
uint8_t  slowCount = REG_TOTAL - REG_FAST;   // shrinks if device rejects 0x1E..0x23
uint8_t  slowFails = 0;
bool     haveData = false;
uint32_t lastOkMs = 0, lastFastMs = 0, lastSlowMs = 0, lastLogMs = 0, lastDetectMs = 0;

bool writableReg(uint16_t a) {
  if (a <= 0x01) return true;                    // V-SET, I-SET
  if (a == 0x0F || a == 0x10 || a == 0x12) return true;  // lock, protect clear, on/off
  if (a >= 0x13 && a <= 0x15) return true;       // F-C, backlight, sleep
  if (a >= 0x1A && a <= 0x1D) return true;       // temp offsets, buzzer, extract-M
  if (a >= 0x1F && a <= 0x23) return true;       // MPPT, BAT-FUL, CP
  if (a >= 0x30 && a <= 0x34) return true;       // WiFi host block (screen icon)
  if (a >= 0x50 && a < 0x50 + 0x10 * 10) return (a & 0x0F) < 14;  // memory groups
  return false;
}

// ======================= SCREEN WiFi ICON ================
// 0x30 MASTER=0x3B3A (WiFi host), 0x31 WIFI-CONFIG, 0x32 WIFI-STATUS, 0x33/0x34 IPv4.
// Icon is shown while the control page is open (it polls /status), cleared when it closes.
const uint32_t PAGE_OPEN_MS   = 6000;
const uint32_t ICON_REFRESH_MS = 10000;
uint32_t lastPageMs = 0, lastIconWriteMs = 0;
uint16_t iconOnStatus = 4;        // WIFI-STATUS value used for "connected" (tunable via /icon?s=)
int16_t  iconWritten = -1;        // last status written (-1 = never)
bool     iconEnabled = true;

uint8_t wsCount();   // ws.h
bool pageOpen() { return wsCount() > 0 || (lastPageMs && millis() - lastPageMs < PAGE_OPEN_MS); }

bool writeIcon(uint16_t status) {
  IPAddress ip = WiFi.isConnected() ? WiFi.localIP() : WiFi.softAPIP();
  uint16_t vals[5] = { status ? (uint16_t)0x3B3A : (uint16_t)0, (uint16_t)(status ? 2 : 0), status,
                       (uint16_t)((ip[0] << 8) | ip[1]), (uint16_t)((ip[2] << 8) | ip[3]) };
  bool ok = mbWriteMulti(0x30, 5, vals);
  if (ok) { iconWritten = status; lastIconWriteMs = millis(); }
  return ok;
}

void updateIcon() {
  uint16_t want = (iconEnabled && pageOpen()) ? iconOnStatus : 0;
  bool refresh = want && millis() - lastIconWriteMs > ICON_REFRESH_MS;
  if ((int16_t)want != iconWritten || refresh) writeIcon(want);
}

void cacheUpdate(uint16_t addr, uint8_t count, const uint16_t *vals) {
  for (uint8_t i = 0; i < count; i++)
    if (addr + i < REG_TOTAL) R[addr + i] = vals[i];
}

bool detectBaud() {
  uint16_t tmp;
  uint32_t order[BAUD_COUNT + 1];
  order[0] = ext.baud;
  uint8_t n = 1;
  for (uint8_t i = 0; i < BAUD_COUNT; i++) if (BAUD_TABLE[i] != ext.baud) order[n++] = BAUD_TABLE[i];
  for (uint8_t i = 0; i < n; i++) {
    mbBegin(order[i]);
    if (mbRead(0x00, 1, &tmp)) {
      if (ext.baud != order[i]) { ext.baud = order[i]; saveConfig(); }
      return true;
    }
  }
  mbBegin(ext.baud);
  return false;
}

// ======================= LOG =============================
const uint16_t LOG_SIZE = 1800;  // 30 minutes @ 1 s (one sample per second, newest last)
LogEntry logBuf[LOG_SIZE];
uint16_t logIndex = 0;
bool     logWrapped = false;
uint32_t logLastSec = 0;          // uptime second of the newest sample

void appendLog() {
  logBuf[logIndex] = { R[0x02], R[0x03] };
  if (++logIndex >= LOG_SIZE) { logIndex = 0; logWrapped = true; }
  logLastSec = millis() / 1000;
}
uint16_t logCount() { return logWrapped ? LOG_SIZE : logIndex; }
const LogEntry &logAt(uint16_t k) {   // k = 0 oldest
  uint16_t start = logWrapped ? logIndex : 0;
  return logBuf[(start + k) % LOG_SIZE];
}

// ======================= WEB =============================
ESP8266WebServer        server(80);
ESP8266HTTPUpdateServer httpUpdater;

#include "page.h"

void sendOK()            { server.send(200, "text/plain", "OK"); }
void sendErr(int c, const char *m) { server.send(c, "text/plain", m); }
long argNum(const char *k, long def = -1) {
  if (!server.hasArg(k)) return def;
  return strtol(server.arg(k).c_str(), nullptr, 0);   // accepts 0x.. hex
}

void handleRoot() { server.send_P(200, "text/html", PAGE_HTML); }

// /icon?s=4  (set WIFI-STATUS used for "connected")   /icon?en=0|1
void handleIcon() {
  if (server.hasArg("s")) { long s = argNum("s"); if (s < 1 || s > 9) return sendErr(400, "s 1..9"); iconOnStatus = s; iconWritten = -1; }
  if (server.hasArg("en")) { iconEnabled = argNum("en", 1) != 0; iconWritten = -1; }
  String j = "{\"s\":" + String(iconOnStatus) + ",\"en\":" + String(iconEnabled ? 1 : 0) + ",\"wi\":" + String(iconWritten) + "}";
  server.send(200, "application/json", j);
}

uint8_t wsCount();   // ws.h

void buildStatus(String &j) {
  j.reserve(660);
  j = F("{\"ok\":");
  j += (haveData && millis() - lastOkMs < 3000) ? 1 : 0;
  j += F(",\"age\":");  j += haveData ? (millis() - lastOkMs) : 0;
  j += F(",\"ext\":");  j += slowCount;
  j += F(",\"wi\":");   j += iconWritten;
  j += F(",\"fast\":"); j += mbBaud >= 38400 ? 1 : 0;
  j += F(",\"ws\":");   j += wsCount();
  appendFeatureStatus(j);
  j += F(",\"r\":[");
  for (uint8_t i = 0; i < REG_TOTAL; i++) { if (i) j += ','; j += R[i]; }
  j += F("]}");
}

void handleStatus() {
  lastPageMs = millis();
  String j;
  buildStatus(j);
  server.send(200, "application/json", j);
}

// /set?v=12.00&i=1.000   (either may be omitted)
void handleSet() {
  bool hv = server.hasArg("v"), hi = server.hasArg("i");
  if (!hv && !hi) return sendErr(400, "need v or i");
  float v = hv ? server.arg("v").toFloat() : R[0] / 100.0f;
  float i = hi ? server.arg("i").toFloat() : R[1] / 1000.0f;
  v = constrain(v, 0.0f, 36.0f);
  i = constrain(i, 0.0f, 6.0f);
  uint16_t vals[2] = { (uint16_t)(v * 100.0f + 0.5f), (uint16_t)(i * 1000.0f + 0.5f) };
  if (rampActive) { cancelRamp(false); if (!hv) vals[0] = R[0]; }   // user takes over during soft start
  bool ok = mbWriteMulti(0x00, 2, vals);
  if (!ok) ok = mbWrite(0x00, vals[0]) && mbWrite(0x01, vals[1]);   // fallback if FC16 unsupported
  if (!ok) return sendErr(500, "ERR");
  cacheUpdate(0x00, 2, vals);
  sendOK();
}

// /power?on=1[&soft=0]
void handlePower() {
  if (argNum("on", 0)) {
    if (!outputOn(argNum("soft", 1) != 0)) return sendErr(500, "ERR");
  } else {
    progStop(false, nullptr);
    cancelRamp(true);
    if (!mbWrite(0x12, 0)) return sendErr(500, "ERR");
    R[0x12] = 0;
  }
  sendOK();
}

void handleLock() {
  uint16_t v = argNum("on", 0) ? 1 : 0;
  if (!mbWrite(0x0F, v)) return sendErr(500, "ERR");
  R[0x0F] = v; sendOK();
}

// /w?a=0x14&v=3
void handleWriteReg() {
  long a = argNum("a"), v = argNum("v");
  if (a < 0 || v < -32768 || v > 65535) return sendErr(400, "bad args");
  if (!writableReg(a)) return sendErr(403, "register not writable");
  uint16_t val = (uint16_t)(v & 0xFFFF);
  if (!mbWrite(a, val)) return sendErr(500, "ERR");
  cacheUpdate(a, 1, &val);
  sendOK();
}

// /wm?a=0x60&v=500,1000,...
void handleWriteMulti() {
  long a = argNum("a");
  String s = server.arg("v");
  if (a < 0 || s.length() == 0) return sendErr(400, "bad args");
  uint16_t vals[30]; uint8_t n = 0;
  int start = 0;
  while (n < 30 && start <= (int)s.length()) {
    int comma = s.indexOf(',', start);
    if (comma < 0) comma = s.length();
    vals[n++] = (uint16_t)(strtol(s.substring(start, comma).c_str(), nullptr, 0) & 0xFFFF);
    start = comma + 1;
  }
  for (uint8_t k = 0; k < n; k++) if (!writableReg(a + k)) return sendErr(403, "register not writable");
  bool ok = mbWriteMulti(a, n, vals);
  if (!ok) { ok = true; for (uint8_t k = 0; k < n && ok; k++) ok = mbWrite(a + k, vals[k]); }
  if (!ok) return sendErr(500, "ERR");
  cacheUpdate(a, n, vals);
  sendOK();
}

// /r?a=0x50&n=14   -> [..]
void handleReadReg() {
  long a = argNum("a"), n = argNum("n", 1);
  if (a < 0 || a > 0xFFFF || n < 1 || n > 60) return sendErr(400, "bad args");
  uint16_t buf[60];
  if (!mbRead(a, n, buf)) {
    String e = F("ERR exc="); e += mbLastExc;
    return sendErr(500, e.c_str());
  }
  String j = "[";
  for (long i = 0; i < n; i++) { if (i) j += ','; j += buf[i]; }
  j += ']';
  server.send(200, "application/json", j);
}

// /mem?n=0..9
void handleMem() {
  long n = argNum("n", 0);
  if (n < 0 || n > 9) return sendErr(400, "n 0..9");
  uint16_t buf[14];
  if (!mbRead(0x50 + n * 0x10, 14, buf)) return sendErr(500, "ERR");
  String j = "[";
  for (uint8_t i = 0; i < 14; i++) { if (i) j += ','; j += buf[i]; }
  j += ']';
  server.send(200, "application/json", j);
}

// /recall?n=1..9   (loads group n into active M0)
void handleRecall() {
  long n = argNum("n", -1);
  if (n < 0 || n > 9) return sendErr(400, "n 0..9");
  if (!mbWrite(0x1D, n)) return sendErr(500, "ERR");
  lastFastMs = 0; lastSlowMs = 0;   // refresh soon
  sendOK();
}

// /baud?c=0..8  -> writes SK120 baud code, then ESP follows
void handleBaud() {
  long c = argNum("c", -1);
  if (c < 0 || c >= BAUD_COUNT) return sendErr(400, "c 0..8");
  bool ok = mbWrite(0x19, c);
  sendOK();               // reply before switching
  delay(50);
  ext.baud = BAUD_TABLE[c];
  saveConfig();
  mbBegin(ext.baud);
  (void)ok;
}

void handleInfo() {
  String staIP = WiFi.localIP().toString();
  if (!WiFi.isConnected()) staIP = "";
  String j = "{";
  j += "\"fw\":\"" FW_VERSION "\",";
  j += "\"apSsid\":\"" + String(ext.apSsid) + "\",";
  j += "\"apIp\":\"" + WiFi.softAPIP().toString() + "\",";
  j += "\"staSsid\":\"" + String(cfg.staSsid) + "\",";
  j += "\"staIp\":\"" + staIP + "\",";
  j += "\"rssi\":" + String(WiFi.isConnected() ? WiFi.RSSI() : 0) + ",";
  j += "\"host\":\"" + String(ext.host) + "\",";
  j += "\"mac\":\"" + WiFi.macAddress() + "\",";
  j += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
  j += "\"up\":" + String(millis() / 1000) + ",";
  j += "\"baud\":" + String(mbBaud) + ",";
  j += "\"id\":" + String(ext.modbusId) + ",";
  j += "\"mbOk\":" + String(mbOkCount) + ",";
  j += "\"mbErr\":" + String(mbErrCount);
  j += "}";
  server.send(200, "application/json", j);
}

String jsonEscape(const String &s) {
  String o; o.reserve(s.length() + 4);
  for (char c : s) { if (c == '"' || c == '\\') o += '\\'; if ((uint8_t)c >= 0x20) o += c; }
  return o;
}

void handleScan() {
  int n = WiFi.scanNetworks();
  String j = "[";
  for (int i = 0; i < n; i++) {
    if (i) j += ",";
    j += "{\"ssid\":\"" + jsonEscape(WiFi.SSID(i)) + "\",\"rssi\":" + String(WiFi.RSSI(i)) +
         ",\"sec\":" + String(WiFi.encryptionType(i) != ENC_TYPE_NONE ? 1 : 0) + "}";
  }
  j += "]";
  WiFi.scanDelete();
  server.send(200, "application/json", j);
}

// /wifi?mode=sta&ssid=..&pass=..
void handleWifi() {
  if (server.arg("mode") != "sta") return sendErr(400, "Bad mode");
  server.arg("ssid").toCharArray(cfg.staSsid, sizeof(cfg.staSsid));
  server.arg("pass").toCharArray(cfg.staPass, sizeof(cfg.staPass));
  saveConfig();
  sendOK();
  delay(100);
  WiFi.begin(cfg.staSsid, cfg.staPass);
}

// /ap?ssid=..&pass=..   (pass >= 8 chars, or empty for open AP)
void handleAp() {
  String s = server.arg("ssid"), p = server.arg("pass");
  if (s.length() < 1 || s.length() > 31) return sendErr(400, "SSID 1..31 chars");
  if (p.length() != 0 && (p.length() < 8 || p.length() > 63)) return sendErr(400, "Password 8..63 chars");
  s.toCharArray(ext.apSsid, sizeof(ext.apSsid));
  p.toCharArray(ext.apPass, sizeof(ext.apPass));
  saveConfig();
  sendOK();
  delay(200);
  WiFi.softAP(ext.apSsid, ext.apPass);
}

// /modbus?id=1
void handleModbusId() {
  long id = argNum("id", -1);
  if (id < 1 || id > 247) return sendErr(400, "id 1..247");
  ext.modbusId = id;
  saveConfig();
  sendOK();
}

void handleReboot() { sendOK(); delay(300); ESP.restart(); }

void handleLogCsv() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.sendHeader("Content-Disposition", "attachment; filename=sk120_log.csv");
  server.send(200, "text/csv", "seconds_ago,voltage_V,current_A,power_W\r\n");
  uint16_t count = logCount();
  String chunk; chunk.reserve(1100);
  char line[48];
  for (uint16_t k = 0; k < count; k++) {
    const LogEntry &e = logAt(k);
    snprintf(line, sizeof(line), "%u,%.2f,%.3f,%.3f\r\n", (unsigned)(count - 1 - k), e.v / 100.0, e.i / 1000.0, e.v * (double)e.i / 100000.0);
    chunk += line;
    if (chunk.length() > 1000) { server.sendContent(chunk); chunk = ""; }
  }
  if (chunk.length()) server.sendContent(chunk);
  server.sendContent("");
}

// /log.json?sec=600&pts=300 -> {"step":2,"v":[...],"i":[...]}  raw values (V x100, A x1000), oldest first
void handleLogJson() {
  long sec = constrain(argNum("sec", 180), 10L, (long)LOG_SIZE);
  long pts = constrain(argNum("pts", 300), 10L, 600L);
  uint16_t count = logCount();
  uint16_t n = min<long>(sec, count);
  uint16_t step = max<long>(1, (n + pts - 1) / pts);
  uint16_t first = count - n;
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", String("{\"step\":") + step + ",\"v\":[");
  String chunk; chunk.reserve(700);
  for (uint8_t pass = 0; pass < 2; pass++) {
    if (pass) chunk += "],\"i\":[";
    bool firstItem = true;
    for (uint16_t k = first + (n - 1) % step; k < count; k += step) {
      if (!firstItem) chunk += ',';
      firstItem = false;
      chunk += pass ? logAt(k).i : logAt(k).v;
      if (chunk.length() > 600) { server.sendContent(chunk); chunk = ""; }
    }
  }
  chunk += "]}";
  server.sendContent(chunk);
  server.sendContent("");
}

#include "features.h"
#include "ws.h"

// ======================= SETUP / LOOP ====================
void setup() {
  loadConfig();
  mbBegin(ext.baud);

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.hostname(ext.host);
  WiFi.softAP(ext.apSsid, ext.apPass);
  if (strlen(cfg.staSsid) > 0) WiFi.begin(cfg.staSsid, cfg.staPass);

  server.on("/",        handleRoot);
  server.on("/status",  handleStatus);
  server.on("/set",     handleSet);
  server.on("/power",   handlePower);
  server.on("/lock",    handleLock);
  server.on("/w",       handleWriteReg);
  server.on("/wm",      handleWriteMulti);
  server.on("/r",       handleReadReg);
  server.on("/mem",     handleMem);
  server.on("/recall",  handleRecall);
  server.on("/baud",    handleBaud);
  server.on("/modbus",  handleModbusId);
  server.on("/info",    handleInfo);
  server.on("/scan",    handleScan);
  server.on("/wifi",    handleWifi);
  server.on("/ap",      handleAp);
  server.on("/reboot",  handleReboot);
  server.on("/icon",    handleIcon);
  server.on("/log.csv", handleLogCsv);
  server.on("/log.json", handleLogJson);
  featuresSetup();
  wsSetup();
  httpUpdater.setup(&server, "/update", "admin", ext.apPass);   // OTA: user admin / AP password
  server.begin();

  MDNS.begin(ext.host);
  MDNS.addService("http", "tcp", 80);

  detectBaud();
  lastDetectMs = millis();
}

void loop() {
  server.handleClient();
  MDNS.update();

  uint32_t now = millis();
  bool fresh = false;

  if (now - lastFastMs >= (mbBaud >= 38400 ? 150u : 400u)) {
    lastFastMs = now;
    uint16_t buf[REG_FAST];
    if (mbRead(0x00, REG_FAST, buf)) {
      cacheUpdate(0x00, REG_FAST, buf);
      haveData = true;
      lastOkMs = millis();
      fresh = true;
    }
  }

  if (now - lastSlowMs >= 2500 && haveData && millis() - lastOkMs < 2000) {
    lastSlowMs = now;
    uint16_t buf[REG_TOTAL - REG_FAST];
    if (mbRead(REG_FAST, slowCount, buf)) {
      cacheUpdate(REG_FAST, slowCount, buf);
      slowFails = 0;
    } else if (++slowFails >= 3 && slowCount > 0x0B) {
      slowCount = 0x0B;            // device without 0x1E..0x23: read 0x13..0x1D only
      slowFails = 0;
    }
  }

  if (haveData && millis() - lastOkMs < 2000) updateIcon();
  featuresLoop();
  wsLoop(fresh);

  if (now - lastLogMs >= 1000) {
    lastLogMs = now;
    if (haveData) appendLog();
  }

  // Lost link for 15 s -> try other baud rates (at most once a minute)
  if (millis() - lastOkMs > 15000 && millis() - lastDetectMs > 60000) {
    lastDetectMs = millis();
    detectBaud();
  }
}
