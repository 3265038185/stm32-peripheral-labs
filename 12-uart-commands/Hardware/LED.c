#include "stm32f10x.h"    
unsigned char flag1=0,flag2=0,flag3=0;// Device header
void LED_Init(void)
{
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2);
}
void LED_Light(unsigned int x,unsigned int y)
{
	//GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2);
	if(x==1)
	{
		if(y==1)
		{	GPIO_ResetBits(GPIOA,GPIO_Pin_0);
			flag1 = 1;
		}
		if(y==0)
		{
		GPIO_SetBits(GPIOA,GPIO_Pin_0);
			flag1 = 0;
		}

	}
	if(x==2)
	{
		if(y==1)
		{GPIO_ResetBits(GPIOA,GPIO_Pin_1);
			flag2 = 1;
		}
		if(y==0)
		{GPIO_SetBits(GPIOA,GPIO_Pin_1);
			flag2 = 0;
		}

	}
	if(x==3)
	{
		if(y==1)
		{
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
			flag3 = 1; 
		}
		if(y==0)
		{
		GPIO_SetBits(GPIOA,GPIO_Pin_2);
			flag3 = 0;
		}

	}
}
unsigned char LED_func(unsigned int x)
{
	if(x==1)
		return flag1;
	if(x==2)
		return flag2;
	if(x==3)
		return flag3;
	return -1;
}
