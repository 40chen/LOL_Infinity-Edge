#pragma once

#include "../services/ImuService.h"
#include "../services/LedService.h"
#include "../services/AudioService.h"
#include "../../include/config.h"

enum class SaberState {
  OFF,
  STARTING,
  STOPPING,
  ON
};

class SaberController {
public:
  void begin(ImuService* imuService, LedService* ledService, AudioService* audioService);
  void tick();

  // External interface for Web API
  void setState(bool newState);
  bool getState() const;

private:
  void handleGesture();
  void handleSwing();
  void handleStrike();
  void toggleState();
  void onWebToggle(bool state);

  void playSwingSound(float gyr);
  void playStrikeSound(float acc);

  SaberState state = SaberState::OFF;

  // 指向服务层的指针（由SystemController管理）
  ImuService* imuService = nullptr;
  LedService* ledService = nullptr;
  AudioService* audioService = nullptr;

  // Timers
  unsigned long btnTimer = 0;

};