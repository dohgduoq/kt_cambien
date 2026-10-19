#include <Arduino.h>
#include <Wire.h>
#include "lib/config.h"
#include "lib/sensors.h"
#include "lib/actuators.h"
#include "lib/display.h"
#include "lib/comm.h"

SystemData g_data = {
  .temperature    = NAN,
  .humidity       = NAN,
  .dhtValid       = false,
  .soilMoisture   = 50,
  .rawSoilADC     = 2048,
  .mode           = MODE_AUTO,
  .relayPump      = false,
  .relayFan       = false,
  .wifiConnected  = false,
  .ipAddress      = "0.0.0.0",
  .displayPage    = 0
};

SemaphoreHandle_t g_dataMutex = NULL;

void TaskSensors(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(2500);

  float t = NAN, h = NAN;
  int soilPercent = 50, rawSoilADC = 2048;
  bool dhtValid = false;

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    readAllSensors(t, h, dhtValid, soilPercent, rawSoilADC);

    if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      g_data.temperature     = t;
      g_data.humidity        = h;
      g_data.dhtValid       = dhtValid;
      g_data.soilMoisture    = soilPercent;
      g_data.rawSoilADC      = rawSoilADC;
      xSemaphoreGive(g_dataMutex);
    }
  }
}

void TaskDisplay(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(300);

  SystemData localCopy{};

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      localCopy = g_data;
      xSemaphoreGive(g_dataMutex);
    }

    renderDisplay(localCopy);
  }
}

void TaskControl(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(20);

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    processControlLogic();
  }
}

void TaskComm(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(10);

  unsigned long lastPrintTime = 0;

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    handleComm();

    if (millis() - lastPrintTime >= 2000) {
      lastPrintTime = millis();
      SystemData copy{};
      if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(30)) == pdTRUE) {
        copy = g_data;
        xSemaphoreGive(g_dataMutex);
      }

      Serial.print("Nhiet do: ");
      if (copy.dhtValid) Serial.print(copy.temperature, 1);
      else Serial.print("--");
      Serial.print(" C | Do am KK: ");
      if (copy.dhtValid) Serial.print(copy.humidity, 1);
      else Serial.print("--");
      Serial.print(" % | Do am dat: ");
      Serial.print(copy.soilMoisture);
      Serial.print(" % | ADC: ");
      Serial.println(copy.rawSoilADC);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  g_dataMutex = xSemaphoreCreateMutex();
  if (g_dataMutex == NULL) {
    
    while (1);
  }

  initActuators();
  initDisplay();
  initSensors();
  initComm();

  xTaskCreatePinnedToCore(TaskSensors, "Task_Sensors", 4096, NULL, 2, NULL, 0); 
  xTaskCreatePinnedToCore(TaskDisplay, "Task_Display", 4096, NULL, 1, NULL, 0); 
  xTaskCreatePinnedToCore(TaskControl, "Task_Control", 4096, NULL, 3, NULL, 1); 
  xTaskCreatePinnedToCore(TaskComm,     "Task_Comm",     4096, NULL, 1, NULL, 1); 

}

void loop() {
  
  vTaskDelete(NULL);
}
