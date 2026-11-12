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

/*===============================================================================
 * 文件：stm32f10x_it.c    项目：2-按键中断（STM32F103 标准外设库）
 *
 * 【这个文件是干什么的？】
 *   这是"中断服务函数"的存放文件。中断服务函数（Interrupt Service Routine，
 *   简称 ISR）是硬件触发中断时 CPU 自动跳过去执行的一段代码。
 *   当某个中断事件发生时，CPU 会暂停正在执行的 main() 循环，
 *   跳到本文件里对应的函数去处理，处理完再回到刚才暂停的地方继续干活。
 *
 * 【本工程用到的知识点】
 *   - 中断向量表（在启动文件 startup_stm32f10x_xx.s 里定义）：它记录了
 *     "哪个中断事件 → 跳到哪个函数"。中断函数的名字必须和启动文件里
 *     约定的名字一模一样（例如 EXTI15_10_IRQHandler），名字写错编译不会
 *     报错，但中断来了 CPU 会跳到默认的空函数，功能就不工作了。
 *   - 下面的 NMI_Handler、HardFault_Handler 等是 Cortex-M3 内核自带的
 *     "异常"处理函数（异常可以理解为系统级的紧急中断），本工程基本用不到，
 *     保留空实现即可——因为"函数存在"本身就占住了中断向量表的位置，
 *     防止程序跑飞后 CPU 找不到处理函数而彻底死机。
 *   - 本工程真正用到的中断服务函数 EXTI15_10_IRQHandler（外部中断按键）
 *     写在 Hardware\NVIC.c 里，不在本文件，初学者可以去那里看。
 *===============================================================================*/

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

/*============================================================
 * 【中文说明：下面的函数都是"内核异常"处理函数】
 *   Cortex-M3 内核遇到严重错误时会触发这些"异常"。本工程代码量小、
 *   逻辑简单，正常情况下不会触发它们。留一个空函数（或死循环）的目的：
 *   让中断向量表里"有函数可跳"，万一出了意外，程序能停在一个可控的位置
 *   （死循环里），而不是跳到未知地址彻底崩溃。
 *   初学者只需知道"这些函数暂时用不到，不要删除"即可。
 *============================================================*/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)	// NMI（不可屏蔽中断）处理函数：平时用不到，空着即可
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)	// 硬错误处理函数：程序非法访问内存等严重错误时触发
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)	// 死循环：让程序停在原地，方便调试器观察出错现场
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)	// 内存管理异常：MPU 内存保护被触发等时进入（本工程用不到）
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)	// 总线错误：访问了不存在的地址等时进入（本工程用不到）
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)	// 用法错误：执行了非法指令等时进入（本工程用不到）
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)	// 系统服务调用异常：操作系统底层才用，裸机工程用不到
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)	// 调试监视异常：调试器相关，平时用不到
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)	// 可挂起的系统服务：操作系统任务切换才用，裸机工程用不到
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)	// 系统滴答定时器中断：本工程的延时函数用的是"查询方式"
{
}							// （见 System\Delay.c），没有开启中断，所以这里保持空

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
/*============================================================
 * 【中文说明：外设中断处理函数】
 *   这里预留了"外设中断"的模板位置。本工程用到的外设中断是
 *   EXTI（外部中断，用于按键），它的处理函数叫 EXTI15_10_IRQHandler，
 *   因为要用到按键消抖等功能，被写在了 Hardware\NVIC.c 里。
 *   函数名一定要和启动文件（startup_stm32f10x_xx.s）的中断向量表
 *   里的名字完全一致，否则中断无法被正确响应！
 *============================================================*/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)	// 模板函数（已注释）：以后新增外设中断时，照这个格式添加
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
