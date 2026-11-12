#include "stm32f10x.h"    
#include "Delay.h"// Device header
#include "sys.h"

/*====================================================================
 * 文件说明：按键模块
 * -------------------------------------------------------------------
 * 硬件：4 个按键接在 PB12~PB15，按键另一端接地（低电平有效）。
 * 引脚配置为"上拉输入"：不按时读到高电平 1，按下时读到低电平 0。
 *
 * 为什么需要"消抖"：机械按键按下/松开的瞬间，金属触点会抖动，
 * 电平会在 0/1 之间快速跳动几次（持续几毫秒~十几毫秒）。如果不
 * 消抖，一次按键会被 CPU 误认为按了好几下。常用消抖方法：
 *   1. 延时法：检测到按下后延时 20ms 再确认（Key_GetNum）；
 *   2. 状态机法：连续多次采样都相同才认定（Key_GetState）。
 *
 * 本文件提供 4 个按键处理函数，由简单到复杂：
 *   Key_GetNum    —— 阻塞式：会一直等到按键松开才返回键号
 *   Key_GetState  —— 状态机：消抖 + 识别按下事件，支持 4 个键
 *   Key_Oneneed   —— 状态机：检测"完整按一下"（松开才算完成）
 *   Key_One       —— 状态机：区分"短按"和"长按"（只检测 PB15）
 *====================================================================*/
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	// 开启 GPIOB 时钟（外设时钟默认关闭，必须先开启）
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	// 上拉输入：不按时读到 1，按下（接地）读到 0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	// 使用 PB12~PB15 共 4 个引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 翻转速度 50MHz（输入模式用不到，写上是规范习惯）
	GPIO_Init(GPIOB,&GPIO_InitStructure);	// 把上面的配置写入 GPIOB 的寄存器
	/*RCC->APB2ENR &= 1<<3;
	GPIOB->CRH &= 0x88880000; 
	*/
}
/*====================================================================
 * Key_GetNum：扫描按键，返回被按下的按键号（阻塞式）
 * @retval 0=没有按键，1/2/3/4=对应按键被按下
 * -------------------------------------------------------------------
 * 工作流程（以 1 号键为例）：
 *   1. 读到 PB12 为 0（按下）；
 *   2. 延时 20ms 跳过机械抖动，再确认还是按下；
 *   3. 进入 while 死等 —— 一直等到按键松开才继续；
 *   4. 松开后再延时 20ms（跳过松开的抖动），返回键号。
 * 注意：函数会"死等"按键松开，等待期间程序干不了别的事，
 *       这就是"阻塞式"的含义。写法简单直观，适合入门。
 *====================================================================*/
unsigned char Key_GetNum(void)
{
	unsigned char KeyNum = 0;	// 记录返回的键号，默认 0（没按键）
	/* 检测 1 号按键（PB12）：按下时引脚读到 0 */
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0)
	{
		Delay_ms(20);	// 延时 20ms：跳过按下瞬间的机械抖动
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0);	// 死等：按键没松开就一直停在这里
		Delay_ms(20);	// 松开后再延时 20ms：跳过松开瞬间的抖动
		KeyNum = 1;		// 确认是 1 号键被按下过，返回键号 1
		
	}
	/* 检测 2 号按键（PB13），逻辑同上 */
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0);
		Delay_ms(20);
		KeyNum = 2;
		
	}
	/* 检测 3 号按键（PB14），逻辑同上 */
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0);
		Delay_ms(20);
		KeyNum = 3;
		
	}
	/* 检测 4 号按键（PB15），逻辑同上 */
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0);
		Delay_ms(20);
		KeyNum = 4;
		
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

	
	
	return KeyNum;	// 把键号返回给调用者（0 表示本次没有按键按下）
}
/*====================================================================
 * Key_GetState：状态机方式扫描按键（支持 4 个键）
 * @retval 1/2/3/4=对应按键被按下，函数"阻塞"到按键松开才返回
 * -------------------------------------------------------------------
 * 状态机思想：把按键的一生分成几个"状态"，程序在每个状态里
 * 只做该状态该做的事，条件满足就跳到下一个状态：
 *
 *   状态0（空闲）  ：没有按键按下。发现任一按键变低 → 状态1
 *   状态1（确认）  ：连续 10 次（每次隔 1ms，共约 10ms）都读到
 *                    同一个按键按下，说明不是抖动 → 返回键号，
 *                    并进入状态3（等松开）
 *   状态3（按住中）：按键还按着就保持；检测到所有键松开，
 *                    连续 10 次确认 → 回到状态0，准备下一次
 *
 * 与 Key_GetNum 相比：用"多次采样"代替"固定延时 20ms"，
 * 消抖更可靠，且返回的是"按下事件"（松开后才返回）。
 *====================================================================*/
unsigned char Key_GetState(void)
{
	unsigned int timecount=0,Keystate=0;	// timecount：连续采样计数；Keystate：当前状态
	while(1)	// 状态机需要不停地轮询按键，所以用死循环
	{
		switch (Keystate)	// 根据当前状态执行对应的处理
		{
			/* 状态0：空闲。只要发现任何一个键按下（读到 0）就转入状态1 */
			case 0:
			{
				/* 直接读 IDR 输入数据寄存器：& (1<<15) 取出 PB15 这一位，
				 * 四个键只要有一个为 0（按下）就满足条件 */
				if((GPIOB->IDR & (1<<15))==0|| (GPIOB->IDR & (1<<14))==0||(GPIOB->IDR & (1<<13))==0||(GPIOB->IDR & (1<<12))==0)
				Keystate = 1;	// 有键按下 → 进入"确认"状态
				
					
				
				break;
			}
			/* 状态1：防抖确认 —— 连续 10 次（约 10ms）都检测到按下才认定 */
			case 1:
			{
				
				/* 检测 4 号键（PB15）是否按下。
				 * IDR 是 GPIOB 的输入数据寄存器，& (1<<15) 取出第 15 位 */
				if((GPIOB->IDR & (1<<15))==0)
				{
					Delay_ms(1);	// 每隔 1ms 采样一次
					timecount++;	// 连续"按下"的次数 +1
					if(timecount==10)	// 连续 10 次 = 约 10ms 电平稳定，消抖完成
					{
						timecount=0;	// 计数器清零
						Keystate = 3;	// 进入"已确认按下"状态
						return 1;		// 返回键号 1
					}
				}
				/* 检测 3 号键（PB14），逻辑同上 */
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
				/* 检测 2 号键（PB13），逻辑同上 */
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
				/* 检测 1 号键（PB12），逻辑同上 */
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
				
				else	// 注意：这个 else 挂在最后一个 if 上 —— 所有键都松开了
				{
					Keystate=0;	// 回到空闲状态
					break;
				}
				
				break;	
				
			}
			/* 状态3：已确认按下 → 在这里等待按键松开 */
			case 3:
			{
				/* 只要发现任意一个按键松开（读到 1） */
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1)
					{
						Delay_ms(1);
						timecount++;
						if(timecount==10)	// 连续 10 次确认松开（消抖）
						{
							timecount=0;
							Keystate = 1;	// 回到"确认"状态，等待下一次按下
						}
					}

					else	// 按键还按着 → 保持状态3，继续等待
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
