#pragma once
#include <Arduino.h>

class Timer {
public:
  bool isExpired(unsigned long interval) {
    if (millis() - last >= interval) {
      last = millis();
      return true;
    }
    return false;
  }

private:
  unsigned long last = 0;
};