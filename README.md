# STM32 外设实验合集

基于 **STM32F103C8T6**（Blue Pill）和 **STM32 标准外设库 V3.5** 的 13 个循序渐进实验，覆盖 GPIO、外部中断、定时器、PWM、输入捕获、ADC、DMA、USART、I2C OLED 等常用外设。每个实验都是独立的 Keil MDK 工程，可直接打开对应目录下的 `Project.uvprojx` 编译。

这些工程来自嵌入式课程学习阶段，代码中的中文注释保留了当时的推导过程；仓库已剔除 `Objects/`、`Listings/`、`*.uvguix.*` 等编译产物和本机 IDE 配置，只保留源码、工程文件和必要的启动/外设库。

## 实验列表

| # | 目录 | 实验内容 | 关键知识点 |
|---|------|----------|------------|
| 01 | [`01-led-blink`](01-led-blink) | LED 闪烁与按键调速 | GPIO 推挽输出、SysTick 延时、EXTI 中断、寄存器/位带操作对比 |
| 02 | [`02-key-interrupt`](02-key-interrupt) | 按键轮询与中断 | GPIO 上拉输入、软件消抖、EXTI、长按/短按/双击状态机 |
| 03 | [`03-timer-seven-segment`](03-timer-seven-segment) | 定时器驱动数码管 | TIM2 1 s 更新中断、TIM3 1 ms 动态扫描、共阳数码管段选/位选 |
| 04 | [`04-timer-countdown`](04-timer-countdown) | 数码管倒计时 | 多定时器协作、EXTI 设置时间、PWM 呼吸灯、LED 流水灯 |
| 05 | [`05-timer-external-clock`](05-timer-external-clock) | 定时器外部时钟计数 | TIM2 ETR 外部脉冲计数、更新中断、OLED 显示 |
| 06 | [`06-seven-segment-alarm`](06-seven-segment-alarm) | 数码管闹钟 | TIM2/TIM3 中断分工、3 路 EXTI、闹钟到点联动 LED/PWM |
| 07 | [`07-input-capture`](07-input-capture) | 输入捕获测频 | TIM2 输入捕获周期/高电平脉宽、TIM3 PWM 同步输出 |
| 08 | [`08-buzzer-music`](08-buzzer-music) | 无源蜂鸣器播放音乐 | TIM3 PWM 频率控制、音符频率表、节拍延时 |
| 09 | [`09-pwm-breathing-led`](09-pwm-breathing-led) | PWM 呼吸灯 | TIM2 PWM 模式、CCR 占空比调节、视觉暂留 |
| 10 | [`10-uart-bluetooth`](10-uart-bluetooth) | 串口 + 蓝牙控制 | USART 收发、HC-05、串口指令解析、数码管/闹钟联动 |
| 11 | [`11-oscilloscope`](11-oscilloscope) | OLED 简易示波器 | ADC 单次转换、TIM3 100 µs 定时采样、128 点波形绘制 |
| 12 | [`12-uart-commands`](12-uart-commands) | 串口指令控制 | 命令缓冲区、字符串解析、PWM/LED/定时器控制 |
| 13 | [`13-dma-adc`](13-dma-adc) | ADC + DMA 波形采集 | ADC1 + DMA1 自动搬运、128 点采样、OLED 曲线显示 |

## 硬件环境

- STM32F103C8T6 最小系统板（72 MHz，64 KB Flash / 20 KB SRAM）
- 0.96 寸 SSD1306 OLED（软件 I2C）
- 4 位共阳数码管（段选 PA0–PA7，位选 PB12–PB15）
- 独立按键、LED、无源蜂鸣器
- HC-05/HC-06 蓝牙串口模块（实验 10）
- USB-TTL 串口模块（实验 10、12）
- ST-Link 或 J-Link 下载器

> 不同实验的引脚分配略有差异，具体以各工程 `Hardware/` 下的初始化和 `main.c` 顶部注释为准。

## 编译与运行

1. 安装 **Keil MDK 5**、STM32F1xx Device Family Pack，以及工程使用的 ARM Compiler（ARMCC 5 或 AC6）。
2. 打开任一实验目录下的 `Project.uvprojx`。
3. 编译后通过 ST-Link/J-Link 下载到开发板。
4. 部分工程包含嵌套的 `4-1OLED` 子工程，可单独打开学习 OLED 驱动。

仓库不包含编译产物，首次编译会重新生成 `Objects/`、`Listings/` 等目录。

## 代码组织

```text
01-led-blink/
├── Hardware/        # LED、按键、EXTI 等板级驱动
├── Library/         # STM32 标准外设库
├── Start/           # 启动文件与系统初始化
├── System/          # SysTick 延时等系统级代码
├── User/            # main.c 与应用逻辑
└── Project.uvprojx  # Keil 工程文件
```

## 说明

- 标准外设库、CMSIS 和启动文件的版权归 STMicroelectronics 所有，按其原始许可使用。
- 部分代码结构参考了江协科技（jiangxiekeji）的 STM32 入门教程，仅用于学习记录。
- 这些是学习阶段的外设实验，不是生产级驱动；中断优先级、错误处理和低功耗设计未做完整工程化处理。

## 相关项目

- [ESP32 + 华为云 IoT 智能家居](https://github.com/3265038185/esp32-huaweicloud-smart-home)
- [MATLAB 数字图像处理实验](https://github.com/3265038185/matlab-digital-image-processing)
- [PCB 设计与蓝桥杯信号源实验](https://github.com/3265038185/hardware-design-portfolio)
