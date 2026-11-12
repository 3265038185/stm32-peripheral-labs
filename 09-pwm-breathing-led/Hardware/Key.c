/* ============================================================================
 * 文件说明：Key.c —— 独立按键驱动（轮询方式 + 状态机方式）
 * ----------------------------------------------------------------------------
 * 【硬件连接】
 *   4 个独立按键分别接在 GPIOB 的 PB12、PB13、PB14、PB15 上，
 *   按键一端接 IO 口，另一端接地（GND），按下时 IO 读到低电平(0)。
 *
 * 【用到的外设/知识点】
 *   - GPIO 输入模式：上拉输入 GPIO_Mode_IPU（按键松开时由内部上拉电阻拉高）
 *   - 按键消抖：机械按键按下/松开瞬间会抖动（电平快速跳变），
 *     通过延时 + 检测稳定电平来消除抖动
 *   - 状态机思想：Key_GetState / Key_One 用"状态 + 转移"的方式
 *     判断长按、短按、双击等复杂动作
 *
 * 【本文件提供的函数】
 *   - Key_Init     ：初始化按键 GPIO
 *   - Key_GetNum   ：轮询扫描，返回被按下按键的编号（0~4），松手后才返回
 *   - Key_GetState ：状态机方式，支持"持续按下时返回一次值"
 *   - Key_Oneneed  ：等待 Key1(PB15) 的一次按下，带超时返回
 *   - Key_One      ：综合状态机，区分 短按/长按/双击（返回 1/2/3）
 * ========================================================================== */

#include "stm32f10x.h"    
#include "Delay.h"// Device header
#include "sys.h"

/* ----------------------------------------------------------------------------
 * 函数名：Key_Init
 * 功能  ：初始化 4 个按键所用的 GPIO 引脚
 * 参数  ：无
 * 返回  ：无
 * 原理  ：GPIO 配置为"上拉输入"模式。上拉输入的意思是：
 *         内部有一个上拉电阻把引脚默认拉到高电平，按键松开时读到 1；
 *         按下时引脚被接地，读到 0。所以"检测到 0"就等于"按键按下"。
 *         注意：GPIO 的配置（CRL/CRH 寄存器）默认是浮空输入，
 *         必须显式改成上拉输入，否则按键状态不稳定。
 * -------------------------------------------------------------------------- */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	// 开启 GPIOB 的时钟。STM32 外设默认时钟是关闭的（为了省电），
	// 用之前必须先开时钟。GPIOB 挂在 APB2 总线上，所以用 RCC_APB2PeriphClockCmd

	GPIO_InitTypeDef GPIO_InitStructure;
	// 定义 GPIO 初始化结构体，下面填充配置参数后再调用 GPIO_Init 生效

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	// 配置为"上拉输入"模式（IPU = Input Pull-Up），按键松开时引脚被内部电阻拉高
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	// 选择要配置的引脚：PB12、PB13、PB14、PB15 四个按键
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// 引脚翻转速度：输入模式下此参数不影响功能，但标准写法需要填（最高 50MHz）
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	// 把上面填好的参数写入 GPIOB 的寄存器（CRH 寄存器），配置正式生效
	/*RCC->APB2ENR &= 1<<3;
	GPIOB->CRH &= 0x88880000; 
	*/
	/* 上面被注释的代码是"寄存器版"写法，与标准库函数等价：
	 *   RCC->APB2ENR &= 1<<3;    → 手动开启 GPIOB 时钟（bit3 对应 GPIOB）
	 *   GPIOB->CRH &= 0x88880000;→ 手动把 PB12~15 配成上拉输入
	 * 标准库函数封装了这些寄存器操作，用起来更安全，不易覆盖其他位 */
}

/* ----------------------------------------------------------------------------
 * 函数名：Key_GetNum
 * 功能  ：轮询扫描按键，返回当前被按下按键的编号
 * 参数  ：无
 * 返回  ：0=没有按键；1=Key1(PB12)；2=Key2(PB13)；3=Key3(PB14)；4=Key4(PB15)
 * 原理  ：依次检测每个按键。检测到按下(读到0)后，先延时 20ms 跳过机械抖动，
 *         while 空转等待"按键松开"，松手后延时 20ms 跳过松开抖动，
 *         最后才返回按键编号。这样"按一次松一次"只返回一次值，
 *         不会因为长按而重复触发。
 * 注意  ：这是个"阻塞式"函数，按下去不松手程序会卡在 while 里等。
 * -------------------------------------------------------------------------- */
unsigned char Key_GetNum(void)
{
	unsigned char KeyNum = 0;   // 默认返回 0（无按键）

	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0)   // 读 PB12：为 0 说明 Key1 被按下
	{
		Delay_ms(20);          // 消抖：刚按下时有 10~20ms 抖动，先等它稳定
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0);  // 死等按键松开
		Delay_ms(20);          // 消抖：等松开抖动也过去
		KeyNum = 1;            // 返回按键编号 1
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0)   // Key2(PB13)：逻辑同上
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0);
		Delay_ms(20);
		KeyNum = 2;
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0)   // Key3(PB14)：逻辑同上
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0);
		Delay_ms(20);
		KeyNum = 3;
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0)   // Key4(PB15)：逻辑同上
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
	/* 上面被注释的代码是"寄存器版"写法：GPIOB->IDR 是输入数据寄存器，
	 * 每一位对应一个引脚的电平。IDR & (1<<12) 取出第 12 位，
	 * ~ 取反后若为真说明该位为 0（按键按下）。原理与标准库函数完全相同 */

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
	/* 上面被注释的代码用的是 sys.h 里的位带操作宏（PBin(12) 等），
	 * 它通过"位带"技术把 GPIOB 的 IDR 寄存器某一个位映射成一个 32 位地址，
	 * 直接读该地址就能得到这个引脚的电平，读写速度比寄存器方式更快 */

	
	/* 最终说明：上面三个被注释的版本分别演示了 寄存器、位带宏、标准库 三种写法，
	 * 当前生效的是最上面的标准库写法。三者效果完全等价 */
	
	return KeyNum;   // 把检测到的按键编号返回给调用者（main 函数）
}

/* ----------------------------------------------------------------------------
 * 函数名：Key_GetState
 * 功能  ：状态机方式扫描按键。支持"按下期间只返回一次"，
 *         适合用来做计数加减（按住一直触发）等功能
 * 参数  ：无
 * 返回  ：0=无有效事件；1/2/3/4 = 对应按键产生了一次"有效按下"
 * 原理  ：（重点理解状态机）
 *   状态0(空闲)：没有按键按下，检测到任意键按下 → 进入状态1
 *   状态1(确认)：持续检测被按下的按键，累计延时计数 timecount，
 *                连续 10ms 都保持按下 → 返回按键编号并进入状态3（防止重复触发）
 *   状态3(防重复)：一直等所有按键松开，松开并稳定 10ms 后 → 回到状态0
 *   好处：不用"死等松手"，按键按下期间函数只返回一次，
 *         而且主循环能一边检测按键一边做别的事。
 * -------------------------------------------------------------------------- */
unsigned char Key_GetState(void)
{
	unsigned int timecount=0,Keystate=0;   // timecount：消抖计数；Keystate：当前状态
	while(1)                               // 这个函数本身是阻塞的，不返回就一直循环
	{
		switch (Keystate)                  // 根据当前状态执行对应的分支
		{
			case 0:                        // ── 状态0：空闲状态 ──
			{
				if((GPIOB->IDR & (1<<15))==0|| (GPIOB->IDR & (1<<14))==0||(GPIOB->IDR & (1<<13))==0||(GPIOB->IDR & (1<<12))==0)
				Keystate = 1;              // 只要任意一个按键被按下（读到 0）就转到状态1
				
					
				
				break;
			}
			case 1:                        // ── 状态1：按键消抖确认状态 ──
			{
				
				if((GPIOB->IDR & (1<<15))==0)   // Key4(PB15) 保持按下
				{
					Delay_ms(1);                // 延时 1ms（消抖采样）
					timecount++;                // 累计按下时间
					if(timecount==10)           // 连续 10ms 都按下 = 按键已稳定
					{
						timecount=0;            // 计数清零
						Keystate = 3;           // 进入状态3（等松手，防止重复触发）
						return 1;               // 返回：Key4 被按下
					}
				}
				if((GPIOB->IDR & (1<<14))==0)   // Key3(PB14)，逻辑同上
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 2;               // 返回：Key3 被按下
					}
				}
				if((GPIOB->IDR & (1<<13))==0)   // Key2(PB13)，逻辑同上
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 3;               // 返回：Key2 被按下
					}
				}
				if((GPIOB->IDR & (1<<12))==0)   // Key1(PB12)，逻辑同上
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 4;               // 返回：Key1 被按下
					}
				}
				
				else                           // 走到这里说明刚才按下的键已经松开了
				{
					Keystate=0;                // 回到空闲状态，重新等待
					break;
				}
				
				break;	
				
			}
			case 3:                        // ── 状态3：等待松手状态（防重复触发）──
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1)
					// 任意一个按键已经松开（读到 1）
					{
						Delay_ms(1);        // 消抖采样
						timecount++;
						if(timecount==10)   // 松开状态稳定 10ms
						{
							timecount=0;
							Keystate = 1;   // 回到"确认状态"，准备检测下一次按下
						}
					}

					else                   // 按键还没松开，继续保持状态3等待
					{
						Keystate=3;
						//break;
					}			
			}			
		}		
	}
}


/* ----------------------------------------------------------------------------
 * 函数名：Key_Oneneed
 * 功能  ：检测 Key4(PB15) 的一次"完整按下"，带超时退出
 * 参数  ：无
 * 返回  ：1=检测到一次有效按下；0=超时（200ms 内没有任何按键动作）
 * 原理  ：与 Key_GetState 类似的小型状态机（状态0/1/3），
 *         不同点：在状态0里每循环一次 i++，超过 200 次（约 200ms）就返回 0，
 *         用于"等待按键一段时间，超时就不等了"的场景（比如短按/长按判定）。
 * -------------------------------------------------------------------------- */
unsigned char Key_Oneneed(void)
{
	unsigned int timecount=0,Keystate=0,i=0;   // i：超时计数
	while(1)
	{		
		switch (Keystate)
		{
			case 0:                            // ── 状态0：空闲 ──
			{
				if((GPIOB->IDR & (1<<15))==0)  // Key4 被按下
				Keystate = 1;                  // 进入确认状态
				
				Delay_ms(1);
				i++;                           // 每 1ms 计数一次
				if(i>200)                      // 超过 200ms 还没有任何按键动作
				{
					return 0;                  // 超时返回 0
				}									
				break;
			}
			case 1:                            // ── 状态1：消抖确认 ──
			{
				
				if((GPIOB->IDR & (1<<15))==0)  // Key4 保持按下
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)          // 稳定按下 10ms
					{
						timecount=0;
						Keystate = 3;          // 进入等松手状态
						return 1;              // 返回：有效按下
					}
				}
				
				
				else                           // 确认过程中松手了
				{
					Keystate=0;                // 回到空闲重新等
					break;
				}
				
				break;	
				
			}
			case 3:                            // ── 状态3：等松手 ──
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)   // 已松开
					{
						Delay_ms(1);
						timecount++;
						if(timecount==10)      // 松开稳定 10ms
						{
							timecount=0;
							Keystate = 1;      // 回确认状态，准备下一次按下
						}
					}

					else                       // 还没松开
					{
						Keystate=3;
						break;
					}			
			}			
		}		
	}
}

/* ----------------------------------------------------------------------------
 * 函数名：Key_One
 * 功能  ：综合按键状态机，能区分 短按、长按、双击 三种操作
 * 参数  ：无
 * 返回  ：1=长按（按住超过约 500ms 后松开）；2=短按；3=双击
 * 原理  ：（5 个状态）
 *   状态0：空闲，Key4 按下 → 状态1
 *   状态1：消抖确认，稳定按下 10ms → 状态2
 *   状态2：按住期间累计 timecount（每 1ms +1），松开 → 状态3
 *   状态3：松开后判断：
 *         - timecount>=500（按了很久）→ 长按，返回 1
 *         - timecount<500（按得短）→ 进入状态4，等待看是否马上又按一次（双击）
 *   状态4：调用 Key_Oneneed 等待第二次按下：
 *         - 200ms 内又按了 → 双击，返回 3
 *         - 超时没按 → 判定为短按，返回 2
 * -------------------------------------------------------------------------- */
unsigned char Key_One(void)
{
	unsigned int timecount=0,Keystate=0,time=0;   // timecount：按住时长计数；time：松开时长计数
	while(1)
	{		
		switch (Keystate)
		{
			case 0:                        // ── 状态0：空闲 ──
			{				
				if((GPIOB->IDR & (1<<15))==0)   // Key4 按下
				Keystate = 1;                   // 进入消抖确认
				break;
			}
			case 1:                        // ── 状态1：消抖确认 ──
			{				
				if((GPIOB->IDR & (1<<15))==0)   // 持续按下
				{
					Delay_ms(1);
					timecount++;                // 累计按住时间
					if(timecount>=10)           // 稳定按下 10ms
					{
						Keystate = 2;           // 进入"按住中"状态
					}
				}
				else                             // 松开（抖动没过去）
				{
					Keystate=0;                  // 回到空闲
					break;
				}				
				break;					
			}
			case 2:                        // ── 状态2：按住中（累计时长）──
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)   // 检测到松开
				{	
					Keystate = 3;                // 进入"已松开，判长/短按"状态																	
				}		
					
				if((GPIOB->IDR & (1<<15))==0)    // 还没松
				{
					Delay_ms(1);
					timecount++;                 // 按住时间继续累加
					Keystate=2;
				}			
				break;			
			}	
			case 3:                        // ── 状态3：已松开，判断长按还是短按 ──
			{
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1)   // 确认一直保持松开
				{
					Delay_ms(1);
					time++;                     // 松开时间计数
					if(time>10)                 // 松开已稳定（超过 10ms）
					{
						if(timecount>=500)      // 之前按了 500ms 以上 → 长按
						{		
							timecount=0;
							time=0;
							Keystate = 1;       // 状态复位，准备下一次
							return 1;           // 返回：长按
						}
						if(timecount<500)       // 按的时间短 → 可能短按，也可能双击
						{
							timecount=0;
							time=0;
							Keystate = 4;       // 进入"等待双击"状态
						}							
					}
				}
				else                            // 松开期间又按下去了（重新按下）
				{
					time=0;
					//Keystate=3;
				}
				break;
			}
			case 4:                        // ── 状态4：等待第二次按下（判断双击）──
			{				
				if(Key_Oneneed()==1)            // 200ms 内检测到第二次按下 → 双击
				{ 					
					Keystate=0;
					return 3;                   // 返回：双击
				}
				else                            // 超时没有第二次按下 → 判定为短按
				{	Keystate=0;
					return 2;                   // 返回：短按
				}
				break;
			}
		}	
	}
}
