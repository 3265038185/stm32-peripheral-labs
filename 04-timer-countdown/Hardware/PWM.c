#include "stm32f10x.h"   
#include "Delay.h"// Device header
//unsigned int arr = 100 - 1,psc = 720 - 1,ccr=50;

/*====================================================================
 * 文件说明：PWM 输出模块（用 TIM4 的通道 3 产生 PWM 波形）
 * -------------------------------------------------------------------
 * 什么是 PWM？PWM（脉宽调制）就是让引脚按一定周期输出"高低电平
 * 交替"的方波，通过改变"高电平占整个周期的时间比例"（占空比），
 * 就可以控制 LED 的亮度（呼吸灯）、电机转速、蜂鸣器音量等。
 *
 * 三个关键参数（记住这三个公式！）：
 *   频率     = 时钟频率 ÷ (PSC+1) ÷ (ARR+1)   —— 波形多快重复一次
 *   占空比   = CCR ÷ (ARR+1)                  —— 高电平占的比例
 *   分辨率   = 1 ÷ (ARR+1)                    —— 占空比能细分到多少
 *
 * 本工程配置：时钟 72MHz，PSC=719，ARR=99 → 频率 1kHz；
 * 占空比用 TIM_SetCompare3 随时修改（0~100 之间变化），
 * 就能得到"由暗到亮、由亮到暗"的呼吸灯效果。
 *====================================================================*/
//占空比= ccr/（arr+1）
//频率= clk/(psc+1)/(arr+1)
//分辨率 = 1/（arr+1）
unsigned int i=0,k;		// i：占空比渐变用的循环变量；k：呼吸灯重复次数

/*====================================================================
 * PWM_Init：PWM 输出初始化
 * -------------------------------------------------------------------
 * 步骤：
 *   1. 开启 TIM4 时钟（APB1）和 GPIOB 时钟（APB2）；
 *   2. 把 PB8 配置为"复用推挽输出"（AF_PP）——因为 PB8 的电平
 *      不再由 GPIO 控制，而是交给 TIM4 通道 3 的波形直接驱动；
 *   3. 配置 TIM4 时基（计数器节奏），决定 PWM 频率；
 *   4. 配置输出比较单元（OC3）：PWM1 模式 + 高电平有效；
 *   5. 启动 TIM4，PWM 波形开始在 PB8 上输出。
 *====================================================================*/
void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);	// 开启 TIM4 时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	// 开启 GPIOB 时钟
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	// 复用推挽输出：引脚交给外设（TIM4）控制
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	// PB8 = TIM4 的通道 3（CH3）输出脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 翻转速度 50MHz
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM4);	// 使用内部时钟（72MHz）
	
	// 时基配置：决定 PWM 的频率
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	// 时钟分频 1 分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数
	TIM_TimeBaseInitStructure.TIM_Period =100  -1 ;	// 自动重装值 ARR = 99
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;	// 预分频器 PSC = 719
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;	// 高级定时器专用，填 0
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
	// PWM 频率 = 72MHz ÷ 720 ÷ 100 = 1kHz（人眼察觉不到闪烁）
	//TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
	// 输出比较结构体：配置"PWM 波形怎么产生"
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);	// 先全部填默认值，再改需要的成员
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	// PWM 模式1：CNT<CCR 输出有效电平，CNT>CCR 输出无效电平
	
	TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;	// 有效电平为高电平（占空比越大越亮）
	
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	// 使能 CH3 输出
	TIM_OCInitStructure.TIM_Pulse = 0;	// 比较值 CCR 初始为 0（先全灭）
	
	TIM_OC3Init(TIM4,&TIM_OCInitStructure);	// 把配置写入 TIM4 的通道 3（对应 PB8）
	//TIM_OC2Init(TIM1,&TIM_OCInitStructure);
	//TIM_OC3Init(TIM1,&TIM_OCInitStructure);
	//TIM_OC4Init(TIM1,&TIM_OCInitStructure);
	TIM_Cmd(TIM4,ENABLE);	// 启动 TIM4，PB8 开始输出 PWM 波形
}

/*====================================================================
 * PWM_LED：呼吸灯效果
 * -------------------------------------------------------------------
 * 原理：占空比 = CCR ÷ (ARR+1)。把比较值 CCR 从 0 慢慢加到 100
 * （LED 由暗变亮），再从 100 减回 0（由亮变暗），重复 3 次。
 * 每次只改 CCR（TIM_SetCompare3），频率和周期不变，波形依然连续。
 * 注意：本函数是"阻塞式"的（里面用 Dms 延时），会一直占用 CPU，
 *       适合作为演示；实际工程建议用中断或状态机实现。
 *====================================================================*/
void PWM_LED(void)
{
	for(k=0;k<3;k++)	// 呼吸效果重复 3 次
	{
	for(i=0;i<=100;i++)	// 占空比从 0% 逐渐升到 100%：灯慢慢变亮
		{
			TIM_SetCompare3(TIM4,i);	// 把通道 3 的比较值设为 i
			Dms(10);	// 每档停留 10ms，让亮度变化可见
		}
		for(i=0;i<=100;i++)	// 占空比从 100% 逐渐降到 0%：灯慢慢变暗
		{
			TIM_SetCompare3(TIM4,100-i);
			Dms(10);
		}
	}
	
	
	
	// 下面是当初调试 TIM1 多通道 PWM 留下的注释代码，未使用：
	//（四路通道同时渐变呼吸，感兴趣可以取消注释试试）
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
//void PWM_Set
