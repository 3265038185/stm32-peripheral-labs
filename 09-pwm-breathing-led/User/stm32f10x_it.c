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
 *   也就是当 CPU 收到各种中断请求后，跳转去执行的函数。
 *
 * 【中断流程回顾】
 *   1. 外设/内核产生中断请求 → 2. NVIC 中断控制器裁决 → 3. CPU 暂停当前代码
 *      → 4. 跳转到对应的中断服务函数执行 → 5. 执行完返回原来停下的地方继续跑。
 *   中断服务函数的名字是"固定"的，不能随便改！名字必须和启动文件
 *   startup_stm32f10x_md.s 里定义的中断向量表一一对应，否则中断来了找不到函数。
 *
 * 【本工程用到了哪些中断？】
 *   本工程（4-PWM呼吸灯）采用纯 PWM + 轮询方式，main.c 里没有配置任何中断，
 *   所以下面这些函数绝大多数都保持空函数（什么都不做）。
 *   函数虽然为空，但绝不能删除！因为启动文件的向量表里已经引用了它们的名字，
 *   删了会导致链接错误。
 *
 * 【下面每个函数的用途简介】
 *   - NMI_Handler      ：不可屏蔽中断（内核级），一般不使用
 *   - HardFault_Handler：硬件错误中断，代码写错（比如非法地址访问）时会跳进来，
 *                        通常在这里死循环方便调试
 *   - MemManage/BusFault/UsageFault：内存管理、总线、用法错误中断（内核级）
 *   - SVC/PendSV/SysTick：操作系统常用中断，裸机工程用不到
 *   - PPP_IRQHandler  ：外设中断的"模板"，真正用哪个外设中断就照这个格式写
 * ========================================================================== */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"   // 本头文件，里面声明了下面这些中断服务函数的原型

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
void NMI_Handler(void)      // NMI（不可屏蔽中断）服务函数：本工程未使用，留空即可
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)   // 硬件错误中断：程序跑飞（如访问非法地址）时会跳到这里
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)                    // 死循环：让程序停在这里，方便用调试器定位错误
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
void SysTick_Handler(void)   // 系统滴答定时器中断：Delay 函数用的是"查询方式"，
                             // 不走这个中断，所以留空即可
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
/* 上方英文注释的意思是：
 * "在这里添加你用到的那几个外设的中断处理函数，
 *  外设中断函数的名字请参考启动文件 startup_stm32f10x_xx.s"。
 * 例如：本工程如果要用 TIM2 更新中断，就要新建一个叫 TIM2_IRQHandler 的函数。
 */

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)   // 外设中断的"占位模板"，PPP 代表任意外设
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
