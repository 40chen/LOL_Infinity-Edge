#include <Arduino.h>
#include "core/SystemController.h"

// 系统协调器 - 负责所有子系统的初始化和事件循环
Core::SystemController saberSystem;

void setup() {
  saberSystem.begin();
}

void loop() {
  saberSystem.update();
}

