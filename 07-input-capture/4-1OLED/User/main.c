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
	
	
	
	LED_Init();
	Key_Init();
	
	//1轮询法

	while(1)
	{
		Song(Key_GetNum(),250);		
	}	
	
	

	//中断按键
	
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

