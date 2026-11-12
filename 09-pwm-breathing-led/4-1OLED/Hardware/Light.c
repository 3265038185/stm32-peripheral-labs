/* ============================================================================
 * 文件说明：Light.c —— 简单点灯函数（按参数 x 点亮对应数量的 LED）
 * ----------------------------------------------------------------------------
 * 【功能】
 *   Light(x)：根据 x 的值点亮"第 1 个 ~ 第 x 个"LED（PA0 开始），
 *   保持 150ms 后返回。x=1~8 只操作 GPIOA，x=9/10 还会操作 GPIOB。
 *
 * 【实现方式】
 *   - 标准库函数版：GPIO_SetBits / GPIO_ResetBits（x=1,2,9,10）
 *   - 寄存器直接赋值版：GPIOA->ODR = 0x...（x=3~8），
 *     直接整体写入输出数据寄存器，一次设置多个引脚
 *
 * 【LED 编号与点亮规律（低电平点亮）】
 *   x=1：PA0 亮          x=5：PA0~PA4 亮（ODR=0xE0，高 5 位输出 0）
 *   x=2：PA0,PA1 亮      x=6：PA0~PA5 亮（ODR=0xC0）
 *   x=3：PA0~PA2 亮      x=7：PA0~PA6 亮（ODR=0x80）
 *   x=4：PA0~PA3 亮      x=8：PA0~PA7 全亮（ODR=0x00）
 *   x=9 ：PA 全亮 + PB0 亮；x=10：PA 全亮 + PB0,PB1 亮
 * ========================================================================== */

#include "stm32f10x.h"    
#include "Delay.h"// Device header

void Light(int x)          // 点亮 x 个 LED
{
	GPIO_SetBits(GPIOA,GPIO_Pin_All);
	// 先把 GPIOA 全部引脚置 1（全部熄灭），作为"先全灭再点灯"的刷新动作，
	// 避免上一种状态残留导致点亮数量不对
	if(x==1)               // 只亮第 1 个灯（PA0）
	{
		
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);   // PA0 输出低电平 → 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		/* 上面两行被注释：原来想在这里延时和全灭，已改到函数末尾统一处理 */
		
		
	}
	else if(x==2)          // 亮前 2 个灯（PA0、PA1）
	{
		
		GPIO_ResetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1);   // PA0、PA1 同时拉低点亮
		//Delay_ms(y);
		
		
	}
	else if(x==3)          // 亮前 3 个灯（PA0~PA2）
	{
		GPIOA->ODR = 0x000000f8;
		// 直接写 ODR 寄存器：0xF8 = 1111 1000b
		// 低 3 位是 0 → PA0~PA2 输出低电平点亮，其余位是 1 → 熄灭
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==4)          // 亮前 4 个灯（PA0~PA3）
	{
		GPIOA->ODR = 0x000000f0;
		// 0xF0 = 1111 0000b：低 4 位为 0 → PA0~PA3 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==5)          // 亮前 5 个灯（PA0~PA4）
	{
		GPIOA->ODR = 0x000000e0;
		// 0xE0 = 1110 0000b：低 5 位为 0 → PA0~PA4 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==6)          // 亮前 6 个灯（PA0~PA5）
	{
		GPIOA->ODR = 0x000000c0;
		// 0xC0 = 1100 0000b：低 6 位为 0 → PA0~PA5 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==7)          // 亮前 7 个灯（PA0~PA6）
	{
		GPIOA->ODR = 0x00000080;
		// 0x80 = 1000 0000b：低 7 位为 0 → PA0~PA6 点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==8)          // 8 个灯全亮（PA0~PA7）
	{
		GPIOA->ODR = 0x00000000;
		// 0x00：低 8 位全部为 0 → PA0~PA7 全部点亮
	//	Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==9)          // 9 个灯：PA 全亮 + PB0 亮
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_All);   // PA0~PA7 全部拉低（全亮）
		GPIO_ResetBits(GPIOB,GPIO_Pin_0);     // PB0 拉低点亮（第 9 个灯）
		// 注意：PB0 要能用，必须先在 LED_Init 里初始化 GPIOB！
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		//GPIO_SetBits(GPIOB,GPIO_Pin_0);
		
	}
	else if(x==10)         // 10 个灯：PA 全亮 + PB0、PB1 亮
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_All);           // PA0~PA7 全亮
		GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);  // PB0、PB1 同时点亮
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		//GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
		
	}
	Delay_ms(150);         // 保持当前灯效 150ms（让眼睛看清），然后函数返回
}
