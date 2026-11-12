/*============================================================================
  文件：Key.c（独立按键驱动）
  ----------------------------------------------------------------------------
  硬件连接：
    - 4 个按键分别接 PB12 / PB13 / PB14 / PB15；
    - 按键另一端接 GND，按下时引脚被拉低到 0V（读到低电平 0），
      松开时引脚由内部上拉电阻拉高（读到高电平 1）。
    所以"读到 0 说明按键被按下"。
  ----------------------------------------------------------------------------
  用到外设/知识点：
    1. GPIO 输入模式：GPIO_Mode_IPU（上拉输入）——
       内部上拉电阻保证按键松开时引脚稳定为高电平，不会悬空乱跳。
    2. 按键消抖：机械按键按下/松开瞬间会产生"抖动"（几十毫秒内电平
       反复跳变），直接判断会误触发。常用办法是"延时再确认"：
       按下后延时 20ms 跳过抖动区，再确认电平是否稳定。
    3. 状态机（Key_GetState / Key_Oneneed / Key_One）：
       用"当前状态 + 输入条件"来决定下一步状态，是嵌入式里处理
       复杂按键（长按、短按、双击）的标准思路，比一堆 if 更清晰。
  ----------------------------------------------------------------------------
  本文件提供 4 个函数：
    Key_Init       —— 初始化按键 GPIO
    Key_GetNum     —— 简单轮询：检测哪个键被按过（带消抖）
    Key_GetState   —— 状态机：等待并返回"松开"时按下的键号
    Key_Oneneed    —— 状态机：只检测 15 号键，按下返回 1，超时返回 0
    Key_One        —— 状态机：长按/短按/双击识别（只针对 15 号键）
  ============================================================================*/
#include "stm32f10x.h"    
#include "Delay.h"// Device header
#include "sys.h"

/*
 * 函数名：Key_Init（按键初始化）
 * 参数：无
 * 返回：无
 * 功能：把 PB12~PB15 配置成"上拉输入"模式，准备读取按键电平。
 * 原理：
 *   1. RCC_APB2PeriphClockCmd：开启 GPIOB 的时钟。STM32 外设默认
 *      是不供电的（省电），必须先"开时钟"才能操作对应外设寄存器。
 *   2. GPIO_Mode_IPU：上拉输入。按键另一端接 GND，
 *      平时引脚被内部上拉电阻拉到 3.3V（高电平），按下后被拉低（低电平）。
 *   3. GPIO_Pin_12~15：因为都在"高 16 位"（PB8~15），配置的是 CRH 寄存器，
 *      标准库自动帮我们处理，不用关心细节。
 *   4. GPIO_Speed_50MHz：输入模式速度其实无所谓，写上它是为了习惯统一。
 */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	// 开启 GPIOB 时钟
	GPIO_InitTypeDef GPIO_InitStructure;					// 定义 GPIO 配置结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			// 上拉输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	// 同时配置 4 个引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// 输出速度（输入模式无影响）
	GPIO_Init(GPIOB,&GPIO_InitStructure);					// 把配置写入 GPIOB 寄存器
	/*RCC->APB2ENR &= 1<<3;
	GPIOB->CRH &= 0x88880000; 
	*/
	/*  上面注释的是"寄存器版"写法，与标准库等效：
	      APB2ENR 的 bit3 置 1 -> 开启 GPIOB 时钟（库函数 1<<3 其实写错了，
	      应为 |= 或直接赋值，这里仅作学习对比）；
	      CRH 低 16 位写 0x8888 -> PB12~15 配置成上拉输入。
	      标准库封装后我们不用关心这些细节，但了解底层更好。  */
}
/*
 * 函数名：Key_GetNum（按键扫描 - 简单轮询版）
 * 参数：无
 * 返回：按下的键号（1~4），没有按键按下时返回 0
 * 功能：依次检测 4 个按键，返回第一个被按下的键号。
 * 原理（按键消抖的标准套路）：
 *   1. 读电平：GPIO_ReadInputDataBit 读取引脚输入电平（IDR 寄存器）；
 *      按下为 0（低电平）。
 *   2. 前消抖：第一次读到低电平后 Delay_ms(20)，
 *      跳过按键刚按下时 10~20ms 的机械抖动区。
 *   3. 等待松开：while 死循环等引脚变回高电平（1），
 *      保证"按一下只触发一次"，按住不放不会重复触发。
 *   4. 后消抖：松开瞬间也有抖动，再延时 20ms 等电平稳定。
 *   5. 返回键号：把 1/2/3/4 保存到 KeyNum 并返回。
 * 缺点：这个函数是"阻塞式"的（等待松开期间程序被卡住），
 *       适合学习入门；讲究效率时用状态机版（Key_GetState）。
 */
unsigned char Key_GetNum(void)
{
	unsigned char KeyNum = 0;		// 默认没有按键按下
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0)	// PB12 被按下（低电平）
	{
		Delay_ms(20);				// 前消抖：跳过按下瞬间的机械抖动
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0);	// 阻塞等待松开
		Delay_ms(20);				// 后消抖：跳过松开瞬间的抖动
		KeyNum = 1;					// 记录键号 1
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0)	// PB13 被按下
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0);
		Delay_ms(20);
		KeyNum = 2;					// 记录键号 2
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0)	// PB14 被按下
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0);
		Delay_ms(20);
		KeyNum = 3;					// 记录键号 3
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0)	// PB15 被按下
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0);
		Delay_ms(20);
		KeyNum = 4;					// 记录键号 4
		
	}
	


	/*if(~(GPIOB->IDR&1<<12))
	{
		Delay_ms(20);
		while(~(GPIOB->IDR&1<<12));
		Delay_ms(20);
		KeyNum = 1;
		
	}
	else if(~(GPIOB->IDR&1<<13))
	{
		Delay_ms(20);
		while(~(GPIOB->IDR&1<<13));
		Delay_ms(20);
		KeyNum = 2;
		
	}
	else if(~(GPIOB->IDR&1<<14))
	{
		Delay_ms(20);
		while(~(GPIOB->IDR&1<<14));
		Delay_ms(20);
		KeyNum = 3;
		
	}
	else if(~(GPIOB->IDR&1<<15))
	{
		Delay_ms(20);
		while(~(GPIOB->IDR&1<<15));
		Delay_ms(20);
		KeyNum = 4;
		
	}
	*/
	
	
	/*if(Key1 == 0)
	{
		Delay_ms(20);
		while(Key1 == 0);
		Delay_ms(20);
		KeyNum = 1;
		
	}
	else if(Key2 == 0)
	{
		Delay_ms(20);
		while(Key2 == 0);
		Delay_ms(20);
		KeyNum = 2;
		
	}
	else if(Key3 == 0)
	{
		Delay_ms(20);
		while(Key3 == 0);
		Delay_ms(20);
		KeyNum = 3;
		
	}
	else if(Key4 == 0)
	{
		Delay_ms(20);
		while(Key4 == 0);
		Delay_ms(20);
		KeyNum = 4;
		
	}
	*/

	
	
	return KeyNum;
}
unsigned char Key_GetState(void)
{
	unsigned int timecount=0,Keystate=0;
	while(1)
	{
		switch (Keystate)
		{
			case 0:
			{
				if((GPIOB->IDR & (1<<15))==0|| (GPIOB->IDR & (1<<14))==0||(GPIOB->IDR & (1<<13))==0||(GPIOB->IDR & (1<<12))==0)
				Keystate = 1;
				
					
				
				break;
			}
			case 1:
			{
				
				if((GPIOB->IDR & (1<<15))==0)
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 1;
					}
				}
				if((GPIOB->IDR & (1<<14))==0)
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 2;
					}
				}
				if((GPIOB->IDR & (1<<13))==0)
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 3;
					}
				}
				if((GPIOB->IDR & (1<<12))==0)
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 4;
					}
				}
				
				else
				{
					Keystate=0;
					break;
				}
				
				break;	
				
			}
			case 3:
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1)
					{
						Delay_ms(1);
						timecount++;
						if(timecount==10)
						{
							timecount=0;
							Keystate = 1;
						}
					}

					else
					{
						Keystate=3;
						//break;
					}			
			}			
		}		
	}
}


unsigned char Key_Oneneed(void)
{
	unsigned int timecount=0,Keystate=0,i=0;	
	while(1)
	{		
		switch (Keystate)
		{
			case 0:
			{
				if((GPIOB->IDR & (1<<15))==0)
				Keystate = 1;
				
				Delay_ms(1);
				i++;
				if(i>200)
				{
					return 0;
				}									
				break;
			}
			case 1:
			{
				
				if((GPIOB->IDR & (1<<15))==0)
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 1;
					}
				}
				
				
				else
				{
					Keystate=0;
					break;
				}
				
				break;	
				
			}
			case 3:
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)
					{
						Delay_ms(1);
						timecount++;
						if(timecount==10)
						{
							timecount=0;
							Keystate = 1;
						}
					}

					else
					{
						Keystate=3;
						break;
					}			
			}			
		}		
	}
}
unsigned char Key_One(void)
{
	unsigned int timecount=0,Keystate=0,time=0;
	while(1)
	{		
		switch (Keystate)
		{
			case 0:
			{				
				if((GPIOB->IDR & (1<<15))==0)
				Keystate = 1;
				break;
			}
			case 1:
			{				
				if((GPIOB->IDR & (1<<15))==0)
				{
					Delay_ms(1);
					timecount++;
					if(timecount>=10)
					{
						Keystate = 2;
					}
				}
				else
				{
					Keystate=0;
					break;
				}				
				break;					
			}
			case 2:
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)
				{	
					Keystate = 3;																	
				}		
					
				if((GPIOB->IDR & (1<<15))==0)
				{
					Delay_ms(1);
					timecount++;
					Keystate=2;
				}			
				break;			
			}	
			case 3:
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)
				{
					Delay_ms(1);
					time++;
					if(time>10)
					{
						if(timecount>=500)
						{		
							timecount=0;
							time=0;
							Keystate = 1;
							return 1;							
						}
						if(timecount<500)
						{
							timecount=0;
							time=0;
							Keystate = 4;
						}							
					}
				}
				else
				{
					time=0;
					//Keystate=3;
				}
				break;
			}
			case 4:
			{				
				if(Key_Oneneed()==1)
				{ 					
					Keystate=0;
					return 3;
				}
				else
				{	Keystate=0;
					return 2;
				}
				break;
			}
		}	
	}
}
