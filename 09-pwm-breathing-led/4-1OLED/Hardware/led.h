/* ============================================================================
 * 文件说明：led.h —— LED 引脚定义头文件（对应 led.c）
 * ----------------------------------------------------------------------------
 * 【LED 引脚分配】
 *   LED0~LED7 → PA0~PA7（用位带宏 PAout(0~7) 控制）
 *   LED8、LED9 → PB0、PB1（用位带宏 PBout(0/1) 控制）
 *
 * 【用法示例】
 *   LED0 = 0;   // PA0 输出低电平 → LED0 点亮（低电平点亮）
 *   LED3 = 1;   // PA3 输出高电平 → LED3 熄灭
 *   LED8 = 0;   // PB0 输出低电平 → LED8 点亮（注意要先初始化 GPIOB！）
 *
 * 【位带宏说明】
 *   PAout(n)/PBout(n) 来自 sys.h：把 ODR（输出数据寄存器）的某一位
 *   映射成独立 32 位地址，赋值 0/1 就是直接控制该引脚输出。
 * ========================================================================== */

#ifndef __LED_H            // 头文件保护，防止重复包含
#define __LED_H	 
#include "sys.h"           // 引入位带操作宏 PAout/PBout

#define LED0 PAout(0)      // PA0 输出口（低电平点亮）
#define LED1 PAout(1)      // PA1 输出口
#define LED2 PAout(2)      // PA2 输出口
#define LED3 PAout(3)      // PA3 输出口
#define LED4 PAout(4)      // PA4 输出口
#define LED5 PAout(5)      // PA5 输出口
#define LED6 PAout(6)      // PA6 输出口
#define LED7 PAout(7)      // PA7 输出口
#define LED8 PBout(0)      // PB0 输出口
#define LED9 PBout(1)      // PB1 输出口


void LED_Init(void);       // 初始化 LED 引脚（推挽输出，初始熄灭）

		 				    
#endif

















