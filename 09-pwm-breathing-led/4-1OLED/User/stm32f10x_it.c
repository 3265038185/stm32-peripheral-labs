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
 * 中文补充说明（初学者必读）
 * ----------------------------------------------------------------------------
 * 【本文件是干什么的？】
 *   这个文件用来存放"中断服务函数"（ISR, Interrupt Service Routine），
 *   也就是 CPU 收到中断请求后跳转去执行的函数。函数名必须与启动文件
 *   startup_stm32f10x_md.s 的中断向量表一一对应，不能随便改！
 *
 * 【本工程（4-1OLED）用到了哪些中断？】
 *   当前 main.c 走的是"轮询法"（Key_GetNum），没有用中断。
 *   但 main.c 里被注释的"中断按键"方案（NVICKey_Init）一旦启用，
 *   就需要在本文件添加外设中断服务函数（比如 EXTI15_10_IRQHandler），
 *   所以下面的内核异常处理函数都保留为空壳，它们是模板的一部分。
 *
 * 【下面函数用途速查】
 *   - HardFault_Handler：硬件错误中断。程序跑飞（非法地址等）会跳进来，
 *                        死循环停在原地，方便调试器定位问题
 *   - NMI/SVC/PendSV/SysTick：内核级中断，裸机工程一般用不到
 *   - PPP_IRQHandler：外设中断模板，真正使用时按格式改写
 * ========================================================================== */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"   // 本头文件，声明了下面这些中断服务函数

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
void NMI_Handler(void)      // NMI（不可屏蔽中断）服务函数：本工程未使用，留空
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)   // 硬件错误中断：程序跑飞（如非法地址访问）时跳到这里
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)                    // 死循环停住，方便用调试器查看出错现场
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
void SysTick_Handler(void)   // 系统滴答定时器中断：Delay 用查询方式，不走中断，留空
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
/* 上方英文注释的意思是：
 * "在这里添加你用到的外设中断处理函数，函数名参考启动文件 startup_stm32f10x_xx.s"。
 * 例如：本子项目若要启用"中断按键"方案，应在这里新建：
 *   void EXTI15_10_IRQHandler(void) { ... }（具体在 Hardware/NVIC.c 中）
 */

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)   // 外设中断占位模板（PPP 代表任意外设）
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
