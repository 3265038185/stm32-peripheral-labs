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
/* =============================================================================
 * 【中文说明】这是 stm32f10x_it.c 对应的头文件，声明所有中断服务函数。
 *
 * 头文件的作用：
 *   1. 让其他 .c 文件（编译单元）知道"这些函数存在"，以便调用；
 *   2. 本文件里的函数都是"中断服务函数"，它们由硬件（中断向量表）
 *      自动调用，不需要别的代码显式调用，所以这里的声明更多是
 *      "自报家门"的作用，保证链接时函数名一致、类型正确。
 *
 * 中断服务函数名是怎么跟硬件挂钩的？
 *   启动文件 startup_stm32f10x_md.s 中有一张"中断向量表"，
 *   里面按顺序列出了每个中断号对应的函数名。只要你的 C 文件里
 *   定义了同名函数（例如 SysTick_Handler），链接器就会把它填进向量表。
 *   因此：函数名必须和向量表里的一模一样，不能乱改！
 * ============================================================================= */
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

//============================================================
// 下面是所有"内核异常/中断"处理函数的声明。
// 分两组理解：
//   - 内核异常（NMI、HardFault 等 9 个）：Cortex-M3 内核自带的，
//     名字和数量是固定的，由 ARM 架构规定，不能删也不能改名；
//   - 外设中断（EXTI、TIM、USART 等）：模板里以注释形式给出
//     示例，本工程实际用的 EXTI1_IRQHandler 定义在 main.c。
// 这些函数的实现在 stm32f10x_it.c 里（目前大多为空函数占位）。
//============================================================
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

#endif /* __STM32F10x_IT_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
