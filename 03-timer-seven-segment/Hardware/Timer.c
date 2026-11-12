#include "stm32f10x.h"    
//extern unsigned int Num;// Device header

/*====================================================================
 * 文件说明：定时器初始化模块
 * -------------------------------------------------------------------
 * 本文件配置了 3 个定时器（TIM2/TIM3/TIM4），原理完全相同：
 * 定时器本质上是一个"自动加 1 的计数器"。它内部的计数器从 0
 * 数到"自动重装值（Period）"后溢出，产生"更新事件"；更新中断
 * 使能 + NVIC 配置好后，CPU 就会跳去执行对应的 IRQHandler。
 *
 * 定时时间计算公式（系统时钟 72MHz，TIM2~TIM4 挂在 APB1 上）：
 *   定时时间 = (预分频器 PSC + 1) × (自动重装值 ARR + 1) ÷ 72MHz
 *
 * 例 1：PSC = 7200-1，ARR = 10000-1  → 7200×10000÷72M = 1 秒
 * 例 2：PSC = 7200-1，ARR = 10-1     → 7200×10÷72M   = 1 毫秒
 *====================================================================*/
void Timer2_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);	// 开启 TIM2 外设时钟（外设时钟默认关闭，必须先开启）

	// 选择"内部时钟"作为计数脉冲源（即来自 RCC 的 72MHz 系统时钟）
	TIM_InternalClockConfig(TIM2);
	// 时基结构体：用来描述定时器"以什么节奏计数、数到哪里溢出"
	TIM_TimeBaseInitTypeDef TIM_TimebaseStructure;
	TIM_TimebaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	// 时钟分频：1 分频（只影响输入捕获滤波，一般填 DIV1）
	TIM_TimebaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数模式：计数器从 0 数到自动重装值
	TIM_TimebaseStructure.TIM_Period = 10000 - 1;	// 自动重装值 ARR = 9999（计数器数到 9999 就溢出）
	TIM_TimebaseStructure.TIM_Prescaler = 7200 - 1;	// 预分频器 PSC = 7199（72MHz ÷ 7200 = 10kHz 的计数频率）
	TIM_TimebaseStructure.TIM_RepetitionCounter = 0;	// 重复计数寄存器（高级定时器专用，通用定时器填 0）
	TIM_TimeBaseInit(TIM2,&TIM_TimebaseStructure);	// 把上面的时基配置写入 TIM2 的寄存器
	// 定时时间 = (PSC+1) × (ARR+1) ÷ 72MHz = 7200 × 10000 ÷ 72M = 1 秒
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);	// 先清除更新标志，防止刚初始化就误触发一次中断
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	// 使能 TIM2 的"更新中断"（溢出时通知 CPU）

	// NVIC：中断"能不能打断主程序"的关键开关
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	// 优先级分组2：2位抢占优先级 + 2位子优先级
	NVIC_InitTypeDef  NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	// 要配置的中断通道：TIM2 全局中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// 使能该中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	// 抢占优先级 0（数字越小越优先）
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	// 子优先级 0
	NVIC_Init(&NVIC_InitStructure);	// 把上面的配置写入 NVIC 寄存器

	TIM_Cmd(TIM2,ENABLE);	// 启动 TIM2：计数器开始跑（DISABLE 则停止）
}
/*====================================================================
 * Timer3_Init：TIM3 初始化
 * -------------------------------------------------------------------
 * 与 Timer2_Init 结构完全相同，只是参数不同：
 *   PSC = 7199，ARR = 9 → 7200 × 10 ÷ 72MHz = 1 毫秒
 * 即每 1 毫秒进一次更新中断，本工程用它做数码管动态扫描。
 * NVIC 抢占/子优先级都设为 1（低于 TIM2 的 0），
 * 这样 TIM2（秒走时）可以优先于 TIM3（扫描）执行。
 *====================================================================*/
void Timer3_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	// 开启 TIM3 时钟
	
	TIM_InternalClockConfig(TIM3);	// 使用内部时钟（72MHz）
	TIM_TimeBaseInitTypeDef TIM_TimebaseStructure;
	TIM_TimebaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	// 时钟分频 1 分频
	TIM_TimebaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数
	TIM_TimebaseStructure.TIM_Period = 10 - 1;	// 自动重装值 ARR = 9
	TIM_TimebaseStructure.TIM_Prescaler = 7200 - 1;	// 预分频器 PSC = 7199
	TIM_TimebaseStructure.TIM_RepetitionCounter = 0;	// 高级定时器专用，填 0
	TIM_TimeBaseInit(TIM3,&TIM_TimebaseStructure);	// 写入寄存器
	// 定时时间 = 7200 × 10 ÷ 72MHz = 1 毫秒
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);	// 清除更新标志
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);	// 使能更新中断
	
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef  NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	// TIM3 全局中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// 使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	// 抢占优先级 1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	// 子优先级 1
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM3,ENABLE);	// 启动 TIM3
}
/*====================================================================
 * Timer4_Init：TIM4 初始化（本工程 main.c 未调用，备用）
 * -------------------------------------------------------------------
 * 与 Timer3 参数完全相同：每 1 毫秒进一次更新中断。
 * 保留这个函数是为了演示"一个定时器一个配置函数"的写法，
 * 想用 TIM4 时只需在 main 里调用 Timer4_Init() 即可。
 *====================================================================*/
void Timer4_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);	// 开启 TIM4 时钟
	
	TIM_InternalClockConfig(TIM4);	// 使用内部时钟（72MHz）
	TIM_TimeBaseInitTypeDef TIM_TimebaseStructure;
	TIM_TimebaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	// 时钟分频 1 分频
	TIM_TimebaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数
	TIM_TimebaseStructure.TIM_Period = 10 - 1;	// 自动重装值 ARR = 9
	TIM_TimebaseStructure.TIM_Prescaler = 7200 - 1;	// 预分频器 PSC = 7199
	TIM_TimebaseStructure.TIM_RepetitionCounter = 0;	// 高级定时器专用，填 0
	TIM_TimeBaseInit(TIM4,&TIM_TimebaseStructure);	// 写入寄存器
	// 定时时间 = 7200 × 10 ÷ 72MHz = 1 毫秒
	TIM_ClearFlag(TIM4,TIM_FLAG_Update);	// 清除更新标志
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);	// 使能更新中断
	
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef  NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;	// TIM4 全局中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// 使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	// 抢占优先级 1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	// 子优先级 1
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4,ENABLE);	// 启动 TIM4
}
//void TIM2_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)
//	{
//		Num++;
//		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
//	}
//}
