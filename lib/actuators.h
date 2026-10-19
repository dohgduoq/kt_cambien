#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>
#include "config.h"

void initActuators();

void applyRelays(bool pumpState, bool fanState);

void updateLeds(SystemMode mode, bool wifiConnected);

void processControlLogic();

#endif
