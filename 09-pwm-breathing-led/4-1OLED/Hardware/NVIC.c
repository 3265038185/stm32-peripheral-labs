/* ============================================================================
 * 文件说明：NVIC.c —— 按键外部中断（EXTI）驱动
 * ----------------------------------------------------------------------------
 * 【本文件的作用】
 *   用"外部中断"的方式来检测按键：按键按下时引脚产生下降沿，
 *   触发 EXTI 中断，CPU 自动跳进 EXTI15_10_IRQHandler() 记录按键编号，
 *   主程序调用 NVIC_GetKey() 查询即可（对应 main.c 中被注释的"中断按键"方案）。
 *
 * 【用到的外设/知识点】
 *   - AFIO：EXTI 中断线要映射到 GPIO 引脚，需打开 AFIO 时钟
 *   - EXTI：外部中断控制器，本工程用"下降沿触发"（按下瞬间 1→0）
 *   - NVIC：中断控制器，负责使能和优先级管理
 *   - 中断标志位：进入中断后要"查询标志 + 清除标志"，否则反复进中断
 * ========================================================================== */

#include "stm32f10x.h"  
#include "Song.h"// Device header
#include "Delay.h"

/* ----------------------------------------------------------------------------
 * 函数名：NVICKey_Init
 * 功能  ：初始化"外部中断检测按键"所需的所有配置
 * 参数  ：无
 * 返回  ：无
 * 配置顺序：
 *   1. 开时钟：GPIOB + AFIO（EXTI 映射需要 AFIO）
 *   2. 配置 GPIOB12~15 上拉输入
 *   3. GPIO_EXTILineConfig：中断线 12~15 连到 GPIOB 引脚 12~15
 *   4. 配置 EXTI：中断模式 + 下降沿触发 + 使能
 *   5. NVIC_PriorityGroupConfig：优先级分组（必须先于 NVIC_Init）
 *   6. 配置 NVIC：使能 EXTI15_10 中断通道并设优先级
 * -------------------------------------------------------------------------- */
void NVICKey_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   // 开 GPIOB 时钟（按键引脚）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    // 开 AFIO 时钟
	// 为什么必须开 AFIO 时钟？EXTI 中断线选择连到哪组 GPIO，
	// 这个"选择"由 AFIO 的 EXTICR 寄存器完成，不开 AFIO 时钟中断不会触发
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   // 上拉输入：松开为高，按下为低
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	
	/* 下面 4 行：把 EXTI 中断线 15/14/13/12 映射到 GPIOB 的引脚 15/14/13/12。
	 * 注意：每条中断线只能连一个引脚，Line15 连了 PB15 就不能再连别的端口引脚 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource15);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource13);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12);

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15;
	// 选择使用 EXTI 的第 12~15 条中断线
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;           // 使能这些中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断模式（事件模式不打断 CPU）
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	// 下降沿触发：按键按下瞬间电平 1→0，正好产生下降沿
	EXTI_Init(&EXTI_InitStructure);                     // 配置写入寄存器
		

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	// 优先级分组 Group2：2 位抢占优先级 + 2 位子优先级。
	// 抢占优先级高的可以打断低的；必须先设置分组再配置 NVIC！
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	// 中断通道：EXTI 线 10~15 共用中断服务函数 EXTI15_10_IRQHandler
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     // 使能中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 抢占优先级 1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;         // 子优先级 1
	NVIC_Init(&NVIC_InitStructure);
}

unsigned int		Keystate=0;   // 全局按键标志：0=无；1~4=对应按键触发过中断

/* ----------------------------------------------------------------------------
 * 函数名：EXTI15_10_IRQHandler
 * 功能  ：外部中断 15~10 中断服务函数（按键按下时自动执行）
 * 参数  ：无
 * 返回  ：无
 * 原理  ：
 *   1. EXTI_GetITStatus() 查询是哪条中断线触发的（查标志位）；
 *   2. 把按键编号写入全局变量 Keystate；
 *   3. 必须 EXTI_ClearITPendingBit() 清除挂起标志！不清除会反复进中断。
 * -------------------------------------------------------------------------- */
void EXTI15_10_IRQHandler(void)
{
	Keystate=0;                                        // 进来先清零（没匹配到就当无按键）
	//unsigned int timecount=0,KeyState = 0;
	if(EXTI_GetITStatus(EXTI_Line12)==SET)             // Line12 触发 → 按键1(PB12)
	{
		Keystate = 1;                                  // 记录按键编号
		EXTI_ClearITPendingBit(EXTI_Line12);           // 清除标志位（重要！）
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line13)==SET)             // 按键2(PB13)
	{
		Keystate = 2;
		EXTI_ClearITPendingBit(EXTI_Line13);
		
	}
	if(EXTI_GetITStatus(EXTI_Line14)==SET)             // 按键3(PB14)
	{
		Keystate =3;
		EXTI_ClearITPendingBit(EXTI_Line14);
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line15)==SET)             // 按键4(PB15)
	{
		Keystate = 4;
		EXTI_ClearITPendingBit(EXTI_Line15);
		
		
	}
}

/* ----------------------------------------------------------------------------
 * 函数名：NVIC_GetKey
 * 功能  ：主程序查询最近一次触发中断的按键
 * 参数  ：无
 * 返回  ：0=无按键；1~4=对应按键按下过
 * 原理  ：直接返回中断服务函数写入的全局变量 Keystate。
 *         中断负责记录，主程序负责查询，互不阻塞。
 * -------------------------------------------------------------------------- */
unsigned int NVIC_GetKey(void)
{
	return Keystate;
}
