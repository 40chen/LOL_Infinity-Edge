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

/**
 *  @brief 更新传感器数据
 */
void ImuDriver::update() {
  
  // 控制读取频率
  if (millis() - lastUpdate < 20) return;
  lastUpdate = millis();

  // 读取原始加速度和陀螺仪数据
  mpu.getMotion6(&accel.x, &accel.y, &accel.z, &gyro.x, &gyro.y, &gyro.z);
}

float ImuDriver::getAccelMagnitude() {
  // 和你原代码保持一致的缩放方式
  float x = abs(accel.x / 100.0f);
  float y = abs(accel.y / 100.0f);
  float z = abs(accel.z / 100.0f);

  return sqrt(x * x + y * y + z * z);
}

float ImuDriver::getGyroMagnitude() {
  float x = abs(gyro.x / 100.0f);
  float y = abs(gyro.y / 100.0f);
  float z = abs(gyro.z / 100.0f);

  return sqrt(x * x + y * y + z * z) / 2.0f;
}