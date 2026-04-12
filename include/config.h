#pragma once

struct Config {
      static constexpr int GESTURE_DELAY = 50; // 手势检测防抖延迟（毫秒）
    static constexpr int HUM_ACTIVATE_DELAY = 2000; // hum激活延迟（毫秒）
  // I2C pins
  static constexpr int I2C_SDA = 1;
  static constexpr int I2C_SCL = 2;
  static constexpr int ES8311_ADDRESS = 0x18;

  // SD card pins
  static constexpr int SD_CLK = 47;
  static constexpr int SD_CMD = 48;
  static constexpr int SD_D0 = 21;
  static constexpr int SD_D1 = -1;
  static constexpr int SD_D2 = -1;
  static constexpr int SD_D3 = -1;

  // LED pins and counts
  static constexpr int LED_PIN1 = 5;
  static constexpr int LED_COUNT1 = 28;
  static constexpr int LED_PIN2 = 4;
  static constexpr int LED_COUNT2 = 28;

  // I2S pins
  static constexpr int I2S_MCK = 38;
  static constexpr int I2S_BCK = 14;
  static constexpr int I2S_WS = 13;
  static constexpr int I2S_DO = 45;

  // Other pins
  static constexpr int PA_ENABLE = 9;

  // Thresholds and timeouts
  static constexpr int SWING_DELAY = 500;
  static constexpr int SWING_DETECTION_THR = 60;
  static constexpr int SWING_L_THR = 80;
  static constexpr int SWING_THR = 180;
  static constexpr int STRIKE_DELAY = 500;
  static constexpr int STRIKE_THR = 40;
  static constexpr int STRIKE_S_THR = 160;
  static constexpr int FLASH_DELAY = 15;
  static constexpr int OPEN_THR = 80;
  static constexpr int GESTURE_ALLOW = 1;
  static constexpr int IMU_CAL_COUNTER = 8;
  static constexpr int HUM_TIMEOUT = 30228;
  static constexpr int PULSE_ALLOW = 1;
  static constexpr int PULSE_AMPL = 10;
  static constexpr int PULSE_DELAY = 30;

  // Audio volume
  static constexpr int AUDIO_VOLUME = 8;
};