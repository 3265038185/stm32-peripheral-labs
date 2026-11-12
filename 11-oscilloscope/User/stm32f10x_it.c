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

/*=============================================================================
 * 文件：stm32f10x_it.c（中断服务函数文件）
 * 功能：存放所有"中断服务函数"（ISR, Interrupt Service Routine）。
 *       当芯片内部发生对应的中断事件时，CPU 会暂停当前任务，
 *       自动跳转到这里对应的函数中执行。
 *
 * 用到的知识点（初学者必懂）：
 *   1. 中断 = "插队"：比如定时器时间到了、ADC 转换完成，硬件就会"喊"CPU 来处理
 *   2. 每个中断函数的名字是固定的（如 SysTick_Handler、TIM3_IRQHandler），
 *      由启动文件 startup_stm32f10x_xx.s 里的中断向量表决定，名字不能乱改
 *   3. 本项目的核心中断函数 TIM3_IRQHandler 写在 Hardware/AD.c 中
 *      （定时采样 ADC），不在本文件里——中断函数写在哪里都可以，
 *      只要名字对得上向量表即可。本文件里的函数本项目中基本用不到
 *   4. 空的中断函数里写 while(1) 死循环的是"错误异常"：
 *      一旦进入说明程序跑飞或出错，故意卡住方便调试时发现
 *
 * 本文件包含两类中断函数：
 *   一、Cortex-M3 内核异常处理（NMI、HardFault、SysTick 等，是内核自带的中断）
 *   二、STM32 外设中断处理（定时器、串口等，本文件模板中已用注释块示例）
 *=============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"		//包含中断函数声明头文件（对应 stm32f10x_it.h）

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
/* 上面这个区域放的是 Cortex-M3 内核自带的中断处理函数（异常处理函数）：
 * 这些中断不是某个外设的，而是 CPU 内核级的，优先级和用法各有不同 */

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
/* NMI = 不可屏蔽中断，优先级最高，几乎无法被阻止。
 * 本项目中未使用，函数体为空：即使意外进入也直接返回，不影响程序 */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
/* HardFault = 硬件错误，程序访问了非法地址、除零等严重错误时进入。
 * 里面的 while(1) 死循环是为了让程序"卡住"，方便调试时用仿真器定位错误位置 */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
/* MemManage = 内存管理错误，MPU（内存保护单元）检测到非法访问时进入 */
void MemManage_Handler(void)
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
/* BusFault = 总线错误，访问不存在的地址空间时进入 */
void BusFault_Handler(void)
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
/* UsageFault = 用法错误，执行了非法指令（如除零）时进入 */
void UsageFault_Handler(void)
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
/* SVC = 系统服务调用，一般用于操作系统（RTOS）中，裸机程序用不到 */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
/* DebugMon = 调试监视器，配合调试器（如 JLINK）使用，平时用不到 */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/* PendSV = 可挂起的系统服务调用，RTOS 任务切换时使用，裸机程序用不到 */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/* SysTick = 系统滴答定时器，是 CPU 内核自带的 24 位倒计数定时器。
 * 本项目 System/Delay.c 里的延时函数用的是"查询方式"（不断读状态寄存器），
 * 没有开 SysTick 中断，所以这里保持空即可。
 * 注意：如果将来想用"中断方式"做定时任务（如每秒刷新一次屏幕），
 * 就把代码写在这个函数里 */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
/* 上面这个区域放的是 STM32 外设的中断处理函数，例如：
 *   TIM3_IRQHandler   —— 定时器 TIM3 的中断（本项目的核心，写在 AD.c 里）
 *   USART1_IRQHandler —— 串口1 接收/发送中断
 *   EXTI0_IRQHandler  —— 外部中断
 * 中断函数的名字必须与启动文件 startup_stm32f10x_xx.s 中的向量表一致，
 * 否则中断发生时 CPU 找不到处理函数，会跳进默认的 HardFault */

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/* PPP 是"外设"的占位符，模板示例。要用某个外设中断时：
 * 1. 把这里取消注释，把 PPP 换成实际外设名（如 TIM3）
 * 2. 在初始化代码中使能该外设的中断并配置 NVIC（见波形项目 AD.c 的 Tim_Init）
 * 本项目的 TIM3 中断函数已经写在 Hardware/AD.c 中，此段保持注释状态即可 */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
