/*============================================================================
  文件：PWM.h（PWM 输出 / 输入捕获 - 头文件）
  ----------------------------------------------------------------------------
  作用：
    1. 防止重复包含；
    2. 声明本模块函数，供 main.c 等文件调用。
  ----------------------------------------------------------------------------
  使用说明：
    - 播放音乐（无源蜂鸣器，本项目用法）：
        PWMOut_Init();                 // 初始化 TIM3_CH1 输出（PA6）
        PWM_Set(440, 500);             // 输出 440Hz 方波，持续 500ms
    - 测量外部信号（PWM 输入捕获）：
        PWMInt_Init();                 // 初始化 TIM2_CH1 输入（PA0）
        PWM_GetFreq();                 // 读频率
        PWM_GetDuty();                 // 读占空比
  ============================================================================*/
#ifndef __PWM_H__
#define __PWM_H__


unsigned int PWM_GetFreq(void);		// 读取输入信号频率（Hz），需先 PWMInt_Init
void PWM_Set(unsigned int Freq,unsigned int time);	// 设置输出频率 Freq(Hz) 并持续 time(ms)
void PWMOut_Init(void);				// PWM 输出初始化（TIM3_CH1 -> PA6，驱动无源蜂鸣器）
void PWMInt_Init(void);				// PWM 输入捕获初始化（TIM2_CH1 -> PA0，测频率/占空比）
unsigned int PWM_GetDuty(void);		// 读取输入信号占空比（0~100），需先 PWMInt_Init


#endif
