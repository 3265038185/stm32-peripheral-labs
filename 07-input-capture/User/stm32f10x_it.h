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

/*==============================================================================
 * 文件：stm32f10x_it.h（中断服务函数的头文件）
 * 功能：对外声明 stm32f10x_it.c 里所有中断服务函数的原型，
 *       这样其他 .c 文件（比如使用中断的模块）就可以调用这些函数了。
 * 原理：头文件就是"说明书"，告诉编译器"这些函数存在、参数和返回值是什么"。
 *       真正干活的函数体写在对应的 .c 文件中。
 * 知识点：函数原型声明、防止头文件重复包含（#ifndef ... #endif 的作用）
 *============================================================================*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_IT_H	/* 如果还没有定义过 __STM32F10x_IT_H 这个宏 */
#define __STM32F10x_IT_H	/* 就定义它（防止本头文件被重复包含，避免重复声明报错） */

#ifdef __cplusplus			/* 如果是C++编译器（我们用的是C，一般不执行） */
 extern "C" {				/* 就用C语言的规则来链接这些函数 */
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"		/* 包含STM32F10x核心头文件，因为下面声明用到了uint32_t等类型 */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* 下面声明的是各个中断服务函数：
   注意：这些函数的"名字"必须和启动文件里中断向量表中的名字完全一致，
   因为中断触发时CPU是按"名字查表"跳转的，名字写错程序就跑飞了 */
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_IT_H */	/* 头文件保护结束 */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
