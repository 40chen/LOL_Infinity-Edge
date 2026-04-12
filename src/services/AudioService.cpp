#include "AudioService.h"

void AudioService::begin(SelfAudioDriver* audioPtr) {
  audio = audioPtr;
  Serial.println("AudioService initialized");
}

void AudioService::loop() {
  if (audio) {
    audio->loop();
  }

  // 嗡鸣声自动管理：在激活状态且没有其他音效播放时，持续播放嗡鸣
  if (humActive && !isPlaying()) {
    if (millis() - humTimer > Config::HUM_TIMEOUT) {
      audio->play("hum1.wav");
      humTimer = millis();
      Serial.println("AudioService: Hum sound auto-playing");
    }
  }
}

void AudioService::setVolume(int volume) {
  if (audio) {
    audio->setVolume(volume);
  }
}

bool AudioService::isPlaying() const {
  if (audio) {
    return audio->isPlaying();
  }
  return false;
}

// ===== 挥动音效实现 =====
void AudioService::playFastSwing() {
  playRandomSound((const char**)fastSwingCache, FAST_SWING_COUNT);
  Serial.println("AudioService: Fast swing sound played");
}

void AudioService::playSlowSwing() {
  playRandomSound((const char**)slowSwingCache, SLOW_SWING_COUNT);
  Serial.println("AudioService: Slow swing sound played");
}

// ===== 敲击音效实现 =====
void AudioService::playStrike() {
  playRandomSound((const char**)strikeCache, STRIKE_COUNT);
  Serial.println("AudioService: Strike sound played");
}

// ===== 启动/关闭音效实现 =====
void AudioService::playTurnOn() {
  playRandomSound((const char**)inCache, IN_SOUND_COUNT);
  Serial.println("AudioService: Turn on sound played");
}

void AudioService::playTurnOff() {
  playRandomSound((const char**)outCache, OUT_SOUND_COUNT);
  Serial.println("AudioService: Turn off sound played");
}

void AudioService::playStartup() {
  if (audio) {
    audio->play("saber.flac");
    Serial.println("AudioService: Startup sound played");
  }
}

void AudioService::setHumActive(bool active) {
  humActive = active;
  if (!active) {
    // 停用hum时重置计时器
    humTimer = 0;
  } else {
    // 激活hum时，延迟启动（给足时间让其他音效先播放）
    humTimer = millis() - Config::HUM_TIMEOUT + Config::HUM_ACTIVATE_DELAY;
  }
  Serial.print("AudioService: Hum ");
  Serial.println(active ? "activated" : "deactivated");
}

// ===== 辅助方法实现 =====
void AudioService::playRandomSound(const char** soundCache, int cacheSize) {
  if (!audio || cacheSize <= 0) {
    return;
  }
  
  int randomIndex = esp_random() % cacheSize;
  audio->play(soundCache[randomIndex]);
}
