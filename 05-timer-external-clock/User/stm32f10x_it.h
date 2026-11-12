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

/* Define to prevent recursive inclusion -------------------------------------*/
/* 防止头文件被重复包含（一个文件里被 #include 多次也不会报重复定义错误） */
#ifndef __STM32F10x_IT_H
#define __STM32F10x_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* 包含 STM32 芯片型号相关的头文件（里面定义了寄存器、外设结构体等） */
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* 下面这些是中断服务函数（ISR）的"声明"（函数原型），
   它们和 stm32f10x_it.c 里的定义一一对应：
   - 声明写在 .h 里，是为了让其他 .c 文件（比如 main.c）知道"有这个函数，可以调用"；
   - 不过中断服务函数一般是由硬件（中断向量表）自动调用的，不用我们手动调用，
     这里声明主要是为了规范 + 方便编译器检查。
   函数名都是 STM32 启动文件里规定好的，不能随意修改。 */

/* 内核（Cortex-M3）异常处理函数： */
void NMI_Handler(void);			// 不可屏蔽中断
void HardFault_Handler(void);	// 硬件错误（程序崩溃会进这里，通常是死循环）
void MemManage_Handler(void);	// 内存管理异常
void BusFault_Handler(void);	// 总线错误
void UsageFault_Handler(void);	// 用法错误（除零、非法指令等）
void SVC_Handler(void);			// 系统服务调用
void DebugMon_Handler(void);	// 调试监视
void PendSV_Handler(void);		// 可挂起的系统服务调用
void SysTick_Handler(void);		// 系统滴答定时器中断

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_IT_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
