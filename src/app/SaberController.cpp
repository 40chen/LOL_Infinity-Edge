#include "SaberController.h"
#include "../../include/config.h"
#include <Arduino.h>

void SaberController::begin(ImuService* imuServicePtr, LedService* ledServicePtr, 
                           AudioService* audioServicePtr) {
  // 存储指向服务层的引用
  imuService = imuServicePtr;
  ledService = ledServicePtr;
  audioService = audioServicePtr;

  Serial.println("Initializing Light Saber Logic...");

  // 初始化颜色和效果
  if (ledService) {
    ledService->setColor(ColorMode::RED);  // 红色初始化
  }

  // 播放启动音
  if (audioService) {
    audioService->playStartup();
  }

  Serial.println("Light Saber Logic initialized");
}

void SaberController::tick() {
  if (!imuService || !ledService || !audioService) {
    return;  // Not properly initialized
  }

  // 更新服务层
  imuService->update();
  audioService->loop();
  ledService->update();

  switch (state)
  {
  case SaberState::STARTING:
    if (!audioService->isPlaying()) {
      state = SaberState::ON;
      audioService->setHumActive(true);  // 启动完成后激活hum
      Serial.println("Saber state changed to ON, hum activated");
    }
    return;
  case SaberState::STOPPING:
    if (!audioService->isPlaying()) {
      state = SaberState::OFF;
      audioService->setHumActive(false);  // 确保hum已关闭
      Serial.println("Saber state changed to OFF");
    }
    return;
  case SaberState::OFF:
    handleGesture(); // Still check for gesture to allow turning on
    return;
  case SaberState::ON:
    handleGesture();
    handleSwing();
    handleStrike();
    return;
  default:
    break;
  }
}

void SaberController::setState(bool newState) {
  if (newState && state == SaberState::OFF) {
    toggleState();
  } else if (!newState && state == SaberState::ON) {
    toggleState();
  }
}

bool SaberController::getState() const {
  return state == SaberState::ON;
}

void SaberController::handleGesture() {
  if (!imuService) {
    return;
  }

  // 使用ImuService检测手势
  if (imuService->detectGesture()) {
    toggleState();
    Serial.println("Gesture detected - triggering ON/OFF sequence");
  }
}

void SaberController::playSwingSound(float gyr) {
  if (!audioService) {
    return;
  }

  if (gyr >= Config::SWING_THR) {
    audioService->playFastSwing();
  } else if (gyr < Config::SWING_THR && gyr > Config::SWING_L_THR) {
    audioService->playSlowSwing();
  } 
}

void SaberController::handleSwing() { 
  if (!imuService || !ledService || state != SaberState::ON) {
    return; 
  }

  // TODO: 考虑优先级队列来处理音效冲突
  if (audioService->isPlaying()) {
    return; // 如果音乐仍在播放，不触发挥动音效
  }

  // 使用ImuService检测挥动
  if (imuService->detectSwing()) {
    float swingIntensity = imuService->getSwingIntensity();
    playSwingSound(swingIntensity);
    ledService->playSwingAnimation();
  }
}

void SaberController::handleStrike() {
  if (!imuService || !ledService || state != SaberState::ON) {
    return; 
  }

  // 使用ImuService检测敲击
  if (imuService->detectStrike()) {
    float strikeIntensity = imuService->getStrikeIntensity();
    playStrikeSound(strikeIntensity);
    ledService->playStrikeAnimation();
    Serial.println("Strike detected! Playing sound and lighting effect");
  }
}

void SaberController::playStrikeSound(float acc) {
  if (!audioService) {
    return;
  }
  audioService->playStrike();
}

void SaberController::toggleState() {
  if (!ledService || !audioService) {
    return;
  }

  if (state == SaberState::OFF) {
    // 开始打开灯光
    ledService->setColor(ColorMode::RED);  // 红色
    audioService->playTurnOn();
    ledService->playTurnOnAnimation();
    audioService->setHumActive(false);  // 暂时禁用，等待启动完成
    state = SaberState::STARTING;
    Serial.println("Light saber starting: state set to STARTING");
  } else if (state == SaberState::ON) {
    // 开始关闭灯光
    audioService->setHumActive(false);  // 立即停止嗡鸣
    audioService->playTurnOff();
    ledService->playTurnOffAnimation();
    state = SaberState::STOPPING;
    Serial.println("Light saber stopping: state set to STOPPING");
  }
}