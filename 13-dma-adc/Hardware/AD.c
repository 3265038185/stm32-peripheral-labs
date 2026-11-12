/*=============================================================================
 * 文件：AD.c（ADC + DMA 外设驱动文件）
 * 功能：初始化 ADC1 和 DMA1，实现"连续采集 + 自动搬运"：
 *         ADC1 连续不断地转换 PA0 引脚上的模拟电压（0~3.3V -> 0~4095），
 *         DMA1 在后台把每次的转换结果自动搬运进数组 AD_Value[128]，
 *         CPU 全程不用干预，主程序随时可以读到最新的 128 个采样点。
 *
 * 用到的外设/知识点：
 *   1. ADC1 规则组通道 0（PA0）：12 位分辨率，转换结果 0~4095
 *      电压换算公式：电压 = ADC值 / 4095 * 3.3V（VREF+接 3.3V 时）
 *   2. ADC 连续转换模式：转完一次立刻自动开始下一次，不需要软件反复触发
 *   3. DMA1 通道1：外设(ADC数据寄存器) -> 存储器(AD_Value数组) 搬运
 *      循环模式：搬完 128 次后自动从头再来，实现数据不停刷新
 *
 * 整体配置顺序（初学者按这个思路理解）：
 *   开时钟 -> 配置ADC时钟分频 -> 配置GPIO(PA0模拟输入) -> 配置规则组通道
 *   -> 配置ADC模式 -> 配置DMA -> 使能DMA和ADC -> ADC校准 -> 软件触发
 *=============================================================================*/
#include "stm32f10x.h"                  // Device header

uint16_t AD_Value[128];					//定义用于存放AD转换结果的全局数组
										//【重点】DMA 会把 ADC 转换结果自动写进这个数组，
										//       数组必须定义为全局变量（在函数外），
										//       这样它的地址才固定不变，DMA 才知道往哪搬。
										//       128 个元素对应 128 次转换，存满后 DMA 循环模式会从头覆盖写。

/**
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：本函数把 ADC1 和 DMA1 一次性配置好，之后不需要再调用，
  *           转换和搬运都由硬件自动完成（详见下面每步的注释）
  */
void AD_Init(void)
{
	/*开启时钟*/
	/* 原理：STM32 的外设平时是"断电"的，必须先把对应总线的时钟打开才能用。
	 * ADC1 挂载在 APB2 总线上，GPIOA 也挂载在 APB2 总线上，DMA1 挂载在 AHB 总线上 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//开启ADC1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟（PA0 引脚属于 GPIOA）
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA1的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择时钟6分频，ADCCLK = 72MHz / 6 = 12MHz
	/* 原因：STM32 规定 ADC 的时钟最高不能超过 14MHz，72MHz 分频 6 倍正好得 12MHz，
	 * 在允许范围内，且是常用的 ADC 工作时钟 */

	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;			//模式：模拟输入（AIN）
	/* 原因：作为 ADC 采样引脚时必须配成模拟输入模式，此时引脚内部的上拉/下拉
	 * 电阻和施密特触发器都被断开，模拟电压可以直接进入 ADC 的采样开关 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;				//引脚：PA0（板子上的电位器或信号输入接在这里）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//速度：50MHz（对模拟输入来说不影响，习惯上照写）
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA0引脚初始化为模拟输入
	
	/*规则组通道配置*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);	//规则组序列1的位置，配置为通道0
	/* 逐个参数解释：
	 * ADC1         —— 使用 ADC1 外设
	 * ADC_Channel_0—— 采样通道：ADC1 的通道 0，对应的引脚正是 PA0
	 * 1            —— 该通道放在规则组序列的第 1 位（本程序只采一个通道）
	 * ADC_SampleTime_55Cycles5 —— 采样时间 55.5 个 ADC 时钟周期：
	 *               采样时间越长，内部采样电容充电越充分，结果越准，
	 *               代价是转换速度变慢；55.5 周期是比较常用的折中值 */
	
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//模式，选择独立模式，即单独使用ADC1
	/* 原因：STM32 有 ADC1 和 ADC2 两个 ADC（部分型号还有 ADC3），
	 * 它们可以联合成"双ADC模式"配合使用；本程序只用 ADC1，所以选独立模式 */
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据对齐，选择右对齐
	/* 原因：12 位转换结果存在 16 位的寄存器里，右对齐表示结果从最低位开始放，
	 * 读出来的数就是 0~4095，最直观；左对齐一般用于把 12 位结果当 16 位整数参与运算的场合 */
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//外部触发，使用软件触发，不需要外部触发
	/* 原因：ADC 可以由定时器、外部引脚等信号触发转换（适合需要精确采样时刻的场景）；
	 * 本程序用软件触发就够，即调用 ADC_SoftwareStartConvCmd 主动发起转换 */
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;							//连续转换，使能，每转换一次规则组序列后立刻开始下一次转换
	/*【重点】连续转换 = 转完一次立即自动转下一次，配合 DMA 循环模式，
	 * 可以让 AD_Value 数组里的数据一直保持最新，实现"实时波形" */
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;								//扫描模式，使能，扫描规则组的序列，扫描数量由ADC_NbrOfChannel确定
	/* 原因：扫描模式用于"一次转换多个通道"（按规则组序列依次扫）；
	 * 本程序只采一个通道，无需扫描，所以失能（失能时规则组只转换序列第 1 个通道） */
	ADC_InitStructure.ADC_NbrOfChannel = 1;										//通道数，为4，扫描规则组的前4个通道
	/* 原因：指定规则组序列中要转换的通道个数，本程序只转换 1 个通道，
	 * 所以写 1（原注释里的"4"是江协模板的写法，与本程序实际配置无关） */
	ADC_Init(ADC1, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC1
	
	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;											//定义结构体变量
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;				//外设基地址，给定形参AddrA
	/* 原因：ADC1->DR 是 ADC1 的"数据寄存器"，每次转换完结果就放在这里，
	 * DMA 就从这个地址取数据（源地址）。(uint32_t) 是把寄存器地址强转成整数给 DMA */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外设数据宽度，选择半字，对应16为的ADC数据寄存器
	/* 原因：ADC 数据寄存器是 16 位（半字），所以搬运时按"半字"为单位，
	 * 一次搬 2 个字节，正好是 0~4095 的转换结果 */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址自增，选择失能，始终以ADC数据寄存器为源
	/* 原因：外设地址只有一个（ADC1->DR 固定不变），所以源地址不自增，
	 * 每次都从同一个寄存器取最新转换结果 */
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)AD_Value;					//存储器基地址，给定存放AD转换结果的全局数组AD_Value
	/* 原因：目的地址是数组 AD_Value 的首地址，转换结果从这里开始存放；
	 * 数组名 AD_Value 本身就代表首地址 */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//存储器数据宽度，选择半字，与源数据宽度对应
	/* 原因：目的端（数组元素）也是 16 位（uint16_t），与外设端数据宽度一致，一一对应 */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能，每次转运后，数组移到下一个位置
	/*【重点】目的地址自增 = 每搬一次，DMA 自动把地址往后移一个元素，
	 * 于是第一次进 AD_Value[0]、第二次进 AD_Value[1]……按顺序填满整个数组 */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//数据传输方向，选择由外设到存储器，ADC数据寄存器转到数组
	/* 原因：数据流动方向 = 外设(ADC) -> 存储器(数组)，所以源是外设。
	 * SRC 在这里指"数据从哪里来" */
	DMA_InitStructure.DMA_BufferSize = 128;										//转运的数据大小（转运次数），与ADC通道数一致
	/* 原因：一次"任务"搬 128 次，正好把 AD_Value[0]~AD_Value[127] 填满，
	 * 对应屏幕上 128 列像素，画一屏波形用 */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//模式，选择循环模式，与ADC的连续转换一致
	/*【重点】循环模式 = 搬完 128 次后，地址自动回到数组开头重新开始，
	 * 与 ADC 的"连续转换"配合，数组里的数据会被不停地刷新，永远是最近 128 个采样点。
	 * 若用普通模式，搬完 128 次后 DMA 就罢工了，需要软件重新触发 */
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
	/* 原因：M2M 模式用于"内存到内存"搬运，需要软件触发；
	 * 本程序是"外设到内存"，由 ADC 转换完成信号自动触发 DMA，所以失能 */
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	/* 原因：多个 DMA 通道同时请求总线时，按优先级高低排队；
	 * 本程序只有这一个 DMA 通道，中等优先级足够 */
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1
	/* 原因：ADC1 的 DMA 请求默认接在 DMA1 的通道 1 上，必须选 DMA1_Channel1，
	 * 选错通道的话数据永远不会搬运 */
	
	/*DMA和ADC使能*/
	DMA_Cmd(DMA1_Channel1, ENABLE);							//DMA1的通道1使能
	/* 先使能 DMA：让它准备好，等待 ADC 发出"转换完成"请求 */
	ADC_DMACmd(ADC1, ENABLE);								//ADC1触发DMA1的信号使能
	/* 原因：打开 ADC 到 DMA 之间的"请求通道"，ADC 每次转换完就会自动通知 DMA 来取数 */
	ADC_Cmd(ADC1, ENABLE);									//ADC1使能
	/* 最后使能 ADC：一切就绪后 ADC 开始工作（此时还没开始转换，需要下面软件触发） */
	
	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	/* 原因：ADC 内部有校准电路，用来修正电容失配等硬件误差，提高转换精度；
	 * 这是官方规定的固定流程，顺序不能乱 */
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);		//等待校准复位完成（SET=1 表示还在校准，循环等待）
	ADC_StartCalibration(ADC1);								//启动校准
	while (ADC_GetCalibrationStatus(ADC1) == SET);			//等待校准完成（校准期间程序停在这里）
	
	/*ADC触发*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//软件触发ADC开始工作，由于ADC处于连续转换模式，故触发一次后ADC就可以一直连续不断地工作
	/*【重点】只需触发这一次！因为配置了"连续转换"，转换完第 128 次会自动继续第 129 次……，
	 * DMA 循环模式也会一直把最新数据搬进数组。此后主程序什么都不用管 */
}
