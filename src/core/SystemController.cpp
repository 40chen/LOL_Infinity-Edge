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

  Serial.println("Initializing service layer...");
  // 初始化服务层 - 服务层使用驱动程序
  imuService.begin(&imu);
  ledService.begin(&led);
  audioService.begin(&audio);

  Serial.println("Initializing application layer...");
  // 应用层通过服务层访问驱动程序
  saber.begin(&imuService, &ledService, &audioService);

  Serial.println("Setting up HTTP API endpoints...");
  // Initialize Web API with direct object references
  webAPI.begin(&server, &saber, &led, &imu);

  Serial.println("===== System initialized successfully =====\n");
}

// Main loop: handle events and update state
void SystemController::update() {

  // Handle incoming HTTP requests (non-blocking)
  server.handleClient();

  // Update saber logic (gestures, effects, etc.)
  saber.tick();
}

}  // namespace Core
