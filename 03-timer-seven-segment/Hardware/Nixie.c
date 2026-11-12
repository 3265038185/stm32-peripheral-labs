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
 * 靠人眼"视觉暂留"看起来 4 位同时在显示（类似放电影）。
 *
 * 段码表 Nixienum 存的是"0~F 和熄灭"对应的 8 位段码：
 *   例如数字 0 的段码 0xC0 = 1100 0000b：共阳接法下，a~g 全亮
 *   （低电平 0），只有 dp 为 1 不亮，所以显示出 "0"。
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
/* 段码表：下标 0~9 对应数字 0~9，10~15 对应字母 A~F，16 对应熄灭。
 * 每个元素是 8 位二进制（0b a b c d e f g dp）：
 *   位=0 表示该段点亮（共阳数码管），位=1 表示该段熄灭。
 * 例如 0xC0（数字0）：除 dp 外全为 0 → a~g 七段全亮，显示 "0"。 */
unsigned char Nixienum[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xff};

/*====================================================================
 * Nixie_ShowNum：让第 x 位数码管显示数字 y
 * @param  x  数码管位号（1~4）
 * @param  y  要显示的数字（0~10，10 及以上的下标对应 A~F/熄灭）
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
			/* 直接向 ODR 寄存器写入段码：
			 * ODR 是 PA0~PA15 的输出数据寄存器，写 1=输出高，写 0=输出低 */
			GPIOA->ODR = Nixienum[y];
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
