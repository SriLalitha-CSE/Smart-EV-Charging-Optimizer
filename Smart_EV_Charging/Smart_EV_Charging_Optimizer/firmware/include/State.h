#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

// ---------------------------------------------------------------------
// Live bay state — SRS 8.3 MQTT telemetry payload fields
// ---------------------------------------------------------------------
extern String bayStatus;              // FREE | CHARGING | FAULT
extern float voltage, current, power; // instantaneous readings (FR-1, FR-2)
extern float energyWh;                // cumulative energy this session (FR-2)
extern float temperature;             // ambient/charger temp (FR-1)

// Edge AI outputs (FR-4)
extern float predictedArrivalProb;    // 0-1
extern int   predictedDurationMin;    // minutes

// Optimization outputs (FR-5, FR-6)
extern String loadDecision;           // ALLOW | THROTTLE | DEFER | MANUAL_OFF | MANUAL_THROTTLE
extern int    throttleLevel;          // 0-100 (%)
extern bool   overloadActive;         // local overload alarm flag
extern bool   manualOverrideActive;   // true after an RPC override (FR-8)

// ---------------------------------------------------------------------
// Shared attributes pushed down from ThingsBoard — SRS 8.4
// ---------------------------------------------------------------------
extern float maxStationLoadW;
extern float overloadCurrentA;
extern int   peakTariffStartHr;
extern int   peakTariffEndHr;
extern float predictionThreshold;

// ---------------------------------------------------------------------
// Session / rolling-feature bookkeeping — SRS 8.6.2 input features
// ---------------------------------------------------------------------
extern unsigned long sessionStartMillis;   // 0 when bay is FREE
extern float recentAvgCurrent;             // rolling average, SRS 8.6.2
extern int   neighborBaysOccupied;         // station-wide congestion (SRS 8.6.2 / 8.7)
extern float neighborTotalPowerW;          // sum of other bays' power (SRS 8.7)
extern float neighborMedianDurationMin;    // median predicted duration of other bays (SRS 8.7)

// ---------------------------------------------------------------------
// Timing / debounce bookkeeping
// ---------------------------------------------------------------------
extern unsigned long lastSampleMillis;
extern unsigned long lastTelemetryMillis;
extern unsigned long lastOptimizeMillis;
extern unsigned long lastPlugInPress;
extern unsigned long lastPlugOutPress;

#endif
