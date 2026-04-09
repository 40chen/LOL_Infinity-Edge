#pragma once

#include "../drivers/ImuDriver.h"
#include "../drivers/LedDriver.h"
#include "../drivers/SaberAudioDriver.h"
#include "../effects/LightEffects.h"
#include "../../include/config.h"

enum class SaberState {
  OFF,
  ON
};

class SaberController {
public:
  void begin(ImuDriver* imu, LedDriver* led, SaberAudioDriver* audio, LightEffects* effects);
  void update();

  // External interface for Web API
  void setState(bool newState);
  bool getState() const;

private:
  void handleGesture();
  void handleSwing();
  void handleStrike();
  void handlePulse();
  void handleHum();
  void toggleState();
  void onWebToggle(bool state);

  SaberState state = SaberState::OFF;

  // Pointers to shared modules (owned by SystemController)
  ImuDriver* imu = nullptr;
  LedDriver* led = nullptr;
  SaberAudioDriver* audio = nullptr;
  LightEffects* effects = nullptr;

  // State variables
  bool lightOnFlag = false;
  bool lightOffFlag = false;
  bool hitFlag = false;
  bool swingFlag = false;
  bool strikeFlag = false;
  bool bzzzFlag = false;

  // IMU gesture variables
  float rolls[Config::IMU_CAL_COUNTER];
  float pitchs[Config::IMU_CAL_COUNTER];
  int gesCounter = 0;
  int countOpen = 0;
  unsigned long strikeTimeout = 0;

  // Audio caches
  const char* swngs[16] = {"swng1.wav", "swng2.wav", "swng3.wav", "swng4.wav", "swng5.wav", "swng6.wav", "swng7.wav", "swng8.wav",
                           "swng9.wav", "swng10.wav", "swng11.wav", "swng12.wav", "swng13.wav", "swng14.wav", "swng15.wav", "swng16.wav"};
  const char* clshCache[10] = {"clsh1.wav", "clsh2.wav", "clsh3.wav", "clsh4.wav", "clsh5.wav", "clsh6.wav", "clsh7.wav", "clsh8.wav", "clsh9.wav", "clsh10.wav"};
  const char* inCache[2] = {"in1.wav", "in2.wav"};
  const char* outCache[2] = {"out1.wav", "out2.wav"};

  // Color and pulse
  int currentColor = 0;
  float pulseOffset = 0;
  float k = 0.9;

  // Timers
  unsigned long humTimer = 0;
  unsigned long gestureTimer = 0;
  unsigned long btnTimer = 0;
  unsigned long pulseTimer = 0;
  unsigned long swingTimer = 0;
  unsigned long swingTimeout = 0;
};