#ifndef EDGE_AI_H
#define EDGE_AI_H

// MODULE 5: Edge AI — builds the SRS 8.6.2 feature vector from live
// state and NTP time, calls the generated model.h predict functions,
// and stores the results in State.h (predictedArrivalProb, predictedDurationMin).
void edgeAiInit(void);          // starts NTP time sync
void edgeAiRunInference(void);  // SRS 8.6.5 on-device inference flow

#endif
