/* ============================================================================
 * 文件说明：main.c —— 4-1OLED 子项目主程序（按键控制 LED 显示）
 * ----------------------------------------------------------------------------
 * 【项目功能】
 *   用 4 个按键（PB12~PB15）控制 10 个 LED（PA0~PA7 + PB0~PB1）的亮灭组合，
 *   按不同的键点亮不同数量的灯，模拟"数字的二进制/流水灯"效果。
 *
 * 【当前生效的代码】
 *   LED_Init() + Key_Init() 初始化后，主循环不断调用：
 *     Song(Key_GetNum(), 250)
 *   即：轮询读取按键编号（Key_GetNum），把对应数量的 LED 点亮 250ms。
 *
 * 【被注释的备选方案（学习方法）】
 *   1. 中断按键版：NVICKey_Init() + NVIC_GetKey()，按键由 EXTI 中断检测；
 *   2. 加减计数版：Key_GetState() 每次按下 Key1 加 1、Key2 减 1；
 *   3. 状态机四按键版：Key_GetState() 分别触发 Song(1~4)；
 *   4. 长按/短按/双击版：Key_One() 返回 1/2/3 分别播放不同灯效。
 *   这些写法展示了对同一个硬件用"轮询/中断/状态机"三种思路的取舍。
 * ========================================================================== */

#include "stm32f10x.h"     // STM32F103 标准库头文件
#include "DELAY.h"         // 软件延时库（Delay_ms）
#include "OLED.h"          // OLED 驱动（本工程未实际使用，预留）
#include "register.h"      // 寄存器操作版灯效（Register 函数，备用）
#include "bit.h"           // 位带操作版灯效（Bit 函数，备用）
#include "Song.h"          // 按键灯效函数 Song（当前主循环使用）
#include "led.h"           // LED 引脚定义（LED0~LED9，PAout/PBout 位带宏）
#include "Key.h"           // 按键驱动（Key_Init / Key_GetNum）
#include "NVIC.h"          // 外部中断按键驱动（备用方案）
#include "Light.h"         // 简单点灯函数 Light（备用）
int main(void)             // 程序入口
{
	//LED使用PA0~3口，低电平点亮（即阳极接正极，阴极接IO口）//IO口采用推挽输出50MHz
	//按键GND，按下低电平
	/* 上面两行注释是硬件连接说明：
	 * - LED：阳极接电源正极，阴极接 IO 口 → IO 输出低电平(0)点亮；
	 *   LED 引脚用推挽输出（强驱动），速度 50MHz。
	 * - 按键：一端接 IO、一端接 GND → 按下时 IO 读到低电平(0)，
	 *   所以检测"读到 0"就是"按键按下"。 */
	
	
	
	LED_Init();             // 初始化 LED 引脚（GPIOA 全部 + GPIOB 的 PB0/PB1 推挽输出）
	Key_Init();             // 初始化按键引脚（GPIOB 的 PB12~PB15 上拉输入）
	
	//1轮询法
	/* 方案一（当前生效）：轮询法。主循环一直不停地"问"按键有没有被按下，
	 * Key_GetNum() 是阻塞式的：没按键返回 0；有按键会等松手后返回编号 1~4。 */

	while(1)                // 主循环
	{
		Song(Key_GetNum(),250);   // 读按键编号 → 点亮对应数量的 LED 并保持 250ms
		                          // Song 的第二个参数 y = 250 是"保持时间"（毫秒）
	}	
	
	

	//中断按键
	/* 方案二（被注释）：中断按键。按键按下时由 EXTI 外部中断"自动记录"，
	 * 主程序不用一直轮询。好处：CPU 空闲时可以做别的事；
	 * 坏处：逻辑比轮询复杂（要配置 EXTI + NVIC）。 */
	
//	NVICKey_Init();
//	while(1)
//	{
//		if(NVIC_GetKey()==1)
//		{
//			Light(1);
//		}
//		if(NVIC_GetKey()==2)
//		{
//			Light(2);
//		}
//		if(NVIC_GetKey()==3)
//		{
//			Light(3);
//		}
//		if(NVIC_GetKey()==4)
//		{
//			Light(4);
//		}
//	}
		
	
	//实现加减功能	
	/* 方案三（被注释）：加减计数。Key1 每按一次 Keycount+1（上限 5），
	 * Key2 每按一次 Keycount-1，然后实时点亮 Keycount 个灯。 */
// unsigned int Keycount=0;
//		while(1)
//		{
//			if(Key_GetState() ==1)
//			{
//				if(Keycount<5)
//				Keycount++;
//				
//			}
//			if(Key_GetState() ==2)
//			{
//				if(Keycount){
//				Keycount--;
//				}
//				//Light(Keycount);
//			}
//			Light(Keycount);
//			
//		}
		
	
//状态机四按键
	/* 方案四（被注释）：状态机四按键。Key_GetState() 的返回值
	 * 在"按下期间只出现一次"，四个键分别触发不同的灯效。 */
//	while(1)
//	{
//		if(Key_GetState() ==1)
//		{
//			Song(1,500);
//		}
//		if(Key_GetState() ==2)
//		{
//			Song(2,500);
//		}
//		if(Key_GetState() ==3)
//		{
//			Song(3,500);
//		}
//		if(Key_GetState() ==4)
//		{
//			
//			Song(4,500);
//		}
//	}	
//	

//长按 短按 双击
	/* 方案五（被注释）：长按/短按/双击。Key_One() 的返回值：
	 * 1=长按、2=短按、3=双击，分别触发不同灯效。 */
//	while(1)
//	{
//		switch(Key_One())
//		{	case 1:
//			{	Song(3,500);
//			break;
//			}
//			case 2:
//			{
//					Song(1,500);
//				break;
//			}
//			case 3:
//			{
//				Song(2,500);
//				break;
//			}
//			
//		}	
//	}
	
}

