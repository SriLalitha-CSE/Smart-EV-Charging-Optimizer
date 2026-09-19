#include "State.h"

// Live bay state
String bayStatus = "FREE";
float voltage = 0.0, current = 0.0, power = 0.0;
float energyWh = 0.0;
float temperature = 0.0;

// Edge AI outputs
float predictedArrivalProb = 0.0;
int   predictedDurationMin = 0;

// Optimization outputs
String loadDecision = "ALLOW";
int    throttleLevel = 100;
bool   overloadActive = false;
bool   manualOverrideActive = false;

// Shared attributes — sensible defaults until ThingsBoard pushes real values (SRS 8.4)
float maxStationLoadW    = 6000.0;
float overloadCurrentA   = 16.0;
int   peakTariffStartHr  = 18;
int   peakTariffEndHr    = 21;
float predictionThreshold = 0.5;

// Rolling features
unsigned long sessionStartMillis = 0;
float recentAvgCurrent = 0.0;
int   neighborBaysOccupied = 0;
float neighborTotalPowerW = 0.0;
float neighborMedianDurationMin = 0.0;

// Timing
unsigned long lastSampleMillis = 0;
unsigned long lastTelemetryMillis = 0;
unsigned long lastOptimizeMillis = 0;
unsigned long lastPlugInPress = 0;
unsigned long lastPlugOutPress = 0;
