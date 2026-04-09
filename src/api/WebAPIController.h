#pragma once

#include <WebServer.h>
#include <functional>

class WebAPIController {
public:
  WebAPIController();
  
  // Initialize with references to modules
  void begin(WebServer* server,
            std::function<void(bool)> saberStateCallback,
            std::function<bool()> saberStateGetter,
            std::function<void(uint8_t, uint8_t, uint8_t)> ledColorCallback,
            std::function<void(float&, float&, float&)> imuDataGetter);

private:
  // HTTP request handlers
  void handleRoot();
  void switchState();
  void getStatus();
  void setLedColor();
  void getIMUData();
  void handleNotFound();
  
  // Helper methods
  String generateHTML(bool saberState);
  String generateJSON(bool saberState);

  WebServer* server;
  
  // Callbacks
  std::function<bool()> saberStateGetter;
  std::function<void(bool)> saberStateSetter;
  std::function<void(uint8_t, uint8_t, uint8_t)> ledColorSetter;
  std::function<void(float&, float&, float&)> imuDataGetter;
};
