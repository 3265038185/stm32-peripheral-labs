/*****************************************************************************
 * 文件名：led.h（System 系统层头文件）
 * 所属工程：1-LED闪烁
 *
 * 【文件功能】
 *   给 10 个 LED 起"名字"：把 LED0~LED9 定义成对应的引脚位带操作宏，
 *   这样代码里直接写 LED0 = 0 / LED0 = 1 就能控制 LED，非常直观。
 *
 * 【引脚分配表】
 *   LED0~LED7 → PA0~PA7（GPIOA 端口）
 *   LED8、LED9 → PB0、PB1（GPIOB 端口）
 *
 * 【LED0~LED9 到底是什么？】
 *   它们来自 sys.h 的位带操作：
 *     PAout(n) = 直接写 PA 口第 n 位的输出电平（0=低电平，1=高电平）；
 *     PBout(n) = 直接写 PB 口第 n 位的输出电平。
 *   因为 LED 是"低电平点亮"，所以：
 *     LEDx = 0 → 引脚输出低电平 → LED 亮；
 *     LEDx = 1 → 引脚输出高电平 → LED 灭。
 *
 * 【使用前提】
 *   使用前必须先调用 LED_Init()（led.c 中）完成引脚配置，
 *   否则直接给这些宏赋值不会有效果。
 *****************************************************************************/
#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

//宏定义：把"引脚位带操作"改名为好记的 LED 名字
#define LED0 PAout(0)   //LED0 = PA0 引脚（输出电平操作）
#define LED1 PAout(1)   //LED1 = PA1 引脚
#define LED2 PAout(2)   //LED2 = PA2 引脚
#define LED3 PAout(3)   //LED3 = PA3 引脚
#define LED4 PAout(4)   //LED4 = PA4 引脚
#define LED5 PAout(5)   //LED5 = PA5 引脚
#define LED6 PAout(6)   //LED6 = PA6 引脚
#define LED7 PAout(7)   //LED7 = PA7 引脚
#define LED8 PBout(0)   //LED8 = PB0 引脚
#define LED9 PBout(1)   //LED9 = PB1 引脚


//LED 初始化函数（实现在 led.c）：配置引脚为推挽输出并默认全灭
void LED_Init(void);

		 				    
#endif

















