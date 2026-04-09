#pragma once

#include "../drivers/WiFiDriver.h"
#include "../drivers/ImuDriver.h"
#include "../drivers/LedDriver.h"
#include "../drivers/SdCardDriver.h"
#include "../drivers/SaberAudioDriver.h"
#include "../app/SaberController.h"
#include "../api/WebAPIController.h"
#include "../effects/LightEffects.h"
#include "../../include/config.h"
#include <WebServer.h>

namespace Core {

// 核心系统协调器 - 管理所有系统组件和初始化流程
// 作用：
//   - 聚合所有子系统（驱动、应用、API）
//   - 协调初始化顺序
//   - 管理主事件循环
class SystemController {
public:
  void begin();
  void update();

private:
  // 驱动层 - 硬件抽象
  WiFiDriver wifi;
  ImuDriver imu;
  LedDriver led;
  SdCardDriver sdCard;
  SaberAudioDriver audio;

  // 效果层 - 视觉效果管理
  LightEffects effects{led};

  // 应用层 - 业务逻辑
  SaberController saber;

  // API层 - 外部接口
  WebServer server{80};
  WebAPIController webAPI;
};

}  // namespace Core
