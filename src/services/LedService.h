#pragma once

#include "../drivers/LedDriver.h"
#include "../../include/config.h"

struct RGB {
  uint8_t r, g, b;
};

/**
 * LED Service Layer
 * 职责：
 *  - 管理所有 LED 控制（驱动 + 效果）
 *  - 处理颜色和脉冲效果
 *  - 处理灯光动画（开启、关闭、敲击）
 *  - 提供高级 LED 操作接口
 * 
 * 不直接调用：App层应通过此Service访问LED功能
 */

// ===== 颜色预设枚举（强类型，直接关联RGB值）=====
enum class ColorMode {
  RED = 0,       // 红色 (255, 0, 0)
  GREEN = 1,     // 绿色 (0, 255, 0)
  BLUE = 2,      // 蓝色 (0, 0, 255)
  PINK = 3,      // 粉红 (255, 0, 255)
  YELLOW = 4,    // 黄色 (255, 255, 0)
  ICE_BLUE = 5   // 冰蓝 (0, 255, 255)
};

// ===== 颜色预设结构体（将枚举和RGB密切关联）=====
struct ColorPreset {
  ColorMode mode;
  const char* name;
  RGB rgb;
  
  // 便捷构造函数
  constexpr ColorPreset(ColorMode m, const char* n, uint8_t r, uint8_t g, uint8_t b)
    : mode(m), name(n), rgb{r, g, b} {}
};

// ===== 扩展颜色库（将所有颜色预设集中定义）=====
// 每个颜色预设包含：枚举值、名称、RGB值
// 示例使用: colorLibrary[static_cast<int>(ColorMode::RED)]
static constexpr ColorPreset colorLibrary[] = {
  ColorPreset(ColorMode::RED,      "RED",      255,   0,   0),
  ColorPreset(ColorMode::GREEN,    "GREEN",      0, 255,   0),
  ColorPreset(ColorMode::BLUE,     "BLUE",       0,   0, 255),
  ColorPreset(ColorMode::PINK,     "PINK",     255,   0, 255),
  ColorPreset(ColorMode::YELLOW,   "YELLOW",   255, 255,   0),
  ColorPreset(ColorMode::ICE_BLUE, "ICE_BLUE",   0, 255, 255)
};

class LedService {
  void resetTurnOffAnimationState();
  void resetStrikeAnimationState();
  void resetTurnOnAnimationState();
  void resetSwingAnimationState();
  void handleTurnOnAnimation(unsigned long currentTime);
  void handleTurnOffAnimation(unsigned long currentTime);
  void handleStrikeAnimation(unsigned long currentTime);
  void handleSwingAnimation(unsigned long currentTime);
  void handlePulse(unsigned long currentTime);
public:
    /**
     * 播放挥动动画
     */
    void playSwingAnimation();
  void begin(LedDriver* ledPtr);

  // ===== 核心循环 =====
  /**
   * 更新所有活跃效果和动画（应在tick中调用）
   */
  void update();

  // ===== 颜色管理 =====
  /**
   * 设置预设颜色（推荐使用，代码可读性高）
   * 示例: setColor(ColorMode::RED)
   */
  void setColor(ColorMode colorMode);

  /**
   * 直接设置自定义RGB颜色
   * 示例: setColor(255, 128, 0)
   */
  void setColor(uint8_t r, uint8_t g, uint8_t b);

  /**
   * 获取当前颜色的RGB值
   */
  RGB getCurrentColor() const;

  /**
   * 获取当前设置的颜色预设
   */
  ColorMode getColor() const { return currentColorMode; }

  // ===== 脉冲效果 =====
  /**
   * 更新脉冲颜色
   * 根据当前颜色应用脉冲偏移
   */
  void applyPulseEffect();


  // ===== 动画效果 =====
  /**
   * 播放转向开启动画
   */
  void playTurnOnAnimation();

  /**
   * 播放转向关闭动画
   */
  void playTurnOffAnimation();

  /**
   * 播放敲击/碰撞闪烁效果
   */
  void playStrikeAnimation();

private:
  // ===== 效果状态管理 =====
  /**
   * 生成随机脉冲偏移值
   */

  // ===== 脉冲参数 =====
  float pulseOffset = 0.0f;
  float smoothingFactor = 0.9f;  // 平滑系数k
  unsigned long pulseTimer = 0;

  // ===== 动画状态 =====
  unsigned long animationTimer = 0;
  bool lightOnActive = false;
  bool lightOffActive = false;
  bool strikeAnimationActive = false;
  bool swingAnimationActive = false;
  uint8_t animationCounter = 0;

  // ===== 颜色管理 =====
  ColorMode currentColorMode = ColorMode::RED;
  uint8_t currentRed = 255;
  uint8_t currentGreen = 0;
  uint8_t currentBlue = 0;

  // ===== 指针 =====
  LedDriver* led = nullptr;
};

