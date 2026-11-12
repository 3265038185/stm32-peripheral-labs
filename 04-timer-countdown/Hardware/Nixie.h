#ifndef __NIXIE_H__
#define __NIXIE_H__

/* 数码管模块头文件：声明数码管初始化和显示函数的原型。
 * 用法示例：
 *   Nixie_ShowNum(2, 5);      → 第 2 位显示数字 5
 *   Nixie_ShowDotNum(2, 5);   → 第 2 位显示带小数点的 5（冒号） */

void Nixie_Init(void);
void Nixie_ShowNum(int x,int y);
void Nixie_ShowDotNum(int x,int y);
#endif
