#include "WiFiDriver.h"
#include <Arduino.h>

WiFiDriver::WiFiDriver() {}

void WiFiDriver::begin() {
  Serial.println("Initializing WiFi Driver...");

  // Create WiFi AP
  Serial.println("Configuring access point...");

  // 设置 Wi-Fi 事件回调函数（使用 lambda 捕获 this）
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.print("Device connected, MAC: ");
    Serial.println(macToString(info.wifi_ap_staconnected.mac));
  }, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STACONNECTED);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.print("Device IP address: ");
    Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
  }, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.print("Device disconnected, MAC: ");
    Serial.println(macToString(info.wifi_ap_stadisconnected.mac));
  }, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);


  // Ensure AP mode is enabled before starting the AP
  WiFi.enableAP(true); 
  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Soft AP creation failed.");
    return;
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  Serial.println("WiFi Driver ready");
}

void WiFiDriver::update() {
  // WiFi AP maintains connection automatically
  // No periodic update needed
}

bool WiFiDriver::isConnected() {
  return WiFi.softAPgetStationNum() > 0;
}

// MAC 地址转换为字符串辅助函数
String WiFiDriver::macToString(const uint8_t *mac) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}
