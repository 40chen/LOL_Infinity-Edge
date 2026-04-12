#pragma once

#include "../drivers/LedDriver.h"
#include "../../include/config.h"

struct RGB {
  uint8_t r, g, b;
};

class LightEffects {
public:
  LightEffects(LedDriver& ledDriver);

  void update();
  void hit();
  void startLightOn();
  void startLightOff();
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void setColorByIndex(byte colorIndex);

  uint8_t getCurrentRed() const { return currentRed; }
  uint8_t getCurrentGreen() const { return currentGreen; }
  uint8_t getCurrentBlue() const { return currentBlue; }
  RGB getCurrentColor() const { return {currentRed, currentGreen, currentBlue}; }

private:
  LedDriver& led;
  unsigned long timer = 0;
  bool lightOnActive = false;
  bool lightOffActive = false;
  bool hitActive = false;
  byte lightCounter = 0;
  uint8_t currentRed = 0, currentGreen = 255, currentBlue = 0; // Default green
};