/* ============================================================================
 * 文件说明：Tim.c —— 定时器初始化（TIM1 时基配置）
 * ----------------------------------------------------------------------------
 * 【说明】
 *   本文件初始化的是高级定时器 TIM1，只配置了"时基单元"（分频、周期等），
 *   没有使能中断、也没有配置输出。当前 4-PWM呼吸灯 项目（main.c）实际
 *   使用的是 PWM.c 里的 TIM2，本文件属于备用/实验性代码，
 *   本工程 main 函数并没有调用 Tim_Init()。
 * ----------------------------------------------------------------------------
 * 【参数计算】
 *   系统时钟 72MHz，PSC=71 → 计数时钟 = 72MHz/(71+1) = 1MHz（每 1us 加 1）
 *   ARR=9999 → 一个周期 = (9999+1) × 1us = 10ms
 *   所以配置好后，如果配合"更新中断"，可以做到每 10ms 产生一次中断。
 * ========================================================================== */

#include "stm32f10x.h"                  // Device header
void Tim_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	// 开启 TIM1 的时钟。注意！TIM1 是"高级定时器"，挂在 APB2 总线上，
	// 和挂在 APB1 上的 TIM2~TIM4 不同，要用 RCC_APB2PeriphClockCmd 开时钟

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	// 时钟分频因子：0 表示不分频（这个参数主要影响输入捕获的滤波采样）
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	// 计数模式：向上计数（CNT 从 0 数到 ARR 后清零）
	TIM_TimeBaseStructure.TIM_Period =9999;
	// 自动重装值 ARR = 9999：计数到 9999 时清零，一个周期 10000 拍
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	// 预分频 PSC = 71：72MHz / (71+1) = 1MHz，计数时钟 1MHz（1us 一计数）
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 100;
	// 重复计数寄存器 RCR = 100：只有 TIM1/TIM8 高级定时器才有。
	// 作用：CNT 每溢出 (RCR+1)=101 次才产生一次更新事件/中断。
	// 如果配合更新中断使用，中断周期 = 10ms × 101 ≈ 1.01s
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
	// 把上述配置写入 TIM1 的 PSC/ARR/RCR 等寄存器
}
