#pragma once

#include "../drivers/ImuDriver.h"
#include "../drivers/LedDriver.h"
#include "../drivers/SdCardDriver.h"
#include "../drivers/SaberAudioDriver.h"
#include "../effects/LightEffects.h"
#include "../../include/config.h"

enum class SaberState {
  OFF,
  ON
};

class SaberController {
public:
  void begin();
  void update();

private:
  void handleGesture();
  void handleSwing();
  void handleStrike();
  void handlePulse();
  void handleHum();
  void toggleState();

  SaberState state = SaberState::OFF;

  ImuDriver imu;
  SdCardDriver sdCard;
  LedDriver led;
  SaberAudioDriver audio;
  LightEffects effects{led};

  // State variables
  bool lightOnFlag = false;
  bool lightOffFlag = false;
  bool hitFlag = false;
  bool swingFlag = false;
  bool strikeFlag = false;
  bool bzzzFlag = false;

  // Timers
  unsigned long humTimer = 0;
  unsigned long gestureTimer = 0;
  unsigned long btnTimer = 0;
  unsigned long pulseTimer = 0;
  unsigned long swingTimer = 0;
  unsigned long swingTimeout = 0;
  unsigned long strikeTimeout = 0;
  unsigned long batteryTimer = 0;
  unsigned long flashTimer = 0;
  unsigned long hitTimer = 0;

  // Gesture detection
  uint32_t gesCounter = 0;
  float rolls[8] = {0};
  float pitchs[8] = {0};
  int countOpen = 0;

  // Color and effects
  byte currentColor = 1; // Default green
  float k = 0.2f;
  int pulseOffset = 0;

  // Audio file arrays
  const char* const inCache[2] = {"in1.wav", "in2.wav"};
  const char* const outCache[2] = {"out1.wav", "out2.wav"};
  const char* const clshCache[10] = {"clsh1.wav", "clsh2.wav", "clsh3.wav", "clsh4.wav", "clsh5.wav",
                                     "clsh6.wav", "clsh7.wav", "clsh8.wav", "clsh9.wav", "clsh10.wav"};
  const char* const swngs[15] = {"swng1.wav", "swng2.wav", "swng3.wav", "swng4.wav", "swng5.wav",
                                 "swng6.wav", "swng7.wav", "swng8.wav", "swng9.wav", "swng10.wav",
                                 "swng11.wav", "swng12.wav", "swng13.wav", "swng14.wav", "swng15.wav"};
};