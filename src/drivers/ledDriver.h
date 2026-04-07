#pragma once
#include <Adafruit_NeoPixel.h>

class LedDriver {
public:
  LedDriver();
  void begin();
  void setColor(uint8_t r, uint8_t g, uint8_t b);
  void setColorStrip1(uint8_t r, uint8_t g, uint8_t b);
  void setColorStrip2(uint8_t r, uint8_t g, uint8_t b);
  void setPixel(int strip, int pixel, uint8_t r, uint8_t g, uint8_t b);
  void show();
  void clear();
  void clearStrip1();
  void clearStrip2();

private:
  Adafruit_NeoPixel strip1;
  Adafruit_NeoPixel strip2;
};