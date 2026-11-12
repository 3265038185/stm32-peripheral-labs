/*============================================================================
  文件：PWM.c（PWM 输出 + PWM 输入捕获）
  ----------------------------------------------------------------------------
  本文件包含两部分功能：
   1. PWM 输出（PWMOut_Init + PWM_Set）：本项目的核心！
      - 用 TIM3_CH1（引脚 PA6）输出可调频率的方波，驱动【无源蜂鸣器】；
      - 无源蜂鸣器内部没有振荡器，必须给它的引脚输入一定频率的方波，
        振膜才会按这个频率振动发出声音。改变方波频率 = 改变音高。
   2. PWM 输入测量（PWMInt_Init + PWM_GetFreq + PWM_GetDuty）：
      - 用 TIM2_CH1（引脚 PA0）做"输入捕获"，测量外部方波信号的
        频率和占空比（比如测遥控器/编码器输出）。
  ----------------------------------------------------------------------------
  PWM 核心公式（初学者必背）：
    计数频率   = 72MHz / (PSC + 1)
    输出频率   = 计数频率 / (ARR + 1)
    占空比     = CCR / (ARR + 1)
    分辨率     = 1 / (ARR + 1)（ARR 越大，频率调节越精细）
  ----------------------------------------------------------------------------
  本工程 PWM 输出频率的计算：
    - 定时器时钟 72MHz，PSC 预分频固定 (7200-1)，
      则计数频率 = 72MHz / 7200 = 10kHz；
    - ARR = 10000-1，则"最高输出频率"= 10kHz / 10000 = 1Hz；
    - PWM_Set 里动态改 PSC = 7200/Freq，则计数频率 = 72MHz / (7200/Freq)
      = Freq * 10kHz，输出频率 = Freq * 10kHz / 10000 = Freq（Hz）。
      即"PSC 改成多大，输出就是多少 Hz"，非常直观！
    - 占空比固定 50%（CCR=5000），方波最"响亮"且谐波少。
  ============================================================================*/
#include "stm32f10x.h"     
#include "Delay.h"// Device header
//unsigned int arr = 100 - 1,psc = 720 - 1,ccr=50;
//占空比= ccr/（arr+1）
//频率= 72m/(psc+1)/(arr+1)
//分辨率 = 1/（arr+1）
/*  上面注释掉的 3 行是"学习笔记"：
    它们总结的公式就是本文件注释里那套，arr 对应 ARR（自动重装值），
    psc 对应 PSC（预分频值），ccr 对应 CCR（比较值）。  */

/*
 * 函数名：PWMOut_Init（PWM 输出初始化 - 驱动无源蜂鸣器）
 * 参数：无
 * 返回：无
 * 功能：配置 TIM3_CH1 为 PWM 输出模式，引脚 PA6，输出 50% 占空比方波。
 * 步骤拆解：
 *   1. RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3)：开启 TIM3 时钟。
 *      注意：TIM2/TIM3/TIM4 挂在 APB1 总线上，和 GPIO 的 APB2 不一样，
 *      开错总线时钟外设是不工作的！
 *   2. GPIOA 配置为 GPIO_Mode_AF_PP（复用推挽输出）：
 *      "复用"表示这个引脚不再由 GPIO 控制，而是交给 TIM3_CH1 输出，
 *      "推挽"表示能主动输出高/低电平，驱动能力好。
 *   3. TIM_InternalClockConfig：选择内部时钟源（72MHz），
 *      不用外部输入信号。
 *   4. 时基配置（决定频率）：
 *      - TIM_Prescaler = 7200-1：72MHz / 7200 = 10kHz 计数频率；
 *      - TIM_Period    = 10000-1：计数到 10000 个脉冲就溢出一次，
 *        所以基础输出频率 = 10kHz / 10000 = 1Hz；
 *      - TIM_CounterMode_Up：向上计数（0->ARR->0 循环）；
 *      - TIM_ClockDivision：采样分频，PWM 不用管，填 DIV1。
 *   5. 输出比较配置（决定占空比和波形）：
 *      - TIM_OCMode_PWM1：PWM 模式 1，CNT < CCR 输出有效电平，
 *        CNT >= CCR 输出无效电平；
 *      - TIM_OCPolarity_High：有效电平是高电平；
 *      - TIM_Pulse = 5000：即 CCR=5000，所以占空比 = 5000/10000 = 50%；
 *      - TIM_OC1Init：注意是"OC1"，对应 CH1 通道，别写成 OC2。
 *   6. TIM_Cmd(TIM3, ENABLE)：最后"总开关"打开定时器，
 *      PWM 才开始输出（这一步漏了，前面的配置全白费）。
 */
void PWMOut_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	// 开启 TIM3 时钟（APB1 总线）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	// 开启 GPIOA 时钟（APB2 总线）
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
//	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2,ENABLE);
/*  上面注释的是"引脚重映射"代码：如果 PA6 被别的功能占了，
    可以把 TIM3_CH1 重映射到别的引脚。本工程 PA6 空闲，不需要。  */

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	// 复用推挽输出：引脚交给 TIM3_CH1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		// PA6 = TIM3_CH1 输出脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 输出速度 50MHz（够用且兼顾 EMI）
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM3);	// 使用内部 72MHz 时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;	// 时钟分频 1（不用滤波采样）
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period =10000  -1 ;		// ARR=9999：计数到 10000 溢出
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;		// PSC=7199：72MHz/7200=10kHz 计数频率
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;	// 重复计数（仅高级定时器有用）
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);		// 先用默认值填充结构体，防止残留垃圾值
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	// PWM 模式 1
	/*  PWM 模式 1 的波形规则：
	    计数器 CNT < 比较值 CCR 时输出有效电平（高），
	    CNT >= CCR 时输出无效电平（低）。
	    所以 CCR 越大，高电平时间越长，占空比越大。  */
	
	TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;	// 有效电平 = 高电平

	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	// 使能该通道的输出
	TIM_OCInitStructure.TIM_Pulse = 5000;	// CCR = 5000 -> 占空比 5000/10000 = 50%
	TIM_OC1Init(TIM3,&TIM_OCInitStructure);	// 初始化通道 1（CH1）
	//TIM_OC2Init(TIM2,&TIM_OCInitStructure);
	
	TIM_Cmd(TIM3,ENABLE);	// 打开 TIM3 总开关，PWM 开始输出
}

/*
 * 函数名：PWMInt_Init（PWM 输入捕获初始化 - 测外部方波的频率/占空比）
 * 参数：无
 * 返回：无
 * 功能：用 TIM2_CH1（PA0）测量外部方波信号的频率和占空比。
 * 原理（"主从模式 + 从模式复位"自动测量，无需中断）：
 *   1. 外部信号从 PA0（TIM2_CH1）进入，先经过输入滤波器
 *      （TIM_ICFilter=0xF，滤掉高频毛刺）；
 *   2. TIM_ICSelection_DirectTI：通道 1 直连捕获自己的输入信号；
 *      TIM_ICPolarity_Rising：上升沿捕获；
 *   3. 关键配置：TIM_SelectInputTrigger(TIM2, TIM_TS_TI1FP1)
 *      把"TI1 的上升沿"设为触发源，TIM_SelectSlaveMode(Reset)
 *      让"每次上升沿到来时计数器自动清零"。
 *      于是：
 *        - 通道1 捕获值 = 一个周期的计数值 -> 可算出频率；
 *        - 通道2 通过"间接捕获"（TIM_PWMIConfig 自动配置）在下降沿
 *          捕获，捕获值 = 高电平宽度 -> 可算出占空比。
 *   4. 配置好时基：PSC=1（计数频率 72MHz/2=36MHz），
 *      ARR=65535，保证低频信号一个周期内不溢出。
 * 使用方式：配合 PWM_GetFreq / PWM_GetDuty 在主循环中读取测量结果。
 */
void PWMInt_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);	// 开启 TIM2 时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	// 开启 GPIOA 时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// 浮空输入：引脚电平完全由外部信号决定
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0;	// PA0 = TIM2_CH1 输入脚
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	TIM_InternalClockConfig(TIM2);	// 内部 72MHz 时钟
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision =TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode  = TIM_CounterMode_Up;	// 向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;	// ARR 拉满，防止低频周期溢出
	TIM_TimeBaseInitStructure.TIM_Prescaler = 2 - 1;	// PSC=1：计数频率 = 72MHz/2 = 36MHz
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;	// 通道 1 负责捕获
	TIM_ICInitStructure.TIM_ICFilter = 0xF;			// 输入滤波 15 个采样周期，滤除毛刺
	TIM_ICInitStructure.TIM_ICPolarity  =TIM_ICPolarity_Rising;	// 上升沿捕获（测周期）
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;		// 捕获分频 1（每次都捕获）
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;	// 直连捕获通道输入
	

	TIM_PWMIConfig(TIM2,&TIM_ICInitStructure);
	/*  TIM_PWMIConfig = "PWM 输入模式"一键配置：
	    自动把通道 1 设为上升沿捕获（测周期）、通道 2 设为下降沿捕获
	    （测高电平宽度），省去手工配置通道 2 的麻烦。  */
	
	TIM_SelectInputTrigger(TIM2,TIM_TS_TI1FP1);	// 触发源 = TI1 上升沿（滤波后的通道1输入）
	TIM_SelectSlaveMode(TIM2,TIM_SlaveMode_Reset);	// 从模式 = 复位：每次上升沿计数器归零
	
	TIM_Cmd(TIM2,ENABLE);	// 启动 TIM2，开始测量
}

/*
 * 函数名：PWM_GetFreq（读取输入信号频率）
 * 参数：无
 * 返回：输入方波的频率（Hz）
 * 原理：因为"每次上升沿计数器都自动清零"，所以通道 1 捕获值
 *       = 一个周期内的计数个数；
 *       计数频率 36MHz / 周期计数值 = 每秒多少个周期 = 频率。
 *       例：捕获值 3600 -> 36MHz/3600 = 10kHz。
 */
unsigned int PWM_GetFreq(void)
{
	return 36000000 / TIM_GetCapture1(TIM2);	// 36MHz / 周期计数值
}

/*
 * 函数名：PWM_GetDuty（读取输入信号占空比）
 * 参数：无
 * 返回：占空比（百分比 0~100）
 * 原理：通道 2 在下降沿捕获，捕获值 = 高电平持续期间计数器走过的
 *       个数（加 1 修正舍入误差）；
 *       占空比 = 高电平时间 / 一个周期时间 = 捕获2 / 捕获1。
 *       例：周期 3600、高电平 1800 -> 占空比 50%。
 */
unsigned int PWM_GetDuty(void)
{
	return (TIM_GetCapture2(TIM2) + 1) * 100 / TIM_GetCapture1(TIM2);	// 高电平宽度/周期 * 100
}

/*
 * 函数名：PWM_Set（设置音符频率并播放指定时长）
 * 参数：Freq —— 音符频率（Hz，0/1 表示休止符），time —— 播放时长（毫秒）
 * 返回：无
 * 功能：main.c 里播放音乐的核心函数：先改 PSC 让 PWM 输出 Freq Hz 的
 *       方波，再延时 time 毫秒（期间蜂鸣器持续响这个音）。
 * 原理（动态改频率的巧妙之处）：
 *   - 保持 ARR=10000 和 CCR=5000 不变，只改 PSC：
 *     计数频率 = 72MHz / (7200/Freq) = Freq * 10kHz，
 *     输出频率 = Freq * 10kHz / 10000 = Freq Hz；
 *   - TIM_PrescalerConfig 是"运行中改 PSC"的库函数，
 *     TIM_PSCReloadMode_Immediate 表示"立即生效"（不等当前周期结束）；
 *   - 占空比始终 50%，所以只改变音高（频率），音量和音色不变。
 *   注意：休止符用 Freq=1（PSC=7200），输出 1Hz 几乎无声，相当于停顿。
 */
void PWM_Set(unsigned int Freq,unsigned int time)
{
	TIM_PrescalerConfig(TIM3,7200/Freq,TIM_PSCReloadMode_Immediate);	// 改 PSC -> 改输出频率
	Delay_ms(time);	// 让该音符持续响 time 毫秒
}

