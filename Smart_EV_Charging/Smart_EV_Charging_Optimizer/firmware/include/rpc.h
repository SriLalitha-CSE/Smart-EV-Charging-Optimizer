#ifndef RPC_H
#define RPC_H

#include <Arduino.h>

// MODULE 8: RPC control — SRS 8.5 (setRelayState, setThrottle, getStatus)
void handleRpc(String requestId, char* payload);

#endif
