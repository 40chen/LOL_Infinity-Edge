
#include "LedService.h"
#include <Arduino.h>
#include "LedService.h"
#include <Arduino.h>

void LedService::begin(LedDriver* ledPtr) {
  led = ledPtr;
  currentRed = 255;  // 初始颜色：红色
  currentGreen = 0;
  currentBlue = 0;
  Serial.println("LedService initialized");
}

// ===== 核心更新 =====
void LedService::update() {
  if (!led) return;
  unsigned long currentTime = millis();

  if (lightOnActive) {
    handleTurnOnAnimation(currentTime);
    return;
  }
  if (lightOffActive) {
    handleTurnOffAnimation(currentTime);
    return;
  }
  if (strikeAnimationActive) {
    handleStrikeAnimation(currentTime);
    return;
  }
  if (swingAnimationActive) {
    handleSwingAnimation(currentTime);
    return;
  }
}

// === 动画处理私有方法 ===
void LedService::handleTurnOnAnimation(unsigned long currentTime) {
  // 每隔 FLASH_DELAY 毫秒推进一帧点亮动画
  if (currentTime - animationTimer <= Config::FLASH_DELAY) return;
  animationTimer = currentTime;

  // 计算对称像素索引
  int leftIdx = animationCounter;
  int rightIdx = Config::LED_COUNT1 - 1 - animationCounter;

  // 点亮主灯条两端对称像素
  led->setPixel(1, leftIdx, currentRed, currentGreen, currentBlue);
  led->setPixel(1, rightIdx, currentRed, currentGreen, currentBlue);
  // 点亮副灯条两端对称像素
  led->setPixel(2, leftIdx, currentRed, currentGreen, currentBlue);
  led->setPixel(2, rightIdx, currentRed, currentGreen, currentBlue);
  led->show();

  animationCounter++;

  // 动画结束条件：推进到中点
  if (animationCounter >= (Config::LED_COUNT1 / 2)) {
    resetTurnOnAnimationState();
  }
}

void LedService::resetTurnOnAnimationState() {
  animationCounter = 0;
  lightOnActive = false;
  Serial.println("LedService: Light-on animation completed");
}

void LedService::handleTurnOffAnimation(unsigned long currentTime) {
  // 每隔 FLASH_DELAY 毫秒推进一帧熄灭动画
  if (currentTime - animationTimer <= Config::FLASH_DELAY) return;
  animationTimer = currentTime;

  // 计算对称像素索引
  int leftIdx = animationCounter;
  int rightIdx = Config::LED_COUNT1 - 1 - animationCounter;

  // 熄灭主灯条两端对称像素
  led->setPixel(1, leftIdx, 0, 0, 0);
  led->setPixel(1, rightIdx, 0, 0, 0);
  // 熄灭副灯条两端对称像素
  led->setPixel(2, leftIdx, 0, 0, 0);
  led->setPixel(2, Config::LED_COUNT2 - 1 - animationCounter, 0, 0, 0);
  led->show();

  // 推进动画计数器
  if (animationCounter == 0) {
    resetTurnOffAnimationState();
  } else {
    animationCounter--;
  }
}

void LedService::resetTurnOffAnimationState() {
  lightOffActive = false;
  Serial.println("LedService: Light-off animation completed, pulse disabled");
}

void LedService::handleStrikeAnimation(unsigned long currentTime) {
  if (currentTime - animationTimer > 300) {
    animationTimer = currentTime;
    led->setColor(currentRed, currentGreen, currentBlue);
    resetStrikeAnimationState();
  }
}

void LedService::resetStrikeAnimationState() {
  strikeAnimationActive = false;
  Serial.println("LedService: Strike animation ended, pulse restored");
}

// 挥动动画处理
void LedService::handleSwingAnimation(unsigned long currentTime) {
  // 挥动动画持续 300ms，结束后恢复原色
  if (currentTime - animationTimer > 300) {
    animationTimer = currentTime;
    led->setColor(currentRed, currentGreen, currentBlue);
    resetSwingAnimationState();
  }
}

void LedService::resetSwingAnimationState() {
  swingAnimationActive = false;
  Serial.println("LedService: Swing animation ended");
}

void LedService::handlePulse(unsigned long currentTime) {
  if (Config::PULSE_ALLOW && !lightOnActive && !lightOffActive && !strikeAnimationActive && !swingAnimationActive) {
    if (currentTime - pulseTimer >= Config::PULSE_DELAY) {
      pulseTimer = currentTime;
      applyPulseEffect();
    }
  }
}

// ===== 颜色管理 =====
void LedService::setColor(ColorMode colorMode) {
  // 存储当前颜色预设
  currentColorMode = colorMode;
  
  // 从颜色库获取预设
  const int index = static_cast<int>(colorMode);
  const ColorPreset& preset = colorLibrary[index];
  
  // 设置RGB值
  setColor(preset.rgb.r, preset.rgb.g, preset.rgb.b);
  
  // 自动启用脉冲效果
  pulseTimer = millis();
  
  // 输出日志
  Serial.print("LedService: Color set to ");
  Serial.println(preset.name);
  Serial.println("LedService: Pulse enabled");
}

void LedService::setColor(uint8_t r, uint8_t g, uint8_t b) {
  if (!led) {
    return;
  }
  
  currentRed = r;
  currentGreen = g;
  currentBlue = b;
  led->setColor(r, g, b);
  
  // 自动启用脉冲效果
  pulseTimer = millis();
  
  Serial.print("LedService: Color set to RGB(");
  Serial.print(r); Serial.print(", ");
  Serial.print(g); Serial.print(", ");
  Serial.print(b); Serial.println(")");
  Serial.println("LedService: Pulse enabled");
}

RGB LedService::getCurrentColor() const {
  return {currentRed, currentGreen, currentBlue};
}

// ===== 脉冲效果 =====

void LedService::applyPulseEffect() {
  if (!led) {
    return;
  }

  // 生成随机偏移
  const int randomValue = esp_random() % 1024;
  const float randomOffset = map(randomValue, 0, 1023, -Config::PULSE_AMPL, Config::PULSE_AMPL);

  // 使用平滑系数更新脉冲偏移
  pulseOffset = pulseOffset * smoothingFactor + randomOffset * (1.0f - smoothingFactor);

  // 对蓝色灯芯进行约束
  if (currentColorMode == ColorMode::BLUE) {
    pulseOffset = constrain(pulseOffset, -15.0f, 5.0f);
  }

  // 获取当前颜色RGB值
  const RGB currentRGB = getCurrentColor();

  // 应用脉冲偏移
  const uint8_t r = constrain(currentRGB.r + pulseOffset, 0, 255);
  const uint8_t g = constrain(currentRGB.g + pulseOffset, 0, 255);
  const uint8_t b = constrain(currentRGB.b + pulseOffset, 0, 255);

  led->setColor(r, g, b);
  Serial.println("LedService: Pulse color updated");
}


void LedService::playTurnOnAnimation() {
  lightOnActive = true;
  animationCounter = 0;
  animationTimer = millis();
  Serial.println("LedService: Turn on animation started");
}

void LedService::playTurnOffAnimation() {
  lightOffActive = true;
  animationCounter = Config::LED_COUNT1 / 2 - 1;
  animationTimer = millis();
  Serial.println("LedService: Turn off animation started");
}

void LedService::playStrikeAnimation() {
  if (!led) return;
  
  // FIXME: 这里的黄色和ColorMode中的黄色不一致
  led->setColor(255, 201, 0); 
  animationTimer = millis();
  strikeAnimationActive = true;
  Serial.println("LedService: Strike animation played");
}

  void LedService::playSwingAnimation() {
    if (!led) return;

    // TODO: 需要定义 swing 的动画效果是什么
    led->setColor(255, 255, 255); 
    animationTimer = millis();
    swingAnimationActive = true;
    Serial.println("LedService: Swing animation played");
  }
