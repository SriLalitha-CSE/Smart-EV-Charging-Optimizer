#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

// MODULE 6: Charger allocation / optimization — SRS 8.7.
// Runs every OPTIMIZATION_CYCLE_MS, sets loadDecision + throttleLevel,
// unless a manual RPC override (FR-8) is currently active.
void runOptimizationCycle(void);

#endif
