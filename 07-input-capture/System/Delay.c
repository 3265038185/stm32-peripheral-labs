/*==============================================================================
 * 文件：Delay.c（软件延时模块）
 * 功能：提供微秒/毫秒/秒三个级别的延时函数，全部基于"系统滴答定时器"
 *       SysTick（Cortex-M3内核自带的一个24位倒计时定时器）实现。
 * 原理：SysTick 从 LOAD 设定的值开始往下数，数到0时 COUNTFLAG 标志置1。
 *       HCLK（系统主时钟）=72MHz，也就是1us数72个时钟周期，
 *       所以延时 xus 微秒 = 让SysTick倒数 72*xus 次。
 * 注意：本延时函数是"查询方式"（死等），会阻塞CPU；
 *       若以后要用中断方式延时，可以开SysTick中断在中断里计数。
 * 知识点：SysTick定时器、CTRL/LOAD/VAL寄存器、查询法延时
 *============================================================================*/
#include "stm32f10x.h"	// STM32头文件（SysTick寄存器定义在这里面）

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{
	SysTick->LOAD = 72 * xus;				// 设置定时器重装值
											// 72MHz=1us数72个数，所以延时xus微秒
											// 就让SysTick从72*xus往下倒计数
	SysTick->VAL = 0x00;					// 清空当前计数值（清零后再启动才准确）
	SysTick->CTRL = 0x00000005;				// 设置时钟源为HCLK，启动定时器
											// CTRL寄存器：bit0=ENABLE启动，
											// bit2=CLKSOURCE选HCLK(72MHz)，
											// 5 = 0b101，即选HCLK并启动
	while(!(SysTick->CTRL & 0x00010000));	// 等待计数到0
											// bit16=COUNTFLAG：数到0时为1，
											// 没数完为0，所以"!"取反后死等
	SysTick->CTRL = 0x00000004;				// 关闭定时器
											// 只写bit2（清bit0的ENABLE），
											// 复位CTRL，防止影响下次延时
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)	// 循环xms次（先用后减：xms为0时循环体不执行）
	{
		Delay_us(1000);	// 1毫秒 = 1000微秒，调用微秒延时实现
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)	// 循环xs次
	{
		Delay_ms(1000);	// 1秒 = 1000毫秒，调用毫秒延时实现
	}
} 
