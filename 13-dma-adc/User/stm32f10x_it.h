/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.h 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/*=============================================================================
 * 文件：stm32f10x_it.h（中断服务函数声明头文件）
 * 功能：声明 stm32f10x_it.c 里的所有中断处理函数。
 *
 * 初学者必懂：
 *   1. 这里只做"声明"，真正的函数实现（函数体）在 stm32f10x_it.c 中
 *   2. 头文件用 #ifndef/#define/#endif 防止重复包含
 *   3. extern "C" 是为了兼容 C++ 编译器：告诉编译器这些函数按 C 语言规则
 *      编译和链接（因为启动文件的向量表是按 C 符号规则写的），
 *      我们平时用 C 语言开发，这段没有实际影响，照抄即可
 *=============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_IT_H
#define __STM32F10x_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* 以下是对中断处理函数的声明，声明和定义（stm32f10x_it.c）一一对应，
 * 名字必须一致，否则编译链接会出错 */

void NMI_Handler(void);			//不可屏蔽中断处理函数
void HardFault_Handler(void);	//硬件错误中断处理函数（程序跑飞时进入）
void MemManage_Handler(void);	//内存管理错误中断处理函数
void BusFault_Handler(void);	//总线错误中断处理函数
void UsageFault_Handler(void);	//用法错误中断处理函数
void SVC_Handler(void);			//系统服务调用中断处理函数
void DebugMon_Handler(void);	//调试监视器中断处理函数
void PendSV_Handler(void);		//可挂起系统服务调用中断处理函数
void SysTick_Handler(void);		//系统滴答定时器中断处理函数

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_IT_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
