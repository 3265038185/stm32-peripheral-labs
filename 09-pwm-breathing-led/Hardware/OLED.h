/* ============================================================================
 * 文件说明：OLED.h —— OLED 驱动头文件（对应 OLED.c）
 * ----------------------------------------------------------------------------
 * 提供 OLED 屏幕的所有操作接口，使用说明：
 *   1. 用之前先调用 OLED_Init() 完成初始化（必须！）；
 *   2. OLED_ShowXxx 系列函数负责显示字符/字符串/各种进制数字；
 *   3. 显示坐标约定：Line 1~4 行，Column 1~16 列；
 *   4. OLED_ShowSignedNum 的 Length 只算数字部分，符号 '+'/'-' 额外占 1 列。
 * ========================================================================== */

#ifndef __OLED_H              // 头文件保护，防止重复包含
#define __OLED_H

void OLED_Init(void);                                // 初始化 OLED（含 I2C 引脚初始化）
void OLED_Clear(void);                               // 清屏（全灭）
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);                // 显示 1 个字符
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);           // 显示字符串
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);        // 十进制无符号数
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);   // 十进制带符号数
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);     // 十六进制数
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);     // 二进制数

#endif
