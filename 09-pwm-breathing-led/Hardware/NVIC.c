/* ============================================================================
 * 文件说明：NVIC.c —— 按键外部中断（EXTI）驱动
 * ----------------------------------------------------------------------------
 * 【本文件的作用】
 *   用"外部中断"的方式来检测按键：平时主程序不用轮询按键，
 *   当按键被按下时，GPIO 引脚产生一个下降沿（高电平变低电平），
 *   触发 EXTI 外部中断，CPU 自动跳进 EXTI15_10_IRQHandler() 中断服务函数，
 *   在里面记录是哪个按键按下的（Keystate 变量），主程序随时查询即可。
 *
 * 【用到的外设/知识点】
 *   - AFIO 时钟：EXTI 引脚要映射到 GPIO 引脚上，需要打开 AFIO 复用时钟
 *   - EXTI 外部中断控制器：检测引脚电平变化（本工程用下降沿触发）
 *   - NVIC 中断控制器：管理中断优先级、使能中断
 *   - 中断优先级分组：NVIC_PriorityGroup_2（2位抢占优先级 + 2位子优先级）
 *   - 中断服务函数名字必须与启动文件一致：EXTI15_10_IRQHandler
 *
 * 【中断触发流程】
 *   按键按下 → 引脚电平 1→0（下降沿）→ EXTI 检测到 → 置中断标志位
 *   → NVIC 裁决（优先级够高）→ CPU 跳进 EXTI15_10_IRQHandler
 *   → 函数里读标志位、记录按键号、清除标志位（不清除会反复进中断！）
 * ========================================================================== */

#include "stm32f10x.h"  
#include "Song.h"// Device header
#include "Delay.h"

/* ----------------------------------------------------------------------------
 * 函数名：NVICKey_Init
 * 功能  ：初始化"外部中断检测按键"所需的所有配置
 * 参数  ：无
 * 返回  ：无
 * 配置步骤（顺序很重要）：
 *   1. 开时钟：GPIOB 时钟 + AFIO 时钟（EXTI 映射需要 AFIO）
 *   2. 配置 GPIOB 引脚 12~15 为"上拉输入"（按键模式）
 *   3. GPIO_EXTILineConfig：把 EXTI 中断线 12~15 连接到 GPIOB 引脚 12~15
 *   4. 配置 EXTI：选择中断线、模式（中断模式）、触发方式（下降沿）
 *   5. NVIC_PriorityGroupConfig：设置优先级分组
 *   6. 配置 NVIC：使能 EXTI15_10 中断通道、设定优先级
 * -------------------------------------------------------------------------- */
void NVICKey_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   // 开启 GPIOB 时钟（按键引脚）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    // 开启 AFIO 时钟
	// 为什么必须开 AFIO 时钟？因为 EXTI 中断线要"选择"连到哪一组 GPIO 上，
	// 这个选择功能由 AFIO 寄存器（EXTICR 外部中断配置寄存器）完成，
	// 不开 AFIO 时钟，EXTI 无法与 GPIOB 建立连接，中断不会触发。
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   // 上拉输入：按键松开时引脚为高电平
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12;  // 4 个按键引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);           // 配置生效
	
	/* 下面 4 行：把 EXTI 中断线 15/14/13/12 分别映射到 GPIOB 的引脚 15/14/13/12。
	 * 注意：EXTI 每条中断线只能连一个引脚，比如 Line15 连了 PB15，
	 * 就不能再连 PA15 或 PC15，否则冲突。 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource15);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource13);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12);

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15;
	// 选择使用 EXTI 的第 12~15 条中断线
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;           // 使能这些中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 模式：中断模式（另一种是事件模式，不进 CPU）
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;   // 触发方式：下降沿触发
	// 下降沿 = 电平从 1 变 0。按键按下瞬间正是 1→0，所以选下降沿；
	// 同理也可以选上升沿（松手瞬间）或双边沿（都触发）。
	EXTI_Init(&EXTI_InitStructure);                     // 配置写入寄存器（IMR 使能寄存器等）
		

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	// 优先级分组：Group2 = 2 位抢占优先级 + 2 位子优先级（共 4 级抢占 + 4 级子优先级）。
	// 抢占优先级高的可以打断低的；同抢占优先级时看子优先级。必须在配置 NVIC 之前设置！
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	// 选择中断通道：EXTI 线 10~15 共用一个中断服务函数 EXTI15_10_IRQHandler
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     // 使能该中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 抢占优先级 = 1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;         // 子优先级 = 1
	NVIC_Init(&NVIC_InitStructure);                     // 配置写入 NVIC 寄存器
}

unsigned int		Keystate=0;   // 全局按键标志：0=无按键；1~4=对应按键被按下（中断里修改，主程序读取）

/* ----------------------------------------------------------------------------
 * 函数名：EXTI15_10_IRQHandler
 * 功能  ：外部中断 15~10 的中断服务函数（按键按下时自动跳进来执行）
 * 参数  ：无
 * 返回  ：无
 * 原理  ：
 *   1. 先用 EXTI_GetITStatus() 查询是哪条中断线触发的（相当于查标志位）；
 *   2. 把对应的按键编号记入全局变量 Keystate；
 *   3. 必须调用 EXTI_ClearITPendingBit() 清除挂起标志位！
 *      如果不清除，中断标志一直为 1，会不停地再次进入本函数（死循环进中断）。
 * -------------------------------------------------------------------------- */
void EXTI15_10_IRQHandler(void)
{
	Keystate=0;                                        // 进来先清 0：如果下面没匹配到就当作没按键
	//unsigned int timecount=0,KeyState = 0;
	if(EXTI_GetITStatus(EXTI_Line12)==SET)             // 查询 EXTI 第 12 线标志：SET 表示触发了
	{
		Keystate = 1;                                  // 记录：按键1(PB12)被按下
		EXTI_ClearITPendingBit(EXTI_Line12);           // 清除该中断线标志位（重要！否则反复进中断）
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line13)==SET)             // 按键2(PB13)，逻辑同上
	{
		Keystate = 2;
		EXTI_ClearITPendingBit(EXTI_Line13);
		
	}
	if(EXTI_GetITStatus(EXTI_Line14)==SET)             // 按键3(PB14)，逻辑同上
	{
		Keystate =3;
		EXTI_ClearITPendingBit(EXTI_Line14);
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line15)==SET)             // 按键4(PB15)，逻辑同上
	{
		Keystate = 4;
		EXTI_ClearITPendingBit(EXTI_Line15);
		
		
	}
}

/* ----------------------------------------------------------------------------
 * 函数名：NVIC_GetKey
 * 功能  ：主程序查询按键状态的接口
 * 参数  ：无
 * 返回  ：0=无按键；1~4=对应按键按下过（读取后由调用者决定何时清零）
 * 原理  ：直接返回中断服务函数写入的全局变量 Keystate。
 *         这样"中断负责记录、主程序负责查询"，主程序不用阻塞等待按键。
 * -------------------------------------------------------------------------- */
unsigned int NVIC_GetKey(void)
{
	return Keystate;
}
