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

/* ============================================================================
 * 中文补充说明（初学者必读）
 * ----------------------------------------------------------------------------
 * 【本文件是干什么的？】
 *   这是 stm32f10x_it.c 的头文件，声明各个"中断服务函数"的原型。
 *
 * 【头文件保护】
 *   #ifndef / #define / #endif 防止同一头文件被重复包含导致重复定义报错。
 *
 * 【extern "C"】
 *   给 C++ 编译器看的：这些函数按 C 语言规则编译。
 *   纯 C 工程中该条件恒为假，不会生效，保留是为了兼容 C++ 工程。
 * ========================================================================== */

/* Define to prevent recursive inclusion -------------------------------------*/
/* 上面的英文注释意思是："定义一个宏，防止头文件被重复包含" */
#ifndef __STM32F10x_IT_H    // 第一次包含时宏未定义 → 继续执行
#define __STM32F10x_IT_H    // 定义宏，防止第二次包含

#ifdef __cplusplus          // 兼容 C++ 工程
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"      // STM32F103 标准库主头文件

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* 上面的英文注释意思是："导出类型/常量/宏/函数声明"，即本模块对外开放的接口 */

/* 下面声明所有中断服务函数（与 stm32f10x_it.c 一一对应） */
void NMI_Handler(void);          // 不可屏蔽中断
void HardFault_Handler(void);    // 硬件错误中断
void MemManage_Handler(void);    // 内存管理错误中断
void BusFault_Handler(void);     // 总线错误中断
void UsageFault_Handler(void);   // 用法错误中断
void SVC_Handler(void);          // 系统服务调用中断
void DebugMon_Handler(void);     // 调试监视中断
void PendSV_Handler(void);       // 可挂起系统服务中断
void SysTick_Handler(void);      // 系统滴答定时器中断

#ifdef __cplusplus          // 与开头 #ifdef 配对收尾
}
#endif

#endif /* __STM32F10x_IT_H */   // 头文件保护出口

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
