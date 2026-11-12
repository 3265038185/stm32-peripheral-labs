/* ============================================================================
 * 文件说明：PWM.c —— PWM 输出初始化（本项目的核心模块！）
 * ----------------------------------------------------------------------------
 * 【功能】
 *   把 TIM2 定时器的通道 1（CH1）配置成 PWM 输出模式，输出引脚为 PA0，
 *   PA0 接 LED（阳极），通过改变 PWM 占空比即可控制 LED 亮度。
 *
 * 【PWM 是什么？】
 *   PWM（脉宽调制）= 输出一串固定频率的方波，但"高电平占一个周期的比例"可变。
 *   LED 对高频方波的响应：由于人眼视觉暂留，看到的亮度 ≈ 占空比。
 *   占空比 = 高电平时间 / 整个周期，占空比越大 LED 越亮。
 *
 * 【关键公式（背下来！）】
 *   - PWM 频率    = 定时器时钟 / (PSC+1) / (ARR+1)
 *   - 占空比      = CCR / (ARR+1) × 100%
 *   - 分辨率（能分多少级亮度）= 1 / (ARR+1)
 *   本工程：72MHz / 720 / 100 = 1000Hz（1kHz），占空比 0%~100% 可调 101 级。
 *
 * 【实现原理】
 *   1. TIM2 的计数器 CNT 从 0 加到 ARR(99)，然后清零重新加（向上计数模式）；
 *   2. 输出比较寄存器 CCR1 里存一个值，硬件自动比较：
 *      - CNT < CCR1 时输出高电平；
 *      - CNT >= CCR1 时输出低电平（PWM 模式 1）；
 *   3. 所以 CCR1 越大，高电平时间越长，占空比越大。main 里用
 *      TIM_SetCompare1(TIM2, i) 修改 CCR1，就能实时改亮度。
 * ========================================================================== */

#include "stm32f10x.h"                  // Device header
//unsigned int arr = 100 - 1,psc = 720 - 1,ccr=50;
//占空比= ccr/（arr+1）
//频率= clk/(psc+1)/(arr+1)
//分辨率 = 1/（arr+1）
/* 上面注释里的公式是本工程 PWM 的核心知识，详细解释见文件头注释 */

/* ----------------------------------------------------------------------------
 * 函数名：PWM_Init
 * 功能  ：初始化 TIM2 的 PWM 输出（PA0 输出 PWM 波形）
 * 参数  ：无
 * 返回  ：无
 * 配置顺序（照抄即可，顺序错了可能不出波形）：
 *   1. 开启外设时钟：TIM2（APB1 总线）+ GPIOA（APB2 总线）
 *   2. 配置 PA0 为"复用推挽输出"（GPIO_Mode_AF_PP）
 *   3. 选择定时器时钟源：内部时钟（72MHz）
 *   4. 配置时基单元：预分频 PSC、自动重装 ARR、计数模式
 *   5. 配置输出比较单元：PWM 模式1、极性高、使能输出、初始 CCR=0
 *   6. TIM_Cmd 使能定时器（不开这个 CNT 不跑，没有波形）
 * -------------------------------------------------------------------------- */
void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	// 开启 TIM2 的时钟。TIM2 属于高级/通用定时器，挂在 APB1 总线上，
	// 所以用 RCC_APB1PeriphClockCmd（注意不是 APB2！）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	// 开启 GPIOA 时钟（PA0 挂在 APB2 总线上）。外设时钟默认全关，必须先开！
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
//	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2,ENABLE);
	/* 上面两行被注释的是"引脚重映射"配置：TIM2 的通道 1 默认就在 PA0，
	 * 不需要重映射。如果想把 PWM 引到其他引脚（比如 PA15 等），
	 * 才需要开 AFIO 时钟并调用 GPIO_PinRemapConfig */

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	// 关键配置：复用推挽输出（AF_PP = Alternate Function Push-Pull）！
	// 为什么是这个模式？因为 PA0 的电平不再由"软件"控制，而是由定时器硬件控制，
	// 所以引脚要"让位"给外设，这就是"复用"。推挽 = 强驱动高低电平，
	// 能直接点亮 LED。模式配错了（比如配成普通推挽输出）引脚就出不了波形！
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;      // 选择 PA0（TIM2_CH1 的默认引脚）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 引脚翻转速度 50MHz（足够快）
	GPIO_Init(GPIOA,&GPIO_InitStructure);          // 配置写入 GPIOA 的 CRL 寄存器
	
	TIM_InternalClockConfig(TIM2);
	// 选择时钟源：内部时钟。定时器可以从内部时钟(72MHz)或外部引脚输入计数，
	// 一般 PWM 输出都用内部时钟。
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	// 时钟分频：TIM_CKD_DIV1 表示输入时钟不分频（这个参数主要影响滤波采样，一般用 DIV1）
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	// 计数模式：向上计数（CNT 从 0 加到 ARR 后清零重来），PWM 最常用
	TIM_TimeBaseInitStructure.TIM_Period =100  -1 ;
	// 自动重装值 ARR = 99：CNT 数到 99 就清零，一个周期共 100 个计数节拍
	// 结合 PSC：PWM 频率 = 72MHz / 720 / 100 = 1000Hz
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;
	// 预分频 PSC = 719：把 72MHz 系统时钟先分频 720 倍，得到 100kHz 计数时钟
	// （72MHz / 720 = 100kHz，即 CNT 每 10us 加 1，一个周期 100 拍 = 1ms）
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	// 重复计数寄存器（RCR）：只有高级定时器 TIM1/TIM8 才有意义，TIM2 填 0 即可
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	// 把时基配置写入 TIM2（PSC、ARR 等寄存器）
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	// 先用库自带的"结构体默认值"初始化，防止有成员忘记赋值导致随机值
	// （随机值可能让 PWM 输出异常，这是个容易踩的坑！）
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	// 输出比较模式：PWM 模式 1。含义：
	//   CNT < CCR 时输出有效电平（高），CNT >= CCR 时输出无效电平（低）。
	// 即 CCR 越大高电平越宽，占空比 = CCR/(ARR+1)。PWM2 模式则正好相反。
	
	TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
	// 输出极性：高电平有效。也就是 CCR 越大 LED 越亮；
	// 如果改成 Low，波形反相，LED 亮度和 CCR 成反比

	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	// 使能通道输出：必须使能，否则 CCR 比较结果不会送到 PA0 引脚上
	TIM_OCInitStructure.TIM_Pulse = 0;
	// 初始 CCR1 = 0：上电时占空比为 0%（LED 灭），稍后由 main 循环修改
	// TIM_Pulse 就是 CCR 的值，相当于 TIM_SetCompare1 设置的初始值
	
	TIM_OC1Init(TIM2,&TIM_OCInitStructure);
	// 把以上配置写入 TIM2 的通道 1（CCR1、CCMR1 寄存器）
	
	TIM_Cmd(TIM2,ENABLE);
	// 使能 TIM2（CR1 寄存器的 CEN 位）：不开这一步 CNT 计数器根本不跑，
	// PA0 上不会有任何波形！这是新手最容易忘的一步。
}
//void PWM_Set
