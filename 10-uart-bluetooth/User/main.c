#include "stm32f10x.h"     
#include "DELAY.h"
#include "OLED.h"
#include "PWM.h"
#include "Timer.h"
#include "Nixie.h"
#include "NVIC.h"
#include "LED.h"
#include "Serial.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include <ctype.h>
extern uint8_t Serial_RxFlag;
unsigned int Count = 0;
unsigned int Shownum1 = 0,Shownum2 = 0,Shownum3 = 0,Shownum4 = 0;
unsigned int Site = 0,Min=0,Sec = 0;
unsigned int timstate = 0,Dot = 0,time=0,Tmin= 0,NowMin = 0,NowSec=0,Lightstate=0,im=0,is=0,js=0,jm=0,ii=0,j=0;
unsigned int bright1=0,bright2 =0;
unsigned int Freq1=0,Freq2 =0,pos=0;
int clockmin = 'M',clocksec = 'S';
char *ptrm,*ptrs;
char mnum[100],snum[100],alp[100];
int main(void)
{

	Serial_Init();
	Nixie_Init();
	
//Nixie_ShowNum(2,0);
	Timer2_Init();
	Timer3_Init();
	

	
	PWM_Init();

	
//	TIM_SetCompare1(TIM4,5000);
//	TIM_PrescalerConfig(TIM4,65535,TIM_PSCReloadMode_Immediate);
	while (1)
	{
		if (Serial_RxFlag == 1)
		{
			
			
			if (strcmp(Serial_RxPacket, "LED1_ON") == 0)
			{
				TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				TIM_SetCompare1(TIM4,10000);
				
			}
				
			else if (strcmp(Serial_RxPacket, "LED1_OFF") == 0)
			{
				TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				TIM_SetCompare1(TIM4,0);
				
				
			}
			else if (strcmp(Serial_RxPacket, "LED2_ON") == 0)
			{
				TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				TIM_SetCompare2(TIM4,10000);
				
			}
			else if (strcmp(Serial_RxPacket, "LED2_OFF") == 0)
			{
				TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				TIM_SetCompare2(TIM4,0);
				
				
			}
			else if (strcmp(Serial_RxPacket, "LED1_B++") == 0)
			{
				TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				bright1 += 2000;
				if(bright1>=10000)
				{
					bright1 = 10000;
				}
				TIM_SetCompare1(TIM4,bright1);
				
				
			}
			else if (strcmp(Serial_RxPacket, "LED2_B++") == 0)
			{
				TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				bright2 += 2000;
				if(bright2>=10000)
				{
					bright2 = 10000;
				}
				TIM_SetCompare2(TIM4,bright2);
				
				
			}
			else if (strcmp(Serial_RxPacket, "LED1_B--") == 0)
			{
				TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				if(bright1>=2000)
				bright1 -= 2000;
				else
				{
					bright1 = 0;
				}
				TIM_SetCompare1(TIM4,bright1);
				
				
			}
			else if (strcmp(Serial_RxPacket, "LED2_B--") == 0)
			{
				TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				if(bright2>=2000)
				bright2 -= 2000;
				else
				{	
					bright2 = 0;
				}
				TIM_SetCompare2(TIM4,bright2);
				
				
			}
			else if (strcmp(Serial_RxPacket, "LED1_F++") == 0)
			{
				//TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				TIM_SetCompare1(TIM4,5000);
				if(Freq1>=3000)
				Freq1-=3000;
				else
				Freq1 = 0;
				TIM_PrescalerConfig(TIM4,Freq1,TIM_PSCReloadMode_Immediate);
				
				
				
			}
			else if (strcmp(Serial_RxPacket, "LED1_F--") == 0)
			{
				//TIM_PrescalerConfig(TIM4,1-1,TIM_PSCReloadMode_Immediate);
				TIM_SetCompare1(TIM4,5000);
				
				Freq1+=3000;
				if(Freq1>=65535)
					Freq1 = 65535;
				
				TIM_PrescalerConfig(TIM4,Freq1,TIM_PSCReloadMode_Immediate);
				//TIM_PrescalerConfig(TIM4,Freq1,TIM_PSCReloadMode_Immediate);
				
				
				
			}
			else if (strcmp(Serial_RxPacket, "Clock_Min++") == 0)
			{
				Min++;
				
				
				
			}
			else if (strcmp(Serial_RxPacket, "Clock_Sec++") == 0)
			{
				Sec++;
				
				
				
			}
			
			else if (strcmp(Serial_RxPacket, "Clock_Start") == 0)
			{
				NowMin = Min;
				NowSec = Sec;
				TIM_Cmd(TIM2,ENABLE);
				
				
				
			}
			else if (strcmp(Serial_RxPacket, "Clock_Stop") == 0)
			{
				
				TIM_Cmd(TIM2,DISABLE);
				
				
			}
			else if (strcmp(Serial_RxPacket, "Clock_Reset") == 0)
			{
				
				
				Min = NowMin;
				Sec = NowSec;
				Count= 0;
				timstate = 0;
				Dot = 0;
				Tmin= 0;
				time=0;
				TIM_Cmd(TIM2,DISABLE);
				
			}

			else
			{
				 while (Serial_RxPacket[ii] != '\0')
				{
					if (isalpha(Serial_RxPacket[ii]))
					{
						alp[j++]=Serial_RxPacket[ii];
					}
					ii++;
				}
				if(strcmp(alp, "M") == 0)
				{
					while (Serial_RxPacket[im] != '\0')
					{
						if (isdigit(Serial_RxPacket[im]))
						{
							mnum[jm++]=Serial_RxPacket[im];
						}
						im++;
					}
				}
				if(strcmp(alp, "S") == 0)
				{
					while (Serial_RxPacket[is] != '\0')
					{
						if (isdigit(Serial_RxPacket[is]))
						{
							snum[js++]=Serial_RxPacket[is];
						}
						is++;
					}
				}
				if(atoi(mnum)<=99)
				Min = atoi(mnum);
				if(atoi(snum)<=60)
				Sec = atoi(snum);
				im=0;is=0;js=0;jm=0;ii=0;j=0;

			}
			
			Serial_RxFlag = 0;
		}
	}
	
}
void TIM2_IRQHandler(void)
{
	
		
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)
	{
		Lightstate = 1;
		Count++;
		Dot = 1;
		if(Count>60)
		{
			Tmin++;
			Count = 0;
		}
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
		if(Lightstate!=0&&Shownum1 ==0&&Shownum2 ==0&&Shownum3 ==0&&Shownum4 ==0)
		{
			
			TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
			
			Tmin = 0;
			Min = 0;
			TIM_Cmd(TIM2,DISABLE);
			Lightstate = 0;
			Count = 0;
			Dot = 0;
			Site = 0;
			Min=0;
			Sec = 0;
			time = 0;
			
		}
	}
	
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) == SET)
	{
	
		Site ++;
		Shownum1 = (Min - Tmin)/10;
		Shownum2 = (Min - Tmin)%10;
		Shownum3 = (Sec - Count)/10;
		Shownum4 = (Sec - Count)%10;
		if(Count!=0)
		{
			if(Sec < Count)
			{
				Count=0;
				Sec = 59;
				Min--;
				
			}
		}
		
		GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
		if(Site == 1)
		{
			Nixie_ShowNum(Site,Shownum1);
		}
		if(Site == 2)
		{
			if(Dot==0)
			{
			Nixie_ShowNum(Site,Shownum2);
			}
			if(Dot == 1)
			{
				Nixie_ShowDotNum(Site,Shownum2);
				time++;
				if(time==50)
				{
					Dot = 0;
					time = 0;
				}
			}
			
		}
		if(Site == 3)
		{
			
			Nixie_ShowNum(Site,Shownum3);
		
		}
		if(Site == 4)
		{
			Nixie_ShowNum(Site,Shownum4);
		}
		if(Site==5)
		{
			Site=0;
		}
		
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
}





	
