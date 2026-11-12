/*****************************************************************************
 * 文件名：Delay.h（System 系统层头文件）
 * 说明：Delay.c 的对外接口声明。main.c、Bit.c、Function.c、
 *       Register.c、Song.c 等文件只要 #include "Delay.h"，
 *       就能调用这里声明的三个延时函数。
 *
 * 三个函数的层次关系：
 *   Delay_us()  ← 最底层（直接操作 SysTick 寄存器）
 *   Delay_ms()  ← 中间层（循环调用 Delay_us(1000)）
 *   Delay_s()   ← 最外层（循环调用 Delay_ms(1000)）
 *
 * 使用提醒：这些是"阻塞式延时"，延时期间 CPU 被 while 占用，
 *          无法同时干别的事（中断除外）。适合简单点灯程序。
 *****************************************************************************/
#ifndef __DELAY_H
#define __DELAY_H

//微秒级延时：参数为微秒数，例：Delay_us(100) 延时 100us
void Delay_us(uint32_t us);
//毫秒级延时：参数为毫秒数，例：Delay_ms(500) 延时 500ms（本工程最常用）
void Delay_ms(uint32_t ms);
//秒级延时：参数为秒数，例：Delay_s(2) 延时 2 秒
void Delay_s(uint32_t s);

#endif
