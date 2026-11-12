/*===============================================================================
 * 文件名称：OLED.h（OLED 显示屏模块头文件）
 * 模块功能：声明 OLED.c 里对外提供的显示函数
 * 屏幕规格：0.96 寸 OLED，SSD1306 驱动，128x64 像素，I2C 接口
 *          （PB8=SCL，PB9=SDA），共 4 行 × 16 列显示
 *===============================================================================
 */
#ifndef __OLED_H
#define __OLED_H

void OLED_Init(void);	// OLED 初始化（I2C 引脚 + SSD1306 配置命令序列）
void OLED_Clear(void);	// 清屏（全灭）
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);	// 显示单个字符
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);	// 显示字符串
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);	// 显示无符号十进制数
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);	// 显示带符号十进制数
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);	// 显示十六进制数
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);	// 显示二进制数

#endif	// 防止重复包含结束
