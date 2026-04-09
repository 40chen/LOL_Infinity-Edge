#pragma once

#include <WiFi.h>

class WiFiDriver {
public:
  WiFiDriver();
  void begin();
  void update();
  bool isConnected();

private:
  const char *ssid = "ESP32S3-Saber";
  const char *password = "12345678";
  
  // 辅助函数 - 转换 MAC 地址为字符串
  static String macToString(const uint8_t *mac);
};
