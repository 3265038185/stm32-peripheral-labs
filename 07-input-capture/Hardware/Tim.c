/*==============================================================================
 * 文件：Tim.c（TIM1定时器模块）
 * 功能：配置高级定时器TIM1的时基单元（计数频率、计数周期），
 *       为后续使用定时中断或输出比较功能做准备。
 * 时基参数计算：
 *   计数频率 = 72MHz / (PSC+1) = 72MHz / (71+1) = 1MHz，
 *   也就是CNT计数器每数一个数正好耗时 1us（微秒）。
 *   ARR = 9999，表示CNT从0数到9999后溢出，一次周期 = 10000个数 = 10ms。
 * 注意：本文件目前只配置了时基，没有开中断（NVIC）也没有启动（TIM_Cmd），
 *       属于"半成品"模块，是江协科技模板的典型写法，供后续练习扩展。
 * 知识点：TIM定时器结构（时钟→预分频PSC→计数器CNT→自动重装ARR）、
 *         时基计算（溢出周期 = (ARR+1)×(PSC+1)/72MHz）
 *============================================================================*/
#include "stm32f10x.h"                  // Device header（STM32寄存器定义和库函数）

/* TIM1时基初始化函数：
   功能：设置TIM1的计数频率和计数周期。
   注：TIM1是"高级定时器"，比其他定时器多了重复计数器（RepetitionCounter）
       和刹车/互补输出等功能，但时基配置的写法和其他定时器一样。 */
void Tim_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);	// 开启TIM1时钟！
									// 注意：TIM1挂在APB2高速总线上（和GPIO一样），
									// TIM2~TIM4则挂在APB1上，开时钟时要选对函数
									// 和参数，搞错的话寄存器写不进去

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	// 定义时基初始化结构体
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	// 时钟分频=1分频（输入滤波采样用，一般填0）
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数模式
									// CNT从0开始：0→1→2→...→ARR→溢出回0
	TIM_TimeBaseStructure.TIM_Period =9999;	// ARR自动重装值=9999：计数周期
									// =（ARR+1）=10000个数
									// 若计数频率1MHz，则10ms溢出一次
	TIM_TimeBaseStructure.TIM_Prescaler = 71;	// PSC预分频=71：72MHz/(71+1)=1MHz
									// 即CNT每数一个数耗时1us
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 100;	// 重复计数器=100（高级定时器独有）
									// 作用：计数溢出N+1次后才产生一次更新事件。
									// 这里填100，表示"少产生101次更新"，
									// 一般做PWM输出时习惯性填0，此值需按需调整
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);	// 把时基配置写入TIM1寄存器
}
