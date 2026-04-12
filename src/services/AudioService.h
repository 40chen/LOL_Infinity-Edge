#pragma once

#include "../drivers/AudioDriver.h"
#include "../../include/config.h"

/**
 * Audio Service Layer
 * 职责：
 *  - 管理所有音效缓存
 *  - 播放各种类型的音效（摇晃、敲击、启动、关闭等）
 *  - 音量控制
 *  - 音效队列管理（优先级）
 * 
 * 不直接调用：App层应通过此Service访问音频功能
 */
class AudioService {
public:
  // ===== 初始化 =====
  void begin(SelfAudioDriver* audioPtr);

  // ===== 核心控制 =====
  /**
   * 处理音频循环（应在tick中调用）
   */
  void loop();

  /**
   * 设置音量
   */
  void setVolume(int volume);

  /**
   * 检查当前是否正在播放
   */
  bool isPlaying() const;

  // ===== 挥动音效 =====
  /**
   * 播放快速挥动音效
   */
  void playFastSwing();

  /**
   * 播放缓慢挥动音效
   */
  void playSlowSwing();

  // ===== 敲击音效 =====
  /**
   * 播放敲击/碰撞音效
   */
  void playStrike();

  // ===== 启动/关闭音效 =====
  /**
   * 播放启动音效（打开灯纪元）
   */
  void playTurnOn();

  /**
   * 播放关闭音效
   */
  void playTurnOff();

  // ===== 环境音效 =====
  /**
   * 设置嗡鸣声是否应该在活跃状态
   * 当为true且没有其他音效播放时，会自动持续播放嗡鸣
   */
  void setHumActive(bool active);

  /**
   * 播放启动动画音效
   */
  void playStartup();

private:
  // ===== 音效缓存数组 =====
  const char* slowSwingCache[8] = {
    "swng1.wav", "swng3.wav", "swng5.wav", "swng7.wav",
    "swng9.wav", "swng11.wav", "swng13.wav", "swng15.wav"
  };

  const char* fastSwingCache[8] = {
    "swng2.wav", "swng4.wav", "swng6.wav", "swng8.wav",
    "swng10.wav", "swng12.wav", "swng14.wav", "swng16.wav"
  };

  const char* strikeCache[10] = {
    "clsh1.wav", "clsh2.wav", "clsh3.wav", "clsh4.wav", "clsh5.wav",
    "clsh6.wav", "clsh7.wav", "clsh8.wav", "clsh9.wav", "clsh10.wav"
  };

  const char* inCache[2] = {"in1.wav", "in2.wav"};
  const char* outCache[2] = {"out1.wav", "out2.wav"};

  static constexpr int FAST_SWING_COUNT = 8;
  static constexpr int SLOW_SWING_COUNT = 8;
  static constexpr int STRIKE_COUNT = 10;
  static constexpr int IN_SOUND_COUNT = 2;
  static constexpr int OUT_SOUND_COUNT = 2;

  // ===== 嗡鸣声状态 =====
  bool humActive = false;           // 嗡鸣是否应该激活
  unsigned long humTimer = 0;       // 嗡鸣播放计时器

  // ===== 指针 =====
  SelfAudioDriver* audio = nullptr;

  // ===== 辅助方法 =====
  /**
   * 从缓存数组中随机选择并播放音效
   */
  void playRandomSound(const char** soundCache, int cacheSize);
};
