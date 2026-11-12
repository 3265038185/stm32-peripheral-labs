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

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/*===============================================================================
 * 中文说明（初学必读）：
 * 这个文件是"中断服务函数（ISR）"的集中存放处。所谓中断，就是 CPU 正在
 * 执行 main 里的代码时，硬件突然有紧急事件（比如定时器溢出、按键按下、
 * 串口收到数据），CPU 会暂停手头工作，跳到对应的中断服务函数里执行，
 * 执行完再回到刚才暂停的地方继续干活。
 * 文件里每个函数的名字都是"出厂规定"的，必须和启动文件 startup_stm32f10x_md.s
 * 中中断向量表里的名字一模一样，改一个字母中断都会失效。
 * 本工程（3-定时器）真正用到的是 TIM2 的更新中断，它的服务函数
 * TIM2_IRQHandler 并没有写在这里，而是直接写在 main.c 里了（两个地方
 * 只要名字相同都可以，但同一个名字只能定义一次）。所以本文件里的这些
 * 函数在本工程里基本都是"空函数"，留着它们只是为了占住中断向量表的位置：
 *   - 空函数：表示"这个中断发生了，但我暂时不想处理它"，程序照常运行；
 *   - 死循环函数（while(1)）：表示"程序出错了，卡死在这里方便调试"，
 *     例如 HardFault_Handler 就是程序崩溃（非法地址、除零等）时的"医院"。
 * 初学者只需看懂结构即可，正常情况下不需要改动本文件。
 *===============================================================================
 */

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
/*            下面这部分是 Cortex-M3 内核的"异常"（比普通中断级别更高）      */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
/* NMI：不可屏蔽中断（Non-Maskable Interrupt），最紧急的异常。
   本工程没有使用，函数留空表示"发生了也不处理"。 */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
/* HardFault：硬件错误异常。程序访问了非法地址、执行了非法指令等严重错误时
   会进入这里。死循环是为了让程序"停在原地"，方便用调试器查看问题在哪。 */
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
/* MemManage：内存管理异常，多出现在使用 MPU（内存保护单元）时访问了
   不允许访问的内存区域。本工程未使用 MPU，一般不会触发。 */
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
/* BusFault：总线错误异常。比如 CPU 访问了一个不存在的地址空间。 */
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
/* UsageFault：用法错误异常。比如除数为 0、未对齐访问、执行了未定义的指令等。 */
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
/* SVC：系统服务调用异常，由 SVC 指令触发，操作系统（如 FreeRTOS）里常用，
   裸机工程用不到。 */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
/* DebugMon：调试监视异常，配合调试器使用，平时不触发。 */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/* PendSV：可挂起的系统服务调用。操作系统做任务切换时用，
   裸机工程用不到。 */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/* SysTick：系统滴答定时器中断（Cortex-M3 内核自带的 24 位递减计数器）。
   注意：System/Delay.c 里的 Delay_us 是用"查询方式"使用 SysTick 的
   （死等计数到 0），并没有开启它的中断，所以这个函数在本工程保持为空。 */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/*  下面这一大块是"外设中断"的服务函数，自己用的外设中断要写在这里。         */
/*  外设中断函数的名字同样必须与启动文件里的向量表名字一致！                 */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/* PPP 只是占位符（Peripheral 的缩写），注释掉的模板，意思是：
   "如果你要用某个外设的中断，就把 PPP 换成外设中断的名字（比如 TIM2_IRQHandler）"。
   本工程的 TIM2 更新中断写在 main.c 里了，所以这里保持注释状态即可。 */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
