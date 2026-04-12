# LOL Infinity Edge - Light Saber System

## 系统概述

LOL Infinity Edge 是一个 ESP32 S3 驱动的智能光剑系统，具有完整的手势识别、音效、LED 动画等交互功能。

## 架构设计

### 分层架构

```
┌──────────────────────────────────────┐
│      App 层 (SaberController)        │
│  业务逻辑、状态管理、交互处理        │
└────────────┬─────────────────────────┘
             │
┌────────────▼──────────────────────────────┐
│       Service 层 - 业务服务             │
│ ┌──────────────┐  ┌──────────────────┐  │
│ │ ImuService   │  │ LedService       │  │
│ │ (传感器处理) │  │ (LED显示+动画)   │  │
│ └──────────────┘  │                  │  │
│ ┌──────────────┐  │ AudioService     │  │
│ │ IMU数据      │  │ (音效管理+自动化)│  │
│ │ 手势识别     │  │                  │  │
│ │ 敲击检测     │  └──────────────────┘  │
│ │ 挥动检测     │                         │
│ └──────────────┘                         │
└─────────────────┬──────────────────────┘
                  │
┌─────────────────▼──────────────────────────┐
│       Driver 层 - 硬件驱动               │
│ ImuDriver  LedDriver  AudioDriver ...     │
└───────────────────────────────────────────┘
```

---

## 状态机 (State Machine)
### LED 动画与脉冲优先级机制

系统所有 LED 动画（开/关/敲击/挥动）和脉冲效果采用严格优先级：

1. 开启动画（Turn-On Animation）
2. 关闭动画（Turn-Off Animation）
3. 敲击动画（Strike Animation）
4. 挥动动画（Swing Animation）
5. 脉冲效果（Pulse Effect）

每次 update() 只会执行最高优先级的动画/效果，避免颜色冲突。
只有所有动画状态都为 false 时，脉冲效果才会生效。

#### 挥动动画（Swing Animation）
ON 状态下挥动时，LED 会短暂变为白色高亮（300ms），动画结束后自动恢复为当前主色。

#### 动画状态变量
每种动画有独立的 Active 标志（如 lightOnActive、strikeAnimationActive、swingAnimationActive），互斥激活。

#### 脉冲效果
仅在无动画时生效，周期性柔和变亮/变暗。

#### 典型优先级流程
```
if (lightOnActive)      // 开启动画
   handleTurnOnAnimation();
else if (lightOffActive) // 关闭动画
   handleTurnOffAnimation();
else if (strikeAnimationActive) // 敲击动画
   handleStrikeAnimation();
else if (swingAnimationActive)  // 挥动动画
   handleSwingAnimation();
else
   handlePulse(); // 脉冲效果
```
光剑共有 4 个主要状态：

### 1️⃣ OFF 状态（关闭）
**描述**: 光剑完全关闭，处于待命状态

**进入条件**:
- 系统初始化
- 从 STOPPING 状态转移（关闭动画完成）
- 手势识别（打开/关闭切换）

**该状态下的行为**:
- ❌ 无 LED 显示
- ❌ 无背景嗡鸣音
- ✅ 持续监听手势（打开/关闭手势识别）
- ✅ 可通过 Web API 激活

**可能的事件**:
- 🎙️ **手势识别** → 在空中拉出打开手势 → 转移到 STARTING 状态

---

### 2️⃣ STARTING 状态（启动中）
**描述**: 光剑正在启动，播放启动动画和音效

**进入条件**:
- 手势识别成功
- 通过 Web API 调用 setState(true)

**该状态下的行为**:

#### 🎵 音效
```
1. 启动音效 (in1.wav 或 in2.wav)
   - 2 个启动音中随机选择
   - 持续约 1-2 秒
   
2. LED 启动动画进行中
   - 不播放 hum（嗡鸣）
   - hum 处于非激活状态
```

#### 💡 LED 动画
```
启动动画 (Turn-On Animation):
├─ 方向: 从两侧向中心
├─ 方式: LED 条带两端同时点亮
├─ 速度: 每 15ms 前进一个 LED
├─ 持续: 约 ~420ms (28/2 × 15)
└─ 效果: 灯条逐渐点亮
```

#### 颜色
- 初始化为**红色** (RGB: 255, 0, 0)
- 保持不变直到 ON 状态

**转移条件**:
- 当音效播放完成（`!audioService->isPlaying()`）
- 自动转移到 **ON** 状态
- hum 自动激活

**时间轴**:
```
0ms      → 播放 in 音效
~1500ms  → LED 启动动画完成
~2000ms  → 音效播放完成，转移到 ON 状态
```

---

### 3️⃣ ON 状态（激活/常态）
**描述**: 光剑激活，完全可用，处于交互状态

**进入条件**:
- STARTING 状态中音效播放完成

**该状态下的行为**:

#### 🎵 音效管理

**背景嗡鸣 (Hum)**
```
自动播放条件:
├─ 当前状态 = ON
├─ hum 激活 = true
├─ 没有其他音效播放（isPlaying() = false）
└─ 超过超时时间（HUM_TIMEOUT = 30228ms ≈ 30s）

播放频率:
├─ 首次激活后延迟 2000ms 开始
├─ 之后每 30s 播放一次 hum1.wav
└─ 持续到离开 ON 状态

自动停止条件:
├─ 播放其他音效（swing, strike, turn off）
└─ 其他音效完成后，等待 HUM_TIMEOUT 再次播放
```

**挥动音效 (Swing)**
```
触发条件:
├─ 检测到挥动手势
├─ 陀螺仪值 > SWING_DETECTION_THR (60)
├─ 距上次挥动 > SWING_DELAY (500ms)
└─ 没有其他音效正在播放

音效分类:
├─ 缓慢挥动 (80 < gyr < 180)
│  └─ 音效: swng1.wav, swng3.wav, ..., swng15.wav (8个)
│  └─ 随机选择
│
└─ 快速挥动 (gyr >= 180)
   └─ 音效: swng2.wav, swng4.wav, ..., swng16.wav (8个)
   └─ 随机选择
```

**敲击音效 (Strike)**
```
触发条件:
├─ 检测到敲击/碰撞
├─ 加速度值: STRIKE_THR (40) < acc < STRIKE_S_THR (160)
├─ 距上次敲击 > STRIKE_DELAY (500ms)
└─ 没有其他音效正在播放

音效:
├─ clsh1.wav - clsh10.wav (10 个碰撞音)
└─ 随机选择

同步效果:
└─ 同时触发 LED 敲击闪烁效果
```

#### 💡 LED 效果

**脉冲效果 (Pulse)**
```
脉冲条件:
├─ 启用: PULSE_ALLOW = 1
├─ 频率: 每 PULSE_DELAY (30ms) 更新一次
└─ 一直活跃直到 OFF 状态

脉冲算法:
├─ 随机偏移范围: ±PULSE_AMPL (±10)
├─ 平滑系数: 0.9 (防止闪烁)
├─ RGB 约束: 0-255 之间
└─ 蓝色模式特殊处理: 偏移在 -15~5 范围内

效果:
└─ 光芒柔和脉动，如呼吸般跳动
```

**敲击闪烁 (Strike Flash)**
```
触发时:
├─ 颜色变为黄色 (RGB: 255, 201, 0) - 碰撞火花
├─ 持续 300ms
└─ 自动恢复为脉冲颜色

工作原理:
└─ 高优先级覆盖脉冲效果，300ms 后恢复
```

#### 🎨 颜色预设
在 ON 状态默认使用红色，可设置为 6 种颜色：
- 🔴 RED (0): RGB(255, 0, 0)
- 🟢 GREEN (1): RGB(0, 255, 0)
- 🔵 BLUE (2): RGB(0, 0, 255)
- 🩷 PINK (3): RGB(255, 0, 255)
- 🟡 YELLOW (4): RGB(255, 255, 0)
- 🔷 ICE_BLUE (5): RGB(0, 255, 255)

#### 🎯 可用的交互手势

**挥动检测**
```
条件:
├─ 陀螺仪角速度 > SWING_DETECTION_THR (60)
├─ 防抖: SWING_DELAY = 500ms
└─ 判重: SWING_L_THR (80) vs SWING_THR (180)

反馈:
├─ 音效: 快/慢挥动音各 8 个
├─ LED: 无特殊反应，脉冲继续
└─ 连续触发不受限（防抖保护）
```

**敲击检测**
```
条件:
├─ 加速度: 40 < acc < 160
├─ 防抖: STRIKE_DELAY = 500ms
└─ 检测：可能来自水平敲击或下压

反馈:
├─ 音效: 10 个碰撞音中随机
├─ LED: 黄色闪烁 300ms
└─ 高优先级（中断脉冲）
```

**手势识别 (任何状态下)**
```
打开/关闭手势:
├─ 通过加速度计检测朝向变化
├─ 检测原理: PITCH 角度范围 > OPEN_THR (80°)
├─ 采样: IMU_CAL_COUNTER = 8 个连续样本
├─ 防抖: 检测后 40 tick 内不再触发
└─ 作用: OFF ↔ ON 切换

在 OFF 状态:
└─ 持续监听，可随时激活打开序列

在 ON 状态:
└─ 监听切换，执行关闭序列
```

---

### 4️⃣ STOPPING 状态（关闭中）
**描述**: 光剑正在关闭，播放关闭动画和音效

**进入条件**:
- 手势识别成功（从 ON 中打开/关闭）
- 通过 Web API 调用 setState(false)
- 从 STARTING 中如果用户中断

**该状态下的行为**:

#### 🎵 音效
```
1. 关闭音效 (out1.wav 或 out2.wav)
   - 2 个关闭音中随机选择
   - 持续约 1-2 秒
   
2. hum 立即停用
   - 不播放背景嗡鸣
   - 但可能会播放 out 音效
```

#### 💡 LED 动画
```
关闭动画 (Turn-Off Animation):
├─ 方向: 从中心向两侧
├─ 方式: LED 条带两端同时熄灭
├─ 速度: 每 15ms 后退一个 LED
├─ 持续: 约 ~420ms (28/2 × 15)
└─ 效果: 灯条逐渐熄灭
```

**转移条件**:
- 当音效播放完成（`!audioService->isPlaying()`）
- 自动转移到 **OFF** 状态

**时间轴**:
```
0ms      → 立即停用 hum
         → 播放 out 音效
~1500ms  → LED 关闭动画完成
~2000ms  → 音效播放完成，转移到 OFF 状态
         → hum 彻底关闭
```

---

## 状态转移图

```
                    ┌──────────────┐
                    │   系统启动    │
                    └────────┬──────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │   OFF 状态      │
                    │ (光剑关闭)      │
                    └────────┬────────┘
                             │
                       手势识别 / Web API
                             │
                             ▼
                    ┌─────────────────┐
                    │  STARTING 状态  │
                    │ (启动动画+音效) │
                    └────────┬────────┘
                             │
                        音效播放完成
                             │
                             ▼
                    ┌─────────────────┐
                    │   ON 状态       │
                    │ (完全激活)      │
                    │ - 脉冲效果      │
                    │ - 手势识别      │
                    │ - hum 自动播放  │
                    └────────┬────────┘
                             │
                       手势识别 / Web API
                             │
                             ▼
                    ┌─────────────────┐
                    │ STOPPING 状态   │
                    │ (关闭动画+音效) │
                    └────────┬────────┘
                             │
                        音效播放完成
                             │
                             ▼
                    ┌─────────────────┐
                    │   OFF 状态      │
                    │ (光剑关闭)      │
                    └─────────────────┘
```

---

## 配置参数

### 传感器参数

| 参数 | 值 | 说明 |
|-----|-----|------|
| `GESTURE_ALLOW` | 1 | 0=禁用手势识别, 1=启用 |
| `IMU_CAL_COUNTER` | 8 | 手势识别样本数 |
| `OPEN_THR` | 80 | 手势识别角度阈值 |

### 挥动检测参数

| 参数 | 值 | 说明 |
|-----|-----|------|
| `SWING_DETECTION_THR` | 60 | 挥动触发阈值（角速度）|
| `SWING_L_THR` | 80 | 缓慢挥动判断下界 |
| `SWING_THR` | 180 | 快速挥动判断上界 |
| `SWING_DELAY` | 500 | 挥动防抖延迟(ms) |

### 敲击检测参数

| 参数 | 值 | 说明 |
|-----|-----|------|
| `STRIKE_THR` | 40 | 敲击触发下界（加速度）|
| `STRIKE_S_THR` | 160 | 敲击触发上界 |
| `STRIKE_DELAY` | 500 | 敲击防抖延迟(ms) |

### LED 参数

| 参数 | 值 | 说明 |
|-----|-----|------|
| `LED_PIN1` | 5 | LED 条带 1 引脚 |
| `LED_COUNT1` | 28 | LED 条带 1 灯珠数 |
| `LED_PIN2` | 4 | LED 条带 2 引脚 |
| `LED_COUNT2` | 28 | LED 条带 2 灯珠数 |
| `FLASH_DELAY` | 15 | 动画帧延迟(ms) |
| `PULSE_ALLOW` | 1 | 0=禁用脉冲, 1=启用 |
| `PULSE_AMPL` | 10 | 脉冲幅度(亮度) |
| `PULSE_DELAY` | 30 | 脉冲更新频率(ms) |

### 音频参数

| 参数 | 值 | 说明 |
|-----|-----|------|
| `HUM_TIMEOUT` | 30228 | hum 播放间隔(ms) |
| `AUDIO_VOLUME` | 8 | 音量等级(0-255) |

---

## 音效清单

### 启动/关闭音效
```
in1.wav   - 打开音效 1
in2.wav   - 打开音效 2
out1.wav  - 关闭音效 1
out2.wav  - 关闭音效 2
saber.flac - 系统启动音
```

### 挥动音效 (16 个总共)
```
缓慢挥动:
  swng1.wav, swng3.wav, swng5.wav, swng7.wav,
  swng9.wav, swng11.wav, swng13.wav, swng15.wav

快速挥动:
  swng2.wav, swng4.wav, swng6.wav, swng8.wav,
  swng10.wav, swng12.wav, swng14.wav, swng16.wav
```

### 敲击音效 (10 个)
```
clsh1.wav, clsh2.wav, clsh3.wav, clsh4.wav, clsh5.wav,
clsh6.wav, clsh7.wav, clsh8.wav, clsh9.wav, clsh10.wav
```

### 环境音效
```
hum1.wav - 背景嗡鸣声，在 ON 状态自动循环播放
```

---

## 特性总结

| 特性 | 适用状态 | 说明 |
|-----|---------|------|
| **手势识别** | OFF, ON | 随时可打开/关闭光剑 |
| **启动动画** | STARTING | LED 从两侧向中心点亮 |
| **关闭动画** | STOPPING | LED 从中心向两侧熄灭 |
| **脉冲效果** | ON | 持续柔和脉动 |
| **快速挥动** | ON | 陀螺仪 > 180，触发快速挥动音 |
| **缓慢挥动** | ON | 陀螺仪 80-180，触发缓慢挥动音 |
| **敲击闪烁** | ON | 加速度 40-160，黄色闪烁+音效 |
| **背景嗡鸣** | ON | 无其他音效时自动循环 |
| **颜色预设** | ON | 6 种预设颜色可选 |
| **Web API** | OFF/ON | 远程控制开关 |

---

## 使用示例

### 通过手势激活
```
1. 手持光剑，处于 OFF 状态
2. 在空中做出「打开」手势（快速向上拉）
3. 系统进入 STARTING 状态
   - 播放 in 音效（in1.wav 或 in2.wav）
   - LED 从两侧点亮（启动动画）
4. 2 秒后进入 ON 状态
   - LED 保持红色并脉动
   - 背景 hum 每 30 秒自动播放一次
5. 挥动光剑 → 触发挥动音效
6. 敲击光剑 → 触发敲击音效 + LED 黄色闪烁
7. 再做出「打开」手势 → 进入 STOPPING 状态
   - 播放 out 音效
   - LED 从中心熄灭
8. 回到 OFF 状态
```

### 通过 Web API 激活
```
# 打开
POST /api/saber/power
{"state": true}

# 关闭
POST /api/saber/power
{"state": false}

# 查询状态
GET /api/saber/power
# 返回: {"state": true/false}
```

---

## 开发与调试

### 快速参考：状态对应的效果

| 状态 | LED | 音效 | 手势监听 |
|-----|-----|------|---------|
| **OFF** | 无 | 无 | ✓ |
| **STARTING** | 启动动画 | in 音效 | ✗ |
| **ON** | 脉冲 + 敲击闪烁 | 挥动/敲击/hum | ✓ |
| **STOPPING** | 关闭动画 | out 音效 | ✗ |

### 修改配置
所有配置参数在 `include/config.h` 中定义，修改后重新编译部署：
```cpp
static constexpr int SWING_THR = 180;       // 调整挥动灵敏度
static constexpr int PULSE_DELAY = 30;      // 调整脉冲频率
static constexpr int HUM_TIMEOUT = 30228;   // 调整 hum 频率
```

### 测试触发
- **快速旋转**：两种挥动音效随机出现
- **快速敲击**：黄色闪烁 + 碰撞音
- **空中上拉**：ON ↔ OFF 转换

### 串口调试
系统通过串口输出详细日志：
```
[AudioService: Hum activated]
[LedService: Turn on animation started]
[ImuService: Swing detected]
[Saber state changed to ON, hum activated]
```

---

**文档更新时间**: 2026-04-12  
**版本**: 1.0  
**架构**: 分层服务设计 (Drivers → Services → App)