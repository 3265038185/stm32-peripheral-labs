/*============================================================================
  文件：main.c（4-1OLED 项目主程序）
  ----------------------------------------------------------------------------
  功能：LED 灯 + 按键综合实验。
  当前生效的代码：
    - 初始化 LED（PA0~PA3，低电平点亮）和按键（PB12~15，按下为低电平）；
    - 主循环：Song(Key_GetNum(), 250) —— 每检测到一个按键按下，
      就调用 Song() 点亮对应数量的 LED 灯并保持 250ms。
  ----------------------------------------------------------------------------
  用到的外设/知识点：
    1. GPIO 推挽输出（LED）：GPIO_Mode_Out_PP，能主动输出高/低电平；
    2. GPIO 上拉输入（按键）：GPIO_Mode_IPU，松开高电平、按下低电平；
    3. 按键消抖：按下后延时 20ms 再确认（见 Key.c 的 Key_GetNum）；
    4. 中断方式按键：EXTI 外部中断（NVIC.c，被注释掉的可选用法）；
    5. 状态机按键：长按/短按/双击识别（Key_One，被注释掉的可选用法）。
  ----------------------------------------------------------------------------
  文件里注释掉的一大堆代码是"备选玩法"，想体验哪种就取消
  对应 while(1) 的注释，同时把上面正运行的 while(1) 注释掉。
  ============================================================================*/
#include "stm32f10x.h"     
#include "DELAY.h"
#include "OLED.h"
#include "register.h"
#include "bit.h"
#include "Song.h"
#include "led.h"
#include "Key.h"
#include "NVIC.h"
#include "Light.h"
int main(void)
{
	//LED使用PA0~3口，低电平点亮（即阳极接正极，阴极接IO口）//IO口采用推挽输出50MHz
	//按键GND，按下低电平
	/*  硬件接线说明：
	    - LED 阳极接 3.3V，阴极（负极）接单片机的 PA0~PA3 引脚，
	      所以"引脚输出低电平时 LED 点亮"，这就是注释里说的
	      "低电平点亮"；
	    - 按键一端接 PB12~PB15，另一端接 GND，
	      按下后引脚被拉到 0V（低电平），所以"按下 = 读到 0"。  */
	
	
	
	LED_Init();		// 初始化 LED 引脚（PA0~PA3 推挽输出）
	Key_Init();		// 初始化按键引脚（PB12~PB15 上拉输入）
	
	//1轮询法
	/*  方式一（当前生效）：轮询按键法
	    Key_GetNum() 会"阻塞式"等待一次按键（带消抖），
	    返回键号 1~4；Song() 根据键号点亮对应数量的 LED：
	      Song(1,250) -> 亮 1 个灯 250ms，Song(2,250) -> 亮 2 个灯……
	    由于 Key_GetNum 会一直等到按键松开才返回，
	    所以一个按键从按下到松开只触发一次。  */
	while(1)
	{
		Song(Key_GetNum(),250);		
	}	
	


	//中断按键
	/*  方式二（备选）：中断方式按键
	    按键按下瞬间触发 EXTI 中断（NVIC.c 里的
	    EXTI15_10_IRQHandler），把键号记进全局变量 Keystate；
	    主循环用 NVIC_GetKey() 查询，再调用 Light() 点亮灯。
	    使用前需要先 NVICKey_Init() 初始化。  */
	
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
	/*  方式三（备选）：用状态机按键实现"加减计数"
	    Key_GetState()==1 表示按键 1 完成一次"按下-松开"，
	    计数加 1（最多 5）；Key_GetState()==2 时计数减 1；
	    Light(Keycount) 用点亮 LED 的个数显示当前计数值。  */
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
/*  方式四（备选）：四个按键分别播放"不同数量 LED"的灯效
    Key_GetState() 会等待一次完整按键，返回 1~4，
    分别调用 Song(1~4, 500)。  */
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
/*  方式五（备选）：PB15 键的长按/短按/双击识别
    Key_One() 返回：
      1 = 长按（按住 >= 500ms）-> Song(3,500)
      2 = 短按（只有一次短按） -> Song(1,500)
      3 = 双击（两次短按）     -> Song(2,500)
    这是状态机按键的进阶玩法，理解原理后非常有意思。  */
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

