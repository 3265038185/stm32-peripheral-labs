/*==============================================================================
 * 文件：PWM.c（PWM输出 + 输入捕获模块）——本项目的核心模块！
 * 功能：本文件实现了"用TIM测量波形，再用TIM复现波形"：
 *       1. PWMOut_Init()：TIM3输出PWM波（PA6引脚）。
 *          PWM输出原理：定时器计数器CNT从0往上数到ARR后清零重新数，
 *          计数过程中CNT<CCR1时引脚输出高电平，CNT>=CCR1时输出低电平。
 *          因此：
 *            输出频率 = 72MHz / (PSC+1) / (ARR+1)   （PSC是预分频值）
 *            占空比   = CCR1 / (ARR+1)
 *       2. PWMInt_Init()：TIM2输入捕获（PA0引脚），测量外部输入方波的
 *          "周期"和"高电平脉宽"。
 *          输入捕获原理：用定时器的CNT计数器计数，当检测到上升沿时，
 *          硬件自动把CNT的值存到捕获寄存器CCR里；当检测到下降沿时，
 *          再存一次。两个边沿之间的计数差值×每计数一次的时间=时间差。
 *          本项目用了"主从模式"：捕获到上升沿时让CNT自动清零重数，
 *          于是"下一次上升沿捕获值"就是完整周期，直接就是频率和脉宽的原始数据。
 *       3. PWM_Set()：把TIM2测到的周期值赋给TIM3的ARR，
 *          把脉宽值赋给TIM3的CCR1，实现"输入什么波形，输出什么波形"。
 * 用到外设：TIM2、TIM3、GPIOA
 * 知识点：PWM输出、输入捕获、PSC/ARR/CCR的计算、主从模式（Reset）、PWM输入模式
 *============================================================================*/
#include "stm32f10x.h"                  // Device header（STM32寄存器定义和标准库函数）
//unsigned int arr = 100 - 1,psc = 720 - 1,ccr=50;	// 被注释的"直接定参数"写法（旧思路）
//占空比= ccr/（arr+1）		// 占空比计算公式：高电平占整个周期的比例
//频率= 72m/(psc+1)/(arr+1)	// 频率计算公式：72MHz先被PSC+1分频，再被ARR+1分频
//分辨率 = 1/（arr+1）		// 分辨率：ARR越大，占空比调节越精细（ARR+1种可能）

/* PWM输出初始化函数（用TIM3，输出引脚PA6）：
   功能：配置TIM3输出频率可变的PWM波。
   实现步骤：①开时钟（TIM3在APB1总线，GPIOA在APB2总线）；
            ②配置PA6为"复用推挽输出"（AF_PP）——这个引脚要交给TIM3来控制，
              所以不能配成普通推挽输出，必须用复用功能；
            ③配置TIM3时基（PSC预分频、ARR自动重装值）；
            ④配置输出比较通道（模式PWM1、输出使能、CCR初值）；
            ⑤启动TIM3（TIM_Cmd）。
   参数计算（本程序）：PSC=2-1=1，ARR=100-1=99，
      计数频率 = 72MHz/(PSC+1) = 72MHz/2 = 36MHz，
      输出频率 = 36MHz/(ARR+1) = 36MHz/100 = 360KHz。
      之后 PWM_Set() 会动态修改ARR和CCR1，所以这里的初值只是"占位"。 */
void PWMOut_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	// ①开启TIM3的时钟（TIM3挂在APB1总线上）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	// ①开启GPIOA的时钟（输出引脚在PA6）
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	// 被注释：重映射才需要AFIO时钟，本项目用默认引脚
//	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2,ENABLE);	// 被注释：若TIM2输出引脚要换位置才用重映射

	GPIO_InitTypeDef GPIO_InitStructure;	// 定义GPIO配置结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	// ②模式=复用推挽输出！
									// 关键点：PA6由TIM3的通道1控制输出波形，
									// 所以GPIO必须配置成"复用"模式(AF_PP)，
									// 普通推挽输出(Out_PP)是CPU直接控制引脚，不能用来输出PWM
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	// 用PA6引脚（TIM3的通道1默认输出引脚）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 翻转速度50MHz（PWM波形翻转较快，速度要配高）
	GPIO_Init(GPIOA,&GPIO_InitStructure);	// 写入寄存器，PA6配置完成
	
	TIM_InternalClockConfig(TIM3);	// 选择TIM3的时钟源=内部时钟（72MHz）。
									// 定时器也可以选外部引脚时钟/编码器模式，这里用默认内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	// 定义时基初始化结构体
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	// 时钟分频=1（滤波用，一般填DIV1）
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 计数模式=向上计数（0→ARR→0循环）
	TIM_TimeBaseInitStructure.TIM_Period =100  -1 ;	// ③ARR自动重装值=99：CNT数到99清零，
									// 决定PWM频率：F = 72M/(PSC+1)/(ARR+1) = 72M/2/100 = 360KHz
	TIM_TimeBaseInitStructure.TIM_Prescaler = 2 - 1;	// ③PSC预分频值=1：72MHz被分成(1+1)=2份，
									// 计数频率变成36MHz，也就是CNT每计1个数用1/36MHz秒
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;	// 重复计数寄存器（仅高级定时器TIM1/TIM8有效，TIM3填0即可）
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);	// 把时基配置写入TIM3寄存器
	
	TIM_OCInitTypeDef TIM_OCInitStructure;	// 定义输出比较（OC）配置结构体
	TIM_OCStructInit(&TIM_OCInitStructure);	// 先把结构体所有成员清零/设为默认值，防止垃圾值
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	// ④比较模式=PWM模式1：
									// 计数CNT < CCR1 时输出有效电平，CNT >= CCR1 时输出无效电平
									// （PWM模式2正好相反，一般用PWM1）
	
	TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;	// 输出极性=高电平有效：
									// CNT<CCR1时引脚输出高电平，CNT>=CCR1时输出低电平
									// （若选Low则整个波形反相）

	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	// 使能OC1输出（把波形送到引脚）
	TIM_OCInitStructure.TIM_Pulse = 0;	// ④CCR1捕获/比较寄存器初值=0（占空比先设为0%）
	TIM_OC1Init(TIM3,&TIM_OCInitStructure);	// 把配置写入TIM3的通道1（CCR1、CCMR1、CCER）
	//TIM_OC2Init(TIM2,&TIM_OCInitStructure);	// 被注释：如果要再输出一路PWM，就再初始化通道2
	
	TIM_Cmd(TIM3,ENABLE);	// ⑤启动TIM3！之前只是配置，这一步才开始真正计数输出
}

/* TIM2输入捕获初始化函数（测量引脚PA0）：
   功能：用TIM2的"PWM输入模式"测量外部输入方波的频率和脉宽。
   原理详解（本项目最难理解的部分）：
     ① TIM2的CNT计数器以固定频率向上计数（PSC=1，计数频率36MHz，
        也就是每计一个数耗时 1/36MHz ≈ 27.8ns）；
     ② 通道1捕获"上升沿"：检测到上升沿时，硬件把当时的CNT值存进CCR1，
        同时（通过TIM_SelectSlaveMode设置主从模式Reset）把CNT清零重新数；
     ③ 通道2捕获"下降沿"：检测到下降沿时，把当时的CNT值存进CCR2；
     ④ 于是：
        CCR1 = 从上升沿开始，经过一个完整周期再到下一个上升沿时的CNT值
             = 一个周期内CNT计数的个数 → 乘以每计数一次的时间=信号的周期
             频率 = 计数频率 / CCR1
        CCR2 = 从上升沿到下降沿之间的CNT计数个数 → 乘以每计数一次的时间
             = 高电平脉宽
     ⑤ 这样只要读 CCR1 和 CCR2 两个寄存器，就同时得到"频率"和"脉宽"！
   实现步骤：开时钟→配GPIO浮空输入→配时基→配输入捕获通道→
             PWM输入模式（通道2自动捕获下降沿）→选触发源→主从Reset模式→启动。 */
void PWMInt_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);	// 开启TIM2时钟（TIM2挂在APB1上）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	// 开启GPIOA时钟（测量引脚在PA0）
	
	GPIO_InitTypeDef GPIO_InitStructure;	// 定义GPIO配置结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// 模式=浮空输入！
									// 输入捕获引脚是"外部信号送进来"，引脚方向是输入，
									// 浮空输入不带上下拉电阻，完全由外部信号决定电平
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0;	// 用PA0引脚（TIM2的通道1默认输入引脚）
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;	// 速度参数（输入模式无实际意义）
	GPIO_Init(GPIOA,&GPIO_InitStructure);	// 写入寄存器，PA0配置完成
	TIM_InternalClockConfig(TIM2);	// 选择TIM2时钟源=内部时钟（72MHz）
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	// 定义时基结构体
	TIM_TimeBaseInitStructure.TIM_ClockDivision =TIM_CKD_DIV1;	// 时钟分频=1
	TIM_TimeBaseInitStructure.TIM_CounterMode  = TIM_CounterMode_Up;	// 向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;	// ARR=65535：让CNT能数满16位不溢出
									// （CNT是16位，最大65535，ARR填最大防止周期太长时溢出）
	TIM_TimeBaseInitStructure.TIM_Prescaler = 2 - 1;	// PSC=1：计数频率=72MHz/2=36MHz，
									// 每计一个数=1/36MHz≈27.8ns，这是"时间刻度"
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;	// 高级定时器专用参数，TIM2填0
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);	// 写入TIM2时基寄存器
	
	TIM_ICInitTypeDef TIM_ICInitStructure;	// 定义输入捕获（IC）配置结构体
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;	// 使用通道1（对应PA0引脚）
	//TIM_ICInitStructure.TIM_ICFilter = 0x0;	// 被注释：输入滤波器（可滤除毛刺），0x0=不滤波
	TIM_ICInitStructure.TIM_ICPolarity  =TIM_ICPolarity_Rising;	// 捕获极性=上升沿触发
									// （通道1负责捕获"上升沿"时刻的CNT值）
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	// 捕获预分频=1分频：每次边沿都捕获
									// （设为2分频的话，每2个边沿才捕获1次）
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;	// 输入选择=直连：
									// 通道1的信号直接来自TI1（PA0引脚），
									// 另一种是"间接"（交叉连接，用于PWM输入模式的通道2）

	// 关键一步！用PWM输入模式配置：一次配置自动完成通道1+通道2的设置。
	// 效果：通道1捕获上升沿（存CCR1），通道2自动配置为捕获下降沿（存CCR2），
	//       一个周期/一个脉宽全都测出来，不用自己手工配通道2
	TIM_PWMIConfig(TIM2,&TIM_ICInitStructure);
	
	TIM_SelectInputTrigger(TIM2,TIM_TS_TI1FP1);	// 选择触发源=TI1FP1（通道1的上升沿信号）
									// 作用：让"上升沿"成为主从模式的触发信号
	TIM_SelectSlaveMode(TIM2,TIM_SlaveMode_Reset);	// 从模式=Reset复位模式！
									// 效果：每次捕获到上升沿（触发信号），
									// 硬件自动把CNT计数器清零重新数，
									// 这样下一次上升沿捕获到的CCR1值
									// 正好等于"一个完整周期内的计数个数"
	
	TIM_Cmd(TIM2,ENABLE);	// 启动TIM2，输入捕获开始工作
}

/* PWM数据同步函数：
   功能：把TIM2测到的数据同步到TIM3，让TIM3输出的波形和输入波形一致。
   原理：
     TIM_GetCapture1(TIM2) = CCR1寄存器 = 输入信号的"周期计数个数"，
         → 赋给TIM3->ARR，则TIM3的输出周期=输入周期（频率一致）；
     TIM_GetCapture2(TIM2) = CCR2寄存器 = 输入信号的"高电平计数个数"（脉宽），
         → 赋给TIM3->CCR1，则TIM3的高电平时长=输入高电平时长（占空比一致）。
   公式验证：输入频率f=36MHz/CCR1，输出频率f=72MHz/(PSC+1)/(ARR+1)
             =72MHz/2/(CCR1+1)≈36MHz/CCR1，两者一致。
   注意：本函数要在主循环里反复调用（不断刷新），
         如果输入波形变化，输出波形也会跟着变。 */
void PWM_Set(void)
{
	TIM3->ARR = TIM_GetCapture1(TIM2);	// 把TIM2通道1捕获值（周期计数）写入TIM3的ARR
										// → 决定TIM3输出PWM的频率
	TIM3->CCR1 = TIM_GetCapture2(TIM2);	// 把TIM2通道2捕获值（脉宽计数）写入TIM3的CCR1
										// → 决定TIM3输出PWM的占空比
}

