/*===============================================================================
 * 文件名称：Key.c（按键驱动文件）
 * 所属工程：4-1OLED（本文件与 3-定时器 工程里的 Key.c 内容相同）
 * 模块功能：驱动 4 个独立按键（接在 GPIOB 的 PB12~PB15 上，按键另一端接 GND），
 *           提供多种读取按键的方式：
 *           1. Key_GetNum()  ：轮询 + 延时消抖，按下并松手后返回按键号
 *           2. Key_GetState()：状态机消抖，按下即返回（不等待松手），
 *                              用 Keystate 状态变量实现"边沿检测"
 *           3. Key_One()     ：高级状态机，能区分"单击/双击/长按"三种操作
 *           4. Key_Oneneed() ：Key_One 的内部辅助函数（等待一次完整按下）
 * 用到的外设/知识点：
 *   1. GPIO 上拉输入（GPIO_Mode_IPU）：按键按下接地 → 引脚变低电平
 *   2. 软件消抖：机械按键按下瞬间会有约 10~20ms 的抖动，用延时避开
 *   3. 状态机：用变量 Keystate 记录按键当前所处状态（未按/按下/松开...），
 *      通过状态转移实现稳定的按键识别，是单片机按键处理的进阶写法
 *   4. 位带操作（sys.h 中的 PBin(n) 宏）：直接读 GPIO 输入数据寄存器的某一位
 * 硬件接线说明：PB12~PB15 接按键，按键另一端接 GND；
 *              上拉输入保证不按时引脚为高电平（读 1），按下时为低电平（读 0）
 *===============================================================================
 */
#include "stm32f10x.h"    
#include "Delay.h"// Device header
#include "sys.h"

/*===============================================================================
 * 函数名称：Key_Init（按键初始化）
 * 功能：    把 PB12~PB15 配置为"上拉输入"
 * 参数：    无
 * 返回值：  无
 * 实现原理：
 *   1. RCC_APB2PeriphClockCmd(GPIOB, ENABLE)：打开 GPIOB 时钟（不打开时钟，
 *      寄存器写不进去，配置无效）
 *   2. GPIO_Mode = GPIO_Mode_IPU（上拉输入）：
 *      —— 内部上拉电阻把引脚默认拉到 3.3V（高电平），
 *         按键按下时把引脚拉到 GND（低电平），程序读 0 就代表"按下了"
 *   3. GPIO_Speed = 50MHz：输入模式下速度参数不影响功能，按惯例填写即可
 *===============================================================================
 */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	// 打开 GPIOB 的时钟（GPIO 在 APB2 总线上）
	GPIO_InitTypeDef GPIO_InitStructure;	// 定义 GPIO 初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	// 上拉输入：不按时读 1，按下（接地）时读 0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	// 使用 PB12~PB15 四个引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 速度 50MHz（输入模式无实际影响）
	GPIO_Init(GPIOB,&GPIO_InitStructure);	// 把配置写入 GPIOB 寄存器，生效！
	/*RCC->APB2ENR &= 1<<3;	// （被注释掉的寄存器写法示例）
	GPIOB->CRH &= 0x88880000;	// 直接用寄存器配置，效果同上，但可读性差
	*/
}

/*===============================================================================
 * 函数名称：Key_GetNum（轮询法读取按键号）
 * 功能：    依次检测 4 个按键，哪个被按下并松手就返回它的编号（1~4）
 * 参数：    无
 * 返回值：  unsigned char：1~4 对应 PB12~PB15 的按键；0 表示没有按键按下
 * 实现原理（软件消抖的标准写法）：
 *   1. 检测按键电平 == 0（按下）。注意机械按键按下瞬间会来回抖动
 *      （电平在 0/1 之间弹跳约 10~20ms），直接判断会"一按变好几次"
 *   2. 先 Delay_ms(20) 跳过抖动期：如果 20ms 后还是低电平，说明确实按下了
 *   3. while(引脚==0) 死等：等按键松手（这是"松手检测"，松手后才继续往下走）
 *   4. 松手后再 Delay_ms(20)：跳过松手时的抖动
 *   5. 确认无误后返回按键号
 * 缺点：此函数会"卡住"等待松手，主循环会被阻塞，适合简单教学场景；
 *       更高级的写法见 Key_GetState（状态机，按下即返回不等待松手）
 *===============================================================================
 */
unsigned char Key_GetNum(void)
{
	unsigned char KeyNum = 0;	// 返回值，默认 0（没有按键按下）
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0)	// 检测 PB12 是否按下（低电平=按下）
	{
		Delay_ms(20);	// 延时消抖：跳过按键按下瞬间的抖动
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 0);	// 死等松手：按键没松开就一直在这一行转圈
		Delay_ms(20);	// 再延时消抖：跳过松手瞬间的抖动
		KeyNum = 1;	// 确认 PB12 完成一次"按下+松开"，返回按键号 1
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0)	// 检测 PB13（按键 2）
	{
		Delay_ms(20);	// 消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13) == 0);	// 等待松手
		Delay_ms(20);	// 松手消抖
		KeyNum = 2;	// 返回按键号 2
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0)	// 检测 PB14（按键 3）
	{
		Delay_ms(20);	// 消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 0);	// 等待松手
		Delay_ms(20);	// 松手消抖
		KeyNum = 3;	// 返回按键号 3
		
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0)	// 检测 PB15（按键 4）
	{
		Delay_ms(20);	// 消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) == 0);	// 等待松手
		Delay_ms(20);	// 松手消抖
		KeyNum = 4;	// 返回按键号 4
		
	}
	


	/*if(~(GPIOB->IDR&1<<12))	// （被注释掉的寄存器写法：IDR 是输入数据寄存器，
	{							//  读某一位判断电平，效果同 GPIO_ReadInputDataBit）
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
	
	
	/*if(Key1 == 0)	// （被注释掉的位带操作写法：Key1~Key4 是 Key.h 里定义的
	{				//  PBin(12)~PBin(15) 宏，直接读引脚的一位，效果同上）
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

	
	
	return KeyNum;	// 返回按键号（0 表示本次调用没有检测到按键）
}

/*===============================================================================
 * 函数名称：Key_GetState（状态机法读取按键，支持 4 个按键）
 * 功能：    按键按下并稳定（约 10ms）后立即返回按键号，不等待松手；
 *           主程序循环调用它，每次按下只会"有效"返回一次
 * 参数：    无
 * 返回值：  unsigned char：1~4 对应四个按键；按键未按下时函数会一直循环等待
 * 实现原理（状态机三态）：
 *   Keystate 变量记录当前状态：
 *   - 状态 0（等待）：没按键时在此空转；检测到任一按键按下 → 状态 1
 *   - 状态 1（确认）：按键保持按下，每次循环延时 1ms 并计数，
 *       连续累计 10ms（timecount==10）认为按下稳定 → 返回按键号并转入状态 3
 *       （延时 1ms + 计数 10 次 = 相当于 10ms 消抖，比固定延时更灵活）
 *       —— 注意：如果这期间按键松开（else 分支），清零回到状态 0
 *   - 状态 3（等待松开）：按键一直按着就在这等着；检测到任一按键松开
 *       并稳定 10ms → 回到状态 1（等待下一次按下）
 *   优点：按下立刻返回，主程序不用被"等待松手"卡住；
 *        用状态机 + 时间计数代替固定延时，消抖更可靠
 * 注意：此函数只有"按下"时才返回，主循环里要持续调用它才能及时响应
 *===============================================================================
 */
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
