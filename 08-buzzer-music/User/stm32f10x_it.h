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

/*---------------------------------------------------------------------------
  补充说明（中文，适合初学者）：
  ---------------------------------------------------------------------------
  这个头文件是 stm32f10x_it.c 的"配套声明文件"。
  里面只是把中断服务函数"声明"一遍（告诉编译器这些函数存在、长什么样），
  具体"定义"（函数体）在 stm32f10x_it.c 中。
  ---------------------------------------------------------------------------
  常见疑问：
  1. 为什么函数名是 NMI_Handler、HardFault_Handler 这种"固定名字"？
     因为 startup_stm32f10x_xx.s 启动文件里的中断向量表已经把这些名字
     和中断源绑定了，我们只能按这个名字写函数，不能改名。
  2. 我们项目真正用到的中断（EXTI15_10_IRQHandler 按键中断）为什么没在这里？
     因为 Keil 的 C 编译规则允许把同名函数写在别的 .c 文件里，
     作者为了把按键逻辑集中管理，把 EXTI 中断函数写在了 Hardware/NVIC.c。
  3. #ifndef / #define / #endif 的作用：
     防止这个头文件被重复包含（重复声明会报错），
     第一次包含时定义宏 __STM32F10x_IT_H，以后再包含就直接跳过。
---------------------------------------------------------------------------*/

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
