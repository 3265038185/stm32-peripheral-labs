/*****************************************************************************
 * 文件名：Function.h（System 系统层头文件）
 * 说明：Function.c 的对外接口声明。
 *       Function() 用"标准库函数"（GPIO_SetBits/ResetBits/Write）
 *       驱动 10 个 LED，与 Register.c（寄存器）、Bit.c（位带）对照学习。
 *
 * 参数含义：
 *   Function(1)：10 个 LED 全亮全灭循环闪烁；
 *   Function(2)：流水灯（PB1→PB0→PA7→…→PA0 逐个点亮）；
 *   Function(3)：第一组(PB0~PB1+PA5~PA7)与第二组(PA0~PA4)交替亮灭。
 *****************************************************************************/
#ifndef __FUNCTION_H__
#define __FUNCTION_H__

//库函数方式 LED 演示函数：x 为模式选择（1/2/3）
void Function(int x);
#endif
