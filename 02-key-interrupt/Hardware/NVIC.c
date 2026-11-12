#include "stm32f10x.h"  
#include "Delay.h"
#include "Light.h"
void NVICKey_Init(void)
{
	

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure); 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_10;
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
		

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 16;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
}
unsigned int	Keystate=0;
void EXTI15_10_IRQHandler(void)
{

	//unsigned int timecount=0,KeyState = 0;
	if(EXTI_GetITStatus(EXTI_Line12)==SET)
	{
		//EXTI_ClearFlag(EXTI_Line12);
		
		EXTI_ClearITPendingBit(EXTI_Line12);
		Dms(20);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==0)
			Keystate = 1;

		
		
	}
	if(EXTI_GetITStatus(EXTI_Line13)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line13);
		Dms(20);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==0)
			Keystate = 2;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line14)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line14);
		Dms(20);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==0)
			Keystate = 3;
		

		
	}
	if(EXTI_GetITStatus(EXTI_Line15)==SET)
	{
		
		EXTI_ClearITPendingBit(EXTI_Line15);
		Dms(20);
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==0)
			Keystate = 4;
		
		//EXTI_ClearITPendingBit(EXTI_Line15);
	}
}
unsigned int NVIC_GetKey(void)
{
	return Keystate;
}
