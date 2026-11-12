/* ============================================================================
 * 文件说明（PWM.c —— PWM 呼吸灯驱动文件）
 * ----------------------------------------------------------------------------
 * 硬件连接：TIM4 的通道 3（CH3）复用输出到 PB8，接一个 LED
 * 用到的外设与知识点：
 *   定时器 TIM4 的 PWM 输出模式（PWM1 模式）
 *   GPIO 复用推挽输出（GPIO_Mode_AF_PP —— 引脚控制权交给定时器）
 *   PWM 原理：计数器从 0 数到 ARR，计数值小于 CCR 时输出高电平、
 *             大于等于 CCR 时输出低电平。改变 CCR 就改变了"占空比"
 *             （高电平所占的比例），LED 的平均电流随之变化 → 亮度变化
 * 三个重要公式（PWM 参数）：
 *   占空比 = CCR / (ARR+1)              → 决定 LED 亮度
 *   频率   = CLK / (PSC+1) / (ARR+1)    → 高于几十 kHz 人眼就看不到闪烁
 *   分辨率 = 1 / (ARR+1)                → 占空比可细分的档位
 * 本配置：频率 = 72MHz / 720 / 100 = 1000Hz = 1kHz
 *         占空比可调 0%~100%，最小档位 1%
 * ========================================================================== */
#include "stm32f10x.h"   
#include "Delay.h"// Device header
//unsigned int arr = 100 - 1,psc = 720 - 1,ccr=50;   // （曾用变量的写法，已注释掉）
//占空比= ccr/（arr+1）
//频率= clk/(psc+1)/(arr+1)
//分辨率 = 1/（arr+1）
unsigned int i=0,k;         // 全局变量：i 用于占空比循环，k 用于呼吸次数循环

/* --------------------------------------------------------------------------
 * 函数功能：初始化 TIM4 的 PWM 输出（通道3 -> PB8）
 * 参数：无　　返回值：无
 * 配置流程（4 步）：
 *   ① 开时钟：TIM4 在 APB1 总线（时钟 72MHz），GPIOB 在 APB2 总线
 *   ② 配引脚：PB8 设为"复用推挽输出"，把引脚控制权交给定时器
 *   ③ 配时基：预分频 720-1、重装值 100-1 → 计数频率 100kHz，PWM 频率 1kHz
 *   ④ 配输出比较：PWM1 模式、高电平有效、输出使能、初始占空比 0
 * ------------------------------------------------------------------------ */
void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);   // 开启 TIM4 的外设时钟（APB1 总线）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); // 开启 GPIOB 的时钟（APB2 总线）
	                                       // 注意：TIM4 和 GPIOB 挂的总线不同，
	                                       // 所以要用两个不同的开时钟函数
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   // 复用推挽输出：
	                                       // 普通输出时引脚由"ODR 寄存器"控制；
	                                       // 复用输出时引脚改由片上外设（TIM4）控制
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;         // 用到的引脚：PB8（TIM4_CH3 的复用引脚）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 输出速度 50MHz
	GPIO_Init(GPIOB,&GPIO_InitStructure);   // 把配置写入 GPIOB 寄存器
	
	TIM_InternalClockConfig(TIM4);          // 选择内部时钟（72MHz）作为计数脉冲源
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;  // 时钟分频 1（不影响定时）
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数（0→ARR）
	TIM_TimeBaseInitStructure.TIM_Period =100  -1 ;   // 自动重装值 ARR = 99
	                                       // → 计数器从 0 数到 99，共 100 个档位
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;  // 预分频 PSC = 719
	                                       // → 计数频率 = 72MHz / 720 = 100kHz
	                                       // → PWM 频率 = 100kHz / 100 = 1kHz
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;  // 重复计数填 0（通用定时器不用）
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);    // 写入 TIM4 的寄存器
	//TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);   // （旧版本：开 TIM1 更新中断，已注释）
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);  // 先把结构体填充为默认值，
	                                         // 防止有成员忘赋值而出现未知结果
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  // PWM 模式 1：
	                                         // 计数器 CNT < CCR 时输出高电平，
	                                         // CNT >= CCR 时输出低电平
	TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;  // 输出极性：高电平有效
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  // 使能 PWM 输出
	TIM_OCInitStructure.TIM_Pulse = 0;       // 初始比较值 CCR = 0 → 初始占空比 0%（LED 灭）
	
	TIM_OC3Init(TIM4,&TIM_OCInitStructure);  // 把配置写入 TIM4 的"通道 3"（CH3 -> PB8）
	//TIM_OC2Init(TIM1,&TIM_OCInitStructure);   // （旧版本：配置 TIM1 的其他通道，
	//TIM_OC3Init(TIM1,&TIM_OCInitStructure);   //  已注释掉，可忽略）
	//TIM_OC4Init(TIM1,&TIM_OCInitStructure);
	TIM_Cmd(TIM4,ENABLE);                    // 启动 TIM4！计数器开始工作，PWM 开始输出
}

/* --------------------------------------------------------------------------
 * 函数功能：呼吸灯效果 —— LED 亮度从暗到亮、再从亮到暗，循环 3 遍
 * 参数：无　　返回值：无
 * 实现原理：
 *   ① 用 TIM_SetCompare3(TIM4, i) 改变比较值 CCR，占空比随之改变：
 *      占空比 = i / 100，i 从 0 增大到 100 → LED 由灭逐渐变亮
 *   ② 每一档停留 Dms(10)（约 10ms），让亮度变化平滑可见
 *   ③ 再从 100 减到 0 → LED 逐渐变暗，完成一次"呼吸"
 *   ④ 外层 for(k=0;k<3;k++) 让"亮→暗"的呼吸过程重复 3 次
 * 注意：呼吸过程中程序一直占用 CPU（软件延时），期间不响应其他任务
 * ------------------------------------------------------------------------ */
void PWM_LED(void)
{
	for(k=0;k<3;k++)                        // 整个呼吸过程循环 3 次
	{
	for(i=0;i<=100;i++)                     // 占空比从 0% 慢慢加到 100%
		{
			TIM_SetCompare3(TIM4,i);        // 设置通道 3 的比较值 = i
			                                // → 占空比 = i/100，LED 亮度随 i 增大
			Dms(10);                        // 每一档停留约 10ms（防止亮度突变）
		}
		for(i=0;i<=100;i++)                 // 占空比从 100% 慢慢减到 0%
		{
			TIM_SetCompare3(TIM4,100-i);    // 设置比较值 = 100-i（i 增大 → 比较值减小）
			Dms(10);                        // 每一档停留约 10ms
		}
	}
	
	
	
	
	
// 下面是被注释掉的"旧版本"：用 TIM1 的 4 个通道依次做呼吸灯，
	// 与当前使用的 TIM4 通道3 方案原理完全相同，可参考理解
//	TIM_CCxCmd(TIM1,TIM_Channel_1,TIM_CCx_Enable);
//		for(i=0;i<=100;i++)
//		{
//			TIM_SetCompare1(TIM1,i);
//			Dms(10);
//		}
//		for(i=0;i<=100;i++)
//		{
//			TIM_SetCompare1(TIM1,100-i);
//			Dms(10);
//		}
//	TIM_CCxCmd(TIM1,TIM_Channel_2,TIM_CCx_Enable);
//		for(i=0;i<=100;i++)
//		{
//			TIM_SetCompare1(TIM1,i);
//			Dms(10);
//		}
//		for(i=0;i<=100;i++)
//		{
//			TIM_SetCompare1(TIM1,100-i);
//			Dms(10);
//		}
//	TIM_CCxCmd(TIM1,TIM_Channel_3,TIM_CCx_Enable);
//		for(i=0;i<=100;i++)
//		{
//			TIM_SetCompare1(TIM1,i);
//			Dms(10);
//		}
//		for(i=0;i<=100;i++)
//		{
//			TIM_SetCompare1(TIM1,100-i);
//			Dms(10);
//		}
//	TIM_CCxCmd(TIM1,TIM_Channel_4,TIM_CCx_Enable);
//		for(i=0;i<=100;i++)
//		{
//			TIM_SetCompare1(TIM1,i);
//			Dms(10);
//		}
//		for(i=0;i<=100;i++)
//		{
//			TIM_SetCompare1(TIM1,100-i);
//			Dms(10);
//		}
}
//void PWM_Set   // （原来准备写的"设置占空比函数"，未完成，已注释掉）
