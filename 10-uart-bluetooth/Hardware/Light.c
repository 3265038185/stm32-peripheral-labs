#include "stm32f10x.h"    
#include "Delay.h"// Device header
void Light(int x)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_All);
	if(x==1)
	{
		
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
		
	}
	else if(x==2)
	{
		
		GPIO_ResetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1);
		//Delay_ms(y);
		
		
	}
	else if(x==3)
	{
		GPIOA->ODR = 0x000000f8;
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==4)
	{
		GPIOA->ODR = 0x000000f0;
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==5)
	{
		GPIOA->ODR = 0x000000e0;
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==6)
	{
		GPIOA->ODR = 0x000000c0;
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==7)
	{
		GPIOA->ODR = 0x00000080;
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==8)
	{
		GPIOA->ODR = 0x00000000;
	//	Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		
	}
	else if(x==9)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_All);
		GPIO_ResetBits(GPIOB,GPIO_Pin_0);
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		//GPIO_SetBits(GPIOB,GPIO_Pin_0);
		
	}
	else if(x==10)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_All);
		GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
		//Delay_ms(y);
		//GPIO_SetBits(GPIOA,GPIO_Pin_All);
		//GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
		
	}
	Delay_ms(150);
}
