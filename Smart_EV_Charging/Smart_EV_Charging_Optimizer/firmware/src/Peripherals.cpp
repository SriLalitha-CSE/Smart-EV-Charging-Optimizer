#include <Arduino.h>
#include <DHT.h>
#include "State.h"
#include "Peripherals.h"
#include "config.h"

DHT dht(DHT_PIN, DHT_TYPE);

// ---------------------------------------------------------------------
// MODULE 1: pin setup
// ---------------------------------------------------------------------
void peripheralsInit(void) {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BTN_PLUGIN, INPUT_PULLUP);
  pinMode(BTN_PLUGOUT, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  dht.begin();
}

// ---------------------------------------------------------------------
// MODULE 2: Sensor Processing — FR-1
// Voltage/current potentiometers (0-4095 ADC) are scaled to the
// physical ranges defined in SRS 8.1: 0-250V / 0-32A.
// ---------------------------------------------------------------------
void readSensors(void) {
  int rawV = analogRead(VOLTAGE_PIN);
  int rawI = analogRead(CURRENT_PIN);

  voltage = (rawV / (float)ADC_MAX) * VOLTAGE_MAX_V;
  current = (rawI / (float)ADC_MAX) * CURRENT_MAX_A;

  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("[Peripherals] WARNING: DHT22 read failed, keeping last temperature.");
  } else {
    temperature = t;
  }

  // Rolling average current — SRS 8.6.2 feature "recentAvgCurrent"
  // Simple exponential moving average, cheap enough for the ESP32 loop.
  const float alpha = 0.2;
  recentAvgCurrent = (recentAvgCurrent == 0.0) ? current : (alpha * current + (1 - alpha) * recentAvgCurrent);
}

// ---------------------------------------------------------------------
// FR-2: Power & Energy Calculation
// power(W) = voltage * current ; energy accumulates only while CHARGING
// ---------------------------------------------------------------------
void computePowerEnergy(void) {
  power = voltage * current;

  static unsigned long lastEnergyMillis = 0;
  unsigned long nowMs = millis();
  if (lastEnergyMillis == 0) lastEnergyMillis = nowMs;

  if (bayStatus == "CHARGING") {
    float hoursElapsed = (nowMs - lastEnergyMillis) / 3600000.0;
    energyWh += power * hoursElapsed;
  }
  lastEnergyMillis = nowMs;
}

// ---------------------------------------------------------------------
// MODULE 3: EV Arrival/Departure Simulation + Bay State Management — FR-3
// BTN_PLUGIN : FREE -> CHARGING (starts a session, resets energy/timer)
// BTN_PLUGOUT: CHARGING/FAULT -> FREE (ends a session)
// Buttons are active-LOW (INPUT_PULLUP), software-debounced.
// ---------------------------------------------------------------------
void checkPlugButtons(void) {
  unsigned long nowMs = millis();

  if (digitalRead(BTN_PLUGIN) == LOW && (nowMs - lastPlugInPress) > BUTTON_DEBOUNCE_MS) {
    lastPlugInPress = nowMs;
    if (bayStatus == "FREE") {
      bayStatus = "CHARGING";
      energyWh = 0.0;
      sessionStartMillis = nowMs;
      manualOverrideActive = false;
      Serial.println("[State] EV plugged in -> CHARGING");
    }
  }

  if (digitalRead(BTN_PLUGOUT) == LOW && (nowMs - lastPlugOutPress) > BUTTON_DEBOUNCE_MS) {
    lastPlugOutPress = nowMs;
    if (bayStatus == "CHARGING" || bayStatus == "FAULT") {
      bayStatus = "FREE";
      sessionStartMillis = 0;
      overloadActive = false;
      manualOverrideActive = false;
      loadDecision = "ALLOW";
      throttleLevel = 100;
      Serial.println("[State] EV unplugged -> FREE");
    }
  }

  // FAULT condition — overcurrent sustained (also raised by optimization.cpp)
  if (current > overloadCurrentA && bayStatus == "CHARGING") {
    bayStatus = "FAULT";
    overloadActive = true;
    Serial.println("[State] Overcurrent -> FAULT");
  }
}

// ---------------------------------------------------------------------
// Relay + LED actuation, driven by bayStatus / throttleLevel
// Green = FREE, Yellow = CHARGING, Red = FAULT/overload
// Relay is PWM-duty-cycled by throttleLevel when CHARGING (SRS 8.7).
// ---------------------------------------------------------------------
void applyRelayAndLeds(void) {
  digitalWrite(LED_GREEN,  bayStatus == "FREE"     ? HIGH : LOW);
  digitalWrite(LED_YELLOW, bayStatus == "CHARGING" ? HIGH : LOW);
  digitalWrite(LED_RED,    (bayStatus == "FAULT" || overloadActive) ? HIGH : LOW);

  if (bayStatus == "CHARGING" && throttleLevel > 0) {
    // Simple duty-cycle emulation of throttling without a hardware PWM relay driver.
    static uint8_t dutyCounter = 0;
    dutyCounter = (dutyCounter + 10) % 100;
    digitalWrite(RELAY_PIN, (dutyCounter < throttleLevel) ? HIGH : LOW);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
}

// ---------------------------------------------------------------------
// Debug print — kept for Serial Monitor sanity checks during development
// ---------------------------------------------------------------------
void printval(void) {
  Serial.print("[");
  Serial.print(BAY_ID);
  Serial.print("] V="); Serial.print(voltage, 1);
  Serial.print(" I="); Serial.print(current, 1);
  Serial.print(" P="); Serial.print(power, 1);
  Serial.print(" T="); Serial.print(temperature, 1);
  Serial.print(" status="); Serial.print(bayStatus);
  Serial.print(" decision="); Serial.print(loadDecision);
  Serial.print(" throttle="); Serial.println(throttleLevel);
}
