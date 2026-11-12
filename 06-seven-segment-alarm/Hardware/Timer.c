/* ============================================================================
 * 文件说明（Timer.c —— 定时器初始化文件）
 * ----------------------------------------------------------------------------
 * 本文件负责配置两个定时器：
 *   TIM2：每 1 秒产生一次"更新中断"，是闹钟倒计时的"秒"来源
 *   TIM3：每 1 毫秒产生一次"更新中断"，是数码管动态扫描的"节拍"来源
 *
 * 相关知识点：
 *   1. 时钟树：TIM2、TIM3 挂在 APB1 总线上。STM32F103 中当 APB1 预分频
 *      ≠1 时，定时器时钟自动 = APB1 时钟 ×2 = 36MHz×2 = 72MHz，
 *      所以这里的定时器实际输入时钟是 72MHz
 *   2. 定时时间公式：溢出周期 T = 1 / (72MHz / (PSC+1) / (ARR+1))
 *      = (PSC+1)×(ARR+1) / 72MHz，其中 PSC=预分频器，ARR=自动重装值
 *   3. 更新中断：计数从 0 数到 ARR 溢出时，产生"更新事件"，若使能了
 *      中断，CPU 就会跳进对应的 IRQHandler（服务函数写在 main.c 中）
 * ========================================================================== */
#include "stm32f10x.h"    
//extern unsigned int Num;// Device header（原来用外部变量计数的写法，已注释）

/* --------------------------------------------------------------------------
 * 函数功能：初始化定时器 TIM2，让它在"每 1 秒"产生一次更新中断
 * 参数：无　　返回值：无
 * 定时计算：
 *   输入时钟 = 72MHz
 *   预分频器 PSC = 7200-1 → 计数频率 = 72MHz / 7200 = 10kHz（每数一次 0.1ms）
 *   自动重装值 ARR = 10000-1 → 数 10000 次才溢出
 *   溢出周期 = 0.1ms × 10000 = 1000ms = 1 秒 ✓
 * 注意：这里最后并没有调用 TIM_Cmd(TIM2,ENABLE) 启动定时器！
 *       它是被"开始/暂停"按键（main.c 中 TIM_Cmd(TIM2,ENABLE)）启动的
 * ------------------------------------------------------------------------ */
void Timer2_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  // 开启 TIM2 的外设时钟！
	                                        // 注意：STM32 的外设默认是"断电"状态，
	                                        // 必须先开时钟才能操作它的寄存器
	TIM_InternalClockConfig(TIM2);         // 选择内部时钟（72MHz）作为计数脉冲源，
	                                       // 也就是"1 个脉冲计 1 次数"
	TIM_TimeBaseInitTypeDef TIM_TimebaseStructure;
	TIM_TimebaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  // 时钟分频 1（滤波用，不影响定时）
	TIM_TimebaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数模式（0→ARR）
	TIM_TimebaseStructure.TIM_Period = 10000 - 1;   // 自动重装值 ARR = 9999（数到 10000 溢出）
	TIM_TimebaseStructure.TIM_Prescaler = 7200 - 1; // 预分频 PSC = 7199（72MHz/7200=10kHz）
	TIM_TimebaseStructure.TIM_RepetitionCounter = 0;// 重复计数（仅高级定时器有用，通用定时器填 0）
	TIM_TimeBaseInit(TIM2,&TIM_TimebaseStructure);   // 把上面的配置写入 TIM2 的寄存器
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);    // 先手动清除"更新标志"。
	                                       // 目的：防止上电瞬间的残留标志造成
	                                       // "一开中断就立刻进一次中断"的误触发
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);        // 使能 TIM2 的"更新中断"：
	                                       // 以后每次计数溢出，硬件都会向
	                                       // 内核发出中断请求（进 TIM2_IRQHandler）
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组方式：组 2
	                                       // （2 位抢占优先级 + 2 位响应优先级）
	NVIC_InitTypeDef  NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; // 要配置的中断通道：TIM2 全局中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能这个中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 抢占优先级 = 1（数字越小越优先）
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;         // 响应优先级 = 1
	NVIC_Init(&NVIC_InitStructure);         // 把配置写入 NVIC 寄存器
	
	//TIM_Cmd(TIM2,ENABLE);                 // （已注释）这里故意不启动定时器，
	                                       // 由开始/暂停按键控制 TIM2 的启停
}

/* --------------------------------------------------------------------------
 * 函数功能：初始化定时器 TIM3，让它在"每 1 毫秒"产生一次更新中断
 * 参数：无　　返回值：无
 * 定时计算：
 *   计数频率 = 72MHz / 7200 = 10kHz（每数一次 0.1ms）
 *   自动重装值 ARR = 10-1 → 数 10 次溢出
 *   溢出周期 = 0.1ms × 10 = 1ms ✓
 * 用途：main.c 的 TIM3_IRQHandler 每 1ms 进来一次，
 *       用于 4 位数码管的动态扫描（每毫秒点亮一位）
 * ------------------------------------------------------------------------ */
void Timer3_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  // 开启 TIM3 的外设时钟
	
	TIM_InternalClockConfig(TIM3);         // 选择内部时钟作为计数脉冲源
	TIM_TimeBaseInitTypeDef TIM_TimebaseStructure;
	TIM_TimebaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;   // 时钟分频 1
	TIM_TimebaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数
	TIM_TimebaseStructure.TIM_Period = 10 - 1;     // 自动重装值 ARR = 9（数到 10 溢出）
	TIM_TimebaseStructure.TIM_Prescaler = 7200 - 1;// 预分频 PSC = 7199（72MHz/7200=10kHz）
	TIM_TimebaseStructure.TIM_RepetitionCounter = 0;  // 重复计数填 0
	TIM_TimeBaseInit(TIM3,&TIM_TimebaseStructure);   // 写入 TIM3 寄存器
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);    // 清除更新标志（防误触发）
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);        // 使能 TIM3 更新中断
	
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // （已注释）
	                                               // 分组方式在 Timer2_Init 中
	                                               // 已设置过，无需重复设置
	NVIC_InitTypeDef  NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  // 中断通道：TIM3 全局中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // 使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 抢占优先级 = 1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;         // 响应优先级 = 1
	NVIC_Init(&NVIC_InitStructure);          // 写入 NVIC 寄存器
	
	TIM_Cmd(TIM3,ENABLE);                    // 启动 TIM3！数码管扫描从此刻开始
}

//void TIM2_IRQHandler(void)                 // （下面的整段代码被注释掉了）
//{                                          // 这是原来写在 Timer.c 里的
//	if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)   // 中断服务函数写法示例：
//	{                                          // 判断标志 → 处理 → 清标志
//		Num++;                               // 每 1 秒 Num+1（简单计数器）
//		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  // 清除中断标志
//	}
//}
