/*=============================================================================
 * 文件：LED.h（LED.c 的头文件）
 * 功能：声明 LED 模块的函数，供 main.c 等文件调用。
 * 说明：本项目的 LED 接在 PA1、PA2 引脚上，低电平点亮（见 LED.c 注释）
 *=============================================================================*/
#ifndef __LED_H			//如果没定义过 __LED_H（头文件还没被包含过）
#define __LED_H			//就定义 __LED_H，并执行下面的声明

void LED_Init(void);	//声明：LED初始化函数
void LED1_ON(void);		//声明：LED1点亮（PA1输出低电平）
void LED1_OFF(void);	//声明：LED1熄灭（PA1输出高电平）
void LED1_Turn(void);	//声明：LED1状态翻转（亮变灭、灭变亮）
void LED2_ON(void);		//声明：LED2点亮（PA2输出低电平）
void LED2_OFF(void);	//声明：LED2熄灭（PA2输出高电平）
void LED2_Turn(void);	//声明：LED2状态翻转（亮变灭、灭变亮）

#endif					//结束条件编译
