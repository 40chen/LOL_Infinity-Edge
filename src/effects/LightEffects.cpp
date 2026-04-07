#include "LightEffects.h"
#include "../../include/config.h"
#include <Arduino.h>

LightEffects::LightEffects(LedDriver& ledDriver) : led(ledDriver) {}

void LightEffects::update() {
  unsigned long currentTime = millis();

  // Handle light-on animation
  if (lightOnActive && (currentTime - timer > Config::FLASH_DELAY)) {
    timer = currentTime;
    led.setPixel(1, lightCounter, currentRed, currentGreen, currentBlue);
    led.setPixel(1, Config::LED_COUNT1 - 1 - lightCounter, currentRed, currentGreen, currentBlue);
    led.setPixel(2, lightCounter, currentRed, currentGreen, currentBlue);
    led.setPixel(2, Config::LED_COUNT2 - 1 - lightCounter, currentRed, currentGreen, currentBlue);
    led.show();
    lightCounter++;

    if (lightCounter >= (Config::LED_COUNT1 / 2)) {
      lightCounter = 0;
      lightOnActive = false;
      Serial.println("Light-on animation completed");
    }
  }

  // Handle light-off animation
  if (lightOffActive && (currentTime - timer > Config::FLASH_DELAY)) {
    timer = currentTime;
    led.setPixel(1, lightCounter, 0, 0, 0);
    led.setPixel(1, Config::LED_COUNT1 - 1 - lightCounter, 0, 0, 0);
    led.setPixel(2, lightCounter, 0, 0, 0);
    led.setPixel(2, Config::LED_COUNT2 - 1 - lightCounter, 0, 0, 0);
    led.show();

    if (lightCounter == 0) {
      lightOffActive = false;
      Serial.println("Light-off animation completed");
    } else {
      lightCounter--;
    }
  }

  // Handle hit effect
  if (hitActive && (currentTime - timer > 300)) {
    timer = currentTime;
    led.setColor(currentRed, currentGreen, currentBlue);
    hitActive = false;
    Serial.println("Hit effect ended, returning to normal color");
  }
}

void LightEffects::hit() {
  led.setColor(255, 201, 0); // Yellow hit effect
  timer = millis();
  hitActive = true;
  Serial.println("Hit effect activated");
}

void LightEffects::startLightOn() {
  lightOnActive = true;
  lightCounter = 0;
  timer = millis();
  Serial.println("Starting light-on animation");
}

void LightEffects::startLightOff() {
  lightOffActive = true;
  lightCounter = Config::LED_COUNT1 / 2 - 1;
  timer = millis();
  Serial.println("Starting light-off animation");
}

void LightEffects::setColor(uint8_t r, uint8_t g, uint8_t b) {
  currentRed = r;
  currentGreen = g;
  currentBlue = b;
}

void LightEffects::setColorByIndex(byte colorIndex) {
  switch (colorIndex) {
    case 0: // Red
      setColor(255, 0, 0); break;
    case 1: // Green
      setColor(0, 255, 0); break;
    case 2: // Blue
      setColor(0, 0, 255); break;
    case 3: // Pink
      setColor(255, 0, 255); break;
    case 4: // Yellow
      setColor(255, 255, 0); break;
    case 5: // Ice Blue
      setColor(0, 255, 255); break;
  }
}