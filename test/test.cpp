#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <MPU6050.h>
#include "SD_MMC.h"
#include "Audio.h"
#include "AudioBoard.h"

// 硬件定义
#define LED_PIN1  5
#define LED_COUNT1  30
#define SD_D0  21
#define SD_CMD 48
#define SD_CLK 47
#define I2S_MCK 38
#define I2S_BCK 14
#define I2S_WS  13
#define I2S_DO  45
#define I2C_SDA 1
#define I2C_SCL 2
#define ES8311_ADDRESS 0x18
#define PA_ENABLE 9

// 参数定义
#define SWING_TIMEOUT 500   // 摆动刷新时间
#define SWING_L_THR 80      // 摆动阈值
#define SWING_THR 180       // 快速摆动阈值
#define STRIKE_THR 40       // 敲击阈值
#define STRIKE_S_THR 160    // 硬敲击阈值
#define FLASH_DELAY 15
#define OPEN_THR 80         // 开机阈值
#define GESTURE_ALLOW 1
#define IMU_CAL_COUNTER (1<<3)
#define HUM_TIMEOUT 30228
#define PULSE_ALLOW 1
#define PULSE_AMPL 10
#define PULSE_DELAY 30

// 全局变量
Adafruit_NeoPixel strip(LED_COUNT1, LED_PIN1, NEO_GRB + NEO_KHZ800);
Audio audio;
DriverPins my_pins;
AudioBoard board(AudioDriverES8311, my_pins);

// MPU6050相关
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long ACC, GYR;
int gyroX, gyroY, gyroZ, accelX, accelY, accelZ;

// 状态标志
boolean lighton_flag = 0, lightoff_flag = 0, hit_flag = 0;
boolean ls_chg_state = 0, ls_state = 0;
boolean bzzz_flag = 0;
boolean swing_flag = 0, swing_allow = 1, strike_flag = 0;
unsigned long humTimer = 0, mpuTimer = 0, gestureTimer = 0;
unsigned long btn_timer = 0, PULSE_timer = 0, swing_timer = 0, swing_timeout = 0;
unsigned long strike_timeout = 0, battery_timer = 0, FLASH_timer = 0, hit_timer = 0;

// 计数器和缓存
uint32_t ges_counter = 0;
float rolls[IMU_CAL_COUNTER] = {0};
float pitchs[IMU_CAL_COUNTER] = {0};

// 其他变量
float k = 0.2;
byte red = 0, green = 0, blue = 0, redOffset = 0, greenOffset = 0, blueOffset = 0;
int PULSEOffset = 0;
byte nowNumber = 0, nowColor = 1;
byte light_counter = 0;

// 音效文件列表
const char* const in_cache[] = {"in1.wav", "in2.wav"};
const char* const out_cache[] = {"out1.wav", "out2.wav"};
const char* const clsh_cache[] = {"clsh1.wav", "clsh2.wav", "clsh3.wav", "clsh4.wav", "clsh5.wav", 
                                  "clsh6.wav", "clsh7.wav", "clsh8.wav", "clsh9.wav", "clsh10.wav"};
const char* const swngs[] = {"swng1.wav", "swng2.wav", "swng3.wav", "swng4.wav", "swng5.wav",
                             "swng6.wav", "swng7.wav", "swng8.wav", "swng9.wav", "swng10.wav",
                             "swng11.wav", "swng12.wav", "swng13.wav", "swng14.wav", "swng15.wav"};



void on_off_sound();
void gestureTick();
void strikeTick();
void swingTick();
void randomPULSE();
void lightTick();
void create_lighton_action();
void create_lightoff_action();
void lightTick();
void setAll(byte red, byte green, byte blue);
void setColor(byte color);
void getMpu6050();
void create_hit_action();


void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Initializing Light Saber System...");

  Wire.begin(I2C_SDA, I2C_SCL);

  // 配置SD卡引脚
  SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, -1, -1, -1);
  // 初始化SD卡（1-bit模式）
  if (!SD_MMC.begin("/sdcard", true, false)) {
    Serial.println("SD card initialization failed");
    return;
  } else {
    Serial.println("SD card initialized successfully");
  }

  // 初始化音频系统
  my_pins.addI2C(PinFunction::CODEC, I2C_SCL, I2C_SDA, ES8311_ADDRESS);
  
  CodecConfig cfg;
  cfg.input_device = ADC_INPUT_ALL;
  cfg.output_device = DAC_OUTPUT_ALL;
  cfg.i2s.bits = BIT_LENGTH_16BITS;
  cfg.i2s.rate = RATE_44K;
  cfg.i2s.fmt = I2S_NORMAL;
  
  if (!board.begin(cfg)) {
    Serial.println("AudioBoard begin failed");
  } else {
    Serial.println("AudioBoard begin successful");
  }
  
  audio.setPinout(I2S_BCK, I2S_WS, I2S_DO, -1, I2S_MCK);
  audio.setVolume(8);
  
  // 延迟再开功放，等codec稳定
  delay(200);
  pinMode(PA_ENABLE, OUTPUT);
  digitalWrite(PA_ENABLE, HIGH);
  delay(100);

  // 初始化MPU6050
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  
  if (accelgyro.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
  }

  // 初始化LED
  strip.begin();
  strip.setBrightness(100);
  strip.show();
  setAll(0, 0, 0); // 初始关闭LED

  Serial.println("System ready! Starting motion detection...");
  
  // 播放开机音效
  audio.connecttoFS(SD_MMC, "font.wav");
  
  // 开机提示
  setAll(0, 255, 0); // 绿色开机指示
  delay(500);
  setAll(0, 0, 0); // 关闭
}


void loop() {
  // 处理音频
  audio.loop();

  // 读取MPU6050数据
  getMpu6050();

  // 处理各种事件
  on_off_sound();
  gestureTick();
  strikeTick();
  swingTick();
  randomPULSE();
  lightTick();

  delay(10); // 短暂延时
}

void getMpu6050() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime < 20) return;  // 限制读取频率到50Hz
  lastTime = millis();

  // 读取真实的MPU6050数据
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // 计算处理后的值
  gyroX = abs(gx / 100);
  gyroY = abs(gy / 100);
  gyroZ = abs(gz / 100);
  accelX = abs(ax / 100);
  accelY = abs(ay / 100);
  accelZ = abs(az / 100);

  // 计算向量和
  ACC = sq((long)accelX) + sq((long)accelY) + sq((long)accelZ);
  ACC = sqrt(ACC);
  GYR = sq((long)gyroX) + sq((long)gyroY) + sq((long)gyroZ);
  GYR = sqrt(GYR) / 2;

  // 调试输出 - 每秒输出一次
  if (millis() % 1000 < 10) {
    Serial.print("ACC: ");
    Serial.print(ACC);
    Serial.print(", GYR: ");
    Serial.print(GYR);
    Serial.print(", State: ");
    Serial.println(ls_state ? "ON" : "OFF");
  }
}

void on_off_sound() {
  if (ls_chg_state) {
    if (!ls_state) {
      nowNumber = esp_random() % 2;
      nowColor = 1;
      setColor(nowColor);
      audio.connecttoFS(SD_MMC, in_cache[nowNumber]);
      humTimer = millis() - HUM_TIMEOUT + 2000; // 假设音效时长2秒
      create_lighton_action();
      ls_state = 1;
      bzzz_flag = 1;
      Serial.println("Light saber turned ON");
    } else {
      bzzz_flag = 0;
      nowNumber = esp_random() % 2;
      audio.connecttoFS(SD_MMC, out_cache[nowNumber]);
      humTimer = millis() - HUM_TIMEOUT + 2000;
      create_lightoff_action();
      ls_state = 0;
      Serial.println("Light saber turned OFF");
    }
    ls_chg_state = 0;
  }

  // 循环播放hum音效
  if ((millis() - humTimer) > HUM_TIMEOUT && bzzz_flag) {
    audio.connecttoFS(SD_MMC, "hum1.wav");
    humTimer = millis();
    swing_flag = 1;
    strike_flag = 0;
    Serial.println("Hum sound playing");
  }
}

void gestureTick() {
  if (GESTURE_ALLOW == 1) {
    static int16_t count_open = 0;
    float roll_min, roll_max;
    float pitch_min, pitch_max;

    if (millis() - gestureTimer > 50) {
      gestureTimer = millis();

      float accX = (float)ax / 2048.0;
      float accY = (float)ay / 2048.0;
      float accZ = (float)az / 2048.0;

      float gyroX_val = (float)gx / 131.0;
      float gyroY_val = (float)gy / 131.0;
      float gyroZ_val = (float)gz / 131.0;

      rolls[ges_counter & (IMU_CAL_COUNTER - 1)] = atan2(accY, accZ) * 57.2974;
      pitchs[ges_counter & (IMU_CAL_COUNTER - 1)] = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 57.2974;

      roll_min = pitch_min = 10000;
      roll_max = pitch_max = -10000;

      for (int i = 0; i < IMU_CAL_COUNTER; i++) {
        roll_min = rolls[i] < roll_min ? rolls[i] : roll_min;
        roll_max = rolls[i] > roll_max ? rolls[i] : roll_max;
        pitch_min = pitchs[i] < pitch_min ? pitchs[i] : pitch_min;
        pitch_max = pitchs[i] > pitch_max ? pitchs[i] : pitch_max;
      }

      if (pitch_max - pitch_min > OPEN_THR && count_open == 0) {
        ls_chg_state = 1;
        count_open = 40;
        Serial.println("Gesture detected - triggering ON/OFF sequence");
      }

      ges_counter++;

      if (count_open > 0) count_open--;
    }
  }
}

void strikeTick() {
  if ((ACC > STRIKE_THR) && (ACC < STRIKE_S_THR) && 
      (millis() - strike_timeout > 500) && 
      ls_state && lighton_flag == 0 && lightoff_flag == 0) {
      
    strike_timeout = millis();
    nowNumber = esp_random() % 10;
    audio.connecttoFS(SD_MMC, clsh_cache[nowNumber]);
    humTimer = millis() - HUM_TIMEOUT + 1000; // 假设敲击音效1秒
    create_hit_action();
    strike_flag = 1;
    Serial.println("Strike detected! Playing sound and lighting effect");
  }
}

void swingTick() {
  if (GYR > 60 && (millis() - swing_timeout > 100) && 
      ls_state && lighton_flag == 0 && lightoff_flag == 0) {
      
    swing_timeout = millis();
    
    if (((millis() - swing_timer) > SWING_TIMEOUT) && swing_flag && !strike_flag) {
      if (GYR >= SWING_THR) {
        nowNumber = (esp_random() % 7) * 2 + 1;
        audio.connecttoFS(SD_MMC, swngs[nowNumber]);
        humTimer = millis() - HUM_TIMEOUT + 1000;
        swing_flag = 0;
        swing_timer = millis();
        Serial.println("Fast swing detected! Playing fast swing sound");
      }
      else if (GYR < SWING_THR && GYR > SWING_L_THR) {
        nowNumber = (esp_random() % 8) * 2 + 1;
        audio.connecttoFS(SD_MMC, swngs[nowNumber]);
        humTimer = millis() - HUM_TIMEOUT + 1000;
        swing_flag = 0;
        swing_timer = millis();
        Serial.println("Slow swing detected! Playing slow swing sound");
      }
    }
  }
}

void randomPULSE() {
  if (PULSE_ALLOW && ls_state && 
      (millis() - PULSE_timer > PULSE_DELAY) && 
      lighton_flag == 0 && lightoff_flag == 0 && hit_flag == 0) {
    PULSE_timer = millis();
    PULSEOffset = PULSEOffset * k + map((esp_random() % 1024), 0, 1024, -PULSE_AMPL, PULSE_AMPL) * (1 - k);
    
    if (nowColor == 0) PULSEOffset = constrain(PULSEOffset, -15, 5);
    
    redOffset = constrain(red + PULSEOffset, 0, 255);
    greenOffset = constrain(green + PULSEOffset, 0, 255);
    blueOffset = constrain(blue + PULSEOffset, 0, 255);
    setAll(redOffset, greenOffset, blueOffset);
  }
}

void create_lighton_action() {
  lighton_flag = 1;
  light_counter = 0;
  Serial.println("Starting light-on animation");
}

void create_lightoff_action() {
  lightoff_flag = 1;
  light_counter = LED_COUNT1 / 2 - 1;
  Serial.println("Starting light-off animation");
}

void create_hit_action() {
  setAll(255, 201, 0); // 黄色敲击特效
  hit_timer = millis();
  hit_flag = 1;
  Serial.println("Hit effect activated");
}

void lightTick() {
  if ((millis() - FLASH_timer > FLASH_DELAY) && lighton_flag) {
    FLASH_timer = millis();
    strip.setPixelColor(light_counter, red, green, blue);
    strip.setPixelColor((LED_COUNT1 - 1 - light_counter), red, green, blue);
    strip.show();
    light_counter++;
    
    if (light_counter >= (LED_COUNT1 / 2 - 1)) {
      light_counter = 0;
      lighton_flag = 0;
      Serial.println("Light-on animation completed");
    }
  }
  
  if ((millis() - FLASH_timer > FLASH_DELAY) && lightoff_flag) {
    FLASH_timer = millis();
    strip.setPixelColor(light_counter, 0, 0, 0);
    strip.setPixelColor((LED_COUNT1 - 1 - light_counter), 0, 0, 0);
    strip.show();
    
    if (light_counter == 0) {
      lightoff_flag = 0;
      Serial.println("Light-off animation completed");
    } else {
      light_counter--;
    }
  }
  
  if ((millis() - hit_timer > 300) && hit_flag && ls_state) { // 300ms后恢复原色
    hit_timer = millis();
    setAll(red, green, blue);
    hit_flag = 0;
    Serial.println("Hit effect ended, returning to normal color");
  }
}

void setColor(byte color) {
  switch (color) {
    case 0: // Red
      red = 255; green = 0; blue = 0; break;
    case 1: // Green
      red = 0; green = 255; blue = 0; break;
    case 2: // Blue
      red = 0; green = 0; blue = 255; break;
    case 3: // Pink
      red = 255; green = 0; blue = 255; break;
    case 4: // Yellow
      red = 255; green = 255; blue = 0; break;
    case 5: // Ice Blue
      red = 0; green = 255; blue = 255; break;
  }
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < LED_COUNT1; i++) {
    strip.setPixelColor(i, red, green, blue);
  }
  strip.show();
}