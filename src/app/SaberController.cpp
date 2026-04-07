#include "SaberController.h"
#include "../../include/config.h"
#include <Arduino.h>

void SaberController::begin() {

  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Short delay to allow serial monitor to connect
  delay(1000);

  Serial.println("Initializing Light Saber System...");

  // Initialize I2C communication with specified SDA and SCL pins
  Wire.begin(Config::I2C_SDA, Config::I2C_SCL);

  // Initialize components
  imu.begin();
  sdCard.begin();
  led.begin();
  audio.begin();

  effects.setColorByIndex(currentColor);

  Serial.println("System ready! Starting motion detection...");

  // Play startup sound
  audio.play("saber.flac");

  // Startup indicator
  led.setColor(0, 255, 0); // Green
  delay(500);
  led.clear();

  Serial.println("Initialization complete");
}

void SaberController::update() {
  unsigned long currentTime = millis();

  imu.update();
  audio.loop();

  handleGesture();
  handleSwing();
  handleStrike();
  handlePulse();
  handleHum();

  effects.update();

  delay(10);
}

void SaberController::handleGesture() {
  if (Config::GESTURE_ALLOW == 1) {
    if (millis() - gestureTimer > 50) {
      gestureTimer = millis();

      float accX = (float)imu.getAx() / 2048.0;
      float accY = (float)imu.getAy() / 2048.0;
      float accZ = (float)imu.getAz() / 2048.0;

      rolls[gesCounter & (Config::IMU_CAL_COUNTER - 1)] = atan2(accY, accZ) * 57.2974;
      pitchs[gesCounter & (Config::IMU_CAL_COUNTER - 1)] = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 57.2974;

      float rollMin = 10000, rollMax = -10000;
      float pitchMin = 10000, pitchMax = -10000;

      for (int i = 0; i < Config::IMU_CAL_COUNTER; i++) {
        rollMin = rolls[i] < rollMin ? rolls[i] : rollMin;
        rollMax = rolls[i] > rollMax ? rolls[i] : rollMax;
        pitchMin = pitchs[i] < pitchMin ? pitchs[i] : pitchMin;
        pitchMax = pitchs[i] > pitchMax ? pitchs[i] : pitchMax;
      }

      if (pitchMax - pitchMin > Config::OPEN_THR && countOpen == 0) {
        toggleState();
        countOpen = 40;
        Serial.println("Gesture detected - triggering ON/OFF sequence");
      }

      gesCounter++;

      if (countOpen > 0) countOpen--;
    }
  }
}

void SaberController::handleSwing() {
  float gyr = imu.getGyroMagnitude();

  if (gyr > 60 && (millis() - swingTimeout > 100) && state == SaberState::ON &&
      lightOnFlag == 0 && lightOffFlag == 0) {

    swingTimeout = millis();

    if ((millis() - swingTimer) > Config::SWING_TIMEOUT && swingFlag && !strikeFlag) {
      byte number;
      if (gyr >= Config::SWING_THR) {
        number = (esp_random() % 7) * 2 + 1;
        audio.play(swngs[number]);
        humTimer = millis() - Config::HUM_TIMEOUT + 1000;
        swingFlag = 0;
        swingTimer = millis();
        Serial.println("Fast swing detected! Playing fast swing sound");
      } else if (gyr < Config::SWING_THR && gyr > Config::SWING_L_THR) {
        number = (esp_random() % 8) * 2 + 1;
        audio.play(swngs[number]);
        humTimer = millis() - Config::HUM_TIMEOUT + 1000;
        swingFlag = 0;
        swingTimer = millis();
        Serial.println("Slow swing detected! Playing slow swing sound");
      }
    }
  }
}

void SaberController::handleStrike() {
  float acc = imu.getAccelMagnitude();

  if ((acc > Config::STRIKE_THR) && (acc < Config::STRIKE_S_THR) &&
      (millis() - strikeTimeout > 500) &&
      state == SaberState::ON && lightOnFlag == 0 && lightOffFlag == 0) {

    strikeTimeout = millis();
    byte number = esp_random() % 10;
    audio.play(clshCache[number]);
    humTimer = millis() - Config::HUM_TIMEOUT + 1000;
    effects.hit();
    strikeFlag = 1;
    Serial.println("Strike detected! Playing sound and lighting effect");
  }
}

void SaberController::handlePulse() {
  if (Config::PULSE_ALLOW && state == SaberState::ON &&
      (millis() - pulseTimer > Config::PULSE_DELAY) &&
      lightOnFlag == 0 && lightOffFlag == 0 && hitFlag == 0) {

    pulseTimer = millis();
    pulseOffset = pulseOffset * k + map((esp_random() % 1024), 0, 1024, -Config::PULSE_AMPL, Config::PULSE_AMPL) * (1 - k);

    if (currentColor == 0) pulseOffset = constrain(pulseOffset, -15, 5);

    uint8_t r = constrain(effects.getCurrentRed() + pulseOffset, 0, 255);
    uint8_t g = constrain(effects.getCurrentGreen() + pulseOffset, 0, 255);
    uint8_t b = constrain(effects.getCurrentBlue() + pulseOffset, 0, 255);
    led.setColor(r, g, b);
  }
}

void SaberController::handleHum() {
  if ((millis() - humTimer) > Config::HUM_TIMEOUT && bzzzFlag) {
    audio.play("hum1.wav");
    humTimer = millis();
    swingFlag = 1;
    strikeFlag = 0;
    Serial.println("Hum sound playing");
  }
}

void SaberController::toggleState() {
  if (state == SaberState::OFF) {
    // Turn ON
    byte number = esp_random() % 2;
    effects.setColorByIndex(currentColor);
    audio.play(inCache[number]);
    humTimer = millis() - Config::HUM_TIMEOUT + 2000;
    effects.startLightOn();
    state = SaberState::ON;
    bzzzFlag = 1;
    Serial.println("Light saber turned ON");
  } else {
    // Turn OFF
    bzzzFlag = 0;
    byte number = esp_random() % 2;
    audio.play(outCache[number]);
    humTimer = millis() - Config::HUM_TIMEOUT + 2000;
    effects.startLightOff();
    state = SaberState::OFF;
    Serial.println("Light saber turned OFF");
  }
}