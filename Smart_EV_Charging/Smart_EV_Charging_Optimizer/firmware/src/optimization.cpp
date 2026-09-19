#include <Arduino.h>
#include <time.h>
#include "optimization.h"
#include "State.h"
#include "config.h"

// ---------------------------------------------------------------------
// MODULE 6: Charger Allocation / Optimization Algorithm — SRS 8.7
//
//   EVERY optimization cycle:
//     read localBayStatus, localPower, predictedArrivalProb/Duration
//     read neighborBayStatuses (via MQTT / shared telemetry)
//     read maxStationLoadW, overloadCurrentA, peakTariff*, predictionThreshold
//
//     IF localCurrent > overloadCurrentA:              THROTTLE 50%, alarm
//     ELSE IF totalStationPower > maxStationLoadW:
//         IF predictedDurationMin > median(neighborDurations): THROTTLE 70%
//         ELSE: ALLOW 100%
//     ELSE IF in peak-tariff window AND FREE AND low predicted arrival:
//         DEFER (throttle 0)
//     ELSE: ALLOW 100%
//
// A manual RPC override (setRelayState / setThrottle, FR-8) takes
// precedence and is only cleared when the bay returns to FREE
// (see Peripherals::checkPlugButtons on plug-out).
// ---------------------------------------------------------------------
void runOptimizationCycle(void) {
  if (manualOverrideActive) {
    // Operator is in control — optimization logic stands down this cycle.
    return;
  }

  float totalStationPower = power + neighborTotalPowerW;

  int currentHour = 12;
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 100)) currentHour = timeinfo.tm_hour;

  bool inPeakWindow;
  if (peakTariffStartHr <= peakTariffEndHr) {
    inPeakWindow = (currentHour >= peakTariffStartHr && currentHour < peakTariffEndHr);
  } else {
    // window wraps past midnight, e.g. 22 -> 6
    inPeakWindow = (currentHour >= peakTariffStartHr || currentHour < peakTariffEndHr);
  }

  if (current > overloadCurrentA) {
    loadDecision = "THROTTLE";
    throttleLevel = 50;
    overloadActive = true;

  } else if (totalStationPower > maxStationLoadW) {
    overloadActive = false;
    if (predictedDurationMin > neighborMedianDurationMin) {
      loadDecision = "THROTTLE";
      throttleLevel = 70;
    } else {
      loadDecision = "ALLOW";
      throttleLevel = 100;
    }

  } else if (inPeakWindow && bayStatus == "FREE" && predictedArrivalProb < predictionThreshold) {
    overloadActive = false;
    loadDecision = "DEFER";
    throttleLevel = 0;

  } else {
    overloadActive = false;
    loadDecision = "ALLOW";
    throttleLevel = 100;
  }
}
