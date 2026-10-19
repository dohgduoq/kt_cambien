#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "config.h"

bool initSensors();

void readAllSensors(float &temp, float &hum, bool &dhtValid, int &soilPercent, int &rawADC);

#endif
