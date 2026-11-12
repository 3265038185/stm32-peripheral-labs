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
 *   这是 stm32f10x_it.c 对应的头文件，作用只有一个：
 *   声明 stm32f10x_it.c 里的各个"中断服务函数"，方便别的文件调用。
 *
 * 【头文件格式回顾】
 *   #ifndef / #define / #endif 三件套叫"头文件保护"：
 *   防止同一个头文件被重复包含（include）两次导致重复定义报错。
 *   第一次包含时 __STM32F10x_IT_H 还没有定义，就执行下面的内容并定义它；
 *   以后再包含时发现宏已定义，整个文件内容直接跳过，不会重复。
 *
 * 【extern "C" 是什么？】
 *   这是给 C++ 编译器看的：告诉它"这里面的函数是按 C 语言的规则编译的"。
 *   我们写单片机程序用 C 语言，这个条件编译永远不会生效，
 *   但保留它是标准模板的做法，能让代码兼容 C++ 工程。
 * ========================================================================== */

/* Define to prevent recursive inclusion -------------------------------------*/
/* 上面的英文注释意思是："定义一个宏，防止头文件被重复包含" */
#ifndef __STM32F10x_IT_H    // 如果宏 __STM32F10x_IT_H 还没有被定义（第一次包含本文件）
#define __STM32F10x_IT_H    // 就定义这个宏，并继续向下执行

#ifdef __cplusplus          // 如果是 C++ 编译器（__cplusplus 宏会被自动定义）
 extern "C" {               // 就用 C 语言的链接规则来编译下面的声明
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"      // 包含 STM32F103 标准库主头文件

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* 上面的英文注释意思是："导出类型/常量/宏/函数声明"，
 * 也就是"本模块对外开放的接口"都放在下面 */

/* 下面声明所有中断服务函数（函数原型），声明后其他 .c 文件才能调用它们 */
void NMI_Handler(void);          // 不可屏蔽中断
void HardFault_Handler(void);    // 硬件错误中断
void MemManage_Handler(void);    // 内存管理错误中断
void BusFault_Handler(void);     // 总线错误中断
void UsageFault_Handler(void);   // 用法错误中断
void SVC_Handler(void);          // 系统服务调用中断
void DebugMon_Handler(void);     // 调试监视中断
void PendSV_Handler(void);       // 可挂起系统服务中断
void SysTick_Handler(void);      // 系统滴答定时器中断

#ifdef __cplusplus          // 对应文件开头的 #ifdef，这里收尾
}
#endif

#endif /* __STM32F10x_IT_H */   // 头文件保护的"出口"，与开头的 #ifndef 配对

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
