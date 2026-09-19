"""
generate_dataset.py
SRS 8.6.4 — synthetic training data for the Edge AI demand-prediction models.

Generates synthetic_ev_data.csv with columns matching SRS 8.6.2 features:
  hourOfDay, dayOfWeek, bayOccupied, recentAvgCurrent, sessionElapsedMin,
  neighborBaysOccupied, historicalArrivalRate
plus the two training targets:
  arrivedWithinWindow (0/1)   -> target for the arrival-probability model
  actualDurationMin (float)   -> target for the duration model (rows where bayOccupied=1)

Usage:
    python generate_dataset.py --rows 8000 --out synthetic_ev_data.csv
"""
import argparse
import numpy as np
import pandas as pd


def hour_arrival_rate(hour: int) -> float:
    """Commute-hour bump (8-10, 18-21), quiet overnight (0-5) — SRS 8.6.4."""
    if 8 <= hour <= 10 or 18 <= hour <= 21:
        return 0.70
    if 0 <= hour <= 5:
        return 0.05
    return 0.25


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rows", type=int, default=8000)
    ap.add_argument("--out", type=str, default="synthetic_ev_data.csv")
    ap.add_argument("--seed", type=int, default=42)
    args = ap.parse_args()

    rng = np.random.default_rng(args.seed)
    n = args.rows

    hourOfDay = rng.integers(0, 24, n)
    dayOfWeek = rng.integers(0, 7, n)
    bayOccupied = rng.integers(0, 2, n)
    neighborBaysOccupied = rng.integers(0, 3, n)  # 0-2 other bays busy (3-bay station)

    # Historical arrival rate = the base rate for that hour (what a real
    # station would precompute from history; here it's the generator function itself).
    historicalArrivalRate = np.array([hour_arrival_rate(h) for h in hourOfDay])

    # recentAvgCurrent: near 0 if free, 5-30A with noise if occupied
    recentAvgCurrent = np.where(
        bayOccupied == 1,
        np.clip(rng.normal(16, 6, n), 1, 32),
        np.clip(rng.normal(0.5, 0.3, n), 0, 2),
    )

    # sessionElapsedMin: 0 if free, otherwise 0-90 min into the session
    sessionElapsedMin = np.where(bayOccupied == 1, rng.uniform(0, 90, n), 0.0)

    # arrivedWithinWindow: Bernoulli draw around historicalArrivalRate,
    # slightly suppressed if the bay itself is already occupied and the
    # station is already crowded (fewer free slots => fewer "new" arrivals recorded).
    congestion_penalty = 1.0 - 0.15 * neighborBaysOccupied
    p_arrival = np.clip(historicalArrivalRate * congestion_penalty, 0.01, 0.95)
    arrivedWithinWindow = rng.binomial(1, p_arrival)

    # actualDurationMin: SoC-based session length (SRS 8.6.4) —
    # arrival SoC 20-80%, target 100%, charging rate tied to recentAvgCurrent.
    arrivalSoC = rng.uniform(20, 80, n)
    chargeRateA = np.clip(recentAvgCurrent, 4, 32)
    # crude kWh-to-minutes model for a simulated ~15kWh pack at 230V
    # (sized so demo sessions land in a realistic 15-90 min range)
    packKWh = 15.0
    neededKWh = packKWh * (100 - arrivalSoC) / 100.0
    chargePowerKW = (230.0 * chargeRateA) / 1000.0
    actualDurationMin = np.clip((neededKWh / np.maximum(chargePowerKW, 0.5)) * 60.0, 15, 240)

    df = pd.DataFrame({
        "hourOfDay": hourOfDay,
        "dayOfWeek": dayOfWeek,
        "bayOccupied": bayOccupied,
        "recentAvgCurrent": np.round(recentAvgCurrent, 2),
        "sessionElapsedMin": np.round(sessionElapsedMin, 1),
        "neighborBaysOccupied": neighborBaysOccupied,
        "historicalArrivalRate": np.round(historicalArrivalRate, 3),
        "arrivedWithinWindow": arrivedWithinWindow,
        "actualDurationMin": np.round(actualDurationMin, 1),
    })

    df.to_csv(args.out, index=False)
    print(f"Wrote {len(df)} rows to {args.out}")
    print(df.head())


if __name__ == "__main__":
    main()
