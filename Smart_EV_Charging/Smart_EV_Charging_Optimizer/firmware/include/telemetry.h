#ifndef TELEMETRY_H
#define TELEMETRY_H

// FR-7: publish full telemetry payload (SRS 8.3 schema)
void publishTelemetry();

// Client-side attribute publish (firmware version), SRS 7.3 topic table
void publishClientAttributes();

#endif
