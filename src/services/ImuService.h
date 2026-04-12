#pragma once

#include "../drivers/ImuDriver.h"
#include "../../include/config.h"

/**
 * IMU Service Layer
 * 职责：
 *  - 处理IMU原始数据
 *  - 检测手势、挥动等事件
 *  - 提供上层应用需要的数据接口
 * 
 * 不直接调用：App层不应直接使用ImuDriver
 */
class ImuService {
public:
  void begin(ImuDriver* imuPtr);
  void update();

  // ===== 手势检测相关 =====
  /**
   * 检测打开/关闭手势（通过设备姿态变化）
   * @return true 如果检测到手势
   */
  bool detectGesture();

  // ===== 挥动检测相关 =====
  /**
   * 检测是否发生了挥动
   * @return true 如果检测到挥动
   */
  bool detectSwing();

  /**
   * 获取挥动的强度 (角速度)
   * @return 角速度大小，用于区分快速/缓慢挥动
   */
  float getSwingIntensity() const { return lastGyroMagnitude; }

  // ===== 敲击检测相关 =====
  /**
   * 检测是否发生了敲击
   * @return true 如果检测到敲击
   */
  bool detectStrike();

  /**
   * 获取敲击的强度 (加速度)
   * @return 加速度大小
   */
  float getStrikeIntensity() const { return lastAccelMagnitude; }

  // ===== 内部状态控制 =====
  /**
   * 重设手势检测计数器（在成功检测手势后调用）
   */

  /**
   * 获取原始加速度数据
   */
  const ImuDriver::Vector3& getAccel() const;

  /**
   * 获取原始陀螺仪数据
   */
  const ImuDriver::Vector3& getGyro() const;

private:
  // ===== 手势检测相关 =====
  /**
   * 存储单个采样数据
   */
  void storeGestureSample(float roll, float pitch);

  /**
   * 检查存储的样本中是否有足够的姿态变化
   */
  bool isGestureDetected() const;

  /**
   * 将原始加速度值转换为G（重力加速度单位）
   */
  static float rawAccelToG(int16_t raw);

  // ===== 挥动检测相关 =====
  /**
   * 判断角速度是否超过挥动检测阈值
   */
  bool isSwingDetected(float gyr) const;

  // ===== 敲击检测相关 =====
  /**
   * 判断加速度是否在有效的敲击范围内
   */
  bool isStrikeDetected(float acc) const;

  // ===== 成员变量 =====
  ImuDriver* imu = nullptr;

  // 手势检测相关
  float rolls[Config::IMU_CAL_COUNTER];
  float pitchs[Config::IMU_CAL_COUNTER];
  int gesCounter = 0;
  int gestureDebounceCounter = 0;

  // 计时器
  unsigned long gestureTimer = 0;
  unsigned long swingTimer = 0;
  unsigned long strikeTimer = 0;

  // 缓存最后一次测量的数据
  float lastAccelMagnitude = 0.0f;
  float lastGyroMagnitude = 0.0f;
};
