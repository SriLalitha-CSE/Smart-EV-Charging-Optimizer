# Edge AI Based Smart EV Charging Station Optimizer
## Project Documentation — Emertxe Internship

---

## PHASE 1 — REQUIREMENT ANALYSIS

### Functional requirements (from SRS Section 3)
| ID | Requirement |
|---|---|
| FR-1 | Sensor data acquisition (voltage, current, temperature) every 5s |
| FR-2 | Power (W) & cumulative energy (Wh) calculation |
| FR-3 | Simulated EV plug-in/plug-out detection, bay state FREE/CHARGING/FAULT |
| FR-4 | Edge AI: arrival probability + remaining duration prediction |
| FR-5 | Charger allocation optimization across bays |
| FR-6 | Peak-load management (throttle/stagger) |
| FR-7 | MQTT telemetry publish to ThingsBoard (JSON) |
| FR-8 | MQTT shared-attribute + RPC handling |
| FR-9 | ThingsBoard dashboard visualization |
| FR-10 | Alarms (overcurrent, sensor fault, high demand, stuck session) |
| FR-11 | Historical data logging |
| FR-12 | (Optional) local OLED/LCD display |

### Hardware requirements
ESP32 DevKit (per bay) · 2× potentiometer (voltage/current sim) · DHT22 (temperature)
· 1-channel relay module · 2× push buttons (plug-in/out) · 3× LED (Green/Yellow/Red)
· 1× 1kΩ resistor per LED.

### Software requirements
Arduino/PlatformIO C++ · WiFi.h · PubSubClient (MQTT) · ArduinoJson · DHT sensor library
· Python 3 (pandas, scikit-learn) · m2cgen/micromlgen · ThingsBoard CE · Wokwi.

### Architecture (SRS 7.1)
Three-layer Edge → Communication → Cloud:
- **Edge (ESP32 per bay):** sensing, Edge AI inference, local optimization, relay/LED control.
- **Communication (MQTT):** `v1/devices/me/telemetry`, `.../attributes`, `.../rpc/request|response/+`.
- **Cloud (ThingsBoard):** dashboards, rule chains, alarms, RPC origin, historical storage.

### Data flow (SRS 8.2)
```
Sensors (V/I/Temp) -> Power/Energy Calc -> Feature Builder (time+history)
   -> Edge AI Inference -> Local Optimization -> Relay/LED Control
                                              -> MQTT Telemetry -> ThingsBoard
```

### Modules needed
1. ESP32 Wokwi simulation (board + sensors + actuators)
2. Sensor processing (voltage/current/temperature, power/energy)
3. EV state management (FREE/CHARGING/FAULT)
4. MQTT + ThingsBoard connectivity (telemetry, attributes, reconnection)
5. Edge AI (arrival probability, duration prediction)
6. Optimization logic (ALLOW/THROTTLE/DEFER)
7. ThingsBoard dashboard
8. RPC control (setRelayState, setThrottle, getStatus)
9. Alarm system
10. Final demo

### Checklist
- [x] ESP32 simulation (Wokwi diagram, `wokwi/diagram.json`)
- [x] Sensor acquisition (`Peripherals.cpp::readSensors`)
- [x] EV arrival/departure simulation (`Peripherals.cpp::checkPlugButtons`)
- [x] Charging bay state management (`State.h/.cpp`, FREE/CHARGING/FAULT)
- [x] Power and energy calculation (`Peripherals.cpp::computePowerEnergy`)
- [x] MQTT communication (`network.cpp`, `telemetry.cpp`)
- [x] ThingsBoard integration (device + dashboard, see `thingsboard/` guide)
- [x] Edge AI prediction (`edge_ai.cpp`, `model.h`, `ai_training/`)
- [x] Charging optimization algorithm (`optimization.cpp`, SRS 8.7 pseudocode)
- [x] Relay control (`Peripherals.cpp::applyRelayAndLeds`)
- [x] Dashboard (`thingsboard/ThingsBoard_Setup_Guide.md` §5)
- [x] RPC commands (`rpc.cpp`: setRelayState, setThrottle, getStatus)
- [x] Alarm system (`thingsboard/ThingsBoard_Setup_Guide.md` §7)
- [x] Final Wokwi demonstration (see Demo Script below)

---

## PHASE 2 — FOLDER STRUCTURE

```
Smart_EV_Charging_Optimizer/
├── firmware/
│   ├── platformio.ini
│   ├── include/
│   │   ├── config.h        # pins, WiFi, TB token, timing
│   │   ├── State.h         # shared live state
│   │   ├── Peripherals.h
│   │   ├── network.h
│   │   ├── telemetry.h
│   │   ├── attributes.h
│   │   ├── rpc.h
│   │   ├── edge_ai.h
│   │   ├── optimization.h
│   │   └── model.h         # Edge AI model (stub, replaced by training export)
│   └── src/
│       ├── main.cpp
│       ├── State.cpp
│       ├── Peripherals.cpp
│       ├── network.cpp
│       ├── telemetry.cpp
│       ├── attributes.cpp
│       ├── rpc.cpp
│       ├── edge_ai.cpp
│       └── optimization.cpp
├── ai_training/
│   ├── requirements.txt
│   ├── generate_dataset.py   # SRS 8.6.4 synthetic dataset
│   └── train_and_export.py   # SRS 8.6.3/9.5 train + export to model.h
├── wokwi/
│   ├── diagram.json
│   └── wokwi.toml
├── thingsboard/
│   └── ThingsBoard_Setup_Guide.md
└── docs/
    └── Project_Documentation.md   (this file)
```

**Where every file goes:** the whole `firmware/` folder is opened directly in
PlatformIO (VS Code) or copied file-by-file into a Wokwi project's editor
tabs (`src/*.cpp` → Wokwi "sketch" tabs, `include/*.h` → Wokwi header tabs).
`wokwi/diagram.json` replaces Wokwi's default diagram. `ai_training/*.py`
run on your laptop (not on the ESP32) and their output (`model.h`) is copied
into `firmware/include/model.h`.

**Multi-bay (2-4 bays):** duplicate the whole project as `BAY_02/`, `BAY_03/`
folders, changing only `BAY_ID` and `TB_TOKEN` in each copy's `config.h`
(and GPIO pins if multiple ESP32s share one Wokwi diagram). Each becomes its
own ThingsBoard device; the dashboard and rule chain don't need to change
structurally (NFR-Scalability).

---

## How to build and test each module

1. **Sensors (Module 1-2):** Open Wokwi, run the simulation, watch Serial
   Monitor for `V=... I=... T=...` lines every 5s. Drag the potentiometers
   and confirm the printed voltage/current change (T-01 baseline check).
2. **EV state (Module 3):** Click the green (plug-in) button — Serial should
   print `EV plugged in -> CHARGING`, yellow LED turns on, energy starts
   climbing. Click red (plug-out) — reverts to FREE (T-02, T-03).
3. **MQTT/ThingsBoard (Module 4):** paste your device token into `config.h`,
   rebuild, run — telemetry should appear on ThingsBoard within 10s (T-01).
4. **Edge AI (Module 5):** watch Serial for `[EdgeAI] arrivalProb=... durationMin=...`
   every sample cycle; confirm probability is higher during 8-10/18-21h
   simulated NTP hours than at night (T-06).
5. **Optimization (Module 6):** raise the current potentiometer above
   `overloadCurrentA` (default 16A) — `loadDecision` should flip to
   `THROTTLE`, red LED on, alarm raised on ThingsBoard (T-04).
6. **RPC (Module 8):** send `setRelayState`/`setThrottle` from the ThingsBoard
   dashboard or device RPC dialog — relay and LEDs should respond immediately (T-07).
7. **Shared attributes (Module 4/6):** change `overloadCurrentA` on
   ThingsBoard while running — new threshold applies on the next 30s
   optimization cycle, no reflash (T-08).
8. **Reconnection (NFR-Reliability):** disconnect Wokwi's simulated network
   (or stop/start the simulation) — firmware should auto-reconnect and
   resume telemetry (T-09).

Expected output for a healthy run: Serial Monitor cycling sensor +
optimization debug lines every 5s, ThingsBoard "Latest telemetry" updating
every 5s, dashboard gauges/charts moving live, and an alarm appearing within
~1 optimization cycle of an induced overcurrent.

---

## Testing Plan (SRS Section 10)

| Test ID | Scenario | Action | Expected Result |
|---|---|---|---|
| T-01 | Telemetry publishing | Power on with valid WiFi/MQTT | Telemetry on ThingsBoard within 10s |
| T-02 | EV plug-in | Press plug-in button | FREE→CHARGING, energy starts incrementing |
| T-03 | EV plug-out | Press plug-out button | CHARGING→FREE, energy stops |
| T-04 | Overcurrent | Current pot above threshold | THROTTLE, red LED, alarm raised |
| T-05 | Peak-load (multi-bay) | Low maxStationLoadW, 2+ bays high power | Higher-duration bay(s) THROTTLE |
| T-06 | Demand prediction sanity | Vary hourOfDay | Higher predictedArrivalProb at peak hours |
| T-07 | RPC override | Send setRelayState | Relay changes immediately |
| T-08 | Shared attribute update | Change overloadCurrentA | New threshold applies, no reflash |
| T-09 | Connectivity loss | Disconnect WiFi | Auto-reconnect, telemetry resumes |
| T-10 | Sensor fault | Stop telemetry >60s | "Device offline" alarm raised |

---

## Presentation (PPT) structure — 12 slides
See `Smart_EV_Charging_Optimizer_Presentation.pptx` for the built deck. Slide map:
1. Title — project name, student/team, mentor
2. Problem statement (unmanaged charging, peak overload — Class 1 framing)
3. Objectives (SRS 1.5)
4. System architecture (Edge → Communication → Cloud)
5. Technology stack
6. Hardware / Wokwi circuit
7. Data flow diagram
8. Edge AI model (features, models, training pipeline)
9. Optimization algorithm (ALLOW/THROTTLE/DEFER logic)
10. ThingsBoard dashboard (screenshot placeholders)
11. Results / testing summary
12. Conclusion + future enhancements (SRS 13)

---

## Presentation script (talking points)

- **Slide 2 (Problem):** "Charging stations today allocate power first-come,
  first-served, with no visibility into upcoming demand. When several EVs
  arrive together, the station can exceed its power supply — we saw this in
  our Class 1 example: 8kW demanded by 3 bays against a 5kW supply, with 5
  more EVs expected. Our system predicts and manages this before it happens."
- **Slide 4 (Architecture):** "We follow a three-layer Edge-Communication-Cloud
  design. Inference happens locally on each ESP32, so decisions keep working
  even with a brief connectivity drop — ThingsBoard adds the cross-bay view,
  history, and alarms on top."
- **Slide 8 (Edge AI):** "We generate a synthetic dataset reflecting realistic
  commute-hour patterns, train a logistic regression for arrival probability
  and a decision tree for session duration, then export both to plain C with
  m2cgen so they run on the ESP32 with no TensorFlow runtime needed."
- **Slide 9 (Optimization):** "Each bay checks its own overcurrent first, then
  the whole station's load against the configured cap, then whether it's a
  peak-tariff hour with low predicted demand — deferring low-priority
  sessions to protect both the supply and the electricity bill."
- **Slide 12 (Conclusion):** "This demonstrates the full Edge AI IoT loop —
  sense, predict, decide locally, report and get controlled centrally — on
  a problem that's only going to get more relevant as EV adoption grows."

---

## 5-minute demo script

| Time | Action | What to say |
|---|---|---|
| 0:00-0:30 | Open Wokwi, start simulation | "Here's BAY_01 — ESP32 with simulated voltage/current sensors, a DHT22, plug buttons, and a relay." |
| 0:30-1:15 | Press plug-in button | "EV arrives — bay flips FREE→CHARGING, yellow LED on, energy counter starts climbing in the Serial Monitor and on ThingsBoard." |
| 1:15-2:00 | Switch to ThingsBoard dashboard | "Live telemetry — voltage, current, power, temperature — updating every 5 seconds. Here's the AI's arrival probability and predicted remaining duration." |
| 2:00-2:45 | Drag current potentiometer above 16A | "Simulating an overcurrent fault — watch loadDecision switch to THROTTLE, red LED on, and an alarm fire on the dashboard." |
| 2:45-3:30 | Send `setRelayState:false` RPC from dashboard | "Operator can override remotely — relay drops immediately, bay reports MANUAL_OFF." |
| 3:30-4:15 | Change `overloadCurrentA` shared attribute live | "Update the threshold from the cloud — no reflash — next optimization cycle picks it up." |
| 4:15-5:00 | Press plug-out, wrap up | "Session ends, bay returns to FREE. That's sensing, Edge AI prediction, local optimization, cloud telemetry, alarms, and remote control, end to end." |

---

## Viva questions and answers

**Q1. Why run the AI model on the ESP32 instead of the cloud?**
A: Low-latency, works even with brief connectivity loss, and keeps cloud
compute cost down — this is the core "Edge AI" concept: inference at the
point of data generation (SRS 7.2).

**Q2. Why MQTT instead of HTTP?**
A: MQTT is lightweight publish/subscribe, holds a persistent connection, and
is what ThingsBoard's device API is built around — well suited to frequent,
small telemetry payloads from constrained devices.

**Q3. How do you generate training data without real charging stations?**
A: A Python script (`generate_dataset.py`) synthesizes realistic patterns —
higher arrival probability during commute hours (8-10AM, 6-9PM), lower
overnight, and session durations derived from a simulated battery SoC and
charging rate (SRS 8.6.4).

**Q4. What do `m2cgen` and `micromlgen` do?**
A: They convert a trained scikit-learn model into plain C code (a `predict()`
function), so no TensorFlow/ML runtime is needed on the microcontroller —
just a header file included in the sketch.

**Q5. What happens if ThingsBoard is unreachable?**
A: The optimization logic keeps running locally using its last-known shared
attributes; `network.cpp`'s `ensureConnected()` retries WiFi/MQTT every loop
until it reconnects, then telemetry resumes (T-09).

**Q6. How does the system decide THROTTLE vs DEFER vs ALLOW?**
A: `optimization.cpp` follows SRS 8.7: overcurrent → THROTTLE 50%; total
station power over cap → THROTTLE 70% or ALLOW depending on relative
predicted duration; peak-tariff hour + FREE bay + low arrival probability →
DEFER; otherwise ALLOW 100%.

**Q7. What's the difference between an automatic decision and a manual RPC override?**
A: An RPC (`setRelayState`/`setThrottle`) sets `manualOverrideActive = true`,
which makes the optimization cycle skip itself entirely until the bay next
returns to FREE — so an operator's command can't be silently reversed
mid-session.

**Q8. How would this scale to a real charging station?**
A: Each additional bay is just another ESP32 provisioned as a new
ThingsBoard device with its own token — no dashboard/cloud redesign needed
(NFR-Scalability). Real hardware would swap the potentiometers for an
ACS712 current sensor and a voltage divider/ZMPT101B.

**Q9. What are the main non-functional requirements you addressed?**
A: Performance (≤5s sample/publish cycle, <200ms inference), reliability
(auto-reconnect), security (per-device access tokens), maintainability
(modular firmware — sensing/AI/network/optimization split into separate
files), and portability (WiFi/MQTT abstracted so Arduino Uno+ESP8266 could
substitute for ESP32).

**Q10. What would you improve given more time?**
A: Replace the lightweight model with a TensorFlow Lite Micro network
trained on real station data, add OTA model updates, and integrate a live
electricity tariff API for genuinely dynamic pricing (SRS Section 13).
