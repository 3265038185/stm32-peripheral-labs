/*=============================================================================
 * 文件：OLED.h（OLED.c 的头文件）
 * 功能：声明 OLED 模块的显示函数，供 main.c 等文件调用。
 * 说明：OLED 屏幕 128x64 像素，坐标体系为：
 *       Line（行）1~4，Column（列）1~16（字符显示时用）
 *       x 0~127、y 0~63（画点/画波形时用）
 *=============================================================================*/
#ifndef __OLED_H			//如果没定义过 __OLED_H（头文件还没被包含过）
#define __OLED_H			//就定义 __OLED_H，并执行下面的声明

void OLED_Init(void);											//OLED初始化（上电配置屏幕参数）
void OLED_Clear(void);											//清屏（全部熄灭）
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);		//显示一个字符
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);	//显示一个字符串
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);		//显示十进制数字
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);	//显示带符号十进制数字
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);	//显示十六进制数字
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);	//显示二进制数字
void OLED_DrawPoint(uint8_t Y,uint8_t X);	//画点（Y=纵坐标0~63，X=横坐标0~127，画波形用）
#endif					//结束条件编译
