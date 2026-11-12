#ifndef __NIXIE_H__
#define __NIXIE_H__

/* 数码管模块头文件：声明数码管初始化和显示函数的原型。
 * 用法示例：Nixie_ShowNum(2, 5);  → 让第 2 位数码管显示数字 5 */

void Nixie_Init(void);
void Nixie_ShowNum(int x,int y);

#endif
