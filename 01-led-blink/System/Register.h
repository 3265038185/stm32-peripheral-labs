/*****************************************************************************
 * 文件名：Register.h（System 系统层头文件）
 * 说明：Register.c 的对外接口声明。
 *       Register() 用"直接操作寄存器"的方式驱动 10 个 LED，
 *       与 Function.c（库函数）、Bit.c（位带）对照学习。
 *       它不依赖 LED_Init()，自己会开时钟（RCC->APB2ENR）并配置
 *       GPIO 模式（GPIOx->CRL），让你看清寄存器底层是怎么回事。
 *
 * 参数含义：
 *   Register(1)：10 个 LED 全亮全灭循环闪烁；
 *   Register(2)：流水灯（PB0→PB1→PA0~PA7 逐个移动亮点）；
 *   Register(3)：交替亮灭演示。
 *****************************************************************************/
#ifndef __REGISTER_H__
#define __REGISTER_H__

//寄存器方式 LED 演示函数：x 为模式选择（1/2/3）
void Register(int x);
#endif
