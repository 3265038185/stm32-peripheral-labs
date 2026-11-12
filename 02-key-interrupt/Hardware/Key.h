/*===============================================================================
 * 文件：Key.h    项目：2-按键中断
 *
 * 【这个文件是干什么的？】
 *   按键模块的头文件：声明 Key.c 里定义的函数，并定义"读按键引脚的宏"。
 *   其他文件只要 #include "Key.h"，就可以调用按键相关的函数了。
 *
 * 【为什么头文件开头要有 #ifndef ... #define ... #endif？】
 *   这叫"防止头文件重复包含"。一个工程里很多 .c 文件都会 #include "Key.h"，
 *   编译器展开时如果没有这个保护，同一个头文件被包含两次就会报"重复定义"
 *   错误。保护原理：第一次包含时 __KEY_H__ 还没定义 → 定义它 → 正常展开内容；
 *   第二次包含时 __KEY_H__ 已经定义了 → 整个文件内容被跳过。
 *
 * 【宏定义 Key1~Key4 是什么？】
 *   PBin(n) 是 sys.h 里用"位带操作"定义的宏，等价于"读取 PBn 引脚电平"。
 *   所以 Key1 就代表 PB12 引脚的电平（1=高电平，0=低电平）。
 *   用宏把引脚抽象成"按键"，写代码更直观：if(Key1 == 0) 就是"PB12 按下"。
 *===============================================================================*/
#ifndef __KEY_H__
#define __KEY_H__
#include "sys.h"
#define Key1 PBin(12)	// 按键1：读 PB12 引脚电平，按下时为 0（低电平）
#define Key2 PBin(13)	// 按键2：读 PB13 引脚电平，按下时为 0（低电平）
#define Key3 PBin(14)	// 按键3：读 PB14 引脚电平，按下时为 0（低电平）
#define Key4 PBin(15)	// 按键4：读 PB15 引脚电平，按下时为 0（低电平）

void Key_Init(void);					// 按键初始化：把 PB12~PB15 配置成上拉输入

unsigned char Key_GetNum(void);			// 轮询扫描（入门版）：返回被按下按键的编号 1~4
unsigned char Key_GetState(void);		// 状态机扫描：识别"按下+松手"，返回按键编号 1~4
unsigned int Key_One(void);				// 状态机扫描：识别 短按/双击/长按，返回 1/2/3
//void Key_One(void);

unsigned int Key_Oneneed(void);			// 辅助函数：判断"200ms 内是否还有第二次按下"（双击用）
unsigned int Key_plus(void);			// 加减计数器：按 PB13 加一、PB14 减一
//unsigned int Key_GetOne(void);




#endif
