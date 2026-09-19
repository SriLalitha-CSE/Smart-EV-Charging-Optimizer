#ifndef MODEL_H
#define MODEL_H

// =======================================================================
// SRS 8.6.3 / 9.5 — Edge AI model, exported from Python (scikit-learn)
// via m2cgen (logistic regression) / micromlgen (decision tree).
//
// >>> THIS FILE IS A WORKING STUB. <<<
// Run ai_training/train_and_export.py to regenerate it with real
// trained coefficients. Until then, these two functions use simple,
// documented rule-of-thumb formulas so the MQTT/ThingsBoard pipeline,
// optimization logic, and dashboard can be built and demoed end-to-end
// BEFORE the trained model is plugged in — exactly as SRS Section 9.3's
// note recommends.
//
// Input feature order (SRS 8.6.2), same order used by both functions:
//   features[0] = hourOfDay          (0-23)
//   features[1] = dayOfWeek          (0-6, 0=Sunday)
//   features[2] = bayOccupied        (0 or 1)
//   features[3] = recentAvgCurrent   (A)
//   features[4] = sessionElapsedMin  (minutes, 0 if FREE)
// =======================================================================

// ---- predictArrival(): logistic-regression-style probability [0,1] ----
inline float predictArrival(float features[5]) {
  float hourOfDay = features[0];

  // Commute-hour bump (8-10 AM, 6-9 PM), quiet overnight (12-5 AM) —
  // mirrors the synthetic dataset described in SRS 8.6.4.
  float base = 0.20f;
  if ((hourOfDay >= 8 && hourOfDay <= 10) || (hourOfDay >= 18 && hourOfDay <= 21)) {
    base = 0.75f;
  } else if (hourOfDay >= 0 && hourOfDay <= 5) {
    base = 0.05f;
  }

  // A bay that's already occupied is less likely to receive a *new* arrival signal.
  if (features[2] > 0.5f) base *= 0.5f;

  if (base < 0.0f) base = 0.0f;
  if (base > 1.0f) base = 1.0f;
  return base;
}

// ---- predictDuration(): decision-tree-style estimate, in minutes ----
inline float predictDuration(float features[5]) {
  float sessionElapsedMin = features[4];
  float recentAvgCurrent  = features[3];

  // Rough model: higher current -> faster charge -> shorter remaining time;
  // longer already-elapsed sessions -> less time remaining. Typical full
  // session modeled at ~60 min for a mid-range current draw.
  float typicalSessionMin = 60.0f - (recentAvgCurrent * 1.2f);
  if (typicalSessionMin < 15.0f) typicalSessionMin = 15.0f;

  float remaining = typicalSessionMin - sessionElapsedMin;
  if (remaining < 0.0f) remaining = 0.0f;
  return remaining;
}

#endif
