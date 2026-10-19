#include "../lib/sensors.h"
#include <DHT.h>

static DHT dht(PIN_DHT11, DHT11);

bool initSensors() {
  dht.begin();

  pinMode(PIN_SOIL_ADC, INPUT);
  analogReadResolution(12);       
  analogSetAttenuation(ADC_11db); 

  return true;
}

void readAllSensors(float &temp, float &hum, bool &dhtValid, int &soilPercent, int &rawADC) {
  
  float dt = dht.readTemperature();
  float dh = dht.readHumidity();
  dhtValid = !isnan(dt) && !isnan(dh) && dt >= -20.0f && dt <= 60.0f && dh >= 0.0f && dh <= 100.0f;
  temp = dhtValid ? dt : NAN;
  hum = dhtValid ? dh : NAN;

  long sumADC = 0;
  for (int i = 0; i < 10; i++) {
    sumADC += analogRead(PIN_SOIL_ADC);
    vTaskDelay(pdMS_TO_TICKS(2)); 
  }
  rawADC = sumADC / 10;

  int percent = map(rawADC, ADC_DRY_AIR, ADC_WET_WATER, 0, 100);
  soilPercent = constrain(percent, 0, 100);
}
