# ThingsBoard Setup Guide
## Smart EV Charging Station Optimizer

Covers Module 4 (device provisioning), Module 7 (dashboard), Module 8 (RPC), Module 9 (alarms).

---

## 1. Create your ThingsBoard account / instance

You have two options:

**Option A — Cloud (fastest for a demo):**
1. Go to https://thingsboard.cloud and sign up for a free account.
2. Log in to the web UI.

**Option B — Self-hosted via Docker (SRS 2.4):**
```bash
docker run -it -p 8080:9090 -p 1883:1883 -p 7070:7070 -p 5683-5688:5683-5688/udp \
  --name tb thingsboard/tb-postgres
```
Wait for it to finish initializing, then open `http://localhost:8080`
(default login `tenant@thingsboard.org` / `tenant`).

---

## 2. Create one device per charging bay

1. In the left menu, go to **Devices**.
2. Click **+ → Add new device**.
3. Name it `BAY_01` (repeat for `BAY_02`, `BAY_03` if you're doing multi-bay).
4. Leave device profile as `default`.
5. Open the new device, go to the **Details** tab, click **Copy access token**.
6. Paste that token into `firmware/include/config.h` as `TB_TOKEN`, and set
   `BAY_ID` to match (`"BAY_01"`, etc). Each bay's firmware folder gets its
   own token — this is what lets ThingsBoard tell the bays apart.

---

## 3. Set shared attributes per device (SRS 8.4)

Open a device → **Attributes** tab → **+ Add** → scope = **Shared attribute**.
Add these five, matching the names the firmware requests:

| Key | Type | Suggested value |
|---|---|---|
| `maxStationLoadW` | number | `6000` |
| `overloadCurrentA` | number | `16` |
| `peakTariffStartHr` | number | `18` |
| `peakTariffEndHr` | number | `21` |
| `predictionThreshold` | number | `0.5` |

The firmware requests these on every MQTT connect and also listens for live
pushes — change a value here while the simulation is running and the ESP32
picks it up on its next optimization cycle, with no reflash (test T-08).

---

## 4. Verify telemetry is arriving

Run the Wokwi simulation (or real hardware) with the token in place. Within
~10 seconds you should see live values under the device's **Latest telemetry**
tab: `voltage`, `current`, `power`, `energyWh`, `temperature`, `bayStatus`,
`predictedArrivalProb`, `predictedDurationMin`, `loadDecision`,
`throttleLevel`, `overloadActive`, `manualOverrideActive` (test T-01).

---

## 5. Build the dashboard (Module 7)

**Dashboards → + Create new dashboard → "EV Charging Station Optimizer".**
Click **Edit** and add these widgets (bind each to the device's latest telemetry
unless noted). Use one widget per row unless you're grouping by bay.

| # | Widget | Widget bundle | Data key(s) | Notes |
|---|---|---|---|---|
| 1 | Voltage gauge | Gauges → Analogue gauge | `voltage` | Range 0-250 |
| 2 | Current gauge | Gauges → Analogue gauge | `current` | Range 0-32 |
| 3 | Power chart | Charts → Time series chart | `power` | |
| 4 | Energy usage chart | Charts → Time series chart | `energyWh` | |
| 5 | Temperature chart | Charts → Time series chart | `temperature` | |
| 6 | Bay status card | Cards → Entities table / Status widget | `bayStatus` | Color-code FREE=green, CHARGING=amber, FAULT=red via widget settings |
| 7 | AI arrival prediction | Cards → Value card, or Charts → Bar chart | `predictedArrivalProb` | Format as % |
| 8 | Remaining charging time | Cards → Value card | `predictedDurationMin` | Suffix "min" |
| 9 | Charging decision | Cards → Value card / Label card | `loadDecision` | |
| 10 | Alarm panel | Alarm widgets → Alarms table | (entity alarms) | See Section 7 below |

**For a 3-bay station**, repeat widgets 1-9 per device, or use an **entity
alias** of "All charging bays" (device type filter) with a multi-series
chart so all three bays plot on the same graph — good for the "Total Station
Load" view (see rule chain aggregation, Section 7).

**Manual override control (Module 8):**
Add a **Control widgets → Switch** (or Button) widget, bind it to the RPC
method `setRelayState` with param `{"state": true/false}`. Add a **Control
widgets → Slider** bound to `setThrottle` with param `{"level": 0-100}`.
These call the RPC methods implemented in `firmware/src/rpc.cpp`.

---

## 6. RPC control (Module 8, SRS 8.5)

Three RPC methods are implemented in firmware (`rpc.cpp`):

| Method | Params | Effect |
|---|---|---|
| `setRelayState` | `{"state": true\|false}` | Manual ON/OFF override |
| `setThrottle` | `{"level": 0-100}` | Manual throttle percentage |
| `getStatus` | `{}` | Returns a full status snapshot |

**To test from the ThingsBoard UI**, use the dashboard control widgets above,
or from a device's **Details → Make an RPC request** dialog, e.g.:
```json
{ "method": "setRelayState", "params": { "state": false } }
```
Expect the relay to switch immediately and `bayStatus`/LEDs to reflect it
within one telemetry cycle (test T-07). Note: once an RPC override fires,
the firmware sets `manualOverrideActive = true` and the optimization loop
stands down until the bay is unplugged (plug-out button) — this is
intentional so an operator's manual command isn't silently overwritten.

---

## 7. Rule chain: alarms + station-wide aggregation (Module 9, SRS 8.9)

Open **Rule Chains → Root Rule Chain → Edit**. Add these nodes after the
existing "Message Type Switch" node (on the "Post telemetry" output):

**a) Overcurrent alarm**
- Add a **Filter → Script (or "check overload")** node:
  ```javascript
  return msg.overloadActive === true;
  ```
- Connect its `True` output to a **Create alarm** node:
  - Alarm type: `Overload`
  - Severity: `CRITICAL`
  - Details script: `"Overload on " + msg.bayId`

**b) Sensor / device offline alarm**
- Add an **Filter → "Device inactivity"** rule node (built into ThingsBoard's
  device profile: Device profile → Alarm Rules → Device Inactivity → set
  60s timeout) which raises a **MAJOR** "Device offline" alarm automatically
  (test T-10) — no custom rule chain node needed for this one.

**c) High-demand-no-free-bays alarm**
- Add a **Filter → Script** node:
  ```javascript
  return msg.predictedArrivalProb > metadata.predictionThreshold; // combine with an aggregation check for "all bays CHARGING" via an asset-level rule, or evaluate station-wide in an external rule using a saved attribute
  ```
- Connect True → **Create alarm** node, severity `WARNING`,
  message `"High demand expected, no free bays"`.

**d) Total station power aggregation**
- Add a **Enrichment → "Originator Attributes"/"Related" or a Script**
  node that, on each bay's telemetry, saves `power` into an **Asset**
  representing the station (create an Asset called `Charging Station`, and
  relate each `BAY_xx` device to it as "Contains"). A simple approach:
  - Script node sums the latest `power` of related bay devices and calls
    `saveTimeseries` on the station asset key `totalStationPower`.
  - Bind dashboard widget "Time-Series — Total Station Load" to this asset.

**e) Scheduled tariff push**
- Add a **Generator node** (or an external cron/script) that periodically
  calls **Save attributes** on each device to push updated
  `peakTariffStartHr` / `peakTariffEndHr` values — demonstrates FR-8/T-08
  without a reflash.

Click **Save**, then **Apply changes** — the root rule chain will now raise
alarms as telemetry comes in.

---

## 8. Historical data (FR-11)

No setup needed — ThingsBoard stores every telemetry point by default.
Use the dashboard's time-series widgets' built-in time-window picker
(top-right of each widget) to show "last hour", "last day", etc. for
trend analysis (utilization / peak vs off-peak).

---

## Quick checklist

- [ ] Device(s) created, access token(s) copied into `config.h`
- [ ] Shared attributes set (5 keys)
- [ ] Telemetry visible under Latest Telemetry
- [ ] Dashboard built with all 10 widgets
- [ ] RPC switch/slider tested from dashboard
- [ ] Overcurrent rule chain node + alarm tested
- [ ] Device-offline alarm rule enabled
- [ ] (Multi-bay) station asset + aggregation node configured
