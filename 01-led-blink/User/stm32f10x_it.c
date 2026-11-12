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
/* =============================================================================
 * 【中文说明】本文件是"中断服务函数（ISR）"集合文件，来自 ST 官方工程模板。
 *
 * 什么是中断服务函数？
 *   单片机运行中，如果发生了"中断"（比如按键按下、定时器溢出），CPU 会
 *   暂停当前正在执行的代码，跳转到对应的中断服务函数去处理，处理完再
 *   回到原来暂停的地方继续执行。这些"跳转目标函数"就写在本文件里。
 *
 * 本工程真正用到的中断：
 *   - EXTI1 外部中断：函数写在 main.c 里（EXTI1_IRQHandler）。
 *     注意：这个函数名在官方模板里没有，工程实际链接的是 main.c 里那个。
 *   - 下面这些 NMI/HardFault/SysTick 等属于 Cortex-M3 内核异常，
 *     本工程基本用不到，保留空函数只是"占位"，防止程序跑飞后没有去处。
 *
 * 所以本文件里都是模板自带的空函数，初学者只要了解它们是干什么的即可。
 * ============================================================================= */
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
//============================================================
// NMI_Handler：不可屏蔽中断（Non-Maskable Interrupt）处理函数
// 作用：NMI 是最高优先级的异常，几乎不会被普通程序触发。
//       本工程用不到，留空即可（空函数 = 什么也不处理）。
//============================================================
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
//============================================================
// HardFault_Handler：硬件错误处理函数（非常重要！）
// 作用：程序出现严重错误时（例如：访问了不存在的地址、除数为 0、
//       写了只读寄存器等），CPU 会跳到这里。
// 代码里 while(1){} 的意思是：一旦出错，就永远停在这个死循环里，
// 方便你用调试器定位问题（程序跑飞后不至于乱执行）。
// 调试时若程序停在这，多半是数组越界、指针乱指或时钟配置不对。
//============================================================
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
//============================================================
// MemManage_Handler：内存管理错误处理函数
// 作用：MPU（内存保护单元）检测到非法内存访问时触发。
//       STM32F103 一般没开 MPU，所以这个函数基本不会被执行。
//       留空的死循环同样是"出错就停住"的保险措施。
//============================================================
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
//============================================================
// BusFault_Handler：总线错误处理函数
// 作用：CPU 访问总线（外部存储、外设）出错时触发，
//       比如访问了未启用时钟的外设寄存器。
// 提醒：初学者如果直接操作某个外设寄存器却没先开它的时钟，
//       程序就可能跳进这里（甚至直接 HardFault）。
//============================================================
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
//============================================================
// UsageFault_Handler：用法错误处理函数
// 作用：执行了非法指令（如除零、未对齐访问）时触发。
//       STM32 默认把除零等错误直接归入 HardFault，
//       所以本函数平时也很少执行，同样是占位保险。
//============================================================
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
//============================================================
// SVC_Handler：系统服务调用（Supervisor Call）处理函数
// 作用：执行 SVC 指令时触发。这是操作系统（RTOS）用来
//       实现"系统调用"的机制。裸机工程用不到，留空即可。
//============================================================
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
//============================================================
// DebugMon_Handler：调试监视（Debug Monitor）处理函数
// 作用：用于调试器（如 J-Link）相关功能，一般用不到，留空。
//============================================================
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//============================================================
// PendSV_Handler：可挂起的系统服务（PendSV）处理函数
// 作用：RTOS 常用它来做"任务切换"。裸机工程用不到，留空。
//============================================================
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//============================================================
// SysTick_Handler：系统滴答定时器中断处理函数
// 特别说明：本工程的延时（Delay_ms/Delay_us，见 System/Delay.c）
//          用的是"查询法"（while 死等 SysTick 计数到 0），
//          并没有开启 SysTick 中断，所以这个函数用不到，留空。
//          如果你以后改用"中断法"做多任务延时，才需要在这里写代码。
//============================================================
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
//============================================================
// 下面是外设中断函数的"模板占位"。
// 如果要用某个外设中断，就把注释打开，并把 PPP 换成外设
// 中断函数名（函数名可在启动文件 startup_stm32f10x_md.s 里查到）。
// 例如本工程的按键中断函数 EXTI1_IRQHandler 就写在 main.c 里。
//============================================================
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
