#include "stm32f10x.h"     
#include "PWM.h"
#include "LED.h"
#include "TIMER.h"
#include "Serial.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include <ctype.h>

extern uint8_t Serial_RxFlag;
unsigned int i,j,num;
unsigned char flag = 0;


char alp[100];
int main(void)
{
	
	Serial_Init();
	PWM_Init();
	Timer2_Init();
	LED_Init();
//IM_Cmd(TIM2,ENABLE);
	Serial_SendString("MCU Start\r\n");
	while (1)
	{
		//
		if (Serial_RxFlag == 1)
			
		
		{
			
			i=1;
			j=0;
			num = atoi(&Serial_RxPacket[0]);
			while (Serial_RxPacket[i] != '\0')
					{
						if (isalpha(Serial_RxPacket[i]))
						{
							alp[j++]=Serial_RxPacket[i];
						}
						i++;
					}
			
			if(strcmp(&Serial_RxPacket[1],"?")==0)
			{
				if(LED_func(num)==1)
				Serial_SendString("LED_ON\r\n");
				if(LED_func(num)==0)
					Serial_SendString("LED_OFF\r\n");
				if (num==4)
				{
					if(flag==0)
					{
						Serial_SendString("LED_OFF\r\n");
						
					}
					else if(flag==1)
					{
						Serial_SendString("LED_Twinkle\r\n");
						
					}
				}
				
			}			
			else if(strcmp(alp,"ON")==0)
			{
				if(num!=4)
				LED_Light(num,1);
				else
				{TIM_Cmd(TIM2,ENABLE);
				    flag=1;
				}
			}	
			else if(strcmp(alp,"OFF")==0)
			{
				if(num!=4)
				LED_Light(num,0);
				else
				{TIM_Cmd(TIM2,DISABLE);
					TIM_SetCompare1(TIM4,0);
				    flag=0;
				}
				
			}				
			else
				Serial_SendString("fault\r\n");
			strcpy(alp, "/0");
			Serial_RxFlag = 0;
		}
	}
	
}






	
