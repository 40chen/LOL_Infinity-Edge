#include "ImuDriver.h"
#include "../../include/config.h"

#include <Wire.h>
#include <MPU6050.h>
#include <math.h>

// 静态 MPU6050 实例（整个模块唯一）
static MPU6050 mpu;

void ImuDriver::begin() {
  mpu.initialize();

  // 和你原代码一致的量程配置
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);

  if (mpu.testConnection()) {
    Serial.println("[IMU] MPU6050 connected");
  } else {
    Serial.println("[IMU] MPU6050 connection FAILED");
  }
}

void ImuDriver::update() {
  // 控制读取频率 ≈ 50Hz（和你原来一致）
  if (millis() - lastUpdate < 20) return;
  lastUpdate = millis();

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
}

float ImuDriver::getAccelMagnitude() {
  // 和你原代码保持一致的缩放方式
  float x = abs(ax / 100.0f);
  float y = abs(ay / 100.0f);
  float z = abs(az / 100.0f);

  return sqrt(x * x + y * y + z * z);
}

float ImuDriver::getGyroMagnitude() {
  float x = abs(gx / 100.0f);
  float y = abs(gy / 100.0f);
  float z = abs(gz / 100.0f);

  return sqrt(x * x + y * y + z * z) / 2.0f;
}