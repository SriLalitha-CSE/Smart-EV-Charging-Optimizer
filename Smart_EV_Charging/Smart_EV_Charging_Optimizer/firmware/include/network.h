#ifndef NETWORK_H
#define NETWORK_H

#include <WiFiClient.h>
#include <PubSubClient.h>

extern WiFiClient espClient;
extern PubSubClient mqtt;

// MODULE 4: MQTT + ThingsBoard connectivity (NFR-Reliability: auto-reconnect)
void networkInit(void);      // sets MQTT server + callback, call once from setup()
void connectWiFi(void);
void connectMQTT(void);
void ensureConnected(void);  // call every loop(): reconnects WiFi/MQTT as needed

#endif
