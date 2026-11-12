#include "stm32f10x.h"  

#include "Delay.h"

/*====================================================================
 * 文件说明：NVIC 中断优先级配置模块
 * -------------------------------------------------------------------
 * NVIC（嵌套向量中断控制器）是 Cortex-M3 内核里的"中断调度器"。
 * 每个外设中断要真正触发，必须满足两个条件：
 *   1. 外设侧：中断源使能（如 TIM_ITConfig、EXTI 中断使能）；
 *   2. NVIC 侧：中断通道使能 + 优先级设置（本文件负责）。
 *
 * 优先级分组（NVIC_PriorityGroup_2）：
 *   4 位优先级拆成 2 位"抢占优先级" + 2 位"子优先级"。
 *   抢占优先级高的中断可以打断抢占优先级低的；
 *   抢占优先级相同时，子优先级高的先执行（不能互相打断）。
 *   数字越小，优先级越高。
 *====================================================================*/
/*====================================================================
 * NVICtim1_Init：配置高级定时器 TIM1 更新中断的优先级
 * -------------------------------------------------------------------
 * 作用：使能 TIM1_UP（TIM1 更新事件）这个中断通道，并设置
 *       抢占优先级 1、子优先级 1。
 * 注意1：本工程实际用的是 TIM2/TIM3 和 EXTI 中断，它们的中断
 *        优先级配置在 Timer.c 里完成（EXTI 的 NVIC 配置应写
 *        在 NVICKey_Init 函数里，本文件目前未提供实现）。
 * 注意2：本函数保留仅为演示 NVIC 配置写法，main.c 中未调用。
 *====================================================================*/
void NVICtim1_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	// 优先级分组2：2位抢占 + 2位子优先级
	// 注意：分组配置只能调用一次，多次调用会改变所有中断的优先级含义
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;	// 选择中断通道：TIM1 更新中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// 使能这个中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	// 抢占优先级 1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	// 子优先级 1
	NVIC_Init(&NVIC_InitStructure);	// 把上面的配置写入 NVIC 寄存器
}
