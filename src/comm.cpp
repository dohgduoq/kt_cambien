#include "../lib/comm.h"
#include <WiFi.h>
#include <WebServer.h>

static WebServer server(WEB_SERVER_PORT);

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Agriculture Dashboard</title>
  <style>
    body { font-family: 'Segoe UI', Arial, sans-serif; background: #f0f2f5; margin: 0; padding: 20px; color: #333; }
    .header { text-align: center; margin-bottom: 25px; }
    .header h1 { margin: 0; color: #1b5e20; font-size: 26px; }
    .header p { margin: 5px 0 0; color: #666; font-size: 14px; }
    .container { max-width: 800px; margin: 0 auto; display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 15px; }
    .card { background: white; border-radius: 12px; padding: 18px; box-shadow: 0 4px 10px rgba(0,0,0,0.06); text-align: center; }
    .card-title { font-size: 14px; color: #777; margin-bottom: 8px; text-transform: uppercase; font-weight: bold; }
    .card-value { font-size: 28px; font-weight: bold; color: #2e7d32; }
    .card-unit { font-size: 14px; color: #888; margin-left: 4px; }
    .control-panel { max-width: 800px; margin: 25px auto 0; background: white; border-radius: 12px; padding: 20px; box-shadow: 0 4px 10px rgba(0,0,0,0.06); }
    .control-title { font-size: 18px; font-weight: bold; margin-bottom: 15px; color: #333; border-bottom: 2px solid #e0e0e0; padding-bottom: 8px; }
    .btn-group { display: flex; flex-wrap: wrap; gap: 12px; justify-content: center; }
    button { padding: 12px 24px; border: none; border-radius: 8px; font-size: 15px; font-weight: bold; cursor: pointer; transition: 0.2s; color: white; }
    .btn-mode { background: #1976d2; }
    .btn-pump { background: #2e7d32; }
    .btn-fan { background: #f57c00; }
    .btn-off { background: #757575 !important; }
    button:active { transform: scale(0.97); }
    .badge { display: inline-block; padding: 4px 10px; border-radius: 20px; font-size: 12px; font-weight: bold; }
    .badge-auto { background: #e8f5e9; color: #2e7d32; }
    .badge-manu { background: #fff3e0; color: #e65100; }
  </style>
</head>
<body>
  <div class="header">
    <h1>HỆ THỐNG NÔNG NGHIỆP HIỆN ĐẠI</h1>
    <p>Giám sát thời gian thực & Điều khiển IOT từ xa </p>
  </div>

  <div class="container">
    <div class="card">
      <div class="card-title">Nhiệt độ không khí (DHT11)</div>
      <div class="card-value"><span id="temp">--</span><span class="card-unit">°C</span></div>
    </div>
    <div class="card">
      <div class="card-title">Độ ẩm không khí (DHT11)</div>
      <div class="card-value"><span id="hum">--</span><span class="card-unit">%</span></div>
    </div>
    <div class="card">
      <div class="card-title">Độ ẩm đất</div>
      <div class="card-value"><span id="soil">--</span><span class="card-unit">%</span></div>
    </div>
  </div>

  <div class="control-panel">
    <div class="control-title">
      BẢNG ĐIỀU KHIỂN CHẤP HÀNH - Chế độ: <span id="mode-badge" class="badge badge-auto">AUTO</span>
    </div>
    <div class="btn-group">
      <button class="btn-mode" onclick="sendAction('mode_toggle')">ĐỔI CHẾ ĐỘ (AUTO/MANUAL)</button>
      <button id="btn-pump" class="btn-pump" onclick="sendAction('pump_toggle')">MÁY BƠM: --</button>
      <button id="btn-fan" class="btn-fan" onclick="sendAction('fan_toggle')">QUẠT: --</button>
    </div>
  </div>

  <script>
    function updateData() {
      fetch('/api/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('temp').innerText = data.dhtValid ? data.temp.toFixed(1) : '--';
          document.getElementById('hum').innerText = data.dhtValid ? data.hum.toFixed(1) : '--';
          document.getElementById('soil').innerText = data.soil;

          const badge = document.getElementById('mode-badge');
          if (data.mode === "AUTO") {
            badge.className = "badge badge-auto";
            badge.innerText = "AUTO (Tự động)";
          } else {
            badge.className = "badge badge-manu";
            badge.innerText = "MANUAL (Thủ công)";
          }

          const btnPump = document.getElementById('btn-pump');
          btnPump.innerText = "MÁY BƠM: " + (data.pump ? "ĐANG BẬT" : "ĐANG TẮT");
          btnPump.className = data.pump ? "btn-pump" : "btn-off";

          const btnFan = document.getElementById('btn-fan');
          btnFan.innerText = "QUẠT: " + (data.fan ? "ĐANG BẬT" : "ĐANG TẮT");
          btnFan.className = data.fan ? "btn-fan" : "btn-off";
        })
        .catch(err => console.error("Loi doc API:", err));
    }

    function sendAction(act) {
      fetch('/api/control?action=' + act, { method: 'POST' })
        .then(() => setTimeout(updateData, 100));
    }

    setInterval(updateData, 2000);
    updateData();
  </script>
</body>
</html>
)rawliteral";

static void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

static void handleApiData() {
  SystemData copy{};
  if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    copy = g_data;
    xSemaphoreGive(g_dataMutex);
  }

  char dt[20], dh[20];
  if (copy.dhtValid) {
    snprintf(dt, sizeof(dt), "%.1f", copy.temperature);
    snprintf(dh, sizeof(dh), "%.1f", copy.humidity);
  } else {
    strcpy(dt, "null");
    strcpy(dh, "null");
  }
  char jsonBuf[300];
  snprintf(jsonBuf, sizeof(jsonBuf),
           "{\"temp\":%s,\"hum\":%s,\"soil\":%d,\"dhtValid\":%s,\"mode\":\"%s\",\"pump\":%s,\"fan\":%s,\"wifi\":%s}",
           dt, dh, copy.soilMoisture,
           copy.dhtValid ? "true" : "false",
           copy.mode == MODE_AUTO ? "AUTO" : "MANUAL",
           copy.relayPump ? "true" : "false",
           copy.relayFan ? "true" : "false",
           copy.wifiConnected ? "true" : "false");

  server.send(200, "application/json", jsonBuf);
}

static void handleApiControl() {
  if (server.hasArg("action")) {
    String action = server.arg("action");

    if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (action == "mode_toggle") {
        g_data.mode = (g_data.mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
        
      } 
      else if (action == "pump_toggle") {
        g_data.mode = MODE_MANUAL;
        g_data.relayPump = !g_data.relayPump;
        
      } 
      else if (action == "fan_toggle") {
        g_data.mode = MODE_MANUAL;
        g_data.relayFan = !g_data.relayFan;
        
      }
      xSemaphoreGive(g_dataMutex);
    } else {
      server.send(503, "text/plain", "Busy; retry");
      return;
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing action argument");
  }
}

void initComm() {

  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);

  if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    g_data.ipAddress = WiFi.softAPIP().toString();
    xSemaphoreGive(g_dataMutex);
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/data", HTTP_GET, handleApiData);
  server.on("/api/control", HTTP_POST, handleApiControl);
  server.begin();
  
}

void handleComm() {
  static unsigned long lastCheckTime = 0;
  static bool wasConnected = false;

  if (millis() - lastCheckTime >= 1000) {
    lastCheckTime = millis();

    bool isConnected = (WiFi.status() == WL_CONNECTED);

    if (isConnected != wasConnected) {
      wasConnected = isConnected;
      if (xSemaphoreTake(g_dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        g_data.wifiConnected = isConnected;
        if (isConnected) {
          g_data.ipAddress = WiFi.localIP().toString();

        } else {
          
          g_data.ipAddress = WiFi.softAPIP().toString();
          
          WiFi.reconnect();
        }
        xSemaphoreGive(g_dataMutex);
      }
    }
  }

  server.handleClient();
}
