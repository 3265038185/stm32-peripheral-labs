/*****************************************************************************
 * 文件名：EXTI.c（Hardware 硬件层）
 * 所属工程：1-LED闪烁
 *
 * 【文件功能】
 *   初始化"按键 + 外部中断"：把 PA1 引脚配置为"上拉输入"，
 *   并使能 EXTI1 外部中断（下降沿触发），按键按下时 CPU 会
 *   自动跳入 EXTI1_IRQHandler()（该函数写在 main.c 里）。
 *
 * 【涉及知识点】
 *   - GPIO 上拉输入模式（GPIO_Mode_IPU）；
 *   - AFIO 复用功能时钟（EXTI 中断线需要它）；
 *   - GPIO_EXTILineConfig：把某个引脚映射到 EXTI 中断线；
 *   - EXTI 外部中断配置（触发方式、模式）；
 *   - NVIC 嵌套向量中断控制器（设置中断优先级、使能中断）。
 *
 * 【EXTI 外部中断原理（通俗版）】
 *   可以想象成"门铃"：按键（PA1）就是门铃按钮，
 *   EXTI1 中断线就是那根从门铃接到客厅的电线，
 *   按下按钮（下降沿）→ 铃响（触发中断）→ 主人开门（执行中断函数）。
 *   配置过程分四步：开时钟 → 配置引脚 → 配置 EXTI → 配置 NVIC。
 *****************************************************************************/
#include "stm32f10x.h"                  // Device header
/*****************************************************************************
 * 函数名：EXTIKey_Init（按键外部中断初始化）
 * 功能：  把 PA1 配置为上拉输入，并使能 EXTI1 下降沿中断 + NVIC 中断
 * 参数：  无
 * 返回值：无
 *
 * 【配置流程】总共四步，缺一不可：
 *   第 1 步：打开相关外设的时钟（GPIO、AFIO）；
 *   第 2 步：把 PA1 配置成上拉输入模式（按键电路需要上拉）；
 *   第 3 步：把 PA1 映射到 EXTI1 中断线，并配置 EXTI 触发方式；
 *   第 4 步：在 NVIC 里使能 EXTI1 中断并设置优先级。
 * 配置完成后，只要 PA1 出现"高→低"跳变，硬件就会自动
 * 调用 EXTI1_IRQHandler()（写在 main.c）。
 *****************************************************************************/
void EXTIKey_Init(void)

{
	//【第 1 步】打开时钟：外设在使用前必须先"通电"（开启时钟），否则寄存器写了无效。
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);   //开启 AFIO 时钟：EXTI 中断线的"引脚映射"功能由 AFIO 管
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  //开启 GPIOA 时钟：PA1 属于 GPIOA 端口
	
	//【第 2 步】配置 PA1 为"上拉输入"模式：
	//   GPIO_Mode_IPU（Input Pull-Up）＝ 内部上拉输入。
	//   按键电路：按键一端接 GND，另一端接 PA1，PA1 经内部上拉电阻接到 3.3V。
	//   没按时：PA1 被上拉电阻拉成高电平；按下时：PA1 被接地拉成低电平。
	//   于是"按下"就对应"高→低"的下降沿，正好作为中断触发条件。
	//   （如果不用上拉，引脚悬空，电平不稳定，按键就无法可靠检测）
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   //模式：上拉输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;       //选择引脚：PA1
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度：输入模式下其实不关心，标准写法填上即可
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);  //把上面的配置真正写入 GPIOA 的寄存器（CRL 等）
	
	//【第 3 步】把 PA1 引脚"映射"到 EXTI1 中断线：
	//   GPIOA 的 16 个引脚（PA0~PA15）分别对应 EXTI0~EXTI15 共 16 条中断线。
	//   本函数把 PA1 接到 EXTI1 上，之后 EXTI1 就专门监视 PA1 的电平变化。
	//   （EXTI 中断线是"多选一"的：同一时刻一条线只能接一个引脚，
	//     如果有多个引脚要中断，得用 EXTI_Line 的不同位 + 引脚共享方式）
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);
	
	
	//【第 4 步】配置 EXTI 中断线的触发方式：
	EXTI_InitTypeDef EXTI_InitSturcture;
	EXTI_InitSturcture.EXTI_Line = EXTI_Line1;        //选择 EXTI1 这条中断线
	EXTI_InitSturcture.EXTI_LineCmd = ENABLE;         //使能这条中断线
	EXTI_InitSturcture.EXTI_Mode= EXTI_Mode_Interrupt; //模式选"中断"（还有一种是事件模式，不产生中断）
	EXTI_InitSturcture.EXTI_Trigger = EXTI_Trigger_Falling; //触发方式：下降沿触发（高→低跳变时触发）
	EXTI_Init(&EXTI_InitSturcture);                    //把配置写入 EXTI 寄存器
	//【补充】除了下降沿，还可以选上升沿（EXIT_Trigger_Rising）或
	//         双边沿（高→低 和 低→高 都触发）。
	
	//【第 5 步】配置 NVIC：中断"总闸"里使能 EXTI1，并定好优先级。
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);   //选择优先级分组方式（2 位抢占 + 2 位子优先级）
	NVIC_InitTypeDef NVIC_InitSturcture;
	NVIC_InitSturcture.NVIC_IRQChannel = EXTI1_IRQn;  //中断通道：EXTI1（即本中断在向量表中的编号）
	NVIC_InitSturcture.NVIC_IRQChannelCmd = ENABLE;   //使能该通道
	NVIC_InitSturcture.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级 0（数字越小越优先）
	NVIC_InitSturcture.NVIC_IRQChannelSubPriority = 0;        //子优先级 0
	NVIC_Init(&NVIC_InitSturcture);                   //写入 NVIC 寄存器，配置生效

}
