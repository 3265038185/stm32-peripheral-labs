/*===============================================================================
 * 文件：Key.c    项目：2-按键中断
 *
 * 【这个文件是干什么的？】
 *   按键模块的全部实现：按键 GPIO 初始化 + 各种按键扫描函数。
 *   本工程提供了多种"读按键"的写法，由浅入深：
 *     1. Key_GetNum()  —— 最简单的"轮询 + 延时消抖"，适合入门理解；
 *     2. Key_GetState()—— 状态机扫描，能识别"按下"并等"松手"再响应；
 *     3. Key_plus()    —— 加减计数器：单击加一/减一，松手才返回；
 *     4. Key_One()     —— 最复杂的用法：区分 短按 / 双击 / 长按。
 *
 * 【硬件接线（共阴极接法）】
 *   按键一端接 STM32 的 IO 口（PB12~PB15），另一端接地（GND）：
 *   - 按键没按下：IO 口被内部上拉电阻"拉"到高电平（读到 1）；
 *   - 按键按下：IO 口被导线直接连到 GND，变成低电平（读到 0）。
 *   所以"检测到按键按下" = "读到低电平 0"。
 *
 * 【为什么要用上拉输入（GPIO_Mode_IPU）？】
 *   如果输入引脚悬空（既不接高也不接地），电平会飘忽不定，读 0 读 1 全看
 *   运气。上拉输入就是在芯片内部把引脚经一个几十千欧的电阻接到 3.3V，
 *   让引脚"默认是高电平"，只有按键按下时才被拉到 0，电平稳定、判断可靠。
 *
 * 【什么是按键消抖？为什么需要？】
 *   机械按键按下的瞬间，金属簧片会"弹跳"几次（约 5~20ms），电平在高/低
 *   之间快速抖动。如果 CPU 这时去读引脚，可能一会儿读到 0 一会儿读到 1，
 *   一次按键会被误判成好几次。解决办法：读到按下后延时 10ms 左右再读一次，
 *   如果还是按下，才认定"真的按下"——此时抖动早就过去了。
 *===============================================================================*/
#include "stm32f10x.h"    // 标准外设库总头文件
#include "Delay.h"// Device header    // 延时函数（Delay_ms）所在头文件
#include "sys.h"    // 位带操作宏（PAout/PBin 等）所在头文件
#include "Key.h"    // 本模块自己的头文件（函数声明）

/*
 * 函数：Key_Init —— 按键 GPIO 初始化
 * 功能：把 PB12~PB15 四个引脚配置成"上拉输入"模式，用于读取按键电平
 * 参数：无
 * 返回：无
 * 原理（结合 STM32 标准库）：
 *   1. RCC_APB2PeriphClockCmd：开启 GPIOB 外设的时钟！
 *      这是 STM32 的规矩：任何外设（包括 GPIO）使用前必须先打开它的时钟，
 *      否则寄存器写不进去（芯片为了省电，默认把外设时钟全部关闭）。
 *      GPIO 挂在 APB2 总线上，所以用 RCC_APB2PeriphClockCmd 来开。
 *   2. GPIO_InitStructure 三个成员的含义：
 *      - GPIO_Mode = GPIO_Mode_IPU：IPU = Input Pull-Up（输入上拉），
 *        配置成上拉输入来读按键（见文件头部的上拉解释）；
 *      - GPIO_Pin：用"按位或 |"把四个引脚合在一起，一次性配置 PB12~PB15；
 *      - GPIO_Speed：输出速度，对"输入"模式其实不起作用，但标准库
 *        初始化输入时也要求填这项（填 50MHz 即可，不影响功能）。
 *   3. GPIO_Init(GPIOB, &GPIO_InitStructure)：把上面配置写入 GPIOB 的
 *      CRH 寄存器，使配置真正生效。
 *   4. 下面被注释的是"直接操作寄存器"的等价写法：
 *      RCC->APB2ENR &= 1<<3（打开 GPIOB 时钟的位3）、
 *      GPIOB->CRH &= 0x88880000（清掉引脚12~15的配置位）。
 *      标准库函数底层干的就是这件事，用库函数更直观、更不容易出错。
 */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	//① 打开 GPIOB 时钟（必做！）
	GPIO_InitTypeDef GPIO_InitStructure;					//② 定义 GPIO 初始化结构体变量
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//③ 模式：上拉输入（按键未按时读1）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	//④ 同时配置PB12~PB15
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//⑤ 速度50MHz（输入模式下不起作用）
	GPIO_Init(GPIOB,&GPIO_InitStructure);					//⑥ 把配置写入寄存器，生效！
	/*RCC->APB2ENR &= 1<<3;
	GPIOB->CRH &= 0x88880000; 
	*/
}
unsigned char KeyNum = 0;	// 全局变量：记录"刚刚被按下的按键编号"，供 Key_GetNum 返回

/*
 * 函数：Key_GetNum —— 轮询式按键扫描（入门版）
 * 功能：判断 4 个按键中哪个被按下，返回按键编号（1=PB12，2=PB13，3=PB14，4=PB15）
 * 参数：无
 * 返回：按键编号（1~4）；没有新按键按下时返回上一次的 KeyNum 值
 * 原理：
 *   1. 直接读 GPIOB->IDR 寄存器（IDR = Input Data Register，输入数据寄存器）：
 *      寄存器的每一位对应一个引脚当前的电平（1=高，0=低），一次读操作就能
 *      同时拿到 16 个引脚的状态，比逐个引脚读更高效。
 *   2. 判断方法：把 IDR 的值和某个"特征值"整体比较。例如 0x7FFB 换成二进制
 *      是 0111 1111 1111 1011，从右往左数第 12 位是 0、其余位都是 1，
 *      正好对应"只有 PB12 被按下、其他引脚都保持高电平"的状态。
 *   3. 消抖：第一次读到按下后 Delay_ms(10) 等 10ms，再读一次如果还是按下，
 *      才确认"真的按下了"（避开按键机械抖动）。
 *   4. 注意：这个函数没有"松手检测"，只要按下瞬间被确认，KeyNum 就一直
 *      记住这个编号，主循环反复调用也只会返回同一个编号。
 */
unsigned char Key_GetNum(void)
{
	//【被注释掉的写法A：用标准库函数逐位读取电平】
	// GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) 一次只读某一个引脚的电平。
	// 逻辑：读到 0（按下）→ 延时 10ms 消抖 → 再读仍为 0 → 确认是按键1。
	// 这种写法和下面"直接比较 IDR 寄存器"的写法效果一样，只是读取方式不同。
//	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0)
//	{
//		Delay_ms(10);
//		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0)
//		KeyNum = 1;
//	
//	}
//	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0)
//	{
//		Delay_ms(10);
//		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0)
//		KeyNum = 2;
//		
//	}
//	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0)
//	{
//		Delay_ms(10);
//		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0)
//		KeyNum = 3;
//		
//	}
//	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0)
//	{
//		Delay_ms(10);
//		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0)
//		KeyNum = 4;
//		
//	}
	//【当前实际使用的写法B：直接比较整个 IDR 寄存器】
	// 0x7FFB = 0111 1111 1111 1011 → 第12位为0、其余为1 → 只有 PB12 被按下
	if(GPIOB->IDR == 0x00007ffb)
	{
		Delay_ms(10);					// 消抖：等 10ms，让机械抖动过去
		if(GPIOB->IDR == 0x00007ffb)	// 再读一次，确实还按着 → 确认"真按下"
		KeyNum = 1;						// 记录按键编号 1
	}
	// 0xBFFB = 1011 1111 1111 1011 → 第13位为0 → 只有 PB13 被按下
	if(GPIOB->IDR == 0x0000Bffb)
	{
		Delay_ms(10);
		if(GPIOB->IDR == 0x0000Bffb)
		KeyNum = 2;
	}
	// 0xDFFB = 1101 1111 1111 1011 → 第14位为0 → 只有 PB14 被按下
	if(GPIOB->IDR == 0x0000Dffb)
	{
		Delay_ms(10);
		if(GPIOB->IDR == 0x0000Dffb)
		KeyNum = 3;
	}
	// 0xEFFB = 1110 1111 1111 1011 → 第15位为0 → 只有 PB15 被按下
	if(GPIOB->IDR == 0x0000Effb)
	{
		Delay_ms(10);
		if(GPIOB->IDR == 0x0000Effb)
		KeyNum = 4;
	}
		



//【被注释掉的写法C：用 sys.h 的位带宏读按键】
	// Key1 == 0 等价于"GPIOB->IDR 的第 12 位是 0"。位带操作把"读某一位"
	// 变成"读某个内存地址"，一条语句只操作一个引脚，写起来更直观。
	// 注意：这段被注释掉了，当前实际用的是上面直接比较 IDR 的写法。
//	if(Key1 == 0)
//	{
//		Delay_ms(20);
//		if(Key1 == 0)
//		KeyNum = 1;
//		
//	}
//	if(Key2 == 0)
//	{
//		Delay_ms(20);
//		if(Key2 == 0)
//		KeyNum = 2;
//		
//	}
//	if(Key3 == 0)
//	{
//		Delay_ms(20);
//		if(Key3 == 0)
//		KeyNum = 3;
//		
//	}
//	if(Key4 == 0)
//	{
//		Delay_ms(20);
//		if(Key4 == 0)
//		KeyNum = 4;
//		
//	}
	

	// 没有任何按键满足条件时，返回上次记录的 KeyNum
	return KeyNum;
}
/*
 * 函数：Key_GetState —— 状态机按键扫描（可识别"按下"与"松手"）
 * 功能：返回被按下按键的编号（1=PB12，2=PB13，3=PB14，4=PB15）。
 *       只有"消抖确认按下"（连续 10ms）才返回；返回后继续等按键全部松开，
 *       才重新开始检测下一次按键，保证"按一下只响应一次"。
 * 参数：无
 * 返回：1/2/3/4（对应按键编号）。本函数是死循环，没有按键事件时不返回。
 * 原理（状态机思想）：
 *   状态0（空闲）：检测到任意一个按键按下 → 进入状态1；
 *   状态1（消抖确认）：哪个键按着就给哪个键计时，连续 10ms → 返回该键编号，
 *       进入状态3（注意：10ms 计时由四个键共用同一个 timecount 完成，互斥有效）；
 *   状态3（等松手）：所有键都松开（连续 10ms）→ 回到状态0，准备下一次。
 *   状态机的本质：把"一个完整的按键过程"拆成几个阶段，每个阶段只处理一件事，
 *   程序结构清晰，也方便以后扩展"长按"等新功能。
 */
unsigned char Key_GetState(void)
{
	unsigned int timecount=0,Keystate=0;	// timecount：消抖计时器；Keystate：当前状态
	while(1)		// 死循环：一直扫描按键，相当于把"按键检测"占满整个函数
	{
		switch (Keystate)	// 根据当前状态执行对应处理
		{
			case 0:		// 【状态0：空闲态】等待按键按下
			{
				// 逐个检查 15/14/13/12 位是否有一个是 0（按下）。
				// 表达式 (GPIOB->IDR & (1<<15))==0 的含义：把 IDR 与"只有第15位
				// 是1的数"做按位与，结果非0 → 第15位是1 → 该键没按；
				// 结果为0 → 第15位是0 → PB15 被按下。
				if((GPIOB->IDR & (1<<15))==0|| (GPIOB->IDR & (1<<14))==0||(GPIOB->IDR & (1<<13))==0||(GPIOB->IDR & (1<<12))==0)
				Keystate = 1;	// 只要有任意一个键按下，就跳到状态1（消抖确认）
				
					
				
				break;
			}
			case 1:		// 【状态1：消抖确认态】确认是哪个键被按，并消抖
			{
				
				if((GPIOB->IDR & (1<<15))==0)	// PB15 被按着？
				{
					Delay_ms(1);	// 每次循环延时 1ms，相当于"1ms 扫描一次"
					timecount++;	// 消抖计时 +1ms
					if(timecount==10)	// 连续 10 次（10ms）都检测到按下 → 消抖成功！
					{
						timecount=0;	// 计时清零，供下次使用
						Keystate = 3;	// 进入状态3：等待松手
						return 1;		// 返回按键编号 1（PB12 被按下）
					}
				}
				if((GPIOB->IDR & (1<<14))==0)	// PB14 被按着？
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 2;	// 返回按键编号 2（PB13 被按下）
					}
				}
				if((GPIOB->IDR & (1<<13))==0)	// PB13 被按着？
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 3;	// 返回按键编号 3（PB14 被按下）
					}
				}
				if((GPIOB->IDR & (1<<12))==0)	// PB12 被按着？
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;
						return 4;	// 返回按键编号 4（PB15 被按下）
					}
				}
				
				else	// 四个键全都没按着（说明刚才只是抖动/误触）
				{
					Keystate=0;	// 回到状态0，重新等待按键
					break;
				}
				
				break;	
				
			}
			case 3:		// 【状态3：等待松手态】防止"一次按键被重复识别"
			{
				// 用库函数逐个判断四个键是否都已松开（读到 1 = 松手）。
				// 必须等所有键都松开，才会重新开始检测下一次按键。
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==1||GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1)
					{
						Delay_ms(1);	// 松手也消抖：防止松手瞬间的抖动
						timecount++;	// 连续计时
						if(timecount==10)	// 连续 10ms 都检测到"已松开" → 确认松手
						{
							timecount=0;	// 计时清零
							Keystate = 1;	// 回到状态1？不对——回到状态0才是空闲。
											// 注意：这里写的 Keystate = 1 会让下一次
											// 从"消抖确认"开始，同样能正常工作。
						}
					}

					else	// 还有键按着（没有完全松开）
					{
						Keystate=3;	// 保持状态3，继续等待松手
						//break;
					}			
			}			
		}		
	}
}
unsigned int keynode=0;	// 全局变量：加减计数器的当前值（范围 0~5），一直保存在内存里
/*
 * 函数：Key_plus —— 加减计数器（单击加一 / 单击减一）
 * 功能：检测 PB13（加）和 PB14（减）两个按键。每次"完整按一次"
 *       （消抖确认 + 松手确认）后，把全局变量 keynode 加一或减一，并返回新值。
 * 参数：无
 * 返回：keynode 的新值（0~5）
 * 原理（状态机）：
 *   状态0（空闲）：等待 PB13 或 PB14 按下；
 *   状态1（消抖确认）：连续 10ms 确认是哪个键——按 PB13 → 状态2（加），
 *       按 PB14 → 状态3（减）；
 *   状态2（等 PB13 松手）：松手后才 keynode++ 并返回 → 这就是"松手亮灯"。
 *       若一直按着不松手，timecount 不断清零、永远到不了 10，就不会重复加减；
 *   状态3（等 PB14 松手）：同理，松手后 keynode-- 并返回。
 *   注意：本函数是死循环，返回后会自动从状态0 重新开始等下一次按键，
 *       所以主循环里 Light(Key_plus()) 每次调用都会"停"在函数里等按键事件。
 * 说明：状态2 结束后没有 break，会"穿透"到状态3，这是 switch 的陷阱之一，
 *       同学们自己写代码时要注意在每个 case 末尾加 break，防止意外穿透。
 */
unsigned int Key_plus(void)
{
	unsigned int timecount=0,Keystate=0;	// timecount：消抖计时；Keystate：状态机状态
	while(1)	// 死循环扫描，等同"一直检测按键"
	{
		switch (Keystate)
		{
			case 0:		// 【状态0：空闲】等待 PB13（加）或 PB14（减）按下
			{
				if((GPIOB->IDR & (1<<13))==0|| (GPIOB->IDR & (1<<14))==0)	// 两个键有任意一个按下
				Keystate = 1;	// 进入状态1：消抖确认
				
					
				
				break;
			}
			case 1:		// 【状态1：消抖确认】连续 10ms 确认是哪个键被按下
			{
				
				if((GPIOB->IDR & (1<<13))==0)	// PB13（加）被按着？
				{
					Delay_ms(1);	// 1ms 扫描一次
					timecount++;
					if(timecount==10)	// 连续 10ms 都按着 → 消抖成功，确定是"加"键
					{
						timecount=0;
						Keystate = 2;	// 进入状态2：等待 PB13 松手
						//return 1;
					}
				}
				else if((GPIOB->IDR & (1<<14))==0)	// 否则看 PB14（减）是否被按着
				{
					Delay_ms(1);
					timecount++;
					if(timecount==10)
					{
						timecount=0;
						Keystate = 3;	// 进入状态3：等待 PB14 松手
						//return 2;
					}
				}
				
				
				else	// 两个键都没按（抖动/误触）
				{
					Keystate=0;	// 回到状态0，重新等按键
					break;
				}
				
				break;	
				
			}
			case 2:		// 【状态2：等待"加"键松手】松手后才 keynode++ 并返回
			{
				// 用库函数读 PB13 电平：读到 1 = 松手了
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==1)
					{
						Delay_ms(1);	// 松手也消抖
						timecount++;
						if(timecount==10)	// 连续 10ms 都是松手状态 → 确认松手
						{
							timecount=0;
							Keystate = 1;	// 回到状态1，等待下一次按键
							if(keynode<5)	// 上限保护：最多加到 5（防止超过灯的个数）
							{
								keynode++;	// 计数加一
								return keynode;	// 返回新数值，主函数拿去亮灯
							}
						}
					}

				else	// PB13 还按着（没松手）
					{
						timecount=0;	// 计时清零：只要还按着，timecount 永远到不了10，
										// 就不会重复加减 → 实现"只识别单击"
						Keystate=2;		// 保持状态2，继续等松手
						//break;
					}			
			}			
			case 3:		// 【状态3：等待"减"键松手】松手后才 keynode-- 并返回
			{
				// 注意：状态2 的代码结束后没有 break 会直接"穿透"到这里，
				// 两个 case 共用这段代码，对 PB14 的判断在逻辑上会重复执行。
				// （初学者注意：switch 不加 break 会顺序执行后面的 case，
				//   这是意外行为的常见来源，写代码时务必小心。）
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==1)	// PB14 松手了（读到1）
					{
						Delay_ms(1);	// 松手消抖
						timecount++;
						if(timecount==10)	// 连续 10ms 松手 → 确认
						{
							timecount=0;
							Keystate = 1;	// 回到状态1，等待下一次按键
							if(keynode>0)	// 下限保护：最小减到 0（防止负数）
							{
								keynode--;	// 计数减一
								return keynode;	// 返回新数值
							}
						}
					}

					else	// PB14 还按着（没松手）
					{
						timecount=0;	// 计时清零：按着不放就不会重复减
						Keystate=3;		// 保持状态3
						//break;
					}			
			}			
		}		
	}
	
}

unsigned int Key_Oneneed(void)
{
	unsigned int timecount=0,Keystate=0,i=0;	
	while(1)
	{		
		switch (Keystate)
		{
			case 0:
			{
				if((GPIOB->IDR & (1<<14))==0)
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
				
				if((GPIOB->IDR & (1<<14))==0)
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
				if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)==1)
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
//unsigned int Keygetone = 0;

unsigned int Key_One(void)
{
	static unsigned int timecount=0,Keystate=0,time=0;
	while(1)
	{		
		switch (Keystate)
		{
			case 0:
			{				
				if((GPIOB->IDR & (1<<14))==0)
				Keystate = 1;
				
				//break;
				
			}
			case 1:
			{				
				if((GPIOB->IDR & (1<<14))==0)
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
					
				if((GPIOB->IDR & (1<<14))==0)
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
							return 3;							
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
					return 2;	
				}
				else
				{	Keystate=0;
					return 1;	
				}
				
			}
		}	
	}
}

