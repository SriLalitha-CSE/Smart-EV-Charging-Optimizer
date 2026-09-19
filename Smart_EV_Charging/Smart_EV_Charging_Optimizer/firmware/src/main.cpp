// =======================================================================
// Edge AI Based Smart EV Charging Station Optimizer — Bay Firmware
// SRS 8.2 Data Flow: Sensors -> Power/Energy -> Feature Builder ->
//                     Edge AI Inference -> Optimization -> Relay/LEDs
//                                                        -> MQTT Telemetry
// =======================================================================
#include <Arduino.h>
#include "config.h"
#include "State.h"
#include "Peripherals.h"
#include "network.h"
#include "telemetry.h"
#include "attributes.h"
#include "rpc.h"
#include "edge_ai.h"
#include "optimization.h"

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Smart EV Charging Bay Optimizer — " BAY_ID " ===");

  peripheralsInit();     // MODULE 1: pins, relay, LEDs, DHT22
  networkInit();          // MODULE 4: WiFi + MQTT setup
  edgeAiInit();           // MODULE 5: NTP time sync for feature building

  connectWiFi();
  connectMQTT();
  publishClientAttributes();
}

void loop() {
  ensureConnected();  // MODULE 4: auto-reconnect WiFi/MQTT (NFR-Reliability)

  unsigned long nowMs = millis();

  // ---- MODULE 3: EV arrival/departure buttons, checked every loop ----
  checkPlugButtons();

  // ---- MODULE 2: sensor sampling, every SENSOR_SAMPLE_INTERVAL_MS ----
  if (nowMs - lastSampleMillis >= SENSOR_SAMPLE_INTERVAL_MS) {
    lastSampleMillis = nowMs;
    readSensors();
    computePowerEnergy();
    edgeAiRunInference();     // MODULE 5: Edge AI prediction
  }

  // ---- MODULE 6: optimization cycle, every OPTIMIZATION_CYCLE_MS ----
  if (nowMs - lastOptimizeMillis >= OPTIMIZATION_CYCLE_MS) {
    lastOptimizeMillis = nowMs;
    runOptimizationCycle();
  }

  applyRelayAndLeds();  // actuate relay + LEDs every loop (fast response)

  // ---- MODULE 4: telemetry publish, every TELEMETRY_PUBLISH_INTERVAL_MS ----
  if (nowMs - lastTelemetryMillis >= TELEMETRY_PUBLISH_INTERVAL_MS) {
    lastTelemetryMillis = nowMs;
    publishTelemetry();
    printval();  // Serial Monitor debug line
  }
}
