#pragma once
// Minimal WebSocket push server (RFC 6455): ws://host/ws (port 80) and ws://host:81/.
// Server -> client: text frames with the same JSON as /status, sent after every fresh device read.
// Client -> server: only close / ping are handled; commands still go over HTTP.

#include <Hash.h>
#include <base64.h>

const uint8_t  WS_MAX_CLIENTS = 3;
const uint32_t WS_MIN_GAP_MS  = 120;     // max ~8 pushes per second
const uint32_t WS_STALL_MS    = 8000;    // drop a client that cannot take data for this long

WiFiServer wsServer(81);
struct WsClient { WiFiClient c; bool live = false; uint32_t lastOk = 0; };
WsClient wsc[WS_MAX_CLIENTS];
uint32_t wsLastPush = 0;
uint32_t wsFrames = 0, wsDropped = 0;

uint8_t wsCount() { uint8_t n = 0; for (auto &w : wsc) if (w.live) n++; return n; }

void wsClose(WsClient &w) {
  if (w.live && w.c.connected()) { uint8_t f[2] = {0x88, 0x00}; w.c.write(f, 2); }
  w.c.stop();
  w.live = false;
}

// Reads the HTTP upgrade request and answers with 101 Switching Protocols.
bool wsHandshake(WiFiClient &c) {
  c.setTimeout(400);
  String key;
  uint32_t t0 = millis();
  while (c.connected() && millis() - t0 < 600) {
    String line = c.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break;                       // end of headers
    if (line.substring(0, 18).equalsIgnoreCase("Sec-WebSocket-Key:")) { key = line.substring(18); key.trim(); }
  }
  if (key.length() < 16) return false;
  uint8_t hash[20];
  sha1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", hash);
  String resp = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
  resp += base64::encode(hash, 20);
  resp += "\r\n\r\n";
  c.print(resp);
  c.setNoDelay(true);
  return true;
}

// Takes over an incoming connection (request line already read) and upgrades it.
bool wsAdopt(WiFiClient nc) {
  int slot = -1;
  for (uint8_t i = 0; i < WS_MAX_CLIENTS; i++) if (!wsc[i].live) { slot = i; break; }
  if (slot < 0) {                                          // full: replace the oldest-idle client
    slot = 0;
    for (uint8_t i = 1; i < WS_MAX_CLIENTS; i++) if (wsc[i].lastOk < wsc[slot].lastOk) slot = i;
    wsClose(wsc[slot]);
  }
  if (!wsHandshake(nc)) return false;
  wsc[slot].c = nc;
  wsc[slot].live = true;
  wsc[slot].lastOk = millis();
  wsLastPush = 0;                                          // send state right away
  return true;
}

// Port 81: plain WebSocket listener.
void wsAccept() {
  WiFiClient nc = wsServer.accept();
  if (!nc) return;
  nc.readStringUntil('\n');                                // request line
  if (!wsAdopt(nc)) nc.stop();
}

// Handles close / ping from the browser; everything else is discarded.
void wsReadIncoming(WsClient &w) {
  if (!w.c.connected()) { w.c.stop(); w.live = false; return; }
  while (w.c.available() >= 2) {
    uint8_t b0 = w.c.read(), b1 = w.c.read();
    uint8_t op = b0 & 0x0F;
    uint32_t len = b1 & 0x7F;
    if (len == 126) { len = (w.c.read() << 8) | w.c.read(); }
    else if (len == 127) { wsClose(w); return; }           // never expected
    uint8_t mask[4] = {0, 0, 0, 0};
    if (b1 & 0x80) for (uint8_t k = 0; k < 4; k++) mask[k] = w.c.read();
    uint8_t payload[125]; uint32_t got = 0;
    uint32_t t0 = millis();
    while (got < len && millis() - t0 < 200) {
      if (w.c.available()) { uint8_t v = w.c.read() ^ mask[got % 4]; if (got < sizeof(payload)) payload[got] = v; got++; }
      else yield();
    }
    if (op == 0x8) { wsClose(w); return; }
    if (op == 0x9) {                                       // ping -> pong
      uint8_t n = min<uint32_t>(len, sizeof(payload));
      uint8_t h[2] = {0x8A, n};
      w.c.write(h, 2); w.c.write(payload, n);
    }
  }
}

void wsSendText(WsClient &w, const String &s) {
  size_t n = s.length();
  uint8_t h[4]; size_t hl;
  h[0] = 0x81;
  if (n < 126) { h[1] = n; hl = 2; }
  else { h[1] = 126; h[2] = n >> 8; h[3] = n & 0xFF; hl = 4; }
  if ((size_t)w.c.availableForWrite() < n + hl) {          // slow client: skip this frame
    wsDropped++;
    if (millis() - w.lastOk > WS_STALL_MS) wsClose(w);
    return;
  }
  w.c.write(h, hl);
  w.c.write((const uint8_t *)s.c_str(), n);
  w.lastOk = millis();
  wsFrames++;
}

void buildStatus(String &j);   // defined in SK120_Full.ino

void wsSetup() {
  wsServer.begin();
  wsServer.setNoDelay(true);
  // Port 80: GET /ws is handed over from the web server (same origin as the page).
  server.addHook([](const String &method, const String &url, WiFiClient *client, ESP8266WebServer::ContentTypeFunction) {
    if (url != "/ws") return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
    WiFiClient c = *client;                                // our copy keeps the TCP connection open
    return wsAdopt(c) ? ESP8266WebServer::CLIENT_IS_GIVEN : ESP8266WebServer::CLIENT_MUST_STOP;
  });
}

// Call often. `fresh` = a new device reading has just been stored in R[].
void wsLoop(bool fresh) {
  wsAccept();
  for (auto &w : wsc) if (w.live) wsReadIncoming(w);
  if (!fresh || !wsCount() || millis() - wsLastPush < WS_MIN_GAP_MS) return;
  wsLastPush = millis();
  String j;
  buildStatus(j);
  for (auto &w : wsc) if (w.live) wsSendText(w, j);
}
