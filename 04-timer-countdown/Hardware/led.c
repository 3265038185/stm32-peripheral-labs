#include "led.h"
#include "stm32f10x.h"  
#include "Delay.h"
unsigned int j;

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//GPIO_InitTypeDef GPIOB_InitStructure;
	//GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//GPIOB_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1;
	//GPIOB_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	//GPIO_Init(GPIOB,&GPIOB_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);
	//GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
}

void LED_Run(void)
{
	
		LED0 = 0;
		Dms(100);
		LED0 = 1;
		LED1 = 0;
		Dms(100);
		LED1 = 1;
		LED2 = 0;
		Dms(100);
		LED2 = 1;
		LED3= 0;
		Dms(100);
		LED3 = 1;
	
	
}




