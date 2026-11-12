/*==============================================================================
 * 文件：Key.h（按键模块头文件）
 * 功能：声明按键模块的函数，并定义了 Key1~Key4 四个宏，方便主程序使用。
 * 知识点：宏定义（#define）、位带操作（PBin(n)）、头文件保护
 *============================================================================*/
#ifndef __KEY_H__
#define __KEY_H__
#include "sys.h"	// 引入sys.h：里面用"位带操作"定义了 PBin(n) 宏（读取引脚电平）

/* 宏定义：给按键引脚起"别名"。
   Key1 = PBin(12) 意思是：Key1 就代表"读PB12引脚的电平"。
   PBin(12) 是位带操作：把GPIOB->IDR寄存器（输入数据寄存器）的第12位
   映射到SRAM中的某个地址，读这个地址=读PB12引脚。
   按下时引脚为低电平，所以写 Key1 == 0 就表示"按键1按下" */
#define Key1 PBin(12)
#define Key2 PBin(13)
#define Key3 PBin(14)
#define Key4 PBin(15)

void Key_Init(void);		// 按键初始化函数声明（配置PB12~15为上拉输入）

unsigned char Key_GetNum(void);		// 阻塞式读取：按一次返回1~4，没按返回0
unsigned char Key_GetState(void);	// 状态机读取：返回当前被按住的按键号
unsigned char Key_One(void);		// 识别"短按/长按/双击"，返回1长按/2单击/3双击
unsigned char Key_Oneneed(void);	// 等待按一次：按下返回1，200ms超时返回0



#endif	/* 头文件保护结束 */
