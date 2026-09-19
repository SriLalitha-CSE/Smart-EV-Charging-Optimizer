#ifndef PERIPHERALS_H
#define PERIPHERALS_H
#include <DHT.h>

extern DHT dht;

// MODULE 1/2: pin setup + sensor acquisition
void peripheralsInit(void);
void readSensors(void);           // FR-1: voltage, current, temperature
void computePowerEnergy(void);    // FR-2: power = V*I, energy accumulation

// MODULE 3: EV arrival/departure + bay state management (FR-3)
void checkPlugButtons(void);

// Actuation
void applyRelayAndLeds(void);     // drives relay + 3 status LEDs from bayStatus/throttleLevel

// Debug print (kept from the original Day-1 sketch)
void printval(void);

#endif
