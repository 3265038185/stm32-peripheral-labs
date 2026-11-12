/*==============================================================================
 * 文件：NVIC.c（中断按键模块）
 * 功能：用"外部中断（EXTI）"来实现按键检测，替代轮询方式。
 *       1. NVICKey_Init()：配置PB12~15为外部中断源，按键按下（下降沿）触发中断；
 *       2. EXTI15_10_IRQHandler()：中断服务函数，哪个按键按下就把全局变量
 *          Keystate 记成几；
 *       3. NVIC_GetKey()：主程序调用，读取最近一次按下的按键号。
 * 整个流程：按键按下 → 引脚电平从高变低（下降沿）→ EXTI硬件捕获到边沿
 *          → 置位中断标志位 → NVIC响应中断 → CPU跳进中断服务函数。
 * 知识点：GPIO外部中断（EXTI）、AFIO时钟与引脚映射（GPIO_EXTILineConfig）、
 *         中断分组（NVIC_PriorityGroupConfig）、抢占优先级/响应优先级
 *============================================================================*/
#include "stm32f10x.h"  
//#include "Song.h"// Device header（暂时用不到，先注释掉）
#include "Delay.h"	// 延时函数头文件（本文件内实际未使用，保留为后续消抖准备）

/* 中断按键初始化函数：
   功能：把PB12~15配置成外部中断输入，使能EXTI中断和NVIC中断控制器。
   实现步骤（共四步，顺序很重要）：
     ①开时钟：GPIOB时钟 + AFIO时钟（使用EXTI必须开AFIO时钟，因为引脚
       与中断线的映射关系在AFIO寄存器里配置）；
     ②配GPIO：PB12~15设为上拉输入；
     ③配EXTI：把PB12~15分别映射到中断线12~15，选择下降沿触发；
     ④配NVIC：配置中断优先级，使能EXTI15_10中断通道。 */
void NVICKey_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	// ①开启GPIOB时钟（按键所在的端口）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	// ①开启AFIO时钟！EXTI引脚映射要用它，
									// 忘了开这个，中断配置无效
	
	GPIO_InitTypeDef GPIO_InitStructure;	// 定义GPIO配置结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	// 上拉输入：按键松开时引脚保持高电平
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12;	// 用到PB12~PB15
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 速度参数（输入模式无实际意义）
	GPIO_Init(GPIOB,&GPIO_InitStructure); 	// ②把配置写入寄存器
	
	// ③引脚映射：把外部中断线15~12分别接到PB15~PB12。
	// 原理：STM32的EXTI只有16条中断线，而引脚有几十个，
	//       必须用GPIO_EXTILineConfig告诉硬件"这条中断线听哪个引脚的"
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource15);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource13);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12);
	EXTI_InitTypeDef EXTI_InitStructure;	// 定义EXTI配置结构体
	EXTI_InitStructure.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15;	// 使用中断线12~15
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;	// 使能这些中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	// 模式=中断（另一种是事件模式EXTI_Mode_Event）
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;	// 触发方式=下降沿触发！
									// 按键按下时电平从高(3.3V)变低(0V)，正好是下降沿
	EXTI_Init(&EXTI_InitStructure);	// 把配置写入EXTI寄存器
		

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	// ④中断分组：NVIC_PriorityGroup_2表示
									// "2位抢占优先级 + 2位响应优先级"，
									// 整个工程只需配置一次，一般放在主函数里，
									// 这里配置也合法
	
	NVIC_InitTypeDef NVIC_InitStructure;	// 定义NVIC配置结构体
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;	// 中断通道：EXTI15_10（中断线10~15共用这个中断号）
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// 使能该中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	// 抢占优先级=1（数字越小优先级越高）
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	// 响应优先级=1
	NVIC_Init(&NVIC_InitStructure);	// 写入NVIC寄存器，中断配置完成
}

unsigned int		Keystate=0;	// 全局变量：记录"最近一次按下的按键号"（1~4，0=没按）
							// 定义为全局变量是因为：中断函数和主程序都要访问它

/* 外部中断服务函数：PB12~15 任一引脚产生下降沿（按键按下）都会跳进这里。
   注意：函数名 EXTI15_10_IRQHandler 是固定的（见启动文件），不能乱改！
   功能：判断是中断线12/13/14/15中的哪一条触发了中断，
        把对应的按键号写入 Keystate，然后清除中断标志位。
   为什么必须清除标志位？如果不清，中断标志一直为SET，会反复触发中断，程序死循环。 */
void EXTI15_10_IRQHandler(void)
{
	Keystate=0;	// 先清零，保证"没按过"时读到的值是0
	//unsigned int timecount=0,KeyState = 0;
	if(EXTI_GetITStatus(EXTI_Line12)==SET)	// 查询中断线12的标志位：SET=触发了
	{
		Keystate = 1;	// 记录：按键1按下
		EXTI_ClearITPendingBit(EXTI_Line12);	// 清除中断线12的挂起标志位（必须！）
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line13)==SET)	// 中断线13触发（按键2）
	{
		Keystate = 2;	// 记录：按键2按下
		EXTI_ClearITPendingBit(EXTI_Line13);	// 清除标志位
		
	}
	if(EXTI_GetITStatus(EXTI_Line14)==SET)	// 中断线14触发（按键3）
	{
		Keystate =3;	// 记录：按键3按下
		EXTI_ClearITPendingBit(EXTI_Line14);	// 清除标志位
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line15)==SET)	// 中断线15触发（按键4）
	{
		Keystate = 4;	// 记录：按键4按下
		EXTI_ClearITPendingBit(EXTI_Line15);	// 清除标志位
		
		
	}
}

/* 获取按键号函数：主程序调用，返回最近一次按下的按键号（1~4）。
   注意：这是"中断法"按键，主程序在while循环里反复调用本函数
   就能知道按键有没有被按过，不需要自己死等。 */
unsigned int NVIC_GetKey(void)
{
	return Keystate;	// 直接把全局变量返回给主程序
}
