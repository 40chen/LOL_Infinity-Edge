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
  int16_t getAx() const { return accel.x; }
  int16_t getAy() const { return accel.y; }
  int16_t getAz() const { return accel.z; }

  int16_t getGx() const { return gyro.x; }
  int16_t getGy() const { return gyro.y; }
  int16_t getGz() const { return gyro.z; }

  struct Vector3 {
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
  };

  const Vector3& getAccel() const { return accel; }
  const Vector3& getGyro() const { return gyro; }

private:

  // 存储加速度数据的结构体
  Vector3 accel;

  // 存储陀螺仪数据的结构体
  Vector3 gyro;

  // 上次更新的时间戳（用于控制读取频率）
  unsigned long lastUpdate = 0;
};