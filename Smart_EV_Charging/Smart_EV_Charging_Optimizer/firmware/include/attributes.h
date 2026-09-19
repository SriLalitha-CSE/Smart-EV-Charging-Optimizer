#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include <ArduinoJson.h>

// MODULE 4 (attributes half): SRS 8.4 shared attributes.
void requestSharedAttributes();          // ask ThingsBoard for current values on connect
void applySharedAttributes(JsonObject attrs); // apply a push/response payload to State.h

#endif
