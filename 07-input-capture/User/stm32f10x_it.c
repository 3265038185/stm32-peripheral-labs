/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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
 * 文件：stm32f10x_it.c（中断服务函数文件）
 * 功能：集中存放所有的"中断服务函数"（ISR，Interrupt Service Routine）。
 *       当单片机发生异常或外设触发中断时，CPU 会暂停当前正在执行的程序，
 *       自动跳转到这里对应的函数中去执行。
 * 本文件中的函数大致分两类：
 *       1. 系统异常处理函数：NMI_Handler（不可屏蔽中断）、HardFault_Handler
 *          （硬件错误）、SysTick_Handler（系统滴答定时器中断）等，这些是
 *          Cortex-M3 内核自带的异常，名字是固定的，不能改；
 *       2. 外设中断处理函数：形如"XX_IRQHandler"的函数，名字由启动文件
 *          （startup_stm32f10x_xx.s）规定，比如 TIM2_IRQHandler 对应定时器2。
 * 注意：本项目的输入捕获/按键中断处理函数写在其他模块中
 *       （如 NVIC.c 里的 EXTI15_10_IRQHandler），所以本文件保持空白模板即可。
 * 知识点：中断向量表、异常处理、IRQHandler 命名规则
 *============================================================================*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	/* 不可屏蔽中断（NMI）：由外部硬件或电源等触发，优先级最高。
	   本项目没有使用，函数体留空即可 */
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* 硬件错误中断：一旦程序访问了非法地址、执行了非法指令等，
     就会跳进这里。里面是死循环，让程序停在这里方便调试（查错）。
     正常运行时这个函数永远不会被调用 */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* 内存管理异常：一般由"访问了不该访问的存储器区域"引起。
     同样用死循环把程序卡住，方便发现问题，本项目未使用 */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* 总线错误：访问外设地址时出错会触发（比如时钟没开启就去操作外设寄存器）。
     死循环卡住程序，方便调试，本项目未使用 */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* 用法错误：执行了未定义的指令或非法状态切换时会触发。
     死循环卡住程序，方便调试，本项目未使用 */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
	/* 系统服务调用异常：操作系统中常用于任务切换（RTOS会用到）。
	   裸机程序中不会触发，函数体留空即可 */
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
	/* 调试监视异常：使用调试器（如J-Link）在线调试时才会用到，本项目未使用 */
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	/* 可挂起的系统服务调用：RTOS任务切换常用，裸机程序不触发，留空即可 */
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	/* 系统滴答定时器中断：每计数溢出一次就触发一次。
	   注意：本项目的延时（Delay.c）是"查询法"实现的（等计数器数完），
	   不依赖这个中断，所以这里保持为空。
	   若以后要用"中断法"延时，就把延时的处理逻辑写在这里 */
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/
/* 上面是被注释掉的"外设中断模板"，名字中PPP是占位符，可以替换成具体外设，
   例如：TIM2_IRQHandler、EXTI15_10_IRQHandler、USART1_IRQHandler 等。
   中断函数的具体名字必须和启动文件（startup_stm32f10x_xx.s）里
   中断向量表中写好的名字一模一样，否则中断来了找不到函数，程序会跑飞。
   本项目真正用到的外设中断（按键EXTI中断）写在 Hardware\NVIC.c 中 */

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
