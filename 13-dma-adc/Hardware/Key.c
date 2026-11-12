/*=============================================================================
 * 文件：Key.c（按键外设驱动文件）
 * 功能：初始化两个独立按键（PB1、PB11），并提供"获取键码"函数，
 *       用于读取按键是否被按下。
 *
 * 用到的外设/知识点：
 *   1. GPIO 上拉输入（IPU）：按键一端接地、一端接引脚。
 *      平时引脚被内部上拉电阻拉到高电平（读回 1）；
 *      按下按键时引脚被接地拉成低电平（读回 0），以此判断"按下"
 *   2. 按键消抖：机械按键按下/松开的瞬间，触点会弹跳抖动，
 *      电平在 0 和 1 之间快速跳变好几毫秒，必须用延时跳过这段抖动，
 *      否则一次按下可能会被误判成多次
 *   3. 阻塞式扫描：本函数用"等待松手"的方式实现，编程简单，
 *      缺点是一旦按住按键不松，程序就卡在这里
 *
 * 注：本项目（DMA+ADC）的主程序暂时没有调用按键函数，写在这里供以后扩展使用
 *=============================================================================*/
#include "stm32f10x.h"                  // Device header
#include "Delay.h"                      // 延时函数头文件（消抖用）

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：把 PB1、PB11 配置成"上拉输入"模式
  */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	/* 原理：GPIOB 挂载在 APB2 总线上，不打开时钟的话引脚配置无效 */
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//模式：上拉输入
	/* 原理：上拉输入 = 内部把一个电阻接到 3.3V 上，引脚默认读到 1；
	 * 按键按下把引脚拉到 0V 时读到 0，电平变化正好用来识别按键动作。
	 * 如果用下拉输入，按键的接法就要反过来（按键接电源） */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;	//引脚：PB1（按键1）和 PB11（按键2）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度：50MHz（输入模式不关心速度，习惯写法）
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//将PB1和PB11引脚初始化为上拉输入
}

/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~2，返回0代表没有按键按下
  * 注意事项：此函数是阻塞式操作，当按键按住不放时，函数会卡住，直到按键松手
  */
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;		//定义变量，默认键码值为0
	
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)			//读PB1输入寄存器的状态，如果为0，则代表按键1按下
	{
		/* 按键1被按下的处理流程（消抖三步曲） */
		Delay_ms(20);											//延时消抖：跳过按下瞬间的机械抖动
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0);	//等待按键松手（按住时程序停在这里）
		Delay_ms(20);											//延时消抖：跳过松开瞬间的机械抖动
		KeyNum = 1;												//置键码为1
	}
	
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)			//读PB11输入寄存器的状态，如果为0，则代表按键2按下
	{
		Delay_ms(20);											//延时消抖：跳过按下瞬间的机械抖动
		while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0);	//等待按键松手（按住时程序停在这里）
		Delay_ms(20);											//延时消抖：跳过松开瞬间的机械抖动
		KeyNum = 2;												//置键码为2
	}
	
	return KeyNum;			//返回键码值，如果没有按键按下，所有if都不成立，则键码为默认值0
}
