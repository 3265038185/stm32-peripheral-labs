/*=============================================================================
 * 文件：AD.c（波形项目的 ADC 采样 + 定时器中断驱动文件）
 * 功能：
 *   1. 配置 ADC1（PA0 通道0）：单次转换模式，软件触发一次采一个点
 *   2. 配置定时器 TIM3：每 100us 溢出一次，进 TIM3 中断
 *   3. 在 TIM3 中断里定时调用 AD_GetValue() 采样，把结果按顺序
 *      存进数组 AD_Value[0]~AD_Value[127]（128 个点 = 一屏波形）
 *
 * 用到的外设/知识点：
 *   1. ADC 单次转换：每触发一次只转换一次，与"连续转换"相对。
 *      本程序用软件触发，采完一个点就停，等下次中断再采
 *   2. 采样时间与转换时间：ADC 时钟 12MHz，采样时间 55.5 周期，
 *      一次完整转换约 55.5+12.5 = 68 周期 ≈ 5.67us，远小于 100us 的采样间隔
 *   3. 定时器基本结构：预分频 PSC + 自动重装载 ARR，
 *      溢出周期 = (PSC+1)*(ARR+1)/72MHz
 *   4. NVIC 中断控制器：配置 TIM3 中断的优先级并使能
 *   5. 全局标志位（flag 系变量）是"中断 <-> 主循环"之间传递消息的信使：
 *        flagb：=0 时表示准备开始新一轮采样（把 a 清零从头存）
 *        flagc：=1 时表示 128 个点已采满（主循环收到后画波形）
 *        flage：用于记录"新一屏的第一个点"，供触发判断用
 *        Same / Same2：最近一次 / 上一次的采样值，用于上升沿触发判断
 *
 * 整体流程：
 *   开时钟 -> 配ADC时钟 -> 配GPIO(PA0模拟输入) -> 配规则组通道 -> 配ADC(单次转换)
 *   -> 使能ADC -> 校准 -> 配置TIM3 -> 配置NVIC -> 开定时器（中断里自动采样）
 *=============================================================================*/
#include "stm32f10x.h"   
// Device header
#include "OLED.h"   
#include "DELAY.h"  
/* 上面两个头文件当前并未使用（延时、显示逻辑在主循环里），保留引用无副作用 */
uint8_t flag = 1;		//全局变量 flag：预留标志位（本程序当前未使用）
uint16_t AD_Value[1000];	//定义用于存放AD转换结果的全局数组
/* 数组开成 1000 个元素，但本程序只用了前 128 个（AD_Value[0]~AD_Value[127]），
 * 128 正好是屏幕宽度，一屏画 128 个采样点。
 * 注意：这里与 DMA 项目不同，数据不是 DMA 搬进来的，
 * 而是 TIM3 中断里逐个用 AD_GetValue() 读进来填的 */
uint8_t a = 0;		//全局变量 a：数组下标，表示"下一个采样点该存到 AD_Value[几]"
//uint8_t flagt = 0;
//uint16_t  index;
//uint8_t flagt;
/* 上面是被注释掉的调试变量，理解主流程后可忽略 */
uint8_t flagc;		//全局标志 flagc：=1 表示 128 个点采满一屏，主循环据此画波形
uint8_t flagb;		//全局标志 flagb：=0 表示新一轮采样开始（中断里把 a 清零）
uint8_t flage;		//全局标志 flage：记录"新一轮的第一个采样点"，用于触发判断
uint16_t Same;		//全局变量 Same：最近一次采样的 ADC 值（触发判断用）
uint16_t Same2;		//全局变量 Same2：上一次采样的 ADC 值（用于判断上升沿）
//uint16_t Display_Value[128];//定义用于存放AD转换结果的全局数组（已注释，未使用）

/**
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：配置 ADC1 为"单次转换、软件触发"，PA0 采样
  */
void AD_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//开启ADC1的时钟
	/* 原理：外设平时"断电"，必须先开时钟；ADC1 和 GPIOA 都在 APB2 总线上 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟（PA0 属于 GPIOA）
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA1的时钟（本程序不用 DMA，注释掉）
	/* 本项目和"DMA+ADC"项目不同：这个波形项目不用 DMA，
	 * 而是在中断里用软件读取，所以不需要开 DMA1 的时钟 */
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择时钟6分频，ADCCLK = 72MHz / 6 = 12MHz
	/* 原理：STM32 规定 ADC 时钟最高不超过 14MHz，6 分频得 12MHz 正好合规 */
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模式：模拟输入
	/* 原理：采样引脚必须配成模拟输入，断开内部上拉下拉，让模拟电压直通 ADC */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;			//引脚：PA0（信号从这里输入）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度：50MHz（对模拟输入无影响，习惯写法）
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA0引脚初始化为模拟输入
	
	/*规则组通道配置*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);	//规则组序列1的位置，配置为通道0
	/* 参数含义：ADC1 的规则组序列第 1 位放"通道0"（对应 PA0），
	 * 采样时间 55.5 个 ADC 时钟周期，时间越长采样越准 */
	
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//模式，选择独立模式，即单独使用ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据对齐，选择右对齐
	/* 原理：12 位结果右对齐存放，读出来就是 0~4095，直观好用 */
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//外部触发，使用软件触发，不需要外部触发
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//连续转换，使能，每转换一次规则组序列后立刻开始下一次转换
	/*【重点】本程序选 DISABLE（单次转换）！与 DMA 项目相反：
	 * 单次转换 = 软件触发一次只转一次。因为本程序由 TIM3 中断定时触发采样，
	 * 不需要 ADC 自己连续转，采一次停一次正合适 */
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;								//扫描模式，使能，扫描规则组的序列，扫描数量由ADC_NbrOfChannel确定
	/* 本程序只采一个通道，不需要扫描，失能 */
	ADC_InitStructure.ADC_NbrOfChannel = 1;										//通道数，为4，扫描规则组的前4个通道
	/* 规则组只转换 1 个通道 */
	ADC_Init(ADC1, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC1
	
//	/*DMA初始化*/
//	DMA_InitTypeDef DMA_InitStructure;											//定义结构体变量
//	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;				//外设基地址，给定形参AddrA
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外设数据宽度，选择半字，对应16为的ADC数据寄存器
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址自增，选择失能，始终以ADC数据寄存器为源
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&AD_Value;					//存储器基地址，给定存放AD转换结果的全局数组AD_Value
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//存储器数据宽度，选择半字，与源数据宽度对应
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能，每次转运后，数组移到下一个位置
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//数据传输方向，选择由外设到存储器，ADC数据寄存器转到数组
//	DMA_InitStructure.DMA_BufferSize = 1000;										//转运的数据大小（转运次数），与ADC通道数一致
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;								//模式，选择循环模式，与ADC的连续转换一致
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
//	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
//	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1
//	
//	/*DMA和ADC使能*/
//	DMA_Cmd(DMA1_Channel1, ENABLE);							//DMA1的通道1使能
//	ADC_DMACmd(ADC1, ENABLE);								//ADC1触发DMA1的信号使能
/* 上面一大段 DMA 配置被注释掉了（波形项目走"中断采样"路线，不用 DMA），
 * 留着是为了和"DMA+ADC"项目对照学习 */
	ADC_Cmd(ADC1, ENABLE);									//ADC1使能
	/* 使能 ADC：ADC 进入工作状态，等待软件触发（此时还没有开始转换） */
	
	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	/* 官方规定的校准流程，顺序不能乱，提高转换精度 */
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);		//等待校准复位完成（SET=1 表示还在进行，循环等待）
	ADC_StartCalibration(ADC1);								//启动校准
	while (ADC_GetCalibrationStatus(ADC1) == SET);			//等待校准完成（期间程序停在这里）
	
	/*ADC触发*/
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//软件触发ADC开始工作，由于ADC处于连续转换模式，故触发一次后ADC就可以一直连续不断地工作
	/* 这里不触发！本程序是"单次转换"，触发工作放在 AD_GetValue() 里，
	 * 每次采样时触发一次即可 */
}
/**
  * 函    数：定时器TIM3初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：配置 TIM3 每 100us 溢出一次，并使能"更新中断"（进 TIM3_IRQHandler）
  */
/* 原理（初学者重点）：
 * 定时器溢出周期 = (预分频+1) x (自动重装载+1) / 72MHz
 *   预分频 PSC = 72-1：把 72MHz 时钟分频成 1MHz（1 微秒计 1 个数）
 *   重装载 ARR = 100-1：数 100 个数（100 微秒）就溢出
 *   溢出周期 = 100us，即每 100us 触发一次中断，采样率 = 10kHz */
void Tim_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);		//开启TIM3的时钟
	/* 注意：定时器 TIM3 挂在 APB1 总线上（和 ADC 的 APB2 不同总线！） */
	
	TIM_InternalClockConfig(TIM3);		//选择内部时钟：TIM3 用系统时钟驱动（不用外部引脚时钟）
	TIM_TimeBaseInitTypeDef TIM_TimebaseStructure;		//定义时基结构体变量
	TIM_TimebaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//时钟分频：1 分频（不影响，默认）
	TIM_TimebaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//计数模式：向上计数（0 数到 99 溢出）
	TIM_TimebaseStructure.TIM_Period = 100 - 1;		//自动重装载值 ARR=99：数到 99 就溢出清零，即 100 个脉冲一个周期
	TIM_TimebaseStructure.TIM_Prescaler = 72 - 1 ;	//预分频值 PSC=71：72MHz / 72 = 1MHz，一个计数脉冲 = 1us
	TIM_TimebaseStructure.TIM_RepetitionCounter = 0;	//重复计数寄存器（高级定时器才有，F103 的 TIM3 用不到，写 0）
	TIM_TimeBaseInit(TIM3,&TIM_TimebaseStructure);		//把结构体交给库函数，完成时基配置
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);	//清除更新标志：先清一次，避免上电后残留标志导致误触发
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);	//使能更新中断：每次计数器溢出（数到 99）就产生中断请求
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置中断优先级分组：2 位抢占优先级 + 2 位子优先级
	/* 原理：NVIC 是 Cortex-M3 的中断控制器，优先级分组规定"抢占优先级"和"子优先级"各占几位，
	 * 分组 2 = 两个各 2 位。本程序只有一个中断源，分组怎么分都行，习惯上写这个 */
	NVIC_InitTypeDef  NVIC_InitStructure;	//定义 NVIC 结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;		//中断通道：选择 TIM3 的中断号
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//使能该中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级：0（最高级，只有一个中断无所谓）
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	//子优先级：0
	NVIC_Init(&NVIC_InitStructure);		//把结构体交给 NVIC_Init，配置生效
	
	TIM_Cmd(TIM3,ENABLE);	//开启定时器：TIM3 开始计时，之后每 100us 进一次 TIM3_IRQHandler
}
/**
  * 函    数：获取一次AD转换结果
  * 参    数：无
  * 返 回 值：ADC转换结果（0~4095，对应 0~3.3V）
  * 说    明：软件触发一次单次转换，等待转换结束后读出结果。
  *          等待期间程序阻塞在这里，但转换只需约 5.7us，几乎感觉不到
  */
uint16_t AD_GetValue(void)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);					//软件触发AD转换一次
	/* 原理：写 ADC 的启动位，ADC 开始转换规则组序列（本程序只有通道0 一个点） */
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		//Delay_us(1);//等待EOC标志位，即等待AD转换结束
	/* EOC = End Of Conversion（转换结束标志），转换完成后硬件自动置 1，
	 * 在它变为 1 之前程序循环等待；等到了说明数据已经转换完成 */
	return ADC_GetConversionValue(ADC1);					//读数据寄存器，得到AD转换的结果
	/* 原理：读 ADC1->DR 数据寄存器（12 位结果，右对齐），
	 * 电压 = 返回值 / 4095 * 3.3V */
}
//void TIM2_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
//	{
//		
//		TIM_Cmd(TIM3,ENABLE);
//			
//		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
//	}
//	
//	
//}
/* 上面是被注释掉的 TIM2 中断函数（旧调试代码），当前使用 TIM3 */
/**
  * 函    数：TIM3更新中断服务函数
  * 参    数：无
  * 返 回 值：无
  * 说    明：每 100us 自动进入一次，完成"定时采样 + 存数组"的工作
  */
/* 原理（初学者重点）：这是整个项目的"心脏"——
 * 中断里做的每件事，主循环只能通过标志位间接感知：
 *   1. flagb==0：新一轮采样开始，把下标 a 清零，从头存数据
 *   2. 采样：AD_GetValue() 采一个点，存进 AD_Value[a]
 *   3. Same = 刚采的值，Same2 记录上一个值（上升沿判断用）
 *   4. a++ 后如果到 128：说明采满一屏，置 flagc=1 通知主循环画波形，
 *      下标 a 归零，准备存下一轮
 * 注意：中断函数里不能放 OLED 显示等耗时的代码，否则会错过下一次中断 */
void TIM3_IRQHandler(void)
{
	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)	//判断是否是"更新中断"（计数器溢出）
	{
		/* 注意：进入中断后必须调用 TIM_ClearITPendingBit 清除中断标志，
		 * 否则会不断重新进入该中断（见函数末尾） */

		if(flagb==0)		//flagb=0 表示新一轮采样开始（主循环在触发点把 flagb 清零）
		{
			a=0;			//数组下标归零：从 AD_Value[0] 开始存
			flagb=1;		//置 1：本轮只执行一次"从头开始"的操作
			flage =1;		//置 1：标记"下一拍采的是新一屏的第一个点"
		}

		AD_Value[a] = AD_GetValue();	//采样一次，存进数组当前下标位置
		Same = AD_Value[a];				//记录本次采样值（触发判断用）
		if(flage)			//如果是新一屏的第一个采样点
		{
			Same2 = AD_Value[a];		//记录它作为"上一次的值"基准
		}
		a++;				//下标加一，指向下一个存储位置
		if(a==128)			//采满 128 个点（一屏）
		{	a = 0;			//下标归零，下次从数组头重新覆盖
			flagc = 1;		//置标志：通知主循环"一屏采完了，可以画波形了"
		}
			

		
			
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);	//清除更新中断标志
		/* 必须清除！否则中断一直挂着，程序会反复进入本函数，造成死循环 */
	}
	
}

