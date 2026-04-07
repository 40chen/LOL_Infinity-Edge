#include "LedDriver.h"
#include "../../include/config.h"

LedDriver::LedDriver()
  : strip1(Config::LED_COUNT1, Config::LED_PIN1, NEO_GRB + NEO_KHZ800),
    strip2(Config::LED_COUNT2, Config::LED_PIN2, NEO_GRB + NEO_KHZ800) {}

void LedDriver::begin() {
  strip1.begin();
  strip1.setBrightness(100);
  strip1.show();

  strip2.begin();
  strip2.setBrightness(100);
  strip2.show();

  clear();
}

void LedDriver::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColorStrip1(r, g, b);
  setColorStrip2(r, g, b);
  show();
}

void LedDriver::setColorStrip1(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < Config::LED_COUNT1; i++) {
    strip1.setPixelColor(i, r, g, b);
  }
}

void LedDriver::setColorStrip2(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < Config::LED_COUNT2; i++) {
    strip2.setPixelColor(i, r, g, b);
  }
}

void LedDriver::setPixel(int strip, int pixel, uint8_t r, uint8_t g, uint8_t b) {
  if (strip == 1) {
    strip1.setPixelColor(pixel, r, g, b);
  } else if (strip == 2) {
    strip2.setPixelColor(pixel, r, g, b);
  }
}

void LedDriver::show() {
  strip1.show();
  strip2.show();
}

void LedDriver::clear() {
  clearStrip1();
  clearStrip2();
  show();
}

void LedDriver::clearStrip1() {
  setColorStrip1(0, 0, 0);
}

void LedDriver::clearStrip2() {
  setColorStrip2(0, 0, 0);
}