#include "SdCardDriver.h"
#include "../../include/config.h"
#include <Arduino.h>
#include "SD_MMC.h"

bool SdCardDriver::begin() {

    // 设置 SD 卡引脚
    // SD_CLK: 时钟线
    // SD_CMD: 控制线
    // SD_D0 ~ SD_D3: 数据线, -1 表示 1bit 模式, 仅使用一个引脚, 其他三个引脚未使用
    SD_MMC.setPins(Config::SD_CLK, Config::SD_CMD, Config::SD_D0, Config::SD_D1, Config::SD_D2, Config::SD_D3);

    // 初始化 SD 卡
    // mount_point: "/sdcard", mode1bit = true, format_if_fail = false
    if (!SD_MMC.begin("/sdcard", true, false)) {
      Serial.println("[SD] initialization FAILED");
      return false;
    }

    Serial.println("[SD] initialization SUCCESS");
    return true;
}