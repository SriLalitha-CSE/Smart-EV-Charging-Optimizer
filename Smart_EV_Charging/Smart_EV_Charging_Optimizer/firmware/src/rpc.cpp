#include <ArduinoJson.h>
#include "rpc.h"
#include "network.h"
#include "State.h"
#include "config.h"

// ---------------------------------------------------------------------
// MODULE 8 / SRS 8.5: RPC command handling.
// Manual overrides set manualOverrideActive = true, which makes
// optimization.cpp stand down until the bay next returns to FREE
// (plug-out), so an operator override cannot be silently reversed by
// the automatic optimization cycle mid-session.
// ---------------------------------------------------------------------
void handleRpc(String requestId, char* payload) {
  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, payload)) return;

  String method = doc["method"] | "";
  JsonObject params = doc["params"];

  StaticJsonDocument<250> response;

  if (method == "setRelayState") {
    bool state = params["state"] | false;
    manualOverrideActive = true;
    throttleLevel = state ? 100 : 0;
    loadDecision = state ? "ALLOW" : "MANUAL_OFF";
    response["success"] = true;
    response["throttleLevel"] = throttleLevel;
    Serial.print(">> RPC setRelayState("); Serial.print(state); Serial.println(") - manual override engaged.");

  } else if (method == "setThrottle") {
    int level = params["level"] | 100;
    level = constrain(level, 0, 100);
    manualOverrideActive = true;
    throttleLevel = level;
    loadDecision = "MANUAL_THROTTLE";
    response["success"] = true;
    response["throttleLevel"] = throttleLevel;
    Serial.print(">> RPC setThrottle("); Serial.print(level); Serial.println(") - manual override engaged.");

  } else if (method == "getStatus") {
    response["bayId"] = BAY_ID;
    response["bayStatus"] = bayStatus;
    response["voltage"] = voltage;
    response["current"] = current;
    response["power"] = power;
    response["energyWh"] = energyWh;
    response["temperature"] = temperature;
    response["loadDecision"] = loadDecision;
    response["throttleLevel"] = throttleLevel;
    response["predictedArrivalProb"] = predictedArrivalProb;
    response["predictedDurationMin"] = predictedDurationMin;
    Serial.println(">> RPC getStatus() - reporting current status.");

  } else {
    response["success"] = false;
    response["error"] = "unknown method";
  }

  char buffer[300];
  serializeJson(response, buffer);
  mqtt.publish(("v1/devices/me/rpc/response/" + requestId).c_str(), buffer);
}
