#include "../lib/display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static bool displayReady = false;

bool initDisplay() {
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    
    displayReady = false;
    return false;
  }

  displayReady = true;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(15, 20);
  display.println(F("SMART AGRICULTURE"));
  display.setCursor(25, 36);
  display.println(F("SYSTEM BOOTING..."));
  display.display();
  
  return true;
}

void renderDisplay(const SystemData &data) {
  if (!displayReady) return;

  display.clearDisplay();
  display.setTextSize(1);

  if (data.displayPage == 0) {

    display.setCursor(0, 0);
    if (data.mode == MODE_AUTO) {
      display.print(F("AUTO "));
    } else {
      display.print(F("MANU "));
    }

    display.print(data.wifiConnected ? F("W:OK ") : F("W:-- "));
    display.print(F("P:")); display.print(data.relayPump ? F("ON ") : F("-- "));
    display.print(F("F:")); display.println(data.relayFan ? F("ON") : F("--"));

    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.setCursor(0, 15);
    display.print(F("Nhiet do: "));
    if (data.dhtValid) display.print(data.temperature, 1);
    else display.print(F("--"));
    display.println(F(" C"));

    display.setCursor(0, 27);
    display.print(F("Do am KK: "));
    if (data.dhtValid) display.print(data.humidity, 1);
    else display.print(F("--"));
    display.println(F(" %"));

    display.setCursor(0, 39);
    display.print(F("Do am dat: "));
    display.print(data.soilMoisture);
    display.println(F(" %"));

    display.setCursor(0, 51);
    display.print(data.dhtValid ? F("DHT11: OK") : F("DHT11: CHO / LOI"));
  } 
  else {

    display.setCursor(10, 0);
    display.println(F("--- IOT NETWORK ---"));
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.setCursor(0, 15);
    display.print(F("Wi-Fi : "));
    display.println(data.wifiConnected ? F("CONNECTED") : F("CONNECTING..."));

    display.setCursor(0, 27);
    display.print(F("IP    : "));
    display.println(data.ipAddress);

    display.setCursor(0, 39);
    display.printf("ADC Dat: %d\n", data.rawSoilADC);

    display.setCursor(0, 52);
    display.println(F("SW4: Ve trang cam bien"));
  }

  display.display();
}
