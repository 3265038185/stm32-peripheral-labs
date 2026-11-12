/*===============================================================================
 * 文件名称：main.c（工程主函数文件）
 * 所属工程：4-1OLED（STM32F103 标准库工程，按键 + LED + 蜂鸣/音效实验）
 * 工程功能：一个"按键实验综合演示"程序：
 *           1. 初始化 4 个 LED（PA0~PA3，低电平点亮）和 4 个按键（PB12~PB15）
 *           2. 主循环里用"轮询法" Key_GetNum() 检测按键，
 *              按键按下后调用 Song(按键号, 250) 做出"点亮相应 LED 并保持片刻"的效果
 *           3. 文件中还保留了多套被注释掉的备选方案，供学习对比：
 *              - 中断按键（NVICKey_Init + EXTI）
 *              - 加减计数（Key_GetState 状态机按键）
 *              - 四按键状态机（Key_GetState 分别响应）
 *              - 长按/短按/双击（Key_One）
 * 用到的外设/知识点：
 *   1. GPIO 输出（推挽 50MHz）驱动 LED；GPIO 上拉输入读取按键
 *   2. 轮询法/中断法/状态机法三种按键读取方式
 *   3. 模块化编程：#include 各硬件模块的头文件，main 只负责"组织逻辑"
 * 硬件接线：
 *   LED ：PA0~PA3（阳极接 3.3V，阴极接 IO，低电平点亮）
 *   按键：PB12~PB15（一端接 IO，一端接 GND，按下为低电平）
 * 整体结构：初始化（LED+按键）→ while(1) 死循环中轮询按键 → 按键响应
 *===============================================================================
 */
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
	
	
	
	LED_Init();	// 初始化 LED：PA0~PA3 配置为推挽输出，并先全部熄灭（置高电平）
	Key_Init();	// 初始化按键：PB12~PB15 配置为上拉输入（不按时高电平，按下低电平）
	
	//1轮询法
	// （当前启用的方案）轮询法：主循环不停地"问"按键有没有被按下

	while(1)	// 主循环：程序永远在这里转圈
	{
		// Key_GetNum()：轮询检测按键（按下并松手才返回按键号 1~4，没有则返回 0）
		// Song(按键号, 250)：点亮对应 LED 组合并保持 250ms 后再熄灭
		Song(Key_GetNum(),250);	// 按 1 号键 → 点亮第 1 种灯效；按 2 号键 → 第 2 种……
	}	
	


	//中断按键
	// （备选方案1，被注释）中断法按键：按键按下时硬件触发 EXTI 中断，
	// 在中断里记录按键号，主循环查询 NVIC_GetKey() 后调用 Light() 亮灯
	// 优点：按键不"占用"主循环时间，主循环可以专心干别的
	
//	NVICKey_Init();	// 初始化外部中断按键（PB12~15 + EXTI 下降沿中断）
//	while(1)
//	{
//		if(NVIC_GetKey()==1)	// 查询中断里记录的按键号
//		{
//			Light(1);	// 是按键 1：执行灯效 1
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
	// （备选方案2，被注释）用状态机按键实现"加一/减一"：
	// 按键 1 按下 → Keycount 加 1（最大 5）；按键 2 按下 → Keycount 减 1（最小 0）
	// 每次变化后调用 Light(Keycount) 显示当前数值对应的灯效
// unsigned int Keycount=0;	// 计数值
//		while(1)
//		{
//			if(Key_GetState() ==1)	// 状态机检测：按下按键 1
//			{
//				if(Keycount<5)	// 上限保护，最多加到 5
//				Keycount++;
//				
//			}
//			if(Key_GetState() ==2)	// 按下按键 2
//			{
//				if(Keycount){	// 下限保护，最小 0（Keycount 非 0 才减）
//				Keycount--;
//				}
//				//Light(Keycount);
//			}
//			Light(Keycount);	// 刷新灯效显示
//			
//		}
		
	
//状态机四按键
// （备选方案3，被注释）四个按键分别触发四种不同的"音乐/灯效"
//	while(1)
//	{
//		if(Key_GetState() ==1)	// 状态机检测按键 1
//		{
//			Song(1,500);	// 播放灯效 1（保持 500ms）
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
// （备选方案4，被注释）单个按键（PB15）的高级识别：
// Key_One() 返回 1=长按 / 2=单击 / 3=双击，对应三种不同灯效
//	while(1)
//	{
//		switch(Key_One())
//		{	case 1:	// 长按
//			{	Song(3,500);
//			break;
//			}
//			case 2:	// 单击
//			{
//					Song(1,500);
//				break;
//			}
//			case 3:	// 双击
//			{
//				Song(2,500);
//				break;
//			}
//			
//		}	
//	}
	
}
