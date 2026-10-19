#include "../lib/actuators.h"

struct Button {
  uint8_t pin;
  bool lastStableState;
  bool lastReading;
  unsigned long lastDebounceTime;
};

static Button btnPump = {PIN_SW1, HIGH, HIGH, 0}; 
static Button btnFan  = {PIN_SW2, HIGH, HIGH, 0}; 
static Button btnMode = {PIN_SW3, HIGH, HIGH, 0}; 
static Button btnPage = {PIN_SW4, HIGH, HIGH, 0}; 

static constexpr unsigned long BUTTON_DEBOUNCE_MS = 30;
static bool checkButtonPressed(Button &btn, const char* btnName) {
  bool pressed = false;
  int reading = digitalRead(btn.pin);

  if (reading != btn.lastReading) {
    btn.lastDebounceTime = millis();
    btn.lastReading = reading;
    
  }

  if ((millis() - btn.lastDebounceTime) >= BUTTON_DEBOUNCE_MS) {
    if (reading != btn.lastStableState) {
      btn.lastStableState = reading;
      if (btn.lastStableState == LOW) {
        pressed = true;
      }
    }
  }
  return pressed;
}

void initActuators() {
  
  pinMode(PIN_SW1, INPUT_PULLUP);
  pinMode(PIN_SW2, INPUT_PULLUP);
  pinMode(PIN_SW3, INPUT_PULLUP);
  pinMode(PIN_SW4, INPUT_PULLUP);

  digitalWrite(PIN_RELAY_PUMP, RELAY_OFF);
  digitalWrite(PIN_RELAY_FAN, RELAY_OFF);
  pinMode(PIN_RELAY_PUMP, OUTPUT);
  pinMode(PIN_RELAY_FAN, OUTPUT);

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);
  digitalWrite(PIN_LED1, HIGH); 
  digitalWrite(PIN_LED2, LOW);
  digitalWrite(PIN_LED3, LOW);

  const uint8_t ledPins[] = {PIN_LED1, PIN_LED2, PIN_LED3};
  for (uint8_t pin : ledPins) digitalWrite(pin, LOW);
  for (uint8_t pin : ledPins) {
    
    digitalWrite(pin, HIGH);
    delay(250);
    digitalWrite(pin, LOW);
  }
  digitalWrite(PIN_LED1, HIGH);
  
}

void applyRelays(bool pumpState, bool fanState) {
  digitalWrite(PIN_RELAY_PUMP, pumpState ? RELAY_ON : RELAY_OFF);
  digitalWrite(PIN_RELAY_FAN,  fanState  ? RELAY_ON : RELAY_OFF);
}

void updateLeds(SystemMode mode, bool wifiConnected) {
  
  digitalWrite(PIN_LED1, HIGH);

  static unsigned long lastBlinkMode = 0;
  static bool led2State = false;
  if (mode == MODE_AUTO) {
    digitalWrite(PIN_LED2, HIGH);
  } else {
    if (millis() - lastBlinkMode >= 500) {
      lastBlinkMode = millis();
      led2State = !led2State;
      digitalWrite(PIN_LED2, led2State ? HIGH : LOW);
    }
  }

  static unsigned long lastBlinkWifi = 0;
  static bool led3State = false;
  if (wifiConnected) {
    digitalWrite(PIN_LED3, HIGH);
  } else {
    if (millis() - lastBlinkWifi >= 250) {
      lastBlinkWifi = millis();
      led3State = !led3State;
      digitalWrite(PIN_LED3, led3State ? HIGH : LOW);
    }
  }
}

void processControlLogic() {
  
  static bool sw1Pressed = false, sw2Pressed = false;
  static bool sw3Pressed = false, sw4Pressed = false;
  sw1Pressed |= checkButtonPressed(btnPump, "SW1 (Pump)");
  sw2Pressed |= checkButtonPressed(btnFan,  "SW2 (Fan)");
  sw3Pressed |= checkButtonPressed(btnMode, "SW3 (Mode)");
  sw4Pressed |= checkButtonPressed(btnPage, "SW4 (OLED)");

  if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    
    if (sw3Pressed) {
      g_data.mode = (g_data.mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
      
    }

    if (sw4Pressed) {
      g_data.displayPage = (g_data.displayPage == 0) ? 1 : 0;
      
    }

    if (sw1Pressed) {
      if (g_data.mode == MODE_AUTO) {
        g_data.mode = MODE_MANUAL;
        
      }
      g_data.relayPump = !g_data.relayPump;
      
    }

    if (sw2Pressed) {
      if (g_data.mode == MODE_AUTO) {
        g_data.mode = MODE_MANUAL;
        
      }
      g_data.relayFan = !g_data.relayFan;
      
    }

    if (g_data.mode == MODE_AUTO) {
      
      if (g_data.soilMoisture < SOIL_DRY_THRESHOLD) {
        g_data.relayPump = true;  
      } else if (g_data.soilMoisture > (SOIL_DRY_THRESHOLD + SOIL_HYSTERESIS)) {
        g_data.relayPump = false; 
      }

      if (g_data.dhtValid && g_data.temperature > TEMP_HIGH_THRESHOLD) {
        g_data.relayFan = true;   
      } else if (g_data.dhtValid && g_data.temperature < (TEMP_HIGH_THRESHOLD - TEMP_HYSTERESIS)) {
        g_data.relayFan = false;  
      }
    }

    applyRelays(g_data.relayPump, g_data.relayFan);

    updateLeds(g_data.mode, g_data.wifiConnected);

    sw1Pressed = sw2Pressed = sw3Pressed = sw4Pressed = false;
    xSemaphoreGive(g_dataMutex);
  }
}
