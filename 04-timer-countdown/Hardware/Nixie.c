#include "stm32f10x.h"                  // Device header

/*====================================================================
 * 文件说明：数码管显示模块
 * -------------------------------------------------------------------
 * 硬件连接（4 位共阳数码管）：
 *   PA0~PA7  —— 8 根"段码"线（a,b,c,d,e,f,g,dp），送数字/字母的
 *               笔画数据。共阳数码管：对应段为 0（低电平）才点亮。
 *   PB12~PB15 —— 4 根"位选"线，哪一位为 1（高电平）就选中哪一位。
 *
 * 为什么需要"动态扫描"：4 位数码管共用同一组段码线（PA0~PA7），
 * 同一时刻只能点亮 1 位。所以程序快速轮流点亮第 1、2、3、4 位，
 * 靠人眼"视觉暂留"看起来 4 位同时在显示（本工程由 TIM3 每 1ms
 * 进一次中断做扫描）。
 *
 * 本文件提供两个显示函数：
 *   Nixie_ShowNum    —— 显示不带小数点的数字；
 *   Nixie_ShowDotNum —— 显示带小数点的数字（用作"冒号"闪烁）。
 *====================================================================*/
void Nixie_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	// 开启 GPIOA 时钟（段码线）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	// 开启 GPIOB 时钟（位选线）
	GPIO_InitTypeDef GPIO_InitStrcture;
	GPIO_InitStrcture.GPIO_Mode = GPIO_Mode_Out_PP;	// 推挽输出：能输出 0 和 1，驱动能力够
	GPIO_InitStrcture.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStrcture.GPIO_Speed = GPIO_Speed_50MHz;	// 翻转速度 50MHz
	GPIO_Init(GPIOA,&GPIO_InitStrcture);	// 把配置写入 PA0~PA7
	GPIO_InitStrcture.GPIO_Pin =GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Init(GPIOB,&GPIO_InitStrcture);	// 把配置写入 PB12~PB15
	// 初始化状态：段码全部置 1（共阳=全灭），位选全部拉低（不选任何位）
	GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7);
	GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	
}
/* 段码表（不带小数点）：下标 0~9 对应数字 0~9，10~15 对应 A~F，16 熄灭。
 * 每个元素是 8 位二进制（0b a b c d e f g dp），位=0 表示该段点亮
 * （共阳数码管）。例如 0xC0 = 1100 0000b：a~g 全 0 → 显示 "0"。 */
unsigned char Nixienum[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xff};

/*====================================================================
 * Nixie_ShowNum：让第 x 位数码管显示数字 y（不带小数点）
 * @param  x  数码管位号（1~4）
 * @param  y  要显示的数字（0~10，再大对应 A~F/熄灭）
 * @retval 无
 * -------------------------------------------------------------------
 * 原理：
 *   1. 先把段码 y 送到 PA0~PA7（所有位共用的段码线）；
 *   2. 再把第 x 位的位选引脚拉高，让这一位"亮起来"。
 * 注意：调用前一般要先"消隐"（熄灭所有位），否则会串位。
 *====================================================================*/
void Nixie_ShowNum(int x,int y)
{
	
	/* 只有 y 在 0~10 范围内才更新段码（防止数组越界） */
	if(y>=0&&y<11)
		{
			/* 这里用 BSRR 寄存器写入段码，与 ODR 写法等价：
			 * BSRR 高 16 位是"复位"（写 1 的那位输出 0），
			 * 低 16 位是"置位"（写 1 的那位输出 1）。
			 * ~Nixienum[y]<<16：把段码取反后放到高 16 位 → 段码为 0
			 * 的位输出低电平（点亮）；
			 * Nixienum[y]：原样放到低 16 位 → 段码为 1 的位输出高电平。
			 * 一次写操作同时完成"置位 + 复位"，不会干扰其他引脚。 */
			GPIOA->BSRR = ~Nixienum[y]<<16|Nixienum[y];
		}
	/* 位选：哪一位要亮，就把对应的位选引脚拉高（共阳数码管高电平选中） */
	if(x==1)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);	// 选中第 1 位（PB12）
	}
	if(x==2)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_13);	// 选中第 2 位（PB13）
	}
	if(x==3)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_14);	// 选中第 3 位（PB14）
	}
	if(x==4)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_15);	// 选中第 4 位（PB15）
	}
}
/* 段码表（带小数点）：与 Nixienum 基本相同，只是把数字的
 * 小数点段（dp，最低位）也点亮了。
 * 例如数字 0 的带点段码是 0x40 = 0100 0000b：a~g 全 0 显示 "0"，
 * dp 位也是 0 → 右下角多一个小数点。本工程用作"冒号"闪烁。 */
unsigned char NixieDotnum[] = {0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,0x08,0x03,0x46,0x21,0x06,0x0e};

/*====================================================================
 * Nixie_ShowDotNum：让第 x 位数码管显示"带小数点"的数字 y
 * @param  x  数码管位号（1~4）
 * @param  y  要显示的数字（0~10）
 * @retval 无
 * -------------------------------------------------------------------
 * 与 Nixie_ShowNum 的区别：段码改用 NixieDotnum 表（多点亮 dp 段），
 * 其余位选逻辑完全一样。
 *====================================================================*/
void Nixie_ShowDotNum(int x,int y)
{
	
	/* 只有 y 在 0~10 范围内才更新段码（防止数组越界） */
	if(y>=0&&y<11)
		{
			/* BSRR 同时"置位+复位"写段码（原理见 Nixie_ShowNum 注释） */
			GPIOA->BSRR =  ~NixieDotnum[y]<<16|NixieDotnum[y];
		}
	/* 位选：哪一位要亮，就把对应的位选引脚拉高 */
	if(x==1)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);	// 选中第 1 位（PB12）
	}
	if(x==2)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_13);	// 选中第 2 位（PB13）
	}
	if(x==3)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_14);	// 选中第 3 位（PB14）
	}
	if(x==4)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_15);	// 选中第 4 位（PB15）
	}
}
