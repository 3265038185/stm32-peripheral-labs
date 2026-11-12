/*===============================================================================
 * 文件名称：Tim.c（定时器驱动文件）
 * 所属工程：3-定时器
 * 模块功能：初始化 TIM2 通用定时器，配置成"外部时钟模式2（ETR 引脚计数）"：
 *           把 PA0 引脚上输入的脉冲作为 TIM2 计数器的时钟信号，
 *           每来一个脉冲计数器 CNT 就加 1，计数到"自动重装值"后溢出，
 *           溢出时产生更新事件/更新中断（在 main.c 的 TIM2_IRQHandler 中处理）。
 * 用到的外设/知识点：
 *   1. RCC 时钟：开启 TIM2（在 APB1 总线上）和 GPIOA（在 APB2 总线上）的时钟
 *   2. GPIO：PA0 配置为"浮空输入"，用来接收外部脉冲（PA0 也是 TIM2 的 ETR 引脚）
 *   3. TIM2 外部时钟模式2：TIM_ETRClockMode2Config 配置 ETR 引脚作为外部时钟
 *   4. 定时计算公式：计数频率 = 外部脉冲频率（这里无预分频），
 *      溢出周期 = 外部脉冲个数（重装值 + 1）个脉冲溢出一次
 *===============================================================================
 */
#include "stm32f10x.h"                  // Device header

/*===============================================================================
 * 函数名称：Tim_Init（TIM2 初始化函数）
 * 功能：    把 TIM2 配置成"外部脉冲计数器"并开启更新中断
 * 参数：    无
 * 返回值：  无
 * 实现原理（逐行讲解）：
 *   1. RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
 *      —— 打开 TIM2 的时钟！STM32 里每个外设都有独立的时钟开关，
 *         不打开时钟，外设完全"不工作"（读寄存器全是 0）。
 *         TIM2 挂在 APB1 总线上（APB1 是低速总线，最高 36MHz）。
 *   2. RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
 *      —— 打开 GPIOA 的时钟（GPIO 都挂在 APB2 高速总线上）。
 *   3. PA0 配置为"浮空输入"：
 *      —— GPIO_Mode_IN_FLOATING 表示引脚既不接上拉也不接下拉，
 *         完全由外部电路决定电平，适合用来接收外部脉冲信号。
 *      —— PA0 正是 TIM2 的 ETR（外部触发）引脚，可以从这里输入脉冲。
 *   4. TIM_ETRClockMode2Config(TIM2, ...)：
 *      —— 把 TIM2 设置为"外部时钟模式2"，即：计数器的时钟不再是
 *         内部 72MHz，而是 ETR 引脚上进来的脉冲！
 *         TIM_ExtTRGPSC_OFF        ：外部触发分频 = 1（来一个脉冲计一次）
 *         TIM_ExtTRGPolarity_NonInverted ：不反相（上升沿有效）
 *         第三个参数 0x00         ：滤波器 = 0（不滤波，信号直接使用）
 *   5. TIM_TimeBaseInit（时基单元配置，这是定时器最核心的部分）：
 *      - TIM_ClockDivision = TIM_CKD_DIV1：
 *          内部时钟分频 = 1，只影响"采样滤波"，与计数无关，一般用 DIV1
 *      - TIM_CounterMode = TIM_CounterMode_Up：
 *          计数模式 = 向上计数，即 CNT 从 0 开始，加 1 加到重装值就溢出
 *      - TIM_Period = 10 - 1（自动重装值 ARR = 9）：
 *          计数器从 0 数到 9 一共 10 个脉冲就溢出一次！
 *          （注意：寄存器里写的是 9，硬件是从 0 开始数的，所以实际周期 = 10）
 *      - TIM_Prescaler = 1 - 1（预分频 PSC = 0）：
 *          分频系数 = 1，即时钟不除频。外部时钟模式下预分频一般设 0
 *      - TIM_RepetitionCounter = 0：
 *          重复计数寄存器（仅高级定时器 TIM1/TIM8 有效），普通定时器写 0 即可
 *   6. TIM_ClearFlag(TIM2, TIM_FLAG_Update)：
 *      —— 先清一次更新标志位，防止上电瞬间残留的脏标志导致"刚开中断就进中断"
 *   7. TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE)：
 *      —— 使能"更新中断"：溢出时允许向 NVIC 发出中断请求
 *         （注意：这只是"外设侧"允许发中断，还要配合 NVIC.c 里配置中断通道）
 *   8. TIM_Cmd(TIM2, ENABLE)：
 *      —— 最后启动定时器！一切配置好之前先别启动，配置完再上电工作
 * 总结定时公式：溢出一次 = 重装值 + 1 = 10 个外部脉冲
 *===============================================================================
 */
void Tim_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);	// 打开 TIM2 外设时钟（TIM2 在 APB1 总线上）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	// 打开 GPIOA 外设时钟（GPIO 在 APB2 总线上）
	GPIO_InitTypeDef GPIO_InitStructure;	// 定义一个 GPIO 初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// PA0 配置为浮空输入（外部脉冲进来）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	// 使用 PA0（TIM2 的 ETR 外部触发引脚）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 输入模式下速度一般不影响，填 50MHz 即可
	GPIO_Init(GPIOA,&GPIO_InitStructure);	// 把上面配置写入 GPIOA 的寄存器，生效！
	// 配置 TIM2 为外部时钟模式2：计数器时钟 = PA0 引脚进来的脉冲
	// （分频 OFF = 来一个脉冲计数一次；不反相 = 上升沿计数；滤波 0 = 不过滤）
	TIM_ETRClockMode2Config(TIM2,TIM_ExtTRGPSC_OFF,TIM_ExtTRGPolarity_NonInverted,0x00);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	// 定义时基初始化结构体
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	// 时钟分频 = 1（一般固定 DIV1）
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数模式（CNT 从 0 往上加）
	TIM_TimeBaseStructure.TIM_Period =10 - 1;	// 自动重装值 ARR = 9：计数到 9 溢出，共 10 个脉冲溢出一次
	TIM_TimeBaseStructure.TIM_Prescaler = 1 - 1;	// 预分频 PSC = 0：不分频（外部时钟模式下一般不分频）
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	// 重复计数 = 0（仅高级定时器有效，普通定时器写 0）
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);	// 把时基配置写入 TIM2 寄存器，生效！
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);	// 清除更新标志位，防止上电残留标志造成"一开中断就进中断"
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	// 使能更新中断：溢出时向 NVIC 申请中断（配合 NVIC.c 使用）
	TIM_Cmd(TIM2,ENABLE);	// 启动 TIM2！配置全部完成后再开始计数
}
