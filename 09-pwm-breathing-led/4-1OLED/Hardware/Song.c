/* ============================================================================
 * 文件说明：Song.c —— 按键灯效函数（按参数 x 点亮对应数量的 LED，保持 y 毫秒）
 * ----------------------------------------------------------------------------
 * 【功能】
 *   Song(x, y) 与 Hardware/Light.c 里的 Light(x) 功能类似，区别是：
 *   本函数在末尾统一"延时 y 毫秒 + 全部熄灭"，让灯效保持 y 毫秒后自动熄灭，
 *   更适合在 main 里循环调用（main.c 当前生效方案就是 Song(Key_GetNum(),250)）。
 *
 * 【LED 编号与点亮规律（低电平点亮）】
 *   x=1：PA0 亮          x=5：PA0~PA4 亮（ODR=0xE0）
 *   x=2：PA0,PA1 亮      x=6：PA0~PA5 亮（ODR=0xC0）
 *   x=3：PA0~PA2 亮      x=7：PA0~PA6 亮（ODR=0x80）
 *   x=4：PA0~PA3 亮      x=8：PA0~PA7 全亮（ODR=0x00）
 *   x=9 ：PA 全亮 + PB0 亮；x=10：PA 全亮 + PB0,PB1 亮
 *
 * 【用到的知识点】
 *   - GPIO_SetBits / GPIO_ResetBits：把 ODR 寄存器某位置 1（灭）/清零（亮）
 *   - GPIOA->ODR = 0x...：直接整体写输出数据寄存器，一次设置多个引脚
 * ========================================================================== */

#include "stm32f10x.h"                 // STM32F103 标准库头文件
#include "Delay.h"                     // 软件延时库（Delay_ms）

/* ----------------------------------------------------------------------------
 * 函数名：Song
 * 功能  ：点亮"第 1 个 ~ 第 x 个"LED（PA0 开始），保持 y 毫秒后全部熄灭
 * 参数  ：x 要点亮的 LED 数量（1~10）；y 灯效保持时间（毫秒）
 * 返回  ：无
 * 原理  ：按 x 的值选择点亮方式（前几个用 ResetBits，数量多时直接写 ODR），
 *         最后统一延时 y 毫秒再全部熄灭，实现"闪一下"的效果。
 * ========================================================================== */
void Song(int x,int y)
{
	if(x==1)
	{
		
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);   // PA0 输出低电平 → 点亮第 1 个灯
		//Delay_ms(y);
		

		
		
	}
	else if(x==2)
	{
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		GPIO_ResetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1);   // PA0、PA1 同时拉低 → 亮前 2 个灯
	//	Delay_ms(y);
		
		
	}
	else if(x==3)
	{
		GPIOA->ODR = 0x000000f8;   // 0xF8 = 1111 1000b：低 3 位为 0 → PA0~PA2 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==4)
	{
		GPIOA->ODR = 0x000000f0;   // 0xF0 = 1111 0000b：低 4 位为 0 → PA0~PA3 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==5)
	{
		GPIOA->ODR = 0x000000e0;   // 0xE0 = 1110 0000b：低 5 位为 0 → PA0~PA4 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==6)
	{
		GPIOA->ODR = 0x000000c0;   // 0xC0 = 1100 0000b：低 6 位为 0 → PA0~PA5 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==7)
	{
		GPIOA->ODR = 0x00000080;   // 0x80 = 1000 0000b：低 7 位为 0 → PA0~PA6 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==8)
	{
		GPIOA->ODR = 0x00000000;   // 0x00：低 8 位全为 0 → PA0~PA7 全部点亮
	//	Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==9)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_All);   // PA0~PA7 全部拉低（全亮）
		GPIO_ResetBits(GPIOB,GPIO_Pin_0);     // PB0 拉低点亮（第 9 个灯）
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		//GPIO_SetBits(GPIOB,GPIO_Pin_0);
		/* 上面被注释的是"熄灭"操作，已统一放到函数末尾处理 */
		
	}
	else if(x==10)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_All);           // PA0~PA7 全亮
		GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);  // PB0、PB1 同时点亮（第 9、10 个灯）
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		//GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
		
	}
	Delay_ms(y);                     // 保持当前灯效 y 毫秒（让眼睛看清）
	GPIO_SetBits(GPIOA,GPIO_Pin_All);   // 全部熄灭，准备下一次调用
	//Delay_ms(100);
	
	
}
