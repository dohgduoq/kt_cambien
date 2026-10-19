#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define PIN_SOIL_ADC        34    
#define PIN_DHT11           18    

#define PIN_I2C_SDA         21    
#define PIN_I2C_SCL         22    

#define PIN_SW1             25    
#define PIN_SW2             26    
#define PIN_SW3             27    
#define PIN_SW4             14    

#define PIN_RELAY_PUMP      32    
#define PIN_RELAY_FAN       33    
#define RELAY_ON            LOW   
#define RELAY_OFF           HIGH

#define PIN_LED1            13    
#define PIN_LED2            12    
#define PIN_LED3            23    

const float TEMP_HIGH_THRESHOLD   = 32.0; 
const float TEMP_HYSTERESIS       = 1.5;  

const int   SOIL_DRY_THRESHOLD    = 40;   
const int   SOIL_HYSTERESIS       = 10;   

const int   ADC_DRY_AIR           = 3500; 
const int   ADC_WET_WATER         = 1300; 

#define WIFI_SSID           "your_wifi_ssid"
#define WIFI_PASSWORD       "your_wifi_password"
#define WIFI_AP_SSID        "esp32"
#define WIFI_AP_PASS        "12345678"

#define WEB_SERVER_PORT     80                    

enum SystemMode {
  MODE_AUTO = 0,
  MODE_MANUAL = 1
};

struct SystemData {
  
  float       temperature;    
  float       humidity;       
  bool        dhtValid;       
  int         soilMoisture;   
  int         rawSoilADC;     

  SystemMode  mode;           
  bool        relayPump;      
  bool        relayFan;       

  bool        wifiConnected;  
  String      ipAddress;      
  uint8_t     displayPage;    
};

extern SystemData g_data;
extern SemaphoreHandle_t g_dataMutex;

#endif
