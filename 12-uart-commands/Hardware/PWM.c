#include "stm32f10x.h"   
//#include "Delay.h"// Device header
//unsigned int arr = 100 - 1,psc = 720 - 1,ccr=50;
//占空比= ccr/（arr+1）
//频率= clk/(psc+1)/(arr+1)
//分辨率 = 1/（arr+1）
//unsigned int i=0,k;
void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM4);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period =100-1 ;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
	//TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	
	TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;

	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	
	TIM_OC1Init(TIM4,&TIM_OCInitStructure);
	//TIM_OC2Init(TIM4,&TIM_OCInitStructure);
//	TIM_OC3Init(TIM4,&TIM_OCInitStructure);
//	TIM_OC4Init(TIM4,&TIM_OCInitStructure);
	//TIM_OC2Init(TIM1,&TIM_OCInitStructure);
	//TIM_OC3Init(TIM1,&TIM_OCInitStructure);
	//TIM_OC4Init(TIM1,&TIM_OCInitStructure);
	TIM_Cmd(TIM4,ENABLE);
}

