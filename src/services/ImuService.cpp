#include "ImuService.h"
#include <math.h>

static constexpr float kRadToDeg = 57.2957795f;

void ImuService::begin(ImuDriver* imuPtr) {
  imu = imuPtr;
  Serial.println("ImuService initialized");
}

void ImuService::update() {
  if (!imu) {
    return;
  }
  
  imu->update();
  
  // 缓存最新的测量値
  lastAccelMagnitude = imu->getAccelMagnitude();
  lastGyroMagnitude = imu->getGyroMagnitude();
}

// ===== 手势检测实现 =====
float ImuService::rawAccelToG(int16_t raw) {
  return static_cast<float>(raw) / 2048.0f;
}

void ImuService::storeGestureSample(float roll, float pitch) {
  const int index = gesCounter & (Config::IMU_CAL_COUNTER - 1);
  rolls[index] = roll;
  pitchs[index] = pitch;
  gesCounter++;
}

bool ImuService::isGestureDetected() const {
  float rollMin = rolls[0];
  float rollMax = rolls[0];
  float pitchMin = pitchs[0];
  float pitchMax = pitchs[0];

  for (int i = 1; i < Config::IMU_CAL_COUNTER; i++) {
    const float roll = rolls[i];
    const float pitch = pitchs[i];
    rollMin = roll < rollMin ? roll : rollMin;
    rollMax = roll > rollMax ? roll : rollMax;
    pitchMin = pitch < pitchMin ? pitch : pitchMin;
    pitchMax = pitch > pitchMax ? pitch : pitchMax;
  }

  return (pitchMax - pitchMin) > Config::OPEN_THR;
}

bool ImuService::detectGesture() {
  if (!imu || Config::GESTURE_ALLOW == 0) {
    return false;
  }
  
  if (millis() - gestureTimer < Config::GESTURE_DELAY) {
    return false; // 限制处理频率
  }

  gestureTimer = millis();

  const auto& accel = imu->getAccel();
  const float accX = rawAccelToG(accel.x);
  const float accY = rawAccelToG(accel.y);
  const float accZ = rawAccelToG(accel.z);

  // 计算Roll和Pitch角度（单位：度）
  const float roll = atan2(accY, accZ) * kRadToDeg;
  const float pitch = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * kRadToDeg;

  storeGestureSample(roll, pitch);

  // 检测手势并带防抖动
  if (gestureDebounceCounter == 0 && isGestureDetected()) {
    gestureDebounceCounter = 40; // 防抖延迟40个tick
    Serial.println("ImuService: Gesture detected");
    return true;
  }

  if (gestureDebounceCounter > 0) {
    gestureDebounceCounter--;
  }

  return false;
}


// ===== 挥动检测实现 =====
bool ImuService::isSwingDetected(float gyr) const {
  return gyr > Config::SWING_DETECTION_THR;
}

bool ImuService::detectSwing() {
  if (!imu) {
    return false;
  }

  if (millis() - swingTimer < Config::SWING_DELAY) {
    return false;
  }

  float gyr = imu->getGyroMagnitude();
  if (!isSwingDetected(gyr)) {
    return false;
  }

  swingTimer = millis();
  Serial.println("ImuService: Swing detected");
  return true;
}

// ===== 敲击检测实现 =====
bool ImuService::isStrikeDetected(float acc) const {
  return acc >= Config::STRIKE_THR && acc <= Config::STRIKE_S_THR;
}

bool ImuService::detectStrike() {
  if (!imu) {
    return false;
  }

  if (millis() - strikeTimer < Config::STRIKE_DELAY) {
    return false;
  }

  float acc = imu->getAccelMagnitude();
  if (!isStrikeDetected(acc)) {
    return false;
  }

  strikeTimer = millis();
  Serial.println("ImuService: Strike detected");
  return true;
}

const ImuDriver::Vector3& ImuService::getAccel() const {
  if (!imu) {
    static const ImuDriver::Vector3 defaultVec{};
    return defaultVec;
  }
  return imu->getAccel();
}

const ImuDriver::Vector3& ImuService::getGyro() const {
  if (!imu) {
    static const ImuDriver::Vector3 defaultVec{};
    return defaultVec;
  }
  return imu->getGyro();
}
