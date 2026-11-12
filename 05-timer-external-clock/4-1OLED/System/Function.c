#include "stm32f10x.h"     
#include "DELAY.h"
#include "led.h"
void Function(int x)
{	
	if(x==1)
	{
		
		
		
		while(1)
		{
			
			GPIO_ResetBits(GPIOA,GPIO_Pin_All);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
			Delay_ms(500);
			GPIO_SetBits(GPIOA,GPIO_Pin_All);
			GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
			Delay_ms(500);
		}
	}
	else if(x==2)
	{
		while(1)
		{
			
			
			GPIO_ResetBits(GPIOB,GPIO_Pin_1);
			Delay_ms(500);
			GPIO_SetBits(GPIOB,GPIO_Pin_1);

			GPIO_ResetBits(GPIOB,GPIO_Pin_0);
			Delay_ms(500);
			GPIO_SetBits(GPIOB,GPIO_Pin_0);
		
			GPIO_ResetBits(GPIOA,GPIO_Pin_7);
			Delay_ms(500);
			GPIO_SetBits(GPIOA,GPIO_Pin_7);
		
			GPIO_ResetBits(GPIOA,GPIO_Pin_6);
			Delay_ms(500);
			GPIO_SetBits(GPIOA,GPIO_Pin_6);

			GPIO_ResetBits(GPIOA,GPIO_Pin_5);
			Delay_ms(500);
			GPIO_SetBits(GPIOA,GPIO_Pin_5);

			GPIO_ResetBits(GPIOA,GPIO_Pin_4);
			Delay_ms(500);
			GPIO_SetBits(GPIOA,GPIO_Pin_4);

			GPIO_ResetBits(GPIOA,GPIO_Pin_3);
			Delay_ms(500);
			GPIO_SetBits(GPIOA,GPIO_Pin_3);

			GPIO_ResetBits(GPIOA,GPIO_Pin_2);
			Delay_ms(500);
			GPIO_SetBits(GPIOA,GPIO_Pin_2);

			GPIO_ResetBits(GPIOA,GPIO_Pin_1);
			Delay_ms(500);
			GPIO_SetBits(GPIOA,GPIO_Pin_1);

			GPIO_ResetBits(GPIOA,GPIO_Pin_0);
			Delay_ms(500);
			GPIO_SetBits(GPIOA,GPIO_Pin_0);
		}
	}
	else if(x==3)
		{
			while(1)
			{
			GPIO_ResetBits(GPIOB,GPIO_Pin_1);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0);
			GPIO_Write(GPIOA,~0x00E0);

			Delay_ms(500);
		
			GPIO_SetBits(GPIOB,GPIO_Pin_1);
			GPIO_SetBits(GPIOB,GPIO_Pin_0);
			GPIO_Write(GPIOA,~0x0000);
		
			Delay_ms(500);

				
			GPIO_Write(GPIOA,~0x001F);
			Delay_ms(500);
			GPIO_Write(GPIOA,~0x0000);
			Delay_ms(500);

				
			
			}
			
		}
		
	
}
