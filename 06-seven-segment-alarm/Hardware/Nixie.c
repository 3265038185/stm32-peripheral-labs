/* ============================================================================
 * 文件说明（Nixie.c —— 四位数码管驱动文件）
 * ----------------------------------------------------------------------------
 * 硬件连接（共阳极数码管，即"低电平点亮"）：
 *   PA0~PA7 —— 段选：控制 8 段（a、b、c、d、e、f、g 和小数点 dp）
 *   PB12~PB15 —— 位选：分别选中第 1~4 位数码管（高电平选中）
 *
 * 两个知识点：
 *   1. 段码表：Nixienum[] 数组里存放的是数字 0~9 对应的"段码"。
 *      因为是共阳极，某一段想点亮就把对应位写成 0（低电平）。
 *      例：数字"0"要亮 a~f 六段 → 段码 0xC0 = 1100 0000
 *      （bit0=a, bit1=b, …, bit6=g, bit7=dp，0 表示亮）
 *   2. 动态扫描：4 位数码管"同时显示"其实是靠轮流点亮实现的——
 *      任何时刻只有一位被位选选中在显示，1ms 换一位，利用人眼
 *      视觉暂留，看起来 4 位就同时在亮了（扫描由 TIM3 中断驱动）
 *
 * 对外提供的函数：
 *   Nixie_Init()          初始化引脚
 *   Nixie_ShowNum(x,y)    在第 x 位显示数字 y（不带小数点）
 *   Nixie_ShowDotNum(x,y) 在第 x 位显示数字 y（带小数点，dp 段点亮）
 * ========================================================================== */
#include "stm32f10x.h"                  // Device header

/* --------------------------------------------------------------------------
 * 函数功能：初始化数码管的引脚（段选 + 位选）
 * 参数：无　　返回值：无
 * 说明：
 *   PA0~PA7 段选（8 个引脚）、PB12~PB15 位选（4 个引脚）全部配置为
 *   "推挽输出"模式 —— 推挽输出能主动输出高/低电平，驱动能力最强，
 *   适合直接点亮 LED/数码管段；速度选 50MHz（最快）保证扫描刷新足够快
 * ------------------------------------------------------------------------ */
void Nixie_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   // 开启 GPIOA 的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   // 开启 GPIOB 的时钟
	                                       // 注意：GPIO 使用前必须先开时钟！
	GPIO_InitTypeDef GPIO_InitStrcture;
	GPIO_InitStrcture.GPIO_Mode = GPIO_Mode_Out_PP;        // 推挽输出模式
	GPIO_InitStrcture.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7;   // 段选：PA0~PA7
	GPIO_InitStrcture.GPIO_Speed = GPIO_Speed_50MHz;       // 输出速度 50MHz
	GPIO_Init(GPIOA,&GPIO_InitStrcture);   // 把配置写入 GPIOA（段选引脚）
	GPIO_InitStrcture.GPIO_Pin =GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;   // 位选：PB12~PB15
	GPIO_Init(GPIOB,&GPIO_InitStrcture);   // 把配置写入 GPIOB（位选引脚）
	GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7);
	                                       // 段选全部输出高电平 → 共阳极数码管
	                                       // 所有段熄灭（高电平=灭）
	GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	                                       // 位选全部输出低电平 → 4 位都不选中
	                                       // 也就是上电瞬间所有位都"关闭"
	
}
/* 段码表：Nixienum[0]~[9] 分别是数字 0~9 的段码（共阳极，0=亮），
 * 后面几个是符号（如 0x88 显示"-"等）。数字与段码对应关系：
 *   数字0 = 0xC0，数字1 = 0xF9，数字2 = 0xA4，数字3 = 0xB0，
 *   数字4 = 0x99，数字5 = 0x92，数字6 = 0x82，数字7 = 0xF8，
 *   数字8 = 0x80，数字9 = 0x90
 * 小技巧：NixieDotnum[] 就是在 Nixienum[] 基础上把 dp（bit7）改成 0，
 * 即"点亮小数点"，所以两个表只差一个 dp 位 */
unsigned char Nixienum[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xff};

/* --------------------------------------------------------------------------
 * 函数功能：在第 x 位（1~4）数码管上显示数字 y（不带小数点）
 * 参数：x —— 位号（1=最左边…4=最右边）
 *       y —— 要显示的数字（0~9，也允许 10~16 显示特殊符号）
 * 返回值：无
 * 实现原理：
 *   GPIOA->BSRR 是 STM32 的"置位/复位寄存器"：
 *     低 16 位写 1 → 对应引脚输出高电平；高 16 位写 1 → 对应引脚输出低电平
 *   本句 ~Nixienum[y]<<16 | Nixienum[y] 意思是：把段码写入低 16 位（置 1），
 *   把段码取反后写入高 16 位（对应位清 0），于是一次操作就把 8 个段引脚
 *   全部设置到位（段码为 1 的段熄灭、为 0 的段点亮）。
 *   注意：本函数只负责"段选"，"位选"（选哪一位亮）由调用方配合完成
 * ------------------------------------------------------------------------ */
void Nixie_ShowNum(int x,int y)
{
	
	if(y>=0&&y<11)                        // 防止越界：只处理 0~10 的段码
		{
			GPIOA->BSRR = ~Nixienum[y]<<16|Nixienum[y];   // 把 y 的段码一次性写到 PA0~PA7
		}
	if(x==1)                              // 选中第 1 位（位选 PB12 输出高电平）
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);
	}
	if(x==2)                              // 选中第 2 位（位选 PB13）
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_13);
	}
	if(x==3)                              // 选中第 3 位（位选 PB14）
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_14);
	}
	if(x==4)                              // 选中第 4 位（位选 PB15）
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_15);
	}
}

/* 带小数点的段码表：段码 = Nixienum 对应段码 & 0x7F（bit7=dp 变 0 → 点亮小数点） */
unsigned char NixieDotnum[] = {0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,0x08,0x03,0x46,0x21,0x06,0x0e};

/* --------------------------------------------------------------------------
 * 函数功能：在第 x 位（1~4）数码管上显示数字 y（带小数点，dp 段同时点亮）
 * 参数：x —— 位号（1~4）；y —— 要显示的数字（0~9）
 * 返回值：无
 * 与 Nixie_ShowNum 的区别：这里用的是 NixieDotnum[] 段码表，
 * 数字右下角的小数点（dp 段）会一起点亮，
 * 本工程里用来做倒计时的"秒点"（模拟电子表的冒号闪烁）
 * ------------------------------------------------------------------------ */
void Nixie_ShowDotNum(int x,int y)
{
	
	if(y>=0&&y<11)                        // 防止越界
		{
			GPIOA->BSRR =  ~NixieDotnum[y]<<16|NixieDotnum[y];   // 写入"带小数点"的段码
		}
	if(x==1)                              // 选中第 1 位
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);
	}
	if(x==2)                              // 选中第 2 位
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_13);
	}
	if(x==3)                              // 选中第 3 位
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_14);
	}
	if(x==4)                              // 选中第 4 位
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_15);
	}
}
