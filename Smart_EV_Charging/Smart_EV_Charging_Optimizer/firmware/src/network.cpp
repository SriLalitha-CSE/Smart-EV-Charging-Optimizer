#include <WiFi.h>
#include <ArduinoJson.h>
#include "network.h"
#include "config.h"
#include "attributes.h"
#include "rpc.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

static void mqttCallback(char* topic, byte* payload, unsigned int length);

// ---------------------------------------------------------------------
void networkInit(void) {
  mqtt.setServer(TB_SERVER, TB_PORT);
  mqtt.setCallback(mqttCallback);
  connectWiFi();
}

// ---------------------------------------------------------------------
void connectWiFi(void) {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " connected." : " FAILED (will retry).");
}

// ---------------------------------------------------------------------
void connectMQTT(void) {
  if (WiFi.status() != WL_CONNECTED) return;
  Serial.print("Connecting to ThingsBoard MQTT...");
  if (mqtt.connect(BAY_ID, TB_TOKEN, NULL)) {
    Serial.println(" connected.");
    mqtt.subscribe("v1/devices/me/attributes");             // push updates (SRS 8.4)
    mqtt.subscribe("v1/devices/me/attributes/response/+");  // reply to our request
    mqtt.subscribe("v1/devices/me/rpc/request/+");          // RPC commands (SRS 8.5)
    requestSharedAttributes();
  } else {
    Serial.print(" failed, rc=");
    Serial.println(mqtt.state());
    delay(1000);
  }
}

// ---------------------------------------------------------------------
// NFR-Reliability: automatic WiFi + MQTT reconnection, non-blocking
// enough for a 5s sample loop (a single blocking connect attempt is
// acceptable here because it only runs when disconnected).
// ---------------------------------------------------------------------
void ensureConnected(void) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
    connectMQTT();
  }
  if (mqtt.connected()) {
    mqtt.loop();
  }
}

// ---------------------------------------------------------------------
// Single MQTT callback, dispatched by topic: attribute push/response vs.
// RPC request (SRS 8.4 / 8.5).
// ---------------------------------------------------------------------
static void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  char buf[400];
  unsigned int n = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
  memcpy(buf, payload, n);
  buf[n] = '\0';

  Serial.print("[MQTT <<] ");
  Serial.print(topicStr);
  Serial.print(" ");
  Serial.println(buf);

  if (topicStr.startsWith("v1/devices/me/rpc/request/")) {
    String requestId = topicStr.substring(topicStr.lastIndexOf('/') + 1);
    handleRpc(requestId, buf);
    return;
  }

  // Both attribute topics carry key/value pairs; the /response/ topic
  // nests them one level under "shared", the push topic does not.
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, buf);
  if (err) return;

  JsonObject attrs = doc.containsKey("shared") ? doc["shared"].as<JsonObject>() : doc.as<JsonObject>();
  applySharedAttributes(attrs);
}
