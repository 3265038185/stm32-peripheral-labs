/*==============================================================================
 * 文件：Key.c（按键模块）
 * 功能：提供四种按键读取方式，按键接在 GPIOB 的 Pin12~Pin15 上，
 *       按键按下时对应引脚读到低电平（0），松开时是高电平（1）。
 *       1. Key_GetNum()    ：阻塞式读取，每按一次返回对应的按键号（1~4）；
 *       2. Key_GetState()  ：状态机消抖，返回"正在按着的键"（1~4）；
 *       3. Key_One()       ：区分"短按/长按/双击"的按键状态机；
 *       4. Key_Oneneed()   ：等待按一次键，按下返回1，超时(约200ms)返回0。
 * 知识点：GPIO上拉输入(IPU)、按键消抖（软件延时消抖）、状态机思想、
 *         位操作（GPIOB->IDR & (1<<15) 读取某一位）
 *============================================================================*/
#include "stm32f10x.h"    
#include "Delay.h"// Device header（延时函数：用于按键消抖）
#include "sys.h"	// 系统头文件：定义了 PBin() 等位带操作宏（部分被注释的代码用）

/* 按键初始化函数：
   功能：把 GPIOB 的 Pin12~15 配置成"上拉输入"模式，用于读取按键状态。
   为什么用上拉输入(IPU)？因为按键电路通常一端接GND、另一端接IO口，
   按下时引脚被拉到0V（低电平），松开时靠内部上拉电阻把引脚拉高到3.3V（高电平），
   这样只要读电平就能知道按键有没有被按下。 */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	// ①开启GPIOB外设的时钟！
									// 注意：STM32所有外设的时钟默认是关闭的（省电），
									// 使用前必须先"打开时钟"，否则寄存器写不进去
	GPIO_InitTypeDef GPIO_InitStructure;	// 定义GPIO初始化结构体（配置参数放在这里）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	// 模式=上拉输入（Input Pull-Up）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	// 用到的引脚：PB12~PB15
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 输出速度50MHz（输入模式时该参数无意义，习惯性填写）
	GPIO_Init(GPIOB,&GPIO_InitStructure);	// ②把配置写入GPIOB的寄存器，完成初始化
	/*RCC->APB2ENR &= 1<<3;
	GPIOB->CRH &= 0x88880000; 
	*/	// 上面是被注释的"寄存器直接操作"写法：
		// 打开GPIOB时钟（APB2ENR的bit3）、配置PB12~15为输入模式（CRH寄存器），
		// 效果和上面的标准库函数一样，这里给大家做个对比参考
}

/* 按键读取函数（阻塞式）：每调用一次，如果检测到按键按下并松开，
   就返回按键号（1~4），否则返回0。
   实现原理：先查引脚电平是否为0（按下），
            是则延时20ms跳过"按下抖动"，然后死等按键松开（消除"松开抖动"），
            最后再延时20ms，返回按键号。
   注意：这是一个"阻塞"函数，如果按键一直不松手，程序就会一直停在这里。 */
unsigned char Key_GetNum(void)
{
	unsigned char KeyNum = 0;	// 定义返回值：默认0表示"没有按键按下"
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0)	// 读PB12：若为0说明按键1被按下
	{
		Delay_ms(20);	// 延时20ms跳过按键"按下"瞬间的机械抖动（防止一次按下读成多次）
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0);	// 死等：直到按键松开（读到高电平1）才继续
		Delay_ms(20);	// 再延时20ms跳过"松开"瞬间的抖动
		KeyNum = 1;		// 确认是一次完整的"按下-松开"，返回按键1
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0)	// 读PB13：按键2按下？
	{
		Delay_ms(20);	// 消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0);	// 等松开
		Delay_ms(20);	// 消抖
		KeyNum = 2;		// 返回按键2
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0)	// 读PB14：按键3按下？
	{
		Delay_ms(20);	// 消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0);	// 等松开
		Delay_ms(20);	// 消抖
		KeyNum = 3;		// 返回按键3
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0)	// 读PB15：按键4按下？
	{
		Delay_ms(20);	// 消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0);	// 等松开
		Delay_ms(20);	// 消抖
		KeyNum = 4;		// 返回按键4
		
	}
	


	/*if(~(GPIOB->IDR&1<<12))	// 被注释的写法1：直接操作寄存器
	{	// ~(GPIOB->IDR&1<<12)：IDR是输入数据寄存器，读它的bit12，
		// 与1<<12相与取出第12位，再取反，按下时=1（真）
		Delay_ms(20);
		while(~(GPIOB->IDR&1<<12));	// 等松开
		Delay_ms(20);
		KeyNum = 1;
		
	}
	else if(~(GPIOB->IDR&1<<13))	// 按键2（写法和上面同理）
	{
		Delay_ms(20);
		while(~(GPIOB->IDR&1<<13));
		Delay_ms(20);
		KeyNum = 2;
		
	}
	else if(~(GPIOB->IDR&1<<14))	// 按键3
	{
		Delay_ms(20);
		while(~(GPIOB->IDR&1<<14));
		Delay_ms(20);
		KeyNum = 3;
		
	}
	else if(~(GPIOB->IDR&1<<15))	// 按键4
	{
		Delay_ms(20);
		while(~(GPIOB->IDR&1<<15));
		Delay_ms(20);
		KeyNum = 4;
		
	}
	*/
	
	
	/*if(Key1 == 0)	// 被注释的写法2：使用sys.h里定义的宏 Key1 = PBin(12)
	{	// PBin(12) 是"位带操作"：把GPIOB的IDR寄存器第12位映射到SRAM的一个地址，
		// 读写这个地址就等于读写该引脚，和直接操作寄存器效果一样但更直观
		Delay_ms(20);
		while(Key1 == 0);	// 等松开
		Delay_ms(20);
		KeyNum = 1;
		
	}
	else if(Key2 == 0)	// 按键2
	{
		Delay_ms(20);
		while(Key2 == 0);
		Delay_ms(20);
		KeyNum = 2;
		
	}
	else if(Key3 == 0)	// 按键3
	{
		Delay_ms(20);
		while(Key3 == 0);
		Delay_ms(20);
		KeyNum = 3;
		
	}
	else if(Key4 == 0)	// 按键4
	{
		Delay_ms(20);
		while(Key4 == 0);
		Delay_ms(20);
		KeyNum = 4;
		
	}
	*/

	
	
	return KeyNum;	// 返回按键号（0表示本次调用没有检测到按键）
}

/* 按键状态机读取函数（非阻塞思路）：
   功能：返回"当前正被按住的按键号"（1~4）。用状态机实现了软件消抖，
   只有确认按键"稳定按下"（连续10ms都读到低电平）才返回按键号，
   比Key_GetNum()的延时消抖更高效，不浪费CPU时间。
   状态说明：
     状态0：等待按键（没检测到按下）
     状态1：检测到按下，正在持续确认（连续计满10ms判定为有效按下）
     状态3：已经确认按下，等待按键松开 */
unsigned char Key_GetState(void)
{
	unsigned int timecount=0,Keystate=0;	// timecount：计时计数用；Keystate：当前状态
	while(1)	// 这个函数是"阻塞"的：一直循环，直到有按键被稳定按下并返回
	{
		switch (Keystate)	// 根据当前状态执行不同的处理
		{
			case 0:	// 状态0：等待按键按下
			{
				// 用"位与"运算读四个按键：GPIOB->IDR&(1<<15) 是取IDR寄存器的bit15，
				// 为0表示PB15是低电平=该按键按下。四个键任一按下就进入状态1
				if((GPIOB->IDR & (1<<15))==0|| (GPIOB->IDR & (1<<14))==0||(GPIOB->IDR & (1<<13))==0||(GPIOB->IDR & (1<<12))==0)
				Keystate = 1;	// 转入状态1：开始消抖确认
				
					
				
				break;	// 跳出switch
			}
			case 1:	// 状态1：消抖确认阶段（连续10次读到低电平才算真按下）
			{
				
				if((GPIOB->IDR & (1<<15))==0)	// 若PB15（按键4）仍为低电平
				{
					Delay_ms(1);	// 延时1ms
					timecount++;	// 计数+1
					if(timecount==10)	// 连续10ms都按着=消抖成功，确认按键4按下
					{
						timecount=0;	// 计数器清零
						Keystate = 3;	// 转入状态3（等待松开）
						return 1;		// 返回按键号1（注意：这里返回的是固定编号1，与引脚无直接对应关系）
					}
				}
				if((GPIOB->IDR & (1<<14))==0)	// 若PB14（按键3）为低电平
				{
					Delay_ms(1);	// 延时1ms
					timecount++;	// 计数+1
					if(timecount==10)	// 连续10ms确认按下
					{
						timecount=0;	// 清零
						Keystate = 3;	// 转入状态3
						return 2;		// 返回按键号2
					}
				}
				if((GPIOB->IDR & (1<<13))==0)	// 若PB13（按键2）为低电平
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 3;	// 返回按键号3
					}
				}
				if((GPIOB->IDR & (1<<12))==0)	// 若PB12（按键1）为低电平
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 4;	// 返回按键号4
					}
				}
				
				else	// 上述判断都未满足（说明刚才按的键已经松开了，抖动未消）
				{
					Keystate=0;	// 回到状态0，重新等待按键
					break;
				}
				
				break;	
				
			}
			case 3:	// 状态3：已确认按键按下，等待松开（松开也做10ms消抖）
			{
				// 四个按键任一松开（读到高电平1）就开始计时
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1)
					{
						Delay_ms(1);	// 延时1ms
						timecount++;	// 计数+1
						if(timecount==10)	// 连续10ms都松开=确认松开，准备下一次检测
						{
							timecount=0;	// 清零
							Keystate = 1;	// 回到状态1（注意：此时若有按键还按着会立即再确认）
						}
					}

					else	// 按键还按着
					{
						Keystate=3;	// 保持状态3不动，继续等待松开
						//break;
					}			
			}			
		}		
	}
}


/* 按键等待函数（带超时）：只关心一个按键（PB15）。
   功能：一直等用户"按一次键"，按下并松开后返回1；
   如果等了大约200ms（200次×1ms）还没有按键，就返回0（超时）。
   用途：常常配合"双击/多击"检测使用，用来判断"两次按键之间的间隔"。 */
unsigned char Key_Oneneed(void)
{
	unsigned int timecount=0,Keystate=0,i=0;	// timecount消抖计数；Keystate状态；i超时计数
	while(1)	// 阻塞等待
	{		
		switch (Keystate)
		{
			case 0:	// 状态0：等待按下
			{
				if((GPIOB->IDR & (1<<15))==0)	// PB15（按键）按下（低电平）
				Keystate = 1;	// 进入消抖状态1
				
				Delay_ms(1);	// 每次循环延时1ms
				i++;			// 超时计数+1
				if(i>200)		// 超过200ms还没按键
				{
					return 0;	// 超时，返回0
				}									
				break;
			}
			case 1:	// 状态1：消抖确认
			{
				
				if((GPIOB->IDR & (1<<15))==0)	// 按键仍然按着（低电平）
				{
					Delay_ms(1);	// 延时1ms
					timecount++;	// 计数+1
					if(timecount==10)	// 连续10ms=消抖成功，确认按下
					{
						timecount=0;	// 清零
						Keystate = 3;	// 进入状态3（等松开）
						return 1;		// 返回1：检测到一次按下
					}
				}
				
				
				else	// 按键松开了（说明刚才只是抖动，不是真按下）
				{
					Keystate=0;	// 回到状态0重新等待
					break;
				}
				
				break;	
				
			}
			case 3:	// 状态3：等待松开
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)	// 按键已松开（高电平）
					{
						Delay_ms(1);	// 延时1ms
						timecount++;	// 计数+1
						if(timecount==10)	// 松开10ms确认
						{
							timecount=0;	// 清零
							Keystate = 1;	// 回到状态1，为下一次按下做准备
						}
					}

					else	// 按键还按着
					{
						Keystate=3;	// 保持等待松开的状态
						break;
					}			
			}			
		}		
	}
}

/* 按键"短按/长按/双击"识别函数（状态机实现）：
   功能：阻塞等待用户对PB15按键进行一次操作，然后返回结果：
         返回1 = 长按（按下的时间 >= 500ms）
         返回2 = 双击（短按两次，且两次间隔 <= 100ms左右）
         返回3 = 单击（只按了一次短按）
   原理：用 timecount 累计"按住的时间"，用 time 累计"松开的时间"，
        按下的总时长>=500ms判为长按；松开的间隔很短又按了一次判为双击。 */
unsigned char Key_One(void)
{
	unsigned int timecount=0,Keystate=0,time=0;	// timecount按住计时；time松开计时；Keystate状态
	while(1)	// 阻塞等待按键操作完成
	{		
		switch (Keystate)
		{
			case 0:	// 状态0：等待按键按下
			{				
				if((GPIOB->IDR & (1<<15))==0)	// PB15按下（低电平）
				Keystate = 1;	// 进入状态1
				break;
			}
			case 1:	// 状态1：按下消抖（连续10ms确认）
			{				
				if((GPIOB->IDR & (1<<15))==0)	// 一直按着
				{
					Delay_ms(1);	// 延时1ms
					timecount++;	// 按住时间+1
					if(timecount>=10)	// 10ms消抖完成
					{
						Keystate = 2;	// 进入状态2：确认已经按下
					}
				}
				else	// 中途松开了（抖动）
				{
					Keystate=0;	// 回到状态0
					break;
				}				
				break;					
			}
			case 2:	// 状态2：按下已确认，继续等待（此处不消抖，直接看是否松开）
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)	// 按键松开（高电平）
				{	
					Keystate = 3;	// 进入状态3：开始测量"松开的时间"																	
				}		
					
				if((GPIOB->IDR & (1<<15))==0)	// 按键还按着（继续按住）
				{
					Delay_ms(1);	// 延时1ms
					timecount++;	// 按住时间继续累计（用于判断长按）
					Keystate=2;		// 保持状态2
				}			
				break;			
			}	
			case 3:	// 状态3：已松开，测量松开时间（判断是单击/双击/长按）
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)	// 仍然松着
				{
					Delay_ms(1);	// 延时1ms
					time++;			// 松开时间+1
					if(time>10)		// 松开超过10ms，说明"这一击"已经结束
					{
						if(timecount>=500)	// 如果刚才按住的总时间>=500ms
						{		
							timecount=0;	// 清零，准备下一次
							time=0;			// 清零
							Keystate = 1;	// 回到状态1等待
							return 1;		// 返回1：判定为"长按"							
						}
						if(timecount<500)	// 按住时间不足500ms = 一次"短按"
						{
							timecount=0;	// 清零
							time=0;			// 清零
							Keystate = 4;	// 进入状态4：等待看是否马上有第二次按下（双击）
						}							
					}
				}
				else	// 松开后又有按下（说明用户又按了一次）
				{
					time=0;	// 松开时间清零，重新计时（准备测双击间隔）
					//Keystate=3;
				}
				break;
			}
			case 4:	// 状态4：刚完成一次短按，等待"第二次按下"（判断双击）
			{				
				if(Key_Oneneed()==1)	// 调用等待函数：200ms内又按了一次
				{ 					
					Keystate=0;	// 回到状态0
					return 3;	// 返回3：判定为"双击"
				}
				else	// 200ms内没有第二次按下
				{	Keystate=0;	// 回到状态0
					return 2;	// 返回2：判定为"单击"
				}
				break;
			}
		}	
	}
}
