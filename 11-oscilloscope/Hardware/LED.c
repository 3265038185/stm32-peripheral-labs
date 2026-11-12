/*=============================================================================
 * 文件：LED.c（LED 外设驱动文件）
 * 功能：初始化板载 LED 并控制其亮灭。
 *
 * 用到的外设/知识点：
 *   1. GPIO 推挽输出（Out_PP）：可以主动输出高电平或低电平，驱动能力强，
 *      适合直接驱动 LED。配置后引脚就变成一个"可编程开关"
 *   2. 电路接法：LED 阳极经电阻接 3.3V，阴极接引脚。
 *      所以"低电平点亮、高电平熄灭"（与直觉相反，注意区分）
 *   3. GPIO_SetBits   —— 置位函数，让引脚输出高电平（BSRR 寄存器）
 *      GPIO_ResetBits —— 复位函数，让引脚输出低电平（BRR 寄存器）
 *
 * 注：本项目的功能以"定时采样 + 显示波形"为主，主程序暂未使用 LED，
 *     此文件供扩展使用（如采样时亮灯指示）
 *=============================================================================*/
#include "stm32f10x.h"                  // Device header

/**
  * 函    数：LED初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：把 PA1、PA2 配置成推挽输出，并默认输出高电平（LED 熄灭）
  */
void LED_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOA的时钟
	/* 原理：GPIOA 挂载在 APB2 总线上，不打开时钟，寄存器写进去也不生效 */
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//模式：通用推挽输出
	/* 原理：推挽输出 = 内部两个 MOS 管一推一拉，
	 * 输出高电平时上面的管子导通（把引脚拉到 3.3V），
	 * 输出低电平时下面的管子导通（把引脚拉到 0V），
	 * 输出能力强，可以直接点亮 LED */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;	//引脚：PA1（LED1）和 PA2（LED2）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度：50MHz 翻转速率（驱动 LED 完全够用）
	GPIO_Init(GPIOA, &GPIO_InitStructure);						//将PA1和PA2引脚初始化为推挽输出
	
	/*设置GPIO初始化后的默认电平*/
	GPIO_SetBits(GPIOA, GPIO_Pin_1 | GPIO_Pin_2);				//设置PA1和PA2引脚为高电平
	/* 原理：初始化完成后引脚电平不确定，先统一置高，
	 * 因为板子上的 LED 是低电平点亮，默认高电平 = 上电时 LED 全灭 */
}

/**
  * 函    数：LED1开启
  * 参    数：无
  * 返 回 值：无
  * 说    明：PA1 输出低电平，LED1 点亮（因为 LED 阴极接 PA1）
  */
void LED1_ON(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);		//设置PA1引脚为低电平
	/* ResetBits 即"复位位"：把 ODR（输出数据寄存器）对应位清零，引脚输出 0V */
}

/**
  * 函    数：LED1关闭
  * 参    数：无
  * 返 回 值：无
  * 说    明：PA1 输出高电平，LED1 熄灭
  */
void LED1_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_1);		//设置PA1引脚为高电平
	/* SetBits 即"置位"：把 ODR（输出数据寄存器）对应位置 1，引脚输出 3.3V */
}

/**
  * 函    数：LED1状态翻转
  * 参    数：无
  * 返 回 值：无
  * 说    明：若 LED1 当前点亮则熄灭，若熄灭则点亮（读当前状态后取反）
  */
void LED1_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_1) == 0)		//获取输出寄存器的状态，如果当前引脚输出低电平
	/* 原理：读的是"输出数据寄存器"ODR，代表 CPU 上一次写出的电平，
	 * 不是读引脚实际电平；用这个来判断当前 LED 是亮还是灭 */
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_1);					//则设置PA1引脚为高电平（LED 灭）
	}
	else													//否则，即当前引脚输出高电平
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);					//则设置PA1引脚为低电平（LED 亮）
	}
}

/**
  * 函    数：LED2开启
  * 参    数：无
  * 返 回 值：无
  * 说    明：PA2 输出低电平，LED2 点亮
  */
void LED2_ON(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);		//设置PA2引脚为低电平
}

/**
  * 函    数：LED2关闭
  * 参    数：无
  * 返 回 值：无
  * 说    明：PA2 输出高电平，LED2 熄灭
  */
void LED2_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_2);		//设置PA2引脚为高电平
}

/**
  * 函    数：LED2状态翻转
  * 参    数：无
  * 返 回 值：无
  * 说    明：若 LED2 当前点亮则熄灭，若熄灭则点亮
  */
void LED2_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == 0)		//获取输出寄存器的状态，如果当前引脚输出低电平
	{                                                  
		GPIO_SetBits(GPIOA, GPIO_Pin_2);               		//则设置PA2引脚为高电平（LED 灭）
	}                                                  
	else                                               		//否则，即当前引脚输出高电平
	{                                                  
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);             		//则设置PA2引脚为低电平（LED 亮）
	}
}
