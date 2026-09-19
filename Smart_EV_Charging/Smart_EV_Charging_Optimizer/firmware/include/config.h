#ifndef CONFIG_H
#define CONFIG_H

// =======================================================================
// Edge AI Based Smart EV Charging Station Optimizer
// Per-bay configuration — SRS Section 8.1 / 8.4 / 9.3
//
// To turn this firmware into BAY_02 / BAY_03, duplicate this whole
// firmware/ folder, then change ONLY the three values in the
// "BAY IDENTITY" block below (BAY_ID + TB_TOKEN) and, if you are
// running multiple ESP32s on ONE shared Wokwi diagram, the GPIO pins
// if they collide with another bay's pins.
// =======================================================================

// ---------------------------------------------------------------------
// BAY IDENTITY  (unique per charging bay / per ThingsBoard device)
// ---------------------------------------------------------------------
#define BAY_ID    "BAY_01"                    // shown in telemetry.bayId
#define TB_TOKEN  "PASTE_BAY01_ACCESS_TOKEN"   // from ThingsBoard > Devices > BAY_01

// ---------------------------------------------------------------------
// WiFi (Wokwi simulated internet)
// ---------------------------------------------------------------------
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASS ""

// ---------------------------------------------------------------------
// ThingsBoard MQTT broker
// ---------------------------------------------------------------------
#define TB_SERVER "demo.thingsboard.io"   // or your self-hosted / thingsboard.cloud host
#define TB_PORT   1883

// ---------------------------------------------------------------------
// Pin configuration — SRS Section 8.1
// (LED_RED moved from GPIO21 -> GPIO23 to keep GPIO21/22 free for an
//  optional I2C OLED, per SRS 8.1 footnote)
// ---------------------------------------------------------------------
#define VOLTAGE_PIN   34   // potentiometer -> simulated 0-250V
#define CURRENT_PIN   35   // potentiometer -> simulated 0-32A
#define DHT_PIN       15
#define RELAY_PIN     26
#define BTN_PLUGIN    32
#define BTN_PLUGOUT   33
#define LED_GREEN     18   // FREE
#define LED_YELLOW    19   // CHARGING
#define LED_RED       23   // FAULT / OVERLOAD

#define DHT_TYPE DHT22

// ---------------------------------------------------------------------
// Timing (FR-1, FR-7, NFR-Performance)
// ---------------------------------------------------------------------
#define SENSOR_SAMPLE_INTERVAL_MS   5000    // FR-1 default sampling interval
#define TELEMETRY_PUBLISH_INTERVAL_MS 5000  // FR-7 publish interval
#define OPTIMIZATION_CYCLE_MS       30000   // SRS 8.7 optimization cycle
#define BUTTON_DEBOUNCE_MS          250

// ---------------------------------------------------------------------
// Sensor scaling (SRS 8.1)
// ---------------------------------------------------------------------
#define ADC_MAX          4095
#define VOLTAGE_MAX_V     250.0
#define CURRENT_MAX_A      32.0

#endif
