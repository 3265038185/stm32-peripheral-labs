#include "stm32f10x.h"                 
#include "led.h"
#include "delay.h"
void Bit(int x)
{
	
	if(x==1)
	{
		while(1)
		{
			LED0 = 0;
			LED1 = 0;
			LED2 = 0;
			LED3 = 0;
			LED4 = 0;
			LED5 = 0;
			LED6 = 0;
			LED7 = 0;
			LED8 = 0;
			LED9 = 0;
			Delay_ms(500);
			LED0 = 1;
			LED1 = 1;
			LED2 = 1;
			LED3 = 1;
			LED4 = 1;
			LED5 = 1;
			LED6 = 1;
			LED7 = 1;
			LED8 = 1;
			LED9 = 1;
			Delay_ms(500);
			
		}
	}
	else if(x==2)
	{
		while(1)
		{
			LED0 = 0;
			Delay_ms(500);
			LED0 = 1;
			LED1 = 0;
			Delay_ms(500);
			LED1 = 1;
			LED2 = 0;
			Delay_ms(500);
			LED2 = 1;			
			LED3 = 0;
			Delay_ms(500);
			LED3 = 1;
			LED4 = 0;
			Delay_ms(500);
			LED4 = 1;
			LED5 = 0;
			Delay_ms(500);
			LED5 = 1;
			LED6 = 0;
			Delay_ms(500);
			LED6 = 1;
			LED7 = 0;
			Delay_ms(500);
			LED7 = 1;
			LED8 = 0;
			Delay_ms(500);
			LED8 = 1;
			LED9 = 0;
			Delay_ms(500);
			LED9 = 1;
		}
	}
	else if(x==3)
	{
		while(1)
		{
			LED9 = 0;
			LED8 = 0;
			LED7 = 0;
			LED6 = 0;
			LED5 = 0;
			Delay_ms(500);
			LED5 = 1;
			LED6 = 1;
			LED7 = 1;
			LED8 = 1;
			LED9 = 1;
			
			LED0 = 0;
			LED1 = 0;
			LED2 = 0;
			LED3 = 0;
			LED4 = 0;
			Delay_ms(500);
			LED0 = 1;
			LED1 = 1;
			LED2 = 1;
			LED3 = 1;
			LED4 = 1;
		}
	}
	
	
}
