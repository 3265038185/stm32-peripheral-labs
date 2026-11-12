/*===============================================================================
 * 文件名称：NVIC.c（中断控制器驱动文件）
 * 模块功能：演示两种"中断触发按键"的方式：
 *           1. NVICKey_Init()：把 4 个按键接到 GPIOB 的 PB12~PB15 上，
 *              用"外部中断 EXTI"实现按键触发——按键按下（电平从高变低，
 *              即下降沿）时，硬件立刻打断主程序，跳进 EXTI15_10_IRQHandler，
 *              在中断里用全局变量 Keystate 记录"哪个按键被按下"；
 *           2. NVIC_GetKey()：主程序查询这个函数，看看中断里记录的按键号。
 * 用到的外设/知识点：
 *   1. EXTI（外部中断）：检测引脚边沿（上升沿/下降沿）并触发中断
 *   2. AFIO（复用功能时钟）：把 EXTI 的"线路"和 GPIO 引脚连接起来时必须开
 *   3. NVIC（嵌套向量中断控制器）：Cortex-M3 内核的中断管理器，
 *      决定哪个中断能进、优先级多高
 *   4. 中断服务函数：EXTI15_10_IRQHandler（EXTI 线 10~15 共用这一个入口）
 * 温馨提示：本工程（3-定时器）的 main.c 实际使用的是 TIM2 定时器中断，
 *           这里的 EXTI 按键中断是"备用方案"，二者原理相通，看懂一个
 *           另一个就懂了。中断里 Keystate 全局变量的读法要注意：
 *           读一次后如果不清零，下次会重复读到同一个值。
 *===============================================================================
 */
#include "stm32f10x.h"  
#include "Song.h"// Device header
#include "Delay.h"

/*===============================================================================
 * 函数名称：NVICKey_Init（外部中断按键初始化）
 * 功能：    配置 PB12~PB15 为"上拉输入"，并使能它们的 EXTI 下降沿中断
 * 参数：    无
 * 返回值：  无
 * 实现原理（逐行讲解）：
 *   1. RCC_APB2PeriphClockCmd(GPIOB, ENABLE)：打开 GPIOB 时钟（GPIO 在 APB2 上）
 *   2. RCC_APB2PeriphClockCmd(AFIO, ENABLE)：打开 AFIO 时钟！
 *      —— 使用 EXTI 外部中断映射功能必须开启 AFIO 时钟，
 *         否则 GPIO_EXTILineConfig 的引脚映射不生效
 *   3. GPIO 配置为 GPIO_Mode_IPU（上拉输入）：
 *      —— 按键另一端接 GND，不按时上拉电阻把引脚拉高（读 1），
 *         按下时引脚被拉到低电平（读 0），这样"下降沿"就是按下动作
 *   4. GPIO_EXTILineConfig(端口, 引脚)：
 *      —— 把 EXTI 线路 12/13/14/15 分别"接线"到 GPIOB 的 12/13/14/15 引脚，
 *         这一步就是"引脚映射"，必须有（配合上面的 AFIO 时钟）
 *   5. EXTI 配置：
 *      - EXTI_Line  = 12~15 线（和引脚号一一对应）
 *      - EXTI_Mode  = 中断模式（还有"事件模式"，事件模式不产生中断）
 *      - EXTI_Trigger = Falling 下降沿触发（高→低的那一瞬间触发）
 *      - 最后 EXTI_Init 写入寄存器
 *   6. NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2)：
 *      —— 设置优先级分组为"2 位抢占优先级 + 2 位子优先级"（共 4 级）
 *         分组必须在所有中断初始化之前配置一次
 *   7. NVIC 通道配置：
 *      - NVIC_IRQChannel = EXTI15_10_IRQn：EXTI 线 10~15 共用一个中断通道
 *      - 抢占优先级 = 1，子优先级 = 1（数越小优先级越高）
 *      - NVIC_Init 写入配置，使能中断通道
 *   8. 配置完成后：按键按下 → EXTI 检测到下降沿 → 硬件自动跳进
 *      EXTI15_10_IRQHandler 执行
 *===============================================================================
 */
void NVICKey_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	// 打开 GPIOB 的时钟（GPIO 在 APB2 高速总线上）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	// 打开 AFIO 时钟（用 EXTI 引脚映射必须开！）
	
	GPIO_InitTypeDef GPIO_InitStructure;	// 定义 GPIO 初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	// 上拉输入：不按时读 1，按下（接地）时读 0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12;	// 使用 PB12~PB15 四个引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 输入模式速度一般不影响，填 50MHz 即可
	GPIO_Init(GPIOB,&GPIO_InitStructure);	// 配置写入 GPIOB 寄存器，生效！
	
	// 引脚映射：把 EXTI 线路 15/14/13/12 分别接到 GPIOB 的 15/14/13/12 引脚
	// （EXTI 线路号与引脚号必须一一对应，比如 Pin15 只能映射到 Line15）
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource15);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource13);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12);
	EXTI_InitTypeDef EXTI_InitStructure;	// 定义 EXTI 初始化结构体
	EXTI_InitStructure.EXTI_Line = EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15;	// 使能 12~15 四条 EXTI 线
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;	// 使能这些线路
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	// 中断模式（触发后进中断服务函数）
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;	// 下降沿触发：按键按下（高→低）的瞬间触发
	EXTI_Init(&EXTI_InitStructure);	// 配置写入 EXTI 寄存器，生效！
		

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	// 优先级分组：2 位抢占 + 2 位子优先级（全工程只配一次）
	
	NVIC_InitTypeDef NVIC_InitStructure;	// 定义 NVIC 初始化结构体
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;	// 中断通道：EXTI 线 10~15 共用一个通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// 使能该中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	// 抢占优先级 = 1（数字越小越优先）
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	// 子优先级 = 1（抢占相同时比子优先级）
	NVIC_Init(&NVIC_InitStructure);	// 配置写入 NVIC 寄存器，生效！
}
unsigned int		Keystate=0;	// 全局变量：记录"哪个按键被按下"（1~4），由中断函数赋值，主程序读取
/*===============================================================================
 * 函数名称：EXTI15_10_IRQHandler（EXTI 外部中断服务函数）
 * 触发条件：PB12~PB15 任一按键按下（下降沿）时，硬件自动跳进来
 * 实现原理：
 *   1. EXTI_GetITStatus(EXTI_LineX) 查询对应中断线标志位是否为 SET，
 *      是 SET 说明这条线确实触发了（12~15 共用一个入口，必须逐个判断）
 *   2. 把对应按键号存进全局变量 Keystate（1 对应 PB12，2 对应 PB13，3 对应 PB14，4 对应 PB15）
 *   3. EXTI_ClearITPendingBit 清除挂起标志位！
 *      —— 必须清！否则中断会反复进入，看起来就像"卡死"在中断里
 * 注意：中断函数里只做"记按键号"这一件轻量的事，具体反应（点亮 LED 等）
 *       放到主循环里根据 Keystate 去处理，这是嵌入式开发的好习惯
 *===============================================================================
 */
void EXTI15_10_IRQHandler(void)
{
	Keystate=0;	// 先把记录清零，再判断（防止读到上一次的旧值）
	//unsigned int timecount=0,KeyState = 0;
	if(EXTI_GetITStatus(EXTI_Line12)==SET)	// 查询 EXTI 线 12 的中断标志位
	{
		Keystate = 1;	// 是 PB12 触发的：记录按键号 1
		EXTI_ClearITPendingBit(EXTI_Line12);	// 清除该线的中断挂起标志（必须清！）
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line13)==SET)	// 查询 EXTI 线 13
	{
		Keystate = 2;	// 是 PB13 触发的：记录按键号 2
		EXTI_ClearITPendingBit(EXTI_Line13);	// 清除挂起标志
		
	}
	if(EXTI_GetITStatus(EXTI_Line14)==SET)	// 查询 EXTI 线 14
	{
		Keystate =3;	// 是 PB14 触发的：记录按键号 3
		EXTI_ClearITPendingBit(EXTI_Line14);	// 清除挂起标志
		//timecount++;
		
		
	}
	if(EXTI_GetITStatus(EXTI_Line15)==SET)	// 查询 EXTI 线 15
	{
		Keystate = 4;	// 是 PB15 触发的：记录按键号 4
		EXTI_ClearITPendingBit(EXTI_Line15);	// 清除挂起标志
		
		
	}
}
/*===============================================================================
 * 函数名称：NVIC_GetKey（获取按键号）
 * 功能：    返回全局变量 Keystate，供主程序查询"刚才哪个按键被按下"
 * 参数：    无
 * 返回值：  unsigned int，按键号 1~4；0 表示没有（或尚未）按键按下
 * 实现原理：中断服务函数里写 Keystate，这里读 Keystate——
 *           这就是嵌入式里经典的"中断写、主循环读"数据传递方式
 *           （中间变量加 volatile 更规范，初学者先理解思路即可）
 *===============================================================================
 */
unsigned int NVIC_GetKey(void)
{
	return Keystate;	// 把中断里记录下来的按键号返回给主程序
}
