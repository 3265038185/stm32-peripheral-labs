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

/* ============================================================================
 * 文件说明（stm32f10x_it.c —— 中断服务函数模板文件）
 * ----------------------------------------------------------------------------
 * 本文件是 ST 官方提供的"中断服务函数模板"，里面预先写好了 Cortex-M3
 * 内核异常（NMI、HardFault 等）的处理函数框架，方便用户在此基础上添加
 * 自己用到的外设中断服务函数。
 *
 * 【重要概念】什么是中断服务函数（ISR）？
 *   当芯片发生某个中断事件（例如定时器计数溢出、按键按下）时，CPU 会
 *   "暂停"手头的工作，自动跳转到这个事件对应的中断服务函数里执行代码，
 *   执行完再回到刚才暂停的地方继续干活。这些函数的名字是"固定的"，
 *   必须和启动文件（Satrt 目录下的 startup_stm32f10x_xx.s）里约定的名字
 *   完全一致，芯片才能正确找到它们。
 *
 * 本工程实际使用的中断服务函数（写在 main.c 中，不在本文件里）：
 *   TIM2_IRQHandler()       —— 每 1 秒一次，负责倒计时"秒"的累加
 *   TIM3_IRQHandler()       —— 每 1 毫秒一次，负责数码管动态扫描
 *   EXTI15_10_IRQHandler()  —— PB10 按键（开始/暂停）
 *   EXTI4_IRQHandler()      —— PB4 按键（复位）
 *   EXTI9_5_IRQHandler()    —— PB9 / PB6 按键（设置分钟 / 秒数）
 *
 * 学习方法提示：下面的每个异常处理函数平时都用不到（空函数），
 *   初学者先理解"为什么有这些空函数"即可，重点看 main.c 里的中断函数。
 * ========================================================================== */

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
/* --------------------------------------------------------------------------
 * NMI_Handler() —— 非屏蔽中断（NMI）处理函数
 * 功能：NMI 是"最高优先级、无法屏蔽"的中断，一般由硬件严重错误触发
 *       （如看门狗、电压监测等），正常运行时几乎不会发生。
 * 说明：这里函数体是空的（什么都不做），因为工程里用不到它。
 *       初学者不必深究，知道有这回事即可。
 * ------------------------------------------------------------------------ */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
/* --------------------------------------------------------------------------
 * HardFault_Handler() —— 硬件错误（Hard Fault）处理函数
 * 功能：当程序出现"致命错误"（例如访问了不存在的地址、除以 0、
 *       栈溢出等）时，CPU 会跳到这里。
 * 说明：函数体里是死循环 while(1){} —— 故意让程序"卡死在这里"。
 *       这样做是为了调试方便：程序一旦跑飞就停在这个循环里，
 *       用调试器暂停后就能发现程序停在了 HardFault，说明程序出错啦。
 *       初学者调试时如果发现程序"死机"，先看看是不是进了这个函数！
 * ------------------------------------------------------------------------ */
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
/* --------------------------------------------------------------------------
 * MemManage_Handler() —— 存储器管理错误处理函数
 * 功能：当程序访问了"不允许访问"的内存区域（比如越界、访问受保护
 *       的存储器）时触发，属于较高级别的错误。
 * 说明：同样用死循环"卡住"，方便调试发现错误。初学者了解即可。
 * ------------------------------------------------------------------------ */
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
/* --------------------------------------------------------------------------
 * BusFault_Handler() —— 总线错误处理函数
 * 功能：当 CPU 访问存储器"总线"时发生错误（例如访问了无效地址）
 *       会触发这个异常。和上面的异常一样，用死循环卡住便于调试。
 * ------------------------------------------------------------------------ */
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
/* --------------------------------------------------------------------------
 * UsageFault_Handler() —— 用法错误处理函数
 * 功能：当程序执行了"非法指令"（例如未定义的指令、非法的对齐访问）
 *       时触发。同样用死循环卡住，方便调试发现。初学者了解即可。
 * ------------------------------------------------------------------------ */
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
/* --------------------------------------------------------------------------
 * SVC_Handler() —— 系统服务调用（SVC）处理函数
 * 功能：SVC 是操作系统（RTOS）用来请求系统服务的指令，比如"进入
 *       特权模式"。本工程没有使用操作系统，所以函数体为空。
 * 说明：如果以后学 FreeRTOS 等操作系统，这个函数可能就会被用到。
 * ------------------------------------------------------------------------ */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
/* --------------------------------------------------------------------------
 * DebugMon_Handler() —— 调试监视器异常处理函数
 * 功能：专门用于调试器（如 J-Link、ST-Link）与芯片通信时使用的异常，
 *       一般工程用不到，函数体为空即可。
 * ------------------------------------------------------------------------ */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/* --------------------------------------------------------------------------
 * PendSV_Handler() —— 可挂起的系统服务调用（PendSV）处理函数
 * 功能：和 SVC 类似，是操作系统做"任务切换"时用的（它可以延迟到
 *       没有其他中断时再执行，避免冲突）。本工程无操作系统，为空即可。
 * ------------------------------------------------------------------------ */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/* --------------------------------------------------------------------------
 * SysTick_Handler() —— 系统滴答定时器（SysTick）中断处理函数
 * 功能：SysTick 是 Cortex-M3 内核自带的 24 位递减计数器（挂在内核上，
 *       不占外设定时器资源），常用于"操作系统心跳"或精确延时。
 * 说明：本工程的 Delay.c 用 SysTick 做延时是"轮询等待"方式
 *       （不断查询计数是否到 0），没有开启中断，所以这里函数体为空。
 *       若以后开启 SysTick 中断，处理代码就写在这个函数里。
 * ------------------------------------------------------------------------ */
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
