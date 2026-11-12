/*============================================================================
  文件：NVIC.c（外部中断 EXTI 驱动按键）
  ----------------------------------------------------------------------------
  功能：把 4 个按键（PB12~PB15）配置成"外部中断"方式检测，
        按键一按下的瞬间，CPU 会立刻进入中断服务函数 EXTI15_10_IRQHandler，
        把对应的键号记到全局变量 Keystate 里；主程序通过 NVIC_GetKey()
        查询最近按下了哪个键。
  ----------------------------------------------------------------------------
  用到的外设/知识点：
    1. EXTI（外部中断/事件控制器）：
       - 每个 GPIO 引脚都可以作为 EXTI 中断源；
       - PB12~15 共用中断线 EXTI_Line12~15，这 4 条线又共同触发
         同一个中断通道 EXTI15_10_IRQn（即 EXTI 第 10~15 号中断线共用一个
         中断服务函数 EXTI15_10_IRQHandler）；
       - 中断触发条件设为"下降沿"：引脚从高电平变低电平（按键按下）时触发。
    2. AFIO 时钟：EXTI 中断线要和具体引脚"连接"，
       需要 GPIO_EXTILineConfig() 选择哪个引脚接到哪条中断线，
       而 AFIO（复用功能时钟）必须开启才能使用这个功能。
    3. NVIC（嵌套向量中断控制器）：
       - 负责"管理"中断的优先级和开关；
       - NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2) 设置分组：
         抢占优先级占 2 位（0~3），子优先级占 2 位（0~3）；
       - 只有"优先级分组 + 使能"配置好，中断才能真正进入 CPU。
  ----------------------------------------------------------------------------
  轮询 vs 中断 对比：
    轮询（Key_GetNum）：主循环不停查引脚电平，简单但浪费 CPU；
    中断（本文件）：按键按下瞬间 CPU 自动响应，省电省时间，
    适合按键事件少、主程序任务重的场合。
  ============================================================================*/
#include "stm32f10x.h"  
//#include "Song.h"// Device header
#include "Delay.h"

/*
 * 函数名：NVICKey_Init（外部中断按键初始化）
 * 参数：无
 * 返回：无
 * 功能：把 PB12~PB15 配置成上拉输入，并挂到 EXTI 中断线上，
 *       配置好中断触发方式（下降沿）和 NVIC 优先级，最后使能。
 * 步骤拆解（结合寄存器理解）：
 *   1. 开 GPIOB 时钟 + AFIO 时钟（EXTI 引脚连接功能需要 AFIO）。
 *   2. 引脚配置为"上拉输入"（GPIO_Mode_IPU）：松开时高电平，按下时低电平。
 *   3. GPIO_EXTILineConfig：把"PBx 引脚"映射到"EXTI_Line x 中断线"，
 *      相当于"把线接上"；不接的话引脚电平变化根本到不了 EXTI。
 *   4. EXTI 配置：
 *      - EXTI_Mode_Interrupt：中断模式（还有事件模式，事件模式不进 CPU）；
 *      - EXTI_Trigger_Falling：下降沿触发，即"高电平 -> 低电平"
 *        的瞬间触发，正好是按键按下的时刻；
 *      - EXTI_LineCmd = ENABLE：使能这 4 条中断线。
 *   5. NVIC 配置：
 *      - 中断通道 EXTI15_10_IRQn：10~15 号中断线共用这一个中断入口；
 *      - 抢占优先级 1、子优先级 1：数字越小优先级越高，
 *        同组（抢占优先级相同）时用子优先级比较。
 *   6. 配置完成后，一旦有按键按下，CPU 自动跳进 EXTI15_10_IRQHandler。
 */
void NVICKey_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	// 开启 GPIOB 时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);		// 开启 AFIO 时钟（EXTI 引脚连接需要）
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			// 上拉输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12;	// 4 个按键引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// 速度（输入模式无实际影响）
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	
	/* 把 4 个引脚分别连接到对应的 EXTI 中断线（PB12->Line12 ... PB15->Line15） */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource15);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource13);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12);
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15;	// 使用 12~15 四条中断线
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				// 使能中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// 中断模式（进入 CPU 中断）
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;	// 下降沿触发（按下瞬间）
	EXTI_Init(&EXTI_InitStructure);
		

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	// 优先级分组2：抢占2位 + 子优先级2位
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;	// 中断通道：EXTI 10~15 号共用
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			// 使能该通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	// 抢占优先级 1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			// 子优先级 1
	NVIC_Init(&NVIC_InitStructure);
}

/*  全局变量：保存"最近一次按下的键号"（0 = 无），中断里写，主程序里读  */
unsigned int		Keystate=0;

/*
 * 函数名：EXTI15_10_IRQHandler（EXTI 10~15 号中断线的中断服务函数）
 * 参数：无
 * 返回：无
 * 功能：按键按下（下降沿）后 CPU 自动跳进来，
 *       通过 EXTI_GetITStatus 判断是哪条中断线触发的，把对应键号
 *       保存到 Keystate，并清除中断标志位。
 * 重要细节（初学者必看）：
 *   1. 函数名必须严格是 EXTI15_10_IRQHandler，不能改，
 *      这是 startup 启动文件中断向量表里写好的名字；
 *   2. EXTI_GetITStatus(EXTI_Line12)==SET 表示 Line12 中断线
 *      产生了中断请求（说明 PB12 被按下）；
 *   3. EXTI_ClearITPendingBit 必须调用！中断标志位（挂起位）是
 *      "写 1 清除"的，不清除的话，中断返回后会再次进入，
 *      造成"中断风暴"，主程序永远卡在中断里。
 */
void EXTI15_10_IRQHandler(void)
{
	Keystate=0;		// 先清零，保证只记录"这一次"的按键
	//unsigned int timecount=0,KeyState = 0;
	if(EXTI_GetITStatus(EXTI_Line12)==SET)	// PB12 触发的中断
	{
		Keystate = 1;						// 记录键号 1
		EXTI_ClearITPendingBit(EXTI_Line12);	// 清除 Line12 中断标志
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line13)==SET)	// PB13 触发的中断
	{
		Keystate = 2;						// 记录键号 2
		EXTI_ClearITPendingBit(EXTI_Line13);
		
	}
	if(EXTI_GetITStatus(EXTI_Line14)==SET)	// PB14 触发的中断
	{
		Keystate =3;						// 记录键号 3
		EXTI_ClearITPendingBit(EXTI_Line14);
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line15)==SET)	// PB15 触发的中断
	{
		Keystate = 4;						// 记录键号 4
		EXTI_ClearITPendingBit(EXTI_Line15);
		
		
	}
}

/*
 * 函数名：NVIC_GetKey（读取按键结果）
 * 参数：无
 * 返回：最近一次按下的键号（1~4），没有按键时为 0
 * 功能：主程序循环里调用它，查询中断里记录下来的按键号。
 * 注意：中断里写、主循环读，两个地方共享同一个全局变量 Keystate，
 *       这是"中断-主程序"通信最简单的方式。
 */
unsigned int NVIC_GetKey(void)
{
	return Keystate;
}
