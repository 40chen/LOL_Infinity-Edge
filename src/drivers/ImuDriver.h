#pragma once

#include <Arduino.h>

class ImuDriver {
public:
  void begin();
  void update();

  // 获取加速度强度（用于敲击检测）
  float getAccelMagnitude();

  // 获取角速度强度（用于挥动检测）
  float getGyroMagnitude();

  // 原始数据（可选）
  int16_t getAx() const { return ax; }
  int16_t getAy() const { return ay; }
  int16_t getAz() const { return az; }

  int16_t getGx() const { return gx; }
  int16_t getGy() const { return gy; }
  int16_t getGz() const { return gz; }

private:
  int16_t ax = 0, ay = 0, az = 0;
  int16_t gx = 0, gy = 0, gz = 0;

  unsigned long lastUpdate = 0;
};