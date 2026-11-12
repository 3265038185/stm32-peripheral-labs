/*============================================================================
  文件：OLED.h（0.96 寸 OLED 显示屏 - 头文件）
  ----------------------------------------------------------------------------
  作用：
    1. 防止重复包含；
    2. 声明 OLED 模块对外提供的全部函数，其他文件 #include 后即可使用。
  ----------------------------------------------------------------------------
  常用调用示例：
    OLED_Init();                        // 上电后先初始化
    OLED_ShowString(1, 1, "Hello!");    // 第 1 行第 1 列显示字符串
    OLED_ShowNum(2, 1, 123, 3);         // 第 2 行显示数字 123（3 位）
    OLED_Clear();                       // 清屏
  ============================================================================*/
#ifndef __OLED_H
#define __OLED_H

void OLED_Init(void);									// 初始化（必须最先调用）
void OLED_Clear(void);									// 清屏
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);			// 显示单个字符
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);		// 显示字符串
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);		// 十进制无符号数
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);	// 十进制带符号数
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);	// 十六进制数
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);	// 二进制数
/*  参数说明（所有显示函数通用）：
      Line   行：1~4（屏幕高 64 像素，每字符占 16 像素，共 4 行）；
      Column 列：1~16（屏幕宽 128 像素，每字符占 8 像素，共 16 列）；
      Length 位数：数字按固定位数显示，位数不够前面补 0。  */

#endif
