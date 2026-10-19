#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "config.h"

bool initDisplay();

void renderDisplay(const SystemData &data);

#endif
