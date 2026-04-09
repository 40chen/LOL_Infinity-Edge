#include "WebAPIController.h"
#include <Arduino.h>

WebAPIController::WebAPIController() : server(nullptr) {}

void WebAPIController::begin(WebServer* webServer,
                            std::function<void(bool)> saberStateSetter,
                            std::function<bool()> saberStateGetter,
                            std::function<void(uint8_t, uint8_t, uint8_t)> ledColorSetter,
                            std::function<void(float&, float&, float&)> imuDataGetter) {
  server = webServer;
  this->saberStateSetter = saberStateSetter;
  this->saberStateGetter = saberStateGetter;
  this->ledColorSetter = ledColorSetter;
  this->imuDataGetter = imuDataGetter;
  
  // Register all HTTP routes
  server->on("/", HTTP_GET, [this]() { handleRoot(); });
  server->on("/api/toggle", HTTP_POST, [this]() { switchState(); });
  server->on("/api/status", HTTP_GET, [this]() { getStatus(); });
  server->on("/api/led", HTTP_POST, [this]() { setLedColor(); });
  server->on("/api/imu", HTTP_GET, [this]() { getIMUData(); });
  server->onNotFound([this]() { handleNotFound(); });
  
  // Start the WebServer
  server->begin();
  
  Serial.println("Web API Controller routes registered and server started");
}

void WebAPIController::handleRoot() {
  bool saberState = saberStateGetter ? saberStateGetter() : false;
  server->send(200, "text/html", generateHTML(saberState));
}

void WebAPIController::switchState() {
  if (saberStateGetter && saberStateSetter) {
    bool currentState = saberStateGetter();
    saberStateSetter(!currentState);
    
    String response = "{\"success\":true,\"newState\":" + String(!currentState ? "true" : "false") + "}";
    server->send(200, "application/json", response);
  } else {
    server->send(500, "application/json", "{\"error\":\"Saber state setter not configured\"}");
  }
}

void WebAPIController::getStatus() {
  bool saberState = saberStateGetter ? saberStateGetter() : false;
  server->send(200, "application/json", generateJSON(saberState));
}

void WebAPIController::setLedColor() {
  if (server->hasArg("r") && server->hasArg("g") && server->hasArg("b")) {
    uint8_t r = server->arg("r").toInt();
    uint8_t g = server->arg("g").toInt();
    uint8_t b = server->arg("b").toInt();
    
    if (ledColorSetter) {
      ledColorSetter(r, g, b);
      String response = "{\"success\":true,\"color\":{\"r\":" + String(r) + ",\"g\":" + String(g) + ",\"b\":" + String(b) + "}}";
      server->send(200, "application/json", response);
    } else {
      server->send(500, "application/json", "{\"error\":\"LED setter not configured\"}");
    }
  } else {
    server->send(400, "application/json", "{\"error\":\"Missing or invalid parameters\"}");
  }
}

void WebAPIController::getIMUData() {
  if (imuDataGetter) {
    float x = 0, y = 0, z = 0;
    imuDataGetter(x, y, z);
    
    String response = "{\"imu\":{\"x\":" + String(x, 2) + ",\"y\":" + String(y, 2) + ",\"z\":" + String(z, 2) + "}}";
    server->send(200, "application/json", response);
  } else {
    server->send(500, "application/json", "{\"error\":\"IMU getter not configured\"}");
  }
}

void WebAPIController::handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: " + server->uri() + "\n";
  message += "Method: " + String(server->method() == HTTP_GET ? "GET" : "POST") + "\n";
  server->send(404, "text/plain", message);
}

String WebAPIController::generateHTML(bool saberState) {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8' name='viewport' content='width=device-width'>";
  html += "<title>ESP32S3 Saber Control</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }";
  html += "h1 { color: #333; margin-top: 0; }";
  html += "button { padding: 10px 20px; margin: 5px; font-size: 16px; cursor: pointer; border: none; border-radius: 4px; background-color: #4CAF50; color: white; }";
  html += "button:hover { background-color: #45a049; }";
  html += ".status { padding: 20px; margin: 0 0 20px 0; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += ".on { background: linear-gradient(135deg, #90EE90 0%, #66BB6A 100%); border-left: 6px solid #28a745; }";
  html += ".off { background: linear-gradient(135deg, #FFB6C6 0%, #FF7F9C 100%); border-left: 6px solid #dc3545; }";
  html += ".status h2 { margin: 0 0 10px 0; color: white; text-shadow: 0 2px 4px rgba(0,0,0,0.2); }";
  html += ".status-info { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }";
  html += ".info-item { background: rgba(255,255,255,0.9); padding: 10px; border-radius: 4px; }";
  html += ".info-label { font-size: 12px; color: #666; font-weight: bold; }";
  html += ".info-value { font-size: 18px; color: #333; font-weight: bold; margin-top: 5px; }";
  html += ".control-section { margin: 20px 0; padding: 15px; background-color: white; border-radius: 4px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }";
  html += ".control-section h3 { margin-top: 0; }";
  html += ".color-picker { display: flex; align-items: center; gap: 10px; }";
  html += "input[type='color'] { width: 80px; height: 80px; border: none; border-radius: 8px; cursor: pointer; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }";
  html += ".color-label { font-size: 14px; color: #666; }";
  html += ".color-display { width: 100px; height: 100px; border-radius: 12px; border: none; box-shadow: 0 6px 20px rgba(0,0,0,0.15), inset 0 1px 0 rgba(255,255,255,0.3); cursor: pointer; transition: transform 0.2s, box-shadow 0.2s; }";
  html += ".color-display:hover { transform: scale(1.05); box-shadow: 0 8px 28px rgba(0,0,0,0.2), inset 0 1px 0 rgba(255,255,255,0.3); }";
  html += ".sensor-data { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 10px; }";
  html += ".sensor-item { background: #f9f9f9; padding: 10px; border-radius: 4px; text-align: center; }";
  html += ".sensor-label { font-size: 12px; color: #999; }";
  html += ".sensor-value { font-size: 16px; font-weight: bold; color: #333; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>⚡ Light Saber Control Panel ⚡</h1>";
  
  html += "<div class='status " + String(saberState ? "on" : "off") + "'>";
  html += "<h2>" + String(saberState ? "🟢 SABER ACTIVE" : "🔴 SABER INACTIVE") + "</h2>";
  html += "</div>";
  
  html += "<div class='control-section'>";
  html += "<h3>💡 Power Control</h3>";
  html += "<button onclick=\"fetch('/api/toggle', {method:'POST'}).then(()=>location.reload())\">Toggle Saber</button>";
  html += "</div>";
  
  html += "<div class='control-section'>";
  html += "<h3>🎨 LED Color Control</h3>";
  html += "<div class='color-picker'>";
  html += "<input type='color' id='colorPicker' value='#FF0000' onchange='setLedColor(this.value)'>";
  html += "</div>";
  html += "</div>";
  
  html += "<div class='control-section'>";
  html += "<h3>📊 Sensor Data</h3>";
  html += "<div class='sensor-data'>";
  html += "<div class='sensor-item'>";
  html += "<div class='sensor-label'>Acceleration X</div>";
  html += "<div class='sensor-value' id='accelX'>0.00g</div>";
  html += "</div>";
  html += "<div class='sensor-item'>";
  html += "<div class='sensor-label'>Acceleration Y</div>";
  html += "<div class='sensor-value' id='accelY'>0.00g</div>";
  html += "</div>";
  html += "<div class='sensor-item'>";
  html += "<div class='sensor-label'>Acceleration Z</div>";
  html += "<div class='sensor-value' id='accelZ'>0.00g</div>";
  html += "</div>";
  html += "</div>";
  html += "<button onclick=\"fetchIMUData()\">Refresh Sensor Data</button>";
  html += "</div>";
  
  html += "<script>";
  html += "function setLedColor(hexColor) {";
  html += "  const r = parseInt(hexColor.substr(1, 2), 16);";
  html += "  const g = parseInt(hexColor.substr(3, 2), 16);";
  html += "  const b = parseInt(hexColor.substr(5, 2), 16);";
  html += "  fetch(`/api/led?r=${r}&g=${g}&b=${b}`, {method:'POST'})";
  html += "    .then(r => r.json())";
  html += "    .then(d => console.log('LED updated:', d))";
  html += "    .catch(e => console.error('Error:', e));";
  html += "}";
  html += "function fetchIMUData() {";
  html += "  fetch('/api/imu')";
  html += "    .then(r => r.json())";
  html += "    .then(d => {";
  html += "      document.getElementById('accelX').textContent = d.imu.x.toFixed(2) + 'g';";
  html += "      document.getElementById('accelY').textContent = d.imu.y.toFixed(2) + 'g';";
  html += "      document.getElementById('accelZ').textContent = d.imu.z.toFixed(2) + 'g';";
  html += "    })";
  html += "    .catch(e => console.error('Error:', e));";
  html += "}";
  html += "fetchIMUData();";
  html += "setInterval(fetchIMUData, 2000);";
  html += "</script>";
  
  html += "</body></html>";
  return html;
}

String WebAPIController::generateJSON(bool saberState) {
  return "{\"status\":\"" + String(saberState ? "ON" : "OFF") + "\"}";
}
