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

/*====================================================================
 * 文件说明：中断服务函数（ISR）模板 —— 由 ST 官方模板提供
 * -------------------------------------------------------------------
 * 这个文件里放的都是"中断服务函数"：CPU 响应中断后会自动跳转
 * 到这里对应的函数去执行，不需要我们手动调用。
 *
 * 1. 上半部分（NMI_Handler ~ SysTick_Handler）是 Cortex-M3 内核
 *    的异常处理函数。裸机程序一般保持空实现即可，如果程序真的
 *    进入了 HardFault（硬件错误），说明代码有 bug（比如野指针、
 *    除零、访问非法地址），死循环卡住是为了方便调试器定位。
 *
 * 2. 下半部分（Peripherals Interrupt Handlers）是外设中断。
 *    函数名必须和启动文件 startup_stm32f10x_hd.s 的中断向量表
 *    里的名字完全一致，中断来了才能找到正确的函数。
 *
 * 注意：本工程把 TIM2 / TIM3 / EXTI 的中断函数写在了 main.c 里，
 *       只要函数名正确（TIM2_IRQHandler、EXTI4_IRQHandler...），
 *       同样会被中断向量表找到，所以这里不需要重复定义。
 *====================================================================*/

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

/* ------------------------ 以下是内核异常处理函数 ------------------------ */

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
/* NMI（不可屏蔽中断）：由严重的硬件错误触发，比如总线供电异常。
 * 裸机程序一般用不到，保留空实现即可。 */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
/* 硬件错误（Hard Fault）：最常见的错误！比如访问不存在的地址
 * （野指针）、非法访问存储器、未对齐访问等。一旦程序进入这个
 * 死循环，就说明代码出了大问题，是调试时最重要的错误信号。 */
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
/* 存储器管理错误：访问了 MPU（存储器保护单元）禁止访问的区域。
 * 只有开启了 MPU 才可能出现，裸机程序一般不会触发。 */
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
/* 总线错误：访问了无效的存储器地址（比如指针指向了错误位置）。
 * 与 Hard Fault 类似，出现时说明程序访问了不存在的地址。 */
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
/* 用法错误：执行了无效指令、除零、未对齐的访问等，也属于
 * 程序 bug 的信号。一般和 Hard Fault 一样需要检查代码逻辑。 */
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
/* 系统服务调用（SVC）：操作系统（RTOS）用来请求系统服务的，
 * 裸机程序用不到，保持空实现即可。 */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
/* 调试监视异常：配合调试器使用，平时用不到，保持空实现即可。 */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/* 可挂起的系统服务（PendSV）：RTOS 做任务切换用的，
 * 裸机程序用不到，保持空实现即可。 */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/* 系统节拍定时器（SysTick）中断：如果用 SysTick 做"精准延时"或
 * 给 RTOS 提供节拍，就把处理代码写在这里。本工程的延时函数
 * （Delay.c）用的是"查询标志位"方式，不依赖中断，所以为空。 */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/* ------------------------ 以下是外设中断服务函数 ------------------------ */
/* 外设中断函数名必须与启动文件中的中断向量表一一对应，例如：
 *   定时器2更新中断 → void TIM2_IRQHandler(void)
 *   定时器3更新中断 → void TIM3_IRQHandler(void)
 *   外部中断4       → void EXTI4_IRQHandler(void)
 *   外部中断9~5     → void EXTI9_5_IRQHandler(void)
 *   外部中断15~10   → void EXTI15_10_IRQHandler(void)
 * 本工程这些中断函数统一写在 main.c 里，因此这里不再重复定义。 */

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
