# Smart EV Charging Station Optimizer (Edge AI + ESP32 + MQTT + ThingsBoard)

Emertxe internship project — full implementation package.

## Quick start
1. **Firmware:** open `firmware/` in PlatformIO (or copy files into a Wokwi
   project). Edit `firmware/include/config.h` — set `TB_TOKEN` after
   creating a device in ThingsBoard (see `thingsboard/ThingsBoard_Setup_Guide.md`).
2. **Wokwi circuit:** use `wokwi/diagram.json` (ESP32, 2 potentiometers,
   DHT22, 2 buttons, 3 LEDs, relay module).
3. **ThingsBoard:** follow `thingsboard/ThingsBoard_Setup_Guide.md` end to
   end — device creation, shared attributes, dashboard widgets, RPC
   controls, alarm rule chain.
4. **Edge AI model:** run `ai_training/generate_dataset.py` then
   `ai_training/train_and_export.py` to regenerate `firmware/include/model.h`
   with real trained coefficients (a working rule-based stub is already in
   place so the rest of the pipeline runs without this step).
5. **Full write-up:** `docs/Project_Documentation.md` — requirement
   analysis, checklist, module-by-module test guide, demo script, viva Q&A.

## What's implemented
All 10 modules from the project brief: ESP32 simulation, sensor processing,
EV state management, MQTT+ThingsBoard telemetry, Edge AI prediction,
optimization (ALLOW/THROTTLE/DEFER), relay/LED control, dashboard, RPC
control (setRelayState/setThrottle/getStatus), and alarms — see the
checklist in `docs/Project_Documentation.md` for the file behind each one.
