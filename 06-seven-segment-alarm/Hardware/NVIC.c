/* ============================================================================
 * 文件说明（NVIC.c —— 中断优先级配置文件）
 * ----------------------------------------------------------------------------
 * NVIC = Nested Vectored Interrupt Controller（嵌套向量中断控制器），
 * 是 Cortex-M3 内核专门管理"中断优先级"的部件。
 * STM32 的中断优先级由两部分组成：
 *   抢占优先级（数字越小越优先，可打断其他中断）
 *   响应优先级（相同抢占优先级时，数字越小越先执行）
 * 分组方式（NVIC_PriorityGroupConfig）决定两者各占几位：
 *   组2：2 位抢占 + 2 位响应，共 4 个档位 × 4 个档位（本工程使用）
 *
 * 注意：main.c 中调用的是 NVICKey_Init()（按键中断初始化），本文件里
 * 实现的却是 NVICtim1_Init()（TIM1 更新中断初始化）——两者名称不同。
 * 实际使用中若需要按键中断，应把"按键对应的 EXTI 中断通道"的 NVIC
 * 配置补全到 NVICKey_Init() 里（本工程原文件中未给出该实现）。
 * ========================================================================== */
#include "stm32f10x.h"  

#include "Delay.h"

/* --------------------------------------------------------------------------
 * 函数功能：配置 TIM1 更新中断的 NVIC 优先级
 * 参数：无　　返回值：无
 * 说明：本工程实际用的中断（TIM2/TIM3 更新中断）在 Timer.c 中各自配置了
 * NVIC，本函数是"高级定时器 TIM1"的示例配置，写法与它们完全一致：
 *   ① 选择分组方式（整个工程设置一次即可，重复设置不影响）
 *   ② 选择要配置的中断通道（TIM1 更新中断）
 *   ③ 设置抢占优先级 = 1、响应优先级 = 1
 *   ④ 调用 NVIC_Init() 写入寄存器
 * ------------------------------------------------------------------------ */
void NVICtim1_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);   // 优先级分组：组 2
	                                       // （2 位抢占优先级 + 2 位响应优先级）
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;   // 中断通道：TIM1 更新中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      // 使能该中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;   // 抢占优先级 = 1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;          // 响应优先级 = 1
	NVIC_Init(&NVIC_InitStructure);            // 把上面的配置写入 NVIC 寄存器
}
