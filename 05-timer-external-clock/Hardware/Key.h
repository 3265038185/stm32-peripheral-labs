/*===============================================================================
 * 文件名称：Key.h（按键模块头文件）
 * 模块功能：声明按键驱动函数，并定义按键引脚的"位带操作"宏
 * 知识点：PBin(n) 是"位带操作"宏，定义在 sys.h 里：
 *         PBin(12) 就相当于"直接读 GPIOB 输入数据寄存器的第 12 位"，
 *         可以当成一个普通变量来使用，比如 if(Key1==0) 判断 PB12 是否按下
 *===============================================================================
 */
#ifndef __KEY_H__
#define __KEY_H__
#include "sys.h"	// 引入位带操作宏（PBin 等）的定义
// 定义 4 个按键的引脚宏：Key1~Key4 对应 PB12~PB15（按键另一端接 GND，按下读 0）
#define Key1 PBin(12)	// 按键 1 = PB12 输入位
#define Key2 PBin(13)	// 按键 2 = PB13 输入位
#define Key3 PBin(14)	// 按键 3 = PB14 输入位
#define Key4 PBin(15)	// 按键 4 = PB15 输入位

void Key_Init(void);	// 按键初始化（PB12~15 配置为上拉输入）

unsigned char Key_GetNum(void);	// 轮询法：按下并松手后返回按键号（1~4）
unsigned char Key_GetState(void);	// 状态机法：按下稳定即返回按键号（1~4）
unsigned char Key_One(void);	// 高级识别：返回 1=长按 2=单击 3=双击（仅 PB15）
unsigned char Key_Oneneed(void);	// 内部辅助函数：等待一次完整按下（带超时）



#endif	// 防止重复包含结束
