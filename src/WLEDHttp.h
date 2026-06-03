#ifndef _WLED_HTTP_H
#define _WLED_HTTP_H

#ifdef WIFI

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <cstring>

#define WLED_HTTP_QUEUE_SIZE 8
#define WLED_HTTP_TIMEOUT_MS 500
#define WLED_HTTP_PAYLOAD_SIZE 96

struct WLEDHttpRequest {
  byte command;
  byte value;
  byte speed;
  byte intensity;
  uint32_t color;
};

static WLEDHttpRequest wledQueue[WLED_HTTP_QUEUE_SIZE];
static byte wledQueueHead = 0;
static byte wledQueueTail = 0;
static byte wledQueueCount = 0;
static portMUX_TYPE wledQueueMutex = portMUX_INITIALIZER_UNLOCKED;

inline bool wled_interface_enabled()
{
  return IS_INTERFACE_ENABLED(interfaces[PED_WLED].midiOut);
}

inline void wled_clear_queue()
{
  portENTER_CRITICAL(&wledQueueMutex);
  wledQueueHead = 0;
  wledQueueTail = 0;
  wledQueueCount = 0;
  portEXIT_CRITICAL(&wledQueueMutex);
}

inline bool wled_configured_address(String& address)
{
  address = wledAddress;
  address.trim();

  if (address.isEmpty()) {
    DPRINT("WLED HTTP: address not configured, dropping request\n");
    return false;
  }

  if (address.indexOf("://") >= 0 || address.indexOf('/') >= 0 || address.indexOf('\\') >= 0 || address.indexOf(' ') >= 0) {
    DPRINT("WLED HTTP: invalid address '%s', expected host/IP[:port]\n", address.c_str());
    return false;
  }

  return true;
}

inline void wled_enqueue(byte command, byte value, byte speed, byte intensity, uint32_t color)
{
  if (!wled_interface_enabled()) return;

  String address;
  if (!wled_configured_address(address)) return;

  const WLEDHttpRequest request = {command, value, speed, intensity, color};

  portENTER_CRITICAL(&wledQueueMutex);
  if (wledQueueCount >= WLED_HTTP_QUEUE_SIZE) {
    portEXIT_CRITICAL(&wledQueueMutex);
    DPRINT("WLED HTTP: queue full, dropping request\n");
    return;
  }

  wledQueue[wledQueueTail] = request;
  wledQueueTail = (wledQueueTail + 1) % WLED_HTTP_QUEUE_SIZE;
  wledQueueCount++;
  portEXIT_CRITICAL(&wledQueueMutex);
}

inline bool wled_dequeue(WLEDHttpRequest& request)
{
  portENTER_CRITICAL(&wledQueueMutex);
  if (wledQueueCount == 0) {
    portEXIT_CRITICAL(&wledQueueMutex);
    return false;
  }

  request = wledQueue[wledQueueHead];
  wledQueueHead = (wledQueueHead + 1) % WLED_HTTP_QUEUE_SIZE;
  wledQueueCount--;
  portEXIT_CRITICAL(&wledQueueMutex);
  return true;
}

inline void wled_run()
{
  if (!wled_interface_enabled()) {
    wled_clear_queue();
    return;
  }

  if (!wifiEnabled || WiFi.status() != WL_CONNECTED) return;

  WLEDHttpRequest request;
  if (!wled_dequeue(request)) return;

  String address;
  if (!wled_configured_address(address)) return;

  char payload[WLED_HTTP_PAYLOAD_SIZE];
  if (!pedalino::build_wled_json_payload(request.command,
                                         request.value,
                                         request.speed,
                                         request.intensity,
                                         request.color,
                                         payload,
                                         sizeof(payload))) {
    DPRINT("WLED HTTP: invalid command %d, dropping request\n", request.command);
    return;
  }

  WiFiClient client;
  HTTPClient http;
  const String url = String("http://") + address + F("/json/state");

  if (!http.begin(client, url)) {
    DPRINT("WLED HTTP: begin failed for %s\n", url.c_str());
    return;
  }

  http.setTimeout(WLED_HTTP_TIMEOUT_MS);
  http.addHeader(F("Content-Type"), F("application/json"));
  const int status = http.POST((uint8_t*)payload, strlen(payload));
  if (status <= 0 || status >= 400) {
    DPRINT("WLED HTTP: POST %s failed with status %d\n", url.c_str(), status);
  }
  http.end();
}

#else

inline void wled_clear_queue() {}
inline void wled_enqueue(byte, byte, byte, byte, uint32_t) {}
inline void wled_run() {}

#endif

#endif
