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

/*===============================================================================
 * 中文说明（初学必读）：
 * 这个文件是"中断服务函数（ISR）的声明头文件"。
 * 头文件的作用：把中断服务函数的"名字"（函数原型）提前告诉编译器，
 * 这样其他文件（比如 stm32f10x_it.c）在定义这些函数时，
 * 编译器就知道"这个函数是有人用过的"，不会报"未声明"的警告。
 * 注意：
 *   1. 中断服务函数的名字都是 STM32 启动文件 startup_stm32f10x_md.s 里
 *      中断向量表"出厂规定"好的，不能随意修改，改一个字母中断就失效；
 *   2. 这些函数一般由硬件（中断向量表）自动调用，不需要我们手动调用，
 *      这里写声明主要是为了规范 + 方便编译器检查；
 *   3. 本工程（4-1OLED）真正用的 EXTI 按键中断函数
 *      EXTI15_10_IRQHandler 写在 Hardware/NVIC.c 里，不在本文件范围内。
 *===============================================================================
 */

/* Define to prevent recursive inclusion -------------------------------------*/
/* 防止头文件被重复包含：同一个文件被 #include 多次时，第二次进来
   发现 __STM32F10x_IT_H 已经定义过，就直接跳过，避免重复定义错误 */
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
/* 下面这些是中断服务函数（ISR）的"声明"（函数原型）：
   它们和 stm32f10x_it.c 里的定义一一对应。前半部分是 Cortex-M3 内核自带的
   "异常"处理函数，后半部分一般是外设中断函数（本工程用到的 EXTI 按键中断
   写在 NVIC.c 里，所以这里没有列出）。 */
void NMI_Handler(void);			// 不可屏蔽中断（最紧急，本工程不用）
void HardFault_Handler(void);	// 硬件错误（程序崩溃进这里，通常是死循环）
void MemManage_Handler(void);	// 内存管理异常（一般不会触发）
void BusFault_Handler(void);	// 总线错误（访问不存在的地址）
void UsageFault_Handler(void);	// 用法错误（除零、非法指令等）
void SVC_Handler(void);			// 系统服务调用（操作系统才用）
void DebugMon_Handler(void);	// 调试监视异常
void PendSV_Handler(void);		// 可挂起的系统服务调用（操作系统才用）
void SysTick_Handler(void);		// 系统滴答定时器中断（本工程用查询方式，不使能中断）

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_IT_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
