#include "stm32f10x.h"                  // Device header
void Nixie_Init(void)
{
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStrcture;
	GPIO_InitStrcture.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStrcture.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStrcture.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStrcture);
	GPIO_InitStrcture.GPIO_Pin =GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Init(GPIOB,&GPIO_InitStrcture);
	GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7);
	GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	
}
unsigned char Nixienum[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xff};
void Nixie_ShowNum(int x,int y)
{
	

	if(y>=0&&y<11)
		{
			GPIOA->BSRR = ~Nixienum[y]<<16|Nixienum[y];
		}
	if(x==1)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);
	}
	if(x==2)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_13);
	}
	if(x==3)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_14);
	}
	if(x==4)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_15);
	}
}
unsigned char NixieDotnum[] = {0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,0x08,0x03,0x46,0x21,0x06,0x0e};
void Nixie_ShowDotNum(int x,int y)
{
	
	if(y>=0&&y<11)
		{
			GPIOA->BSRR =  ~NixieDotnum[y]<<16|NixieDotnum[y];
		}
	if(x==1)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);
	}
	if(x==2)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_13);
	}
	if(x==3)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_14);
	}
	if(x==4)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_15);
	}
}
