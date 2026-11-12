/*****************************************************************************
 * 文件名：Song.c（System 系统层）
 * 所属工程：1-LED闪烁
 *
 * 【文件功能】
 *   用 10 个 LED 的"不同点亮组合"来模拟《铃儿响叮当》的音调——
 *   每个音符 x 对应一种 LED 点亮图案，点亮 y 毫秒后再全部熄灭，
 *   配合 main.c 里预存的音符数组（pitch + length），依次调用
 *   Song(x, y) 就能"播完"一首曲子。
 *
 * 【设计思路（注意：这不是真实音乐，是"视觉节奏"）】
 *   - 不同音符 x 点亮不同数量的 LED：LED 亮得越多，图案越"大"，
 *     在视觉上形成音调高低起伏的效果；
 *   - 持续时间 y 决定这个音符"响"多久（拍子长短）；
 *   - 每个分支结尾都调用 GPIO_SetBits(GPIOA, GPIO_Pin_All) 把灯全灭，
 *     形成音符之间的间隔，就像音乐里的休止。
 *
 * 【两种控制方式混用】
 *   本文件同时用了库函数（GPIO_ResetBits/GPIO_SetBits）和
 *   直接写寄存器（GPIOA->ODR = ...），都是为了演示两种写法等效。
 *****************************************************************************/
#include "stm32f10x.h"       
#include "Delay.h"

/*****************************************************************************
 * 函数名：Song
 * 功能：  按指定"音符"点亮对应的 LED 组合，持续 y 毫秒后熄灭
 * 参数：  x —— 音符编号 1~10，决定点哪些 LED（见各分支注释）；
 *         y —— 持续时间（毫秒），决定这个音符的长短。
 * 返回值：无
 *
 * 【调用示例】
 *   Song(1, 300)：音符 1，持续 300ms。
 *   main.c 中把《铃儿响叮当》的音符数组逐个取出调用本函数。
 *
 * 【为什么最后还要再延时 100ms？】
 *   函数末尾统一 Delay_ms(100)，相当于给每个音符后加一个
 *   固定的"换气时间"，让音符与音符之间有短暂的间隔，更接近真实节奏。
 *****************************************************************************/
void Song(int x,int y )
{

	//音符 1：只点亮 LED0（PA0）—— 最少 LED = 最低的"音调"
	if(x==1)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);       //PA0 输出低电平 → LED0 亮
		Delay_ms(y);                            //保持这个图案 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //GPIOA 全部输出高电平 → 全灭（音符结束）
		
		
	}
	//音符 2：点亮 LED0、LED1（PA0、PA1）—— 比音符 1 多亮一个
	else if(x==2)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1); //PA0、PA1 输出低电平 → LED0、LED1 亮
		Delay_ms(y);                            //保持 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //全灭
		
	}
	//音符 3：点亮 PA5~PA7（3 个 LED）
	else if(x==3)
	{
		GPIOA->ODR = 0x000000f8;                //0xF8 = 1111 1000：PA0~PA2 灭（1），
		                                        //PA3~PA7 亮（0）→ 实际点亮 PA3~PA7 共 5 个
		Delay_ms(y);                            //保持 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //全灭
		
	}
	//音符 4：点亮 PA4~PA7（4 个 LED）
	else if(x==4)
	{
		GPIOA->ODR = 0x000000f0;                //0xF0 = 1111 0000：PA0~PA3 灭，PA4~PA7 亮
		Delay_ms(y);                            //保持 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //全灭
		
	}
	//音符 5：点亮 PA5~PA7（3 个 LED）
	else if(x==5)
	{
		GPIOA->ODR = 0x000000e0;                //0xE0 = 1110 0000：PA0~PA4 灭，PA5~PA7 亮
		Delay_ms(y);                            //保持 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //全灭
		
	}
	//音符 6：点亮 PA6、PA7（2 个 LED）
	else if(x==6)
	{
		GPIOA->ODR = 0x000000c0;                //0xC0 = 1100 0000：PA0~PA5 灭，PA6、PA7 亮
		Delay_ms(y);                            //保持 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //全灭
		
	}
	//音符 7：只点亮 PA7（1 个 LED）
	else if(x==7)
	{
		GPIOA->ODR = 0x00000080;                //0x80 = 1000 0000：只有 PA7 亮
		Delay_ms(y);                            //保持 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //全灭
		
	}
	//音符 8：PA0~PA7 全亮（最"高"的图案）
	else if(x==8)
	{
		GPIOA->ODR = 0x00000000;                //全 0 → PA0~PA7 全部输出低电平 → 全亮
		Delay_ms(y);                            //保持 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //全灭
		
	}
	//音符 9：PA0~PA7 全亮 + LED8（PB0）—— 用到 B 端口了
	else if(x==9)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_All);     //PA 全部输出低电平 → PA0~PA7 全亮
		GPIO_ResetBits(GPIOB,GPIO_Pin_0);       //PB0 输出低电平 → LED8 也亮
		Delay_ms(y);                            //保持 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //PA 全灭
		GPIO_SetBits(GPIOB,GPIO_Pin_0);         //LED8 熄灭
		
	}
	//音符 10：全部 10 个 LED 全亮（最饱满的图案）
	else if(x==10)
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_All);     //PA 全部输出低电平 → PA0~PA7 全亮
		GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1); //PB0、PB1 输出低电平 → LED8、LED9 亮
		Delay_ms(y);                            //保持 y 毫秒
		GPIO_SetBits(GPIOA,GPIO_Pin_All);       //PA 全灭
		GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1); //LED8、LED9 熄灭
		
	}
	Delay_ms(100);                              //统一加 100ms"换气时间"，音符间留出间隔
	
	
}
