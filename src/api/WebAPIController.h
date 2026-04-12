#pragma once

#include <WebServer.h>

class SaberController;
class LedDriver;
class ImuDriver;

class WebAPIController {
public:
  WebAPIController();
  
  // Initialize with direct object references
  void begin(WebServer* server,
            SaberController* saber,
            LedDriver* led,
            ImuDriver* imu);

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
  
  // Direct object references
  SaberController* saber;
  LedDriver* led;
  ImuDriver* imu;
};
