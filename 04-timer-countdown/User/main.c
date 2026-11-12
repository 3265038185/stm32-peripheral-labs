#include "stm32f10x.h"     
#include "DELAY.h"
#include "OLED.h"
#include "PWM.h"
#include "Timer.h"
#include "Nixie.h"
#include "NVIC.h"
#include "LED.h"

/*====================================================================
 * 文件说明：3-34 定时器作业 —— 秒表/倒计时主程序
 * -------------------------------------------------------------------
 * 功能：用 4 位数码管做一个"分钟:秒"的倒计时表。
 *   1. 按键设置时间：PB9 每按一次分钟 +1，PB4 / PC15 每按一次秒 +1；
 *   2. 按键启动/暂停：PB10 按一次开始倒计时，再按一次暂停；
 *   3. 计时：TIM2 每 1 秒中断一次，秒数 -1；秒数减到 0 时向分钟借位；
 *   4. 显示：TIM3 每 1 毫秒中断一次，负责数码管动态扫描（第 1、2 位
 *      显示分钟，第 3、4 位显示秒），秒位小数点每秒闪烁一次；
 *   5. 倒计时归零后：LED 逐个点亮跑马灯（LED_Run），再执行 PWM
 *      呼吸灯效果（PWM_LED），然后所有状态复位、停止计时。
 * -------------------------------------------------------------------
 * 用到的知识点：定时器（TIM2/TIM3 更新中断）、NVIC 中断优先级、
 * EXTI 外部中断按键、数码管动态扫描、PWM 输出（TIM4）、GPIO 输出。
 * 注意：本文件把中断服务函数集中写在 main.c 里，方便初学者查看，
 *       实际工程建议把 TIM2/TIM3/EXTI 的中断函数分开放到对应模块。
 *====================================================================*/

/* 全局变量说明：
 * Count     —— 已走秒数（TIM2 每秒 +1，满 60 秒向 Tmin 进 1）
 * Shownum1~4 —— 数码管第 1~4 位当前要显示的数字（由 TIM3 计算）
 * Site      —— 动态扫描位置（1~4 循环，TIM3 每 1ms 切换一位）
 * Min       —— 设定的分钟数（倒计时起始值，用按键调整）
 * Sec       —— 设定的秒数（倒计时起始值，用按键调整）
 * timstate  —— 计时器状态：0=停止，1=运行（按键切换）
 * Dot       —— 小数点标志：1=秒位要显示小数点（每秒闪烁一次）
 * time      —— 小数点闪烁计时（亮 50ms 后熄灭）
 * Tmin      —— 已经走完的分钟数（剩余分钟 = Min - Tmin）
 * NowMin / NowSec —— 预留变量（本工程未使用）
 * Lightstate—— 计时正在进行标志（用于判断是否已归零）
 * ii        —— 预留变量（本工程未使用） */
unsigned int Count = 0;
unsigned int Shownum1 = 0,Shownum2 = 0,Shownum3 = 0,Shownum4 = 0;
unsigned int Site = 0,Min=0,Sec = 0;
unsigned int timstate = 0,Dot = 0,time=0,Tmin= 0,NowMin = 0,NowSec=0,Lightstate=0,ii=0;
/* 主函数：只做初始化。初始化完成后程序没有 while 死循环，
 * 计时、扫描、按键响应等所有工作都交给定时器和 EXTI 中断完成。 */
int main(void)
{

//	
	/* 初始化数码管：PA0~PA7 段码输出（共阳，低电平点亮），PB12~PB15 位选输出 */
	Nixie_Init();
	/* 初始化外部中断按键（EXTI）：给 PB4/PB9/PB10 等按键配置中断
	 * （函数声明在 NVIC.h，本工程 NVIC.c 中见 NVICtim1_Init 参考写法） */
	NVICKey_Init();
//Nixie_ShowNum(2,3);
	/* 初始化定时器2：每 1 秒进一次更新中断，负责秒的走时 */
	Timer2_Init();
	/* 初始化定时器3：每 1 毫秒进一次更新中断，负责数码管动态扫描 */
	Timer3_Init();
	/* 初始化 LED：PA8~PA11 推挽输出，低电平点亮（初始全部熄灭） */
	LED_Init();

	
	/* 初始化 PWM：用 TIM4 的通道 3（PB8）输出 PWM 波形，
	 * 给呼吸灯效果用。PB8 与 Nixie 的段码不冲突 */
	PWM_Init();

	//PWM_LED();
	
}
/*====================================================================
 * TIM2 更新中断服务函数（每 1 秒自动进入一次）
 * -------------------------------------------------------------------
 * 作用：秒的"走时"。
 *   1. 记录已走秒数 Count（满 60 秒进 1 分钟 Tmin）；
 *   2. 置 Dot = 1，通知 TIM3 让秒位小数点亮起来（每秒闪一下）；
 *   3. 判断倒计时是否走完：当 4 位数码管要显示的数字全是 0，
 *      说明时间到了，于是跑 LED 跑马灯 + PWM 呼吸灯，然后复位
 *      所有变量并停止 TIM2（倒计时结束，不再走时）。
 *====================================================================*/
void TIM2_IRQHandler(void)
{
	
		
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)
	{
		Lightstate = 1;		// 标记"计时正在进行中"
		Count++;			// 已走秒数 +1
		Dot = 1;			// 请求显示小数点（让它闪一下）
		if(Count>60)		// 满 60 秒 = 1 分钟
		{
			Tmin++;			// 已走的分钟数 +1
			Count = 0;		// 秒数清零，重新从 0 开始计
		}
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
		/* 判断倒计时是否走完：Shownum1~4 是 TIM3 算好的
		 * 剩余时间显示值，4 位全为 0 说明时间已归零 */
		if(Lightstate!=0&&Shownum1 ==0&&Shownum2 ==0&&Shownum3 ==0&&Shownum4 ==0)
		{
			
			TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
			LED_Run();		// 时间到！LED 跑马灯效果
			PWM_LED();		// 再执行 PWM 呼吸灯效果
			/* 下面把倒计时相关变量全部复位，并关闭 TIM2，
			 * 让系统回到"待设置"状态，等待下次按键启动 */
			Tmin = 0;
			Min = 0;
			TIM_Cmd(TIM2,DISABLE);	// 关闭 TIM2（停止走时）
			Lightstate = 0;
			Count = 0;
			Dot = 0;
			Site = 0;
			Min=0;
			Sec = 0;
			time = 0;
			//TIM_Cmd(TIM3,DISABLE);
		}
	}
	
}

/*====================================================================
 * TIM3 更新中断服务函数（每 1 毫秒自动进入一次）
 * -------------------------------------------------------------------
 * 作用：数码管"动态扫描"。
 *   1. 先根据 Min/Tmin/Count 算出剩余时间的 4 位显示数字；
 *   2. 消隐（熄灭所有位）→ 按 Site 点亮对应的那一位；
 *   3. 秒位（第 2 位）根据 Dot 标志显示带小数点的数字，
 *      小数点点亮 50ms 后熄灭，实现每秒闪一下的"冒号"效果；
 *   4. Site 到 5 后回 0，让扫描 1~4 位无限循环。
 *====================================================================*/
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) == SET)
	{
	
		Site ++;		// 扫描位置 +1（1→2→3→4→回到1）
		/* 计算 4 位要显示的数字：
		 * 剩余分钟 = Min - Tmin（设定值 - 已走分钟）
		 * 剩余秒   = Sec - Count（设定值 - 已走秒）
		 * 第1位=分钟的十位，第2位=分钟的个位
		 * 第3位=秒的十位，  第4位=秒的个位 */
		Shownum1 = (Min - Tmin)/10;
		Shownum2 = (Min - Tmin)%10;
		Shownum3 = (Sec - Count)/10;
		Shownum4 = (Sec - Count)%10;
		/* 借位处理：当剩余秒数（Sec - Count）变成负数时，
		 * 说明当前秒已经走完了，向分钟借 1：秒回到 59，分钟 -1 */
		if(Count!=0)
		{
			if(Sec < Count)
			{
				Count=0;
				Sec = 59;
				Min--;
				
			}
		}
		
		/* 消隐：先把 4 个位选引脚全部拉低（熄灭所有位），
		 * 防止切换到下一位时残影串位 */
		GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
		if(Site == 1)
		{
			Nixie_ShowNum(Site,Shownum1);	// 第 1 位：分钟的十位
		}
		if(Site == 2)
		{
			if(Dot==0)		// Dot=0：不显示小数点
			{
			Nixie_ShowNum(Site,Shownum2);	// 第 2 位：分钟的个位
			}
			if(Dot == 1)	// Dot=1：显示带小数点的版本（秒位冒号闪一下）
			{
				Nixie_ShowDotNum(Site,Shownum2);
				time++;
				if(time==50)	// 小数点显示约 50ms 后熄灭
				{
					Dot = 0;
					time = 0;
				}
			}
			
		}
		if(Site == 3)
		{
			
			Nixie_ShowNum(Site,Shownum3);	// 第 3 位：秒的十位
		
		}
		if(Site == 4)
		{
			Nixie_ShowNum(Site,Shownum4);	// 第 4 位：秒的个位
		}
		if(Site==5)		// 扫完第 4 位后回到第 1 位，形成循环扫描
		{
			Site=0;
		}
		
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);	// 清除更新中断标志，防止反复进中断
	}
}
/*====================================================================
 * EXTI15_10_IRQHandler —— 外部中断服务函数（PB10、PC13、PC15 等共用）
 * -------------------------------------------------------------------
 * 进入条件：EXTI 配置的引脚（10~15 号线）上出现电平跳变。
 * 在本工程里用到两路：
 *   EXTI_Line15（PC15）：按键"秒 +1"；
 *   EXTI_Line10（PB10）：按键"启动 / 暂停"倒计时。
 * 注意：同一根中断线只能接一个引脚，PC13 那路已被注释掉不用。
 *====================================================================*/
void EXTI15_10_IRQHandler(void)
{

	
//	if(EXTI_GetITStatus(EXTI_Line13)==SET)
//	{		
//		EXTI_ClearITPendingBit(EXTI_Line13);
//		Dms(20);
//		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)==0)
//		{   Min++;
//			if(Min>99)
//			{
//				Min=0;
//			}
//		}	
//		
//	}
	/* 判断是不是 EXTI_Line15（PC15 按键）引起的中断 */
	if(EXTI_GetITStatus(EXTI_Line15)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line15);	// 先清中断标志（必须！）
		Dms(20);	// 软件延时 20ms 消抖，等按键电平稳定
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15)==0)	// 再次确认按键确实按下
		{   Sec++;			// 秒 +1
			if(Sec>59)		// 超过 59 秒就归零（秒最大 59）
			{
				Sec=0;
			}
		}	
	}
	/* 判断是不是 EXTI_Line10（PB10 按键）引起的中断 */
	if(EXTI_GetITStatus(EXTI_Line10)==SET)
	{
		NowMin = Min;	// 记录当前设定值（本工程暂未使用）
		NowSec = Sec;
		EXTI_ClearITPendingBit(EXTI_Line10);	// 清中断标志
		Dms(20);	// 消抖
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10)==0)
		{	if(timstate==0)		// 当前是停止状态 → 启动计时
			{
				TIM_Cmd(TIM2,ENABLE);	// 使能 TIM2，开始走秒
			timstate = 1;		// 状态改为"运行中"
			}
			else 				// 当前是运行状态 → 暂停计时
			{TIM_Cmd(TIM2,DISABLE);	// 关闭 TIM2，暂停走秒
			timstate = 0;		// 状态改为"已停止"
			}
		}
	}
	
}

//void EXTI4_IRQHandler(void)
//{
//	if(EXTI_GetITStatus(EXTI_Line4)==SET)
//	{		
//		EXTI_ClearITPendingBit(EXTI_Line4);
//		Dms(20);
//		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)==0)
//		{  
//			TIM_Cmd(TIM2,DISABLE);
//			Min=NowMin ;
//			Sec = 0;
//			Count= 0;
//			timstate = 0;
//			Dot = 0;
//			Tmin= 0;
//			time=0;
//		}	
//		
//	}
//}

/*====================================================================
 * EXTI4_IRQHandler —— 外部中断 4 服务函数（PB4 按键）
 * -------------------------------------------------------------------
 * 作用：PB4 按键每按一次，秒 +1（用于设定倒计时的秒数）。
 * 注：上方注释掉的旧版函数原本是"复位计时"功能，现在已简化。
 *====================================================================*/
void EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4)==SET)
	{		
		EXTI_ClearITPendingBit(EXTI_Line4);
		Dms(20);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)==0)
		{  
			Sec++;
			if(Sec>59)
			{
				Sec=0;
			}
		}	
		
	}
}

/*====================================================================
 * EXTI9_5_IRQHandler —— 外部中断 9~5 服务函数（PB9 按键）
 * -------------------------------------------------------------------
 * 作用：PB9 按键每按一次，分钟 +1（用于设定倒计时的分钟数，
 *       最大 99 分钟，超过自动回到 0）。
 *====================================================================*/
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line9)==SET)
	{		
		EXTI_ClearITPendingBit(EXTI_Line9);	// 清中断标志
		Dms(20);	// 软件消抖 20ms
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)==0)	// 确认按键按下
		{   Min++;			// 分钟 +1
			if(Min>99)		// 超过 99 归零
			{
				Min=0;
			}
		}	
	}
}

	
