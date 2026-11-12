/*============================================================================
  文件：Tim.c（通用定时器 TIM1 初始化）
  ----------------------------------------------------------------------------
  功能：把高级定时器 TIM1 配置成"向上计数"的基础时基模式。
  ----------------------------------------------------------------------------
  定时器知识点：
    - 定时器 = 一个不停"数数"的计数器 CNT，数到 ARR（自动重装值）就
      溢出归零，再来一个"计数周期"。溢出时可触发中断或事件；
    - 计数频率 = 时钟频率 / (PSC + 1)；
    - 溢出周期 = (PSC + 1) * (ARR + 1) / 时钟频率。
  ----------------------------------------------------------------------------
  本文件配置的计算（供参考）：
    时钟 72MHz，PSC=71 -> 计数频率 = 72MHz/72 = 1MHz（1us 数一次）；
    ARR=9999 -> 每 10000us = 10ms 溢出一次。
    TIM1 是"高级定时器"，多一个重复计数寄存器 RCR（本项目填 100），
    可用于"每 N 次溢出才更新"的高级用法，普通应用填 0 即可。
  ----------------------------------------------------------------------------
  注意：本文件只做了时基初始化，没有打开 TIM_Cmd 总开关，
  也没有配置中断/输出，属于"半成品模板"，需要时再补充。
  ============================================================================*/
#include "stm32f10x.h"                  // Device header

/*
 * 函数名：Tim_Init（TIM1 时基初始化）
 * 参数：无
 * 返回：无
 * 功能：配置 TIM1 的时基（预分频、周期、计数方向），为后续使用做准备。
 * 参数含义：
 *   - TIM_Prescaler = 71：预分频值 PSC=71，实际分频系数是 71+1=72，
 *     72MHz / 72 = 1MHz，即计数器每 1us 加 1；
 *   - TIM_Period = 9999：自动重装值 ARR=9999，计数到 10000 溢出，
 *     所以定时周期 = 10000us = 10ms；
 *   - TIM_CounterMode_Up：向上计数（CNT 从 0 数到 ARR 再归零）；
 *   - TIM_ClockDivision = 0：时钟分频 1，不影响计数；
 *   - TIM_RepetitionCounter = 100：重复计数（仅 TIM1/TIM8 有），
 *     表示"每 100 次溢出才更新影子寄存器"，一般工程填 0；
 *   - TIM_TimeBaseInit(TIM1, ...)：注意 TIM1 是 APB2 总线上的
 *     高级定时器（不是 APB1 的 TIM2/3/4），所以时钟由 APB2 提供，
 *     这里开头开的就是 RCC_APB2Periph_TIM1。
 */
void Tim_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);	// 开启 TIM1 时钟（挂载在 APB2 总线）

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	// 时钟分频 1
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数
	TIM_TimeBaseStructure.TIM_Period =9999;		// ARR=9999：10ms 溢出一次
	TIM_TimeBaseStructure.TIM_Prescaler = 71;	// PSC=71：计数频率 1MHz（1us 加 1）
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 100;	// 重复计数 100 次才更新
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
}
