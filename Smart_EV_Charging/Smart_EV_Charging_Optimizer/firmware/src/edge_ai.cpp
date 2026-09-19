#include <Arduino.h>
#include <time.h>
#include "edge_ai.h"
#include "State.h"
#include "model.h"

// ---------------------------------------------------------------------
// SRS 8.6.5 step 1: NTP-synced clock, so hourOfDay/dayOfWeek features
// are real wall-clock values instead of millis()-since-boot.
// Wokwi's simulated internet supports NTP through pool.ntp.org.
// ---------------------------------------------------------------------
void edgeAiInit(void) {
  configTime(5 * 3600 + 1800, 0, "pool.ntp.org", "time.nist.gov"); // IST offset; adjust for your timezone
}

// ---------------------------------------------------------------------
// SRS 8.6.5: build feature vector -> call predict() -> store outputs
// ---------------------------------------------------------------------
void edgeAiRunInference(void) {
  struct tm timeinfo;
  int hourOfDay = 12;   // fallback if NTP hasn't synced yet
  int dayOfWeek = 0;
  if (getLocalTime(&timeinfo, 100)) {
    hourOfDay = timeinfo.tm_hour;
    dayOfWeek = timeinfo.tm_wday;
  }

  float sessionElapsedMin = 0.0f;
  if (bayStatus == "CHARGING" && sessionStartMillis > 0) {
    sessionElapsedMin = (millis() - sessionStartMillis) / 60000.0f;
  }

  // SRS 8.6.2 feature order
  float features[5] = {
    (float)hourOfDay,
    (float)dayOfWeek,
    (bayStatus == "CHARGING") ? 1.0f : 0.0f,
    recentAvgCurrent,
    sessionElapsedMin
  };

  predictedArrivalProb  = predictArrival(features);
  predictedDurationMin  = (int)predictDuration(features);

  Serial.print("[EdgeAI] arrivalProb="); Serial.print(predictedArrivalProb, 2);
  Serial.print(" durationMin="); Serial.println(predictedDurationMin);
}
