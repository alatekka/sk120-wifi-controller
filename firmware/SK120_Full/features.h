#pragma once
// v3.1 features: soft start, auto-off timer, daily schedule, step program, battery charge job,
// notifications (Telegram + MQTT / Home Assistant), SK120 screensaver clock.
// Included from SK120_Full.ino after the Modbus helpers, cache (R[]) and web server objects.

#include <WiFiClientSecure.h>
#include <ArduinoMqttClient.h>
#include <time.h>

// ======================= OPTIONS (EEPROM @512) ===========
struct Step { uint16_t v, i, sec; };      // v x100, i x1000
const uint8_t MAX_STEPS = 10;

struct Opts {
  uint32_t magic;
  uint16_t softMs;           // soft-start ramp duration, 0 = off
  int16_t  tzMin;            // local time offset, minutes
  uint8_t  clockEn;          // push clock to SK120 screensaver
  uint8_t  schedEn;
  uint16_t schedOn, schedOff;  // minute of day, 0xFFFF = unused
  uint8_t  tgEn;
  char     tgToken[52];
  char     tgChat[20];
  uint8_t  mqEn;
  char     mqHost[40];
  uint16_t mqPort;
  char     mqUser[24];
  char     mqPass[32];
  char     mqBase[24];
  uint8_t  nSteps;
  uint8_t  repeat;           // 0 = run once, 255 = forever, else extra loops
  Step     steps[MAX_STEPS];
};
const uint32_t OPTS_MAGIC  = 0x5A120310;
const int      OPTS_OFFSET = 512;
Opts opt;

void saveOpts() { EEPROM.put(OPTS_OFFSET, opt); EEPROM.commit(); }

void loadOpts() {
  EEPROM.get(OPTS_OFFSET, opt);
  if (opt.magic != OPTS_MAGIC) {
    memset(&opt, 0, sizeof(opt));
    opt.magic = OPTS_MAGIC;
    opt.tzMin = 120;
    opt.clockEn = 1;
    opt.schedOn = opt.schedOff = 0xFFFF;
    opt.mqPort = 1883;
    strcpy(opt.mqBase, "sk120");
    saveOpts();
  }
  opt.tgToken[51] = 0; opt.tgChat[19] = 0; opt.mqHost[39] = 0;
  opt.mqUser[23] = 0; opt.mqPass[31] = 0; opt.mqBase[23] = 0;
  if (opt.nSteps > MAX_STEPS) opt.nSteps = 0;
}

// ======================= TIME ============================
bool     timeValid() { return time(nullptr) > 1700000000; }
uint32_t localEpoch() { return (uint32_t)(time(nullptr) + (int32_t)opt.tzMin * 60); }
int      localMinuteOfDay() { uint32_t t = localEpoch(); return (t / 60) % 1440; }

// ======================= NOTIFICATIONS ===================
String   tgQueue;            // pending Telegram text (one at a time, newest wins)
String   lastTgResult = "-";
MqttClient *mqtt = nullptr;
WiFiClient  mqttNet;
uint32_t lastMqttTry = 0, lastMqttPub = 0;
bool     mqttDiscoverySent = false;

String urlEncode(const String &s) {
  String o; char hex[4];
  for (uint8_t c : s) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') o += (char)c;
    else { snprintf(hex, sizeof(hex), "%%%02X", c); o += hex; }
  }
  return o;
}

void mqttPublish(const String &topic, const String &payload, bool retain = false) {
  if (!mqtt || !mqtt->connected()) return;
  mqtt->beginMessage(topic, payload.length(), retain);
  mqtt->print(payload);
  mqtt->endMessage();
}

void notify(const String &msg) {
  if (opt.tgEn && opt.tgToken[0] && opt.tgChat[0]) tgQueue = msg;
  mqttPublish(String(opt.mqBase) + "/event", msg);
}

bool sendTelegramNow(const String &msg) {
  if (!WiFi.isConnected()) { lastTgResult = "no WiFi"; return false; }
  if (ESP.getMaxFreeBlockSize() < 18000) { lastTgResult = "low memory"; return false; }
  WiFiClientSecure c;
  c.setInsecure();
  c.setBufferSizes(16384, 512);
  c.setTimeout(6000);
  if (!c.connect("api.telegram.org", 443)) { lastTgResult = "connect failed"; return false; }
  String path = String("/bot") + opt.tgToken + "/sendMessage?chat_id=" + urlEncode(opt.tgChat) + "&text=" + urlEncode(msg);
  c.print(String("GET ") + path + " HTTP/1.1\r\nHost: api.telegram.org\r\nConnection: close\r\n\r\n");
  String status = c.readStringUntil('\n');
  c.stop();
  bool ok = status.indexOf(" 200") > 0;
  lastTgResult = ok ? "OK" : (status.length() ? status.substring(0, 40) : String("no reply"));
  return ok;
}

// ======================= SOFT START ======================
bool     rampActive = false;
uint16_t rampTarget = 0;
uint32_t rampT0 = 0, rampLastMs = 0;

void cancelRamp(bool restoreTarget) {
  if (!rampActive) return;
  rampActive = false;
  if (restoreTarget) { mbWrite(0x00, rampTarget); R[0] = rampTarget; }
}

// Turns the output on, ramping V-SET from 0 when soft start is enabled.
bool outputOn(bool allowSoft) {
  if (allowSoft && opt.softMs >= 200) {
    rampTarget = R[0];
    if (!mbWrite(0x00, 0)) return false;
    R[0] = 0;
    if (!mbWrite(0x12, 1)) { mbWrite(0x00, rampTarget); R[0] = rampTarget; return false; }
    R[0x12] = 1;
    rampActive = true; rampT0 = rampLastMs = millis();
    return true;
  }
  if (!mbWrite(0x12, 1)) return false;
  R[0x12] = 1;
  return true;
}

void rampLoop() {
  if (!rampActive) return;
  if (R[0x12] == 0) { cancelRamp(true); return; }       // switched off on the device
  uint32_t now = millis();
  if (now - rampLastMs < 120) return;
  rampLastMs = now;
  uint32_t el = now - rampT0;
  uint16_t v = el >= opt.softMs ? rampTarget : (uint16_t)((uint32_t)rampTarget * el / opt.softMs);
  if (mbWrite(0x00, v)) R[0] = v;
  if (el >= opt.softMs) rampActive = false;
}

// ======================= AUTO-OFF TIMER ==================
uint32_t offAtMs = 0;        // 0 = no timer

// ======================= STEP PROGRAM ====================
bool     progRunning = false;
uint8_t  progIdx = 0;
uint16_t progLoopsDone = 0;
uint32_t progStepEnd = 0;

bool setVI(uint16_t v, uint16_t i) {
  uint16_t vals[2] = { v, i };
  bool ok = mbWriteMulti(0x00, 2, vals);
  if (!ok) ok = mbWrite(0x00, v) && mbWrite(0x01, i);
  if (ok) cacheUpdate(0x00, 2, vals);
  return ok;
}

void progStop(bool turnOff, const char *why) {
  if (!progRunning) return;
  progRunning = false;
  cancelRamp(false);
  if (turnOff && mbWrite(0x12, 0)) R[0x12] = 0;
  if (why) notify(why);
}

bool progStart() {
  if (opt.nSteps == 0) return false;
  progIdx = 0; progLoopsDone = 0;
  if (!setVI(opt.steps[0].v, opt.steps[0].i)) return false;
  if (!outputOn(true)) return false;
  progRunning = true;
  progStepEnd = millis() + (uint32_t)opt.steps[0].sec * 1000UL;
  return true;
}

void progLoop() {
  if (!progRunning) return;
  if (R[0x12] == 0 && !rampActive) { progStop(false, "Program stopped: output was switched off"); return; }
  if (rampActive) { progStepEnd = millis() + (uint32_t)opt.steps[progIdx].sec * 1000UL; return; }  // step time starts after soft start
  if ((int32_t)(millis() - progStepEnd) < 0) return;
  progIdx++;
  if (progIdx >= opt.nSteps) {
    progIdx = 0;
    progLoopsDone++;
    if (opt.repeat != 255 && progLoopsDone > opt.repeat) { progStop(true, "Step program finished"); return; }
  }
  cancelRamp(false);
  setVI(opt.steps[progIdx].v, opt.steps[progIdx].i);
  progStepEnd = millis() + (uint32_t)opt.steps[progIdx].sec * 1000UL;
}

uint32_t progLeftSec() { return progRunning ? (uint32_t)max<int32_t>(0, (int32_t)(progStepEnd - millis())) / 1000 : 0; }

// ======================= BATTERY CHARGE JOB ==============
bool     chgActive = false;
uint32_t chgBaseAh = 0, chgBaseWh = 0, chgT0 = 0, chgCapMah = 0;
uint32_t ahNow() { return ((uint32_t)R[7] << 16) | R[6]; }
uint32_t whNow() { return ((uint32_t)R[9] << 16) | R[8]; }

void chgFinish(const char *why) {
  if (!chgActive) return;
  chgActive = false;
  uint32_t mah = ahNow() - chgBaseAh;
  uint32_t mins = (millis() - chgT0) / 60000;
  String m = String(why) + ": " + mah + " mAh";
  if (chgCapMah) m += " (" + String(mah * 100 / chgCapMah) + "%)";
  m += ", " + String((whNow() - chgBaseWh) / 1000.0, 2) + " Wh, " + mins + " min";
  notify(m);
}

void chgLoop() {
  if (chgActive && R[0x12] == 0) chgFinish(R[0x10] ? "Charging stopped by protection" : "Battery charged");
}

// ======================= EVENT WATCH =====================
const char *PROT_NAMES[] = {"OK","OVP","OCP","OPP","LVP","OAH","OHP","OTP","OEP","OWH","ICP","ETP"};
uint16_t lastProt = 0;

void eventLoop() {
  uint16_t p = R[0x10];
  if (p != lastProt) {
    if (p) notify(String("SK120 protection tripped: ") + (p < 12 ? PROT_NAMES[p] : String(p).c_str()) +
                  " (Vout " + String(R[2] / 100.0, 2) + " V, Iout " + String(R[3] / 1000.0, 3) + " A)");
    lastProt = p;
  }
  if (offAtMs && (int32_t)(millis() - offAtMs) >= 0) {
    offAtMs = 0;
    progStop(false, nullptr);
    cancelRamp(true);
    if (mbWrite(0x12, 0)) R[0x12] = 0;
    notify("Timer finished: output switched OFF");
  }
}

// ======================= DAILY SCHEDULE ==================
int lastSchedMinute = -1;
void schedLoop() {
  if (!opt.schedEn || !timeValid()) return;
  int m = localMinuteOfDay();
  if (m == lastSchedMinute) return;
  lastSchedMinute = m;
  if (m == opt.schedOn && R[0x12] == 0) { outputOn(true); notify("Schedule: output ON"); }
  if (m == opt.schedOff && R[0x12] == 1) { progStop(false, nullptr); cancelRamp(true); if (mbWrite(0x12, 0)) R[0x12] = 0; notify("Schedule: output OFF"); }
}

// ======================= SCREEN CLOCK ====================
uint32_t lastClockMs = 0;
int8_t   clockLastOk = -1;     // -1 never written, 0 failed, 1 ok
uint8_t  clockFails = 0;       // device without 0x0200 block (older SK120) -> give up after 3
void clockLoop() {
  if (!opt.clockEn || clockFails >= 3 || !timeValid() || millis() - lastClockMs < 10000) return;
  lastClockMs = millis();
  uint32_t t = localEpoch();
  uint16_t regs[21] = {0};
  regs[0] = t & 0xFFFF;
  regs[1] = t >> 16;
  regs[2] = 0x0003;          // time synced
  regs[3] = 60;              // weather: N/A
  regs[12] = regs[15] = regs[18] = 60;
  clockLastOk = mbWriteMulti(0x0200, 21, regs) ? 1 : 0;
  clockFails = clockLastOk ? 0 : clockFails + 1;
}

// ======================= MQTT / HOME ASSISTANT ===========
String mqBase() { return String(opt.mqBase); }

void mqttDiscovery() {
  String id = WiFi.macAddress(); id.replace(":", "");
  String dev = "\"dev\":{\"ids\":[\"sk120_" + id + "\"],\"name\":\"XY-SK120\",\"mf\":\"XY\",\"mdl\":\"SK120\",\"sw\":\"" FW_VERSION "\"}";
  String st = mqBase() + "/state";
  struct S { const char *key, *name, *unit, *cls; };
  const S sensors[] = {
    {"v", "Output voltage", "V", "voltage"}, {"i", "Output current", "A", "current"},
    {"p", "Output power", "W", "power"},     {"vin", "Input voltage", "V", "voltage"},
    {"t", "Temperature", "°C", "temperature"}, {"wh", "Energy", "Wh", "energy"},
    {"ah", "Capacity", "Ah", ""},            {"prot", "Protection", "", ""}, {"mode", "Mode", "", ""}};
  for (auto &s : sensors) {
    String cfg = String("{\"name\":\"") + s.name + "\",\"uniq_id\":\"sk120_" + id + "_" + s.key +
                 "\",\"stat_t\":\"" + st + "\",\"val_tpl\":\"{{value_json." + s.key + "}}\"";
    if (s.unit[0]) cfg += String(",\"unit_of_meas\":\"") + s.unit + "\"";
    if (s.cls[0])  cfg += String(",\"dev_cla\":\"") + s.cls + "\"";
    if (!strcmp(s.key, "wh")) cfg += ",\"stat_cla\":\"total_increasing\"";
    else if (s.unit[0]) cfg += ",\"stat_cla\":\"measurement\"";
    cfg += "," + dev + "}";
    mqttPublish("homeassistant/sensor/sk120_" + id + "/" + s.key + "/config", cfg, true);
  }
  String nv = "{\"name\":\"Set voltage\",\"uniq_id\":\"sk120_" + id + "_vs\",\"stat_t\":\"" + st +
              "\",\"val_tpl\":\"{{value_json.vs}}\",\"cmd_t\":\"" + mqBase() + "/set/voltage\",\"min\":0,\"max\":36,\"step\":0.01,\"unit_of_meas\":\"V\",\"mode\":\"box\"," + dev + "}";
  mqttPublish("homeassistant/number/sk120_" + id + "/vs/config", nv, true);
  String ni = "{\"name\":\"Set current\",\"uniq_id\":\"sk120_" + id + "_is\",\"stat_t\":\"" + st +
              "\",\"val_tpl\":\"{{value_json.is}}\",\"cmd_t\":\"" + mqBase() + "/set/current\",\"min\":0,\"max\":6,\"step\":0.001,\"unit_of_meas\":\"A\",\"mode\":\"box\"," + dev + "}";
  mqttPublish("homeassistant/number/sk120_" + id + "/is/config", ni, true);
  String sw = "{\"name\":\"Output\",\"uniq_id\":\"sk120_" + id + "_out\",\"stat_t\":\"" + st +
              "\",\"val_tpl\":\"{{value_json.on}}\",\"cmd_t\":\"" + mqBase() + "/set/output\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\"," + dev + "}";
  mqttPublish("homeassistant/switch/sk120_" + id + "/out/config", sw, true);
  mqttDiscoverySent = true;
}

void onMqttMessage(int len) {
  String topic = mqtt->messageTopic();
  String msg;
  while (mqtt->available() && msg.length() < 32) msg += (char)mqtt->read();
  msg.trim();
  String base = mqBase() + "/set/";
  if (!topic.startsWith(base)) return;
  String what = topic.substring(base.length());
  if (what == "voltage") { float v = constrain(msg.toFloat(), 0.0f, 36.0f); cancelRamp(false); setVI((uint16_t)(v * 100 + 0.5f), R[1]); }
  else if (what == "current") { float i = constrain(msg.toFloat(), 0.0f, 6.0f); setVI(R[0], (uint16_t)(i * 1000 + 0.5f)); }
  else if (what == "output") {
    bool on = msg.equalsIgnoreCase("ON") || msg == "1";
    if (on) outputOn(true);
    else { progStop(false, nullptr); cancelRamp(true); if (mbWrite(0x12, 0)) R[0x12] = 0; }
  }
  lastMqttPub = 0;   // publish new state soon
}

String mqttState() {
  char b[300];
  snprintf(b, sizeof(b),
    "{\"v\":%.2f,\"i\":%.3f,\"p\":%.2f,\"vin\":%.2f,\"vs\":%.2f,\"is\":%.3f,\"t\":%.1f,\"wh\":%.3f,\"ah\":%.3f,"
    "\"on\":\"%s\",\"mode\":\"%s\",\"prot\":\"%s\",\"lock\":%d}",
    R[2] / 100.0, R[3] / 1000.0, R[4] / 100.0, R[5] / 100.0, R[0] / 100.0, R[1] / 1000.0,
    (int16_t)R[13] / 10.0, whNow() / 1000.0, ahNow() / 1000.0,
    R[0x12] ? "ON" : "OFF", R[0x11] ? "CC" : "CV", R[0x10] < 12 ? PROT_NAMES[R[0x10]] : "?", R[0x0F] ? 1 : 0);
  return String(b);
}

void mqttLoop() {
  if (!opt.mqEn || !opt.mqHost[0] || !WiFi.isConnected()) {
    if (mqtt && mqtt->connected()) mqtt->stop();
    return;
  }
  if (!mqtt) {
    mqtt = new MqttClient(mqttNet);
    mqtt->onMessage(onMqttMessage);
    mqtt->setConnectionTimeout(3000);
  }
  if (!mqtt->connected()) {
    if (millis() - lastMqttTry < 30000 && lastMqttTry) return;
    lastMqttTry = millis();
    String id = "sk120-" + WiFi.macAddress(); id.replace(":", "");
    mqtt->setId(id);
    if (opt.mqUser[0]) mqtt->setUsernamePassword(opt.mqUser, opt.mqPass);
    mqtt->beginWill(mqBase() + "/status", true, 1); mqtt->print("offline"); mqtt->endWill();
    if (!mqtt->connect(opt.mqHost, opt.mqPort)) return;
    mqttPublish(mqBase() + "/status", "online", true);
    mqtt->subscribe(mqBase() + "/set/#");
    mqttDiscoverySent = false;
  }
  mqtt->poll();
  if (!mqttDiscoverySent) mqttDiscovery();
  if (millis() - lastMqttPub > 2000) { lastMqttPub = millis(); mqttPublish(mqBase() + "/state", mqttState()); }
}

bool mqttConnected() { return mqtt && mqtt->connected(); }

// ======================= WEB HANDLERS ====================
// /timer?sec=N  (0 = cancel)
void handleTimer() {
  long s = argNum("sec", -1);
  if (s < 0 || s > 7L * 24 * 3600) return sendErr(400, "sec 0..604800");
  offAtMs = s ? millis() + (uint32_t)s * 1000UL : 0;
  if (offAtMs == 0 && s) offAtMs = 1;
  sendOK();
}

// /prog?run=1|0
void handleProg() {
  long run = argNum("run", -1);
  if (run == 1) { if (!progStart()) return sendErr(500, "no steps or Modbus error"); }
  else if (run == 0) progStop(true, nullptr);
  sendOK();
}

// /charge?run=1&cap=2000   /charge?run=0
void handleCharge() {
  long run = argNum("run", -1);
  if (run == 1) {
    chgBaseAh = ahNow(); chgBaseWh = whNow(); chgT0 = millis();
    chgCapMah = max(0L, argNum("cap", 0));
    cancelRamp(false);
    if (!outputOn(false)) return sendErr(500, "ERR");
    chgActive = true;
  } else if (run == 0) {
    chgActive = false;
    if (mbWrite(0x12, 0)) R[0x12] = 0;
  }
  sendOK();
}

void copyArg(const char *k, char *dst, size_t n) {
  if (!server.hasArg(k)) return;
  String v = server.arg(k);
  if (v == "********") return;                 // masked secret left unchanged
  v.toCharArray(dst, n);
}

// /opts            -> JSON
// /opts?k=v&...    -> update then JSON
void handleOpts() {
  bool changed = false;
  if (server.hasArg("soft"))   { opt.softMs = constrain(argNum("soft"), 0L, 60000L); changed = true; }
  if (server.hasArg("tz"))     { opt.tzMin = constrain(argNum("tz"), -720L, 840L); changed = true; }
  if (server.hasArg("clock"))  { opt.clockEn = argNum("clock") ? 1 : 0; clockFails = 0; clockLastOk = -1; lastClockMs = 0; changed = true; }
  if (server.hasArg("schedEn")){ opt.schedEn = argNum("schedEn") ? 1 : 0; changed = true; }
  if (server.hasArg("schedOn")){ long m = argNum("schedOn"); opt.schedOn = (m >= 0 && m < 1440) ? m : 0xFFFF; changed = true; }
  if (server.hasArg("schedOff")){ long m = argNum("schedOff"); opt.schedOff = (m >= 0 && m < 1440) ? m : 0xFFFF; changed = true; }
  if (server.hasArg("tgEn"))   { opt.tgEn = argNum("tgEn") ? 1 : 0; changed = true; }
  if (server.hasArg("tgToken") || server.hasArg("tgChat")) { copyArg("tgToken", opt.tgToken, sizeof(opt.tgToken)); copyArg("tgChat", opt.tgChat, sizeof(opt.tgChat)); changed = true; }
  if (server.hasArg("mqEn") || server.hasArg("mqHost")) {
    if (server.hasArg("mqEn")) opt.mqEn = argNum("mqEn") ? 1 : 0;
    copyArg("mqHost", opt.mqHost, sizeof(opt.mqHost));
    if (server.hasArg("mqPort")) opt.mqPort = constrain(argNum("mqPort"), 1L, 65535L);
    copyArg("mqUser", opt.mqUser, sizeof(opt.mqUser));
    copyArg("mqPass", opt.mqPass, sizeof(opt.mqPass));
    if (server.hasArg("mqBase") && server.arg("mqBase").length()) copyArg("mqBase", opt.mqBase, sizeof(opt.mqBase));
    if (mqtt && mqtt->connected()) mqtt->stop();
    lastMqttTry = 0;
    changed = true;
  }
  if (server.hasArg("steps")) {               // "500,1000,60;1200,500,30"
    if (progRunning) return sendErr(409, "stop the program first");
    String s = server.arg("steps");
    uint8_t n = 0; int pos = 0;
    while (n < MAX_STEPS && pos < (int)s.length()) {
      int semi = s.indexOf(';', pos); if (semi < 0) semi = s.length();
      String part = s.substring(pos, semi);
      int c1 = part.indexOf(','), c2 = part.indexOf(',', c1 + 1);
      if (c1 > 0 && c2 > c1) {
        long v = part.substring(0, c1).toInt(), i = part.substring(c1 + 1, c2).toInt(), sec = part.substring(c2 + 1).toInt();
        if (v < 0 || v > 3600 || i < 0 || i > 6000 || sec < 1 || sec > 65535) return sendErr(400, "bad step");
        opt.steps[n++] = { (uint16_t)v, (uint16_t)i, (uint16_t)sec };
      }
      pos = semi + 1;
    }
    opt.nSteps = n;
    if (server.hasArg("repeat")) opt.repeat = constrain(argNum("repeat"), 0L, 255L);
    changed = true;
  }
  if (changed) saveOpts();

  String j = "{\"soft\":" + String(opt.softMs) + ",\"tz\":" + String(opt.tzMin) + ",\"clock\":" + String(opt.clockEn) +
             ",\"clkOk\":" + String(clockLastOk) +
             ",\"schedEn\":" + String(opt.schedEn) + ",\"schedOn\":" + String(opt.schedOn == 0xFFFF ? -1 : opt.schedOn) +
             ",\"schedOff\":" + String(opt.schedOff == 0xFFFF ? -1 : opt.schedOff) +
             ",\"tgEn\":" + String(opt.tgEn) + ",\"tgToken\":\"" + (opt.tgToken[0] ? "********" : "") +
             "\",\"tgChat\":\"" + jsonEscape(opt.tgChat) + "\",\"tgLast\":\"" + jsonEscape(lastTgResult) +
             "\",\"mqEn\":" + String(opt.mqEn) + ",\"mqHost\":\"" + jsonEscape(opt.mqHost) + "\",\"mqPort\":" + String(opt.mqPort) +
             ",\"mqUser\":\"" + jsonEscape(opt.mqUser) + "\",\"mqPass\":\"" + (opt.mqPass[0] ? "********" : "") +
             "\",\"mqBase\":\"" + jsonEscape(opt.mqBase) + "\",\"mqOk\":" + String(mqttConnected() ? 1 : 0) +
             ",\"repeat\":" + String(opt.repeat) + ",\"steps\":[";
  for (uint8_t k = 0; k < opt.nSteps; k++) {
    if (k) j += ',';
    j += "[" + String(opt.steps[k].v) + "," + String(opt.steps[k].i) + "," + String(opt.steps[k].sec) + "]";
  }
  j += "]}";
  server.send(200, "application/json", j);
}

// /notify-test  -> sends a Telegram test message now
void handleNotifyTest() {
  bool ok = sendTelegramNow("SK120: test message ✔");
  server.send(ok ? 200 : 500, "text/plain", lastTgResult);
}

// Extra fields appended to /status
void appendFeatureStatus(String &j) {
  j += F(",\"ramp\":");  j += rampActive ? 1 : 0;
  j += F(",\"tOff\":");  j += offAtMs ? (uint32_t)max<int32_t>(0, (int32_t)(offAtMs - millis())) / 1000 : 0;
  j += F(",\"timer\":"); j += offAtMs ? 1 : 0;
  j += F(",\"prog\":");  j += progRunning ? progIdx + 1 : 0;
  j += F(",\"progN\":"); j += opt.nSteps;
  j += F(",\"progLeft\":"); j += progLeftSec();
  j += F(",\"progLoop\":"); j += progLoopsDone;
  j += F(",\"chg\":");   j += chgActive ? 1 : 0;
  j += F(",\"chgMah\":"); j += chgActive ? ahNow() - chgBaseAh : 0;
  j += F(",\"chgMwh\":"); j += chgActive ? whNow() - chgBaseWh : 0;
  j += F(",\"chgSec\":"); j += chgActive ? (millis() - chgT0) / 1000 : 0;
  j += F(",\"chgCap\":"); j += chgCapMah;
  j += F(",\"time\":");  j += timeValid() ? localEpoch() : 0;
}

void featuresSetup() {
  loadOpts();
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  server.on("/timer",  handleTimer);
  server.on("/prog",   handleProg);
  server.on("/charge", handleCharge);
  server.on("/opts",   handleOpts);
  server.on("/notify-test", handleNotifyTest);
}

void featuresLoop() {
  rampLoop();
  if (!haveData || millis() - lastOkMs > 3000) return;   // act only on fresh device data
  eventLoop();
  progLoop();
  chgLoop();
  schedLoop();
  clockLoop();
  mqttLoop();
  if (tgQueue.length()) { String m = tgQueue; tgQueue = ""; sendTelegramNow(m); }
}
