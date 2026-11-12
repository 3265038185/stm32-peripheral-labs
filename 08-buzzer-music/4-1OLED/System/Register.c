#include "stm32f10x.h"  
#include "Delay.h"
void Register(int x)
{
	RCC->APB2ENR = 0x0000000c;
	GPIOA->CRL = 0x33333333;
	GPIOB->CRL = 0x33333333;
	if(x==1)
	{
		while(1)
		{	
			GPIOB->ODR = 0x00000000;
			GPIOA->ODR = 0x00000000;
			Delay_ms(500);
			GPIOB->ODR = 0x0000000f;
			GPIOA->ODR = 0x000000ff;
			Delay_ms(500);
				
		}
	}
	else if(x==2)
	{
		while(1)
		{
			GPIOB->ODR = 0x00000001;
			Delay_ms(500);
			GPIOB->ODR = 0x00000002;
			Delay_ms(500);
			GPIOB->ODR = 0x00000003;


			GPIOA->ODR = 0x0000007f;
			Delay_ms(500);
			GPIOA->ODR = 0x000000bf;
			Delay_ms(500);
			GPIOA->ODR = 0x000000df;
			Delay_ms(500);
			GPIOA->ODR = 0x000000ef;
			Delay_ms(500);
			GPIOA->ODR = 0x000000f7;
			Delay_ms(500);
			GPIOA->ODR = 0x000000fb;
			Delay_ms(500);
			GPIOA->ODR = 0x000000fd;
			Delay_ms(500);
			GPIOA->ODR = 0x000000fe;
			Delay_ms(500);
			GPIOA->ODR = 0x000000ff;
			
		}
	}
	else if(x==3)
	{
		while(1)
		{
			GPIOB->ODR = 0x00000000;
			GPIOA->ODR = 0x0000001f;
			Delay_ms(500);
			GPIOB->ODR = 0x0000000f;
			GPIOA->ODR = 0x000000ff;
			Delay_ms(500);

		}
	}
	
}
