#include <ArduinoJson.h>
#include "telemetry.h"
#include "network.h"
#include "State.h"
#include "config.h"

// ---------------------------------------------------------------------
// FR-7 telemetry publish — full SRS 8.3 schema:
//   bayId, voltage, current, power, energyWh, temperature, bayStatus,
//   predictedArrivalProb, predictedDurationMin, loadDecision, throttleLevel
// Plus two additive fields (beyond the SRS baseline) so ThingsBoard rule
// chains and the dashboard don't need extra derived logic:
//   overloadActive        -> lets the Overcurrent alarm rule be a 1-line filter
//   manualOverrideActive  -> lets the dashboard show "under operator control"
// ---------------------------------------------------------------------
void publishTelemetry() {
  if (!mqtt.connected()) return;

  StaticJsonDocument<450> doc;
  doc["bayId"] = BAY_ID;
  doc["voltage"] = round(voltage * 10) / 10.0;
  doc["current"] = round(current * 10) / 10.0;
  doc["power"] = round(power * 10) / 10.0;
  doc["energyWh"] = round(energyWh * 10) / 10.0;
  doc["temperature"] = round(temperature * 10) / 10.0;
  doc["bayStatus"] = bayStatus;
  doc["predictedArrivalProb"] = round(predictedArrivalProb * 100) / 100.0;
  doc["predictedDurationMin"] = predictedDurationMin;
  doc["loadDecision"] = loadDecision;
  doc["throttleLevel"] = throttleLevel;
  doc["overloadActive"] = overloadActive;
  doc["manualOverrideActive"] = manualOverrideActive;

  char buffer[450];
  serializeJson(doc, buffer);

  mqtt.publish("v1/devices/me/telemetry", buffer);
  Serial.print("[MQTT >>] ");
  Serial.println(buffer);
}

// ---------------------------------------------------------------------
// Client-side attribute publish — firmware version, per SRS 7.3 topic table.
// Called once after MQTT connects; cheap and useful during grading/demo
// to confirm which firmware revision a device is running.
// ---------------------------------------------------------------------
void publishClientAttributes() {
  if (!mqtt.connected()) return;
  StaticJsonDocument<128> doc;
  doc["firmwareVersion"] = "1.0.0";
  doc["bayId"] = BAY_ID;
  char buffer[128];
  serializeJson(doc, buffer);
  mqtt.publish("v1/devices/me/attributes", buffer);
}
