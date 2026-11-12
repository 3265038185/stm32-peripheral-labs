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

/*---------------------------------------------------------------------------
  补充说明（中文，适合初学者）：
  ---------------------------------------------------------------------------
  这个文件是"中断服务函数"的统一存放处（模板文件）。
  什么是中断？—— CPU 正在干活时，突然来了一个紧急事件
  （比如定时器时间到了、按键按下了、串口收到数据了），
  CPU 会暂停当前工作，跳到对应的"中断服务函数"去处理，处理完再回来。
  ---------------------------------------------------------------------------
  本文件内容分两大类：
    1. Cortex-M3 内核异常（前面 NMI_Handler ~ SysTick_Handler 部分）：
       这是 CPU 自带的系统级异常，一般不需要修改。
       其中 HardFault_Handler（硬件错误）最常被碰到——程序跑飞、
       数组越界等都会跳进它里面的 while(1) 死循环，让程序"卡死"，
       方便我们通过调试器发现问题。
    2. STM32F103 外设中断（后面的 PPP_IRQHandler 部分）：
       当我们用到某个外设的中断时（如 EXTI 按键中断、TIM 定时器中断），
       就要按照 startup 启动文件里规定的函数名，在这里补充对应的
       中断服务函数，例如本项目按键中断用的是 EXTI15_10_IRQHandler，
       它写在 Hardware/NVIC.c 里（因为那里更方便管理按键逻辑）。
  ---------------------------------------------------------------------------
  注意：中断服务函数的名字必须和 startup_stm32f10x_xx.s 启动文件里的
  名字一模一样，否则中断来了却找不到函数，程序会直接跳进 HardFault。
---------------------------------------------------------------------------*/

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
/*  中文解释：NMI（不可屏蔽中断），由硬件/外部引脚触发，一般用不到，留空即可。 */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
/*  中文解释：硬错误中断。程序出错（如非法地址、除零、栈溢出）时会进入，
    里面的 while(1) 死循环就是"程序卡死"现象，调试时靠断点/串口打印找原因。 */
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
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/*  中文解释：SysTick（系统滴答定时器）中断服务函数。
    本项目的 Delay.c 用的是"查询方式"（等待计数到0，不用中断），
    所以这里留空即可；如果以后做"操作系统/时间片调度"才需要在这里写代码。 */
void SysTick_Handler(void)
{
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

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
