#include "stm32f10x.h"  
#include "Song.h"// Device header
#include "Delay.h"
void NVICKey_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource15);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource13);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12);
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
		

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
}
unsigned int		Keystate=0;
void EXTI15_10_IRQHandler(void)
{
	Keystate=0;
	//unsigned int timecount=0,KeyState = 0;
	if(EXTI_GetITStatus(EXTI_Line12)==SET)
	{
		Keystate = 1;
		EXTI_ClearITPendingBit(EXTI_Line12);
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line13)==SET)
	{
		Keystate = 2;
		EXTI_ClearITPendingBit(EXTI_Line13);
		
	}
	if(EXTI_GetITStatus(EXTI_Line14)==SET)
	{
		Keystate =3;
		EXTI_ClearITPendingBit(EXTI_Line14);
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line15)==SET)
	{
		Keystate = 4;
		EXTI_ClearITPendingBit(EXTI_Line15);
		
		
	}
}
unsigned int NVIC_GetKey(void)
{
	return Keystate;
}
