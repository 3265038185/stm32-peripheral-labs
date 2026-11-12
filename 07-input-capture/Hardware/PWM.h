/*==============================================================================
 * 文件：PWM.h（PWM输出+输入捕获模块头文件）
 * 功能：声明PWM模块对外提供的三个函数，供main.c调用。
 * 模块结构：PWMOut_Init输出、PWMInt_Init测量、PWM_Set同步数据
 *============================================================================*/
#ifndef __PWM_H__
#define __PWM_H__


void PWM_Set(void);		// 数据同步：把TIM2测到的周期/脉宽写到TIM3的ARR/CCR1
void PWMOut_Init(void);	// PWM输出初始化：TIM3在PA6输出PWM波
void PWMInt_Init(void);	// 输入捕获初始化：TIM2在PA0测量外部方波的频率和脉宽


#endif	/* 头文件保护结束 */
