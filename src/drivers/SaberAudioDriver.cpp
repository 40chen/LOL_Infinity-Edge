#include "SaberAudioDriver.h"
#include "../../include/config.h"
#include <Arduino.h>
#include "SD_MMC.h"

SaberAudioDriver::SaberAudioDriver() : board(nullptr) {}

void SaberAudioDriver::begin() {
  Serial.println("[Audio] Initializing...");

  // Configure I2C pins for codec
  pins.addI2C(PinFunction::CODEC, Config::I2C_SCL, Config::I2C_SDA, Config::ES8311_ADDRESS);

  // Codec configuration
  CodecConfig cfg;
  cfg.input_device = ADC_INPUT_ALL;
  cfg.output_device = DAC_OUTPUT_ALL;
  cfg.i2s.bits = BIT_LENGTH_16BITS;
  cfg.i2s.rate = RATE_44K;
  cfg.i2s.fmt = I2S_NORMAL;

  if (board != nullptr) {
    delete board;
  }

  board = new AudioBoard(AudioDriverES8311, pins);
  if (!board->begin(cfg)) {
    Serial.println("[Audio] AudioBoard begin failed");
  } else {
    Serial.println("[Audio] AudioBoard begin successful");
  }

  audio.setPinout(Config::I2S_BCK, Config::I2S_WS, Config::I2S_DO, -1, Config::I2S_MCK);
  audio.setVolume(Config::AUDIO_VOLUME);

  // Delay for codec stabilization
  delay(200);
  
  // Enable power amplifier after codec is stable
  pinMode(Config::PA_ENABLE, OUTPUT); 
  digitalWrite(Config::PA_ENABLE, HIGH);

  delay(100);

  Serial.println("[Audio] Initialization complete");
}

void SaberAudioDriver::play(const char* file) {
  Serial.print("[Audio] Playing: ");
  Serial.println(file);
  audio.connecttoFS(SD_MMC, file);
}

void SaberAudioDriver::loop() {
  audio.loop();
}

void SaberAudioDriver::setVolume(int volume) {
  audio.setVolume(volume);
}