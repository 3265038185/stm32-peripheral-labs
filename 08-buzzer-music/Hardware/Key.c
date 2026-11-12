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
	/*  上面注释的版本用 sys.h 里的位带宏 Key1~Key4（PBin(12)~PBin(15)），
	    功能与上面完全相同，只是写法不同——相当于"读 IDR 寄存器第 n 位"。  */

	
	/*  返回键号：
	      如果是 0，说明这轮扫描没有检测到任何按键按下。  */
	return KeyNum;
}

/*
 * 函数名：Key_GetState（状态机版按键扫描 - 四键通用）
 * 参数：无
 * 返回：检测到"按下并松开一次"的键号（1~4）
 * 功能：用"有限状态机"实现带消抖的按键检测，并自动等待按键松开，
 *       所以调用一次 = 完成一次完整的"按下-确认-松开"过程。
 * 状态机设计（三种状态，Keystate 变量记录当前状态）：
 *   状态 0（KEY_NULL，空闲）：等待任意一个按键按下
 *       任一键读到低电平 -> 进入状态 1。
 *   状态 1（消抖确认/按下中）：确认是哪个键，并持续消抖计数
 *       每隔 1ms 检查一次，连续 10 次（约 10ms）都还是低电平，
 *       说明抖动已经过去、按键确实按下了 -> 返回该键号，进入状态 3。
 *       期间如果所有键都松开了（高电平）-> 回到状态 0。
 *   状态 3（等待松开）：等按键完全松开
 *       连续 10ms 读到高电平 -> 回到状态 1，等待下一次按下；
 *       如果还是低电平 -> 保持状态 3（按住不放就一直在等）。
 * 为什么要用状态机？
 *   状态 0/1/3 分别对应"没按/按下确认/等待松开"，
 *   比单纯 if 更严谨，能完美解决抖动和"按住不放重复触发"的问题。
 */
unsigned char Key_GetState(void)
{
	unsigned int timecount=0,Keystate=0;	// timecount：消抖计数；Keystate：当前状态
	while(1)	// 状态机主循环：不返回就一直检测（返回代表一次按键完成）
	{
		switch (Keystate)	// 根据当前状态决定下一步做什么
		{
			case 0:		// ===== 状态 0：空闲，等待按键按下 =====
			{
				/* 4 个按键任意一个为低电平（按下）就进入状态 1 */
				if((GPIOB->IDR & (1<<15))==0|| (GPIOB->IDR & (1<<14))==0||(GPIOB->IDR & (1<<13))==0||(GPIOB->IDR & (1<<12))==0)
				Keystate = 1;
				/*  GPIOB->IDR 是"输入数据寄存器"，bit15~12 对应 PB15~12 的电平，
				    (1<<15) 表示"把 1 左移 15 位"，与运算后：
				    该位为 0（按下）时整个表达式为 0，取反判断即可。
				    这里写成 ==0 表示"读到低电平"。  */
				
					
				
				break;
			}
			case 1:		// ===== 状态 1：按下确认（消抖 + 区分键号） =====
			{
				/* 以下 4 个 if 结构相同，分别判断 PB15/PB14/PB13/PB12：
				   每隔 1ms 检查一次，timecount 连续累计 10 次（10ms）
				   都保持低电平，说明不是抖动而是真的按下 -> 返回键号 */
				if((GPIOB->IDR & (1<<15))==0)
				{
					Delay_ms(1);			// 1ms 采样一次
					timecount++;			// 连续按下计数 +1
					if(timecount==10)		// 累计 10 次 = 稳定按下 10ms
					{
						timecount=0;		// 计数清零，供下次使用
						Keystate = 3;		// 转入"等待松开"状态
						return 1;			// 返回键号 1（PB15）
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
						return 2;			// 返回键号 2（PB14）
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
						return 3;			// 返回键号 3（PB13）
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
						return 4;			// 返回键号 4（PB12）
					}
				}
				
				else	// 4 个键都松开了（说明刚才只是抖动）
				{
					Keystate=0;				// 回到状态 0 重新等待
					break;
				}
				
				break;	
				
			}
			case 3:		// ===== 状态 3：等待按键完全松开 =====
			{
				/* 任意一个键变成高电平（松开）时开始计时，
				   连续 10ms 保持高电平 -> 确定已松开，回到状态 1；
				   若仍是低电平（按住不放）-> 保持状态 3 继续等 */
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1)
					{
						Delay_ms(1);
						timecount++;
						if(timecount==10)
						{
							timecount=0;
							Keystate = 1;	// 松开完成，回到"等待下一次按下"
						}
					}

					else
					{
						Keystate=3;			// 还按着，继续等待
						//break;
					}			
			}			
		}		
	}
}


/*
 * 函数名：Key_Oneneed（状态机版按键检测 - 只针对 PB15）
 * 参数：无
 * 返回：1 = PB15 被按下一次；0 = 超过 200ms 都没有按下（超时）
 * 功能：和 Key_GetState 思路相同（状态 0/1/3 三步消抖），
 *       但只检测 PB15 一个键，并加了一个"超时保护"：
 *       在状态 0 里每 1ms 让 i 加 1，超过 200 次（200ms）仍没按下
 *       就直接返回 0，避免死循环卡死主程序。
 * 用途：它被 Key_One 调用，用来判断"双击时第二次按下是否出现"。
 */
unsigned char Key_Oneneed(void)
{
	unsigned int timecount=0,Keystate=0,i=0;	// i：超时计数
	while(1)
	{		
		switch (Keystate)
		{
			case 0:		// ===== 状态 0：空闲等待 =====
			{
				if((GPIOB->IDR & (1<<15))==0)	// PB15 按下 -> 进入确认状态
				Keystate = 1;
				
				Delay_ms(1);	// 每 1ms 检查一次
				i++;			// 超时计数 +1
				if(i>200)		// 超过 200ms 还没按下
				{
					return 0;	// 超时返回 0（告诉调用者"没有第二次按下"）
				}									
				break;
			}
			case 1:		// ===== 状态 1：按下确认（10ms 消抖） =====
			{
				
				if((GPIOB->IDR & (1<<15))==0)
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)	// 连续按下 10ms，确认不是抖动
					{
						timecount=0;
						Keystate = 3;
						return 1;		// 确认按下 -> 返回 1
					}
				}
				
				
				else	// 中途松开了 -> 回到空闲
				{
					Keystate=0;
					break;
				}
				
				break;	
				
			}
			case 3:		// ===== 状态 3：等待松开 =====
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)	// 松开了
					{
						Delay_ms(1);
						timecount++;
						if(timecount==10)	// 连续 10ms 高电平 -> 已松开
						{
							timecount=0;
							Keystate = 1;	// 回到等待下一次按下
						}
					}

					else
					{
						Keystate=3;		// 还按着，继续等
						break;
					}			
			}			
		}		
	}
}
/*
 * 函数名：Key_One（状态机 - 长按 / 短按 / 双击 识别）
 * 参数：无
 * 返回：1 = 长按；2 = 双击（第二次按下是短按）；3 = 短按（只有一次按下）
 * 功能：只针对 PB15，识别三种按键行为：
 *         - 短按：按下后很快松开（< 500ms）然后不再按
 *         - 长按：按住超过 500ms 才松开
 *         - 双击：短按两次（两次间隔 < 200ms）
 * 状态机设计（5 个状态）：
 *   状态 0：空闲，等待按下
 *   状态 1：按下消抖确认（连续 10ms），确认后进状态 2
 *   状态 2：按住中，等待松开（松开 -> 状态 3）
 *   状态 3：已松开，开始计时（time 累计 >10ms 才做判断），
 *           timecount 是"这次按住的总时长"：
 *             >=500ms -> 长按，返回 1；
 *             <500ms  -> 短按，进入状态 4 判断是否还有第二次按下
 *   状态 4：等待"第二次按下"（调用 Key_Oneneed 最多等 200ms）：
 *            第二次按下 -> 双击，返回 3；
 *            超时没按   -> 只有一次短按，返回 2。
 * 小结：这就是嵌入式常见的"按键多功能识别"思路，
 *       核心是"用时间量区分行为"：按住时长 > 500ms 是长按，
 *       两次按下间隔 < 200ms 是双击。
 */
unsigned char Key_One(void)
{
	unsigned int timecount=0,Keystate=0,time=0;	// time：松开后的计时
	while(1)
	{		
		switch (Keystate)
		{
			case 0:		// ===== 状态 0：空闲，等待 PB15 按下 =====
			{				
				if((GPIOB->IDR & (1<<15))==0)	// 按下（低电平）-> 进入确认
				Keystate = 1;
				break;
			}
			case 1:		// ===== 状态 1：按下消抖确认 =====
			{				
				if((GPIOB->IDR & (1<<15))==0)
				{
					Delay_ms(1);
					timecount++;
					if(timecount>=10)	// 连续按下 10ms，确认真的按下
					{
						Keystate = 2;	// 进入"按住中"状态
					}
				}
				else	// 中途松开 -> 只是抖动，回到空闲
				{
					Keystate=0;
					break;
				}				
				break;					
			}
			case 2:		// ===== 状态 2：按住中（等待松开） =====
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)	// 松开了
				{	
					Keystate = 3;	// 进入"松开后计时判断"状态																	
				}		
					
				if((GPIOB->IDR & (1<<15))==0)
				{
					Delay_ms(1);
					timecount++;	// 按住期间持续计时（timecount = 按住总时长 ms）
					Keystate=2;		// 保持按住状态
				}			
				break;			
			}	
			case 3:		// ===== 状态 3：已松开，判断长按/短按 =====
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)	// 确认还处于松开状态
				{
					Delay_ms(1);
					time++;			// 松开后的计时 +1ms
					if(time>10)		// 松开稳定 10ms 后再做判断（避免抖动）
					{
						if(timecount>=500)	// 按住总时长 >= 500ms
						{		
							timecount=0;	// 清零，为下一次按键做准备
							time=0;
							Keystate = 1;	// 回到等待（其实马上要返回了）
							return 1;		// 返回 1：识别为【长按】							
						}
						if(timecount<500)	// 按住总时长 < 500ms
						{
							timecount=0;
							time=0;
							Keystate = 4;	// 进入"等待第二次按下"状态
						}							
					}
				}
				else	// 又按下去了（误判），重新计时
				{
					time=0;
					//Keystate=3;
				}
				break;
			}
			case 4:		// ===== 状态 4：等待第二次按下（判断是否双击） =====
			{				
				if(Key_Oneneed()==1)	// 200ms 内又按下了一次
				{ 					
					Keystate=0;			// 复位状态机
					return 3;			// 返回 3：识别为【双击】
				}
				else	// 200ms 内没有第二次按下
				{	Keystate=0;			// 复位状态机
					return 2;			// 返回 2：只有一次【短按】
				}
				break;
			}
		}	
	}
}
