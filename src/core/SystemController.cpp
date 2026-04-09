#include "SystemController.h"
#include "../../include/config.h"
#include <Arduino.h>
#include <Wire.h>

namespace Core {

void SystemController::begin() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n===== LOL Infinity Edge System Starting =====");
  Serial.println("Initializing hardware drivers...");

  // Initialize I2C bus
  Wire.begin(Config::I2C_SDA, Config::I2C_SCL);

  // Initialize drivers in correct order
  // (some drivers may depend on others being initialized first)
  imu.begin();
  sdCard.begin();
  led.begin();
  audio.begin();
  wifi.begin();

  Serial.println("Initializing application layer...");
  saber.begin(&imu, &led, &audio, &effects);

  Serial.println("Setting up HTTP API endpoints...");
  // Initialize Web API with callbacks to control the saber
  webAPI.begin(&server,
    // Callback: set saber state
    [this](bool state) { saber.setState(state); },

    // Callback: get current saber state
    [this]() { return saber.getState(); },

    // Callback: set LED color
    [this](uint8_t r, uint8_t g, uint8_t b) { led.setColor(r, g, b); },

    // Callback: get IMU acceleration data
    [this](float& x, float& y, float& z) {
      x = imu.getAx() / 2048.0f;
      y = imu.getAy() / 2048.0f;
      z = imu.getAz() / 2048.0f;
    });

  Serial.println("===== System initialized successfully =====\n");
}

void SystemController::update() {
  // Handle incoming HTTP requests (non-blocking)
  server.handleClient();

  // Update application logic (motion detection, state management)
  saber.update();
}

}  // namespace Core
