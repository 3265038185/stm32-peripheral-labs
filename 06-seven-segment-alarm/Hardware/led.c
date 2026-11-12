/* ============================================================================
 * 文件说明（led.c —— LED 流水灯驱动文件）
 * ----------------------------------------------------------------------------
 * 硬件连接：4 个 LED 接在 PA8、PA9、PA10、PA11 上，低电平点亮
 *           （引脚输出 0 → LED 亮；输出 1 → LED 灭）
 * 引脚宏定义（在 led.h 中）：
 *   LED0 = PAout(9)，LED1 = PAout(10)，LED2 = PAout(11)，LED3 = PAout(8)
 *   PAout(n) 是位带操作宏：用一条语句单独控制某一个引脚输出高/低电平，
 *   不会影响同端口其他引脚（比读改写整个 ODR 寄存器更安全）
 * 对外提供的函数：
 *   LED_Init()   初始化引脚
 *   LED_Run()    流水灯效果：4 个 LED 从左到右依次点亮再依次熄灭
 * ========================================================================== */
#include "led.h"
#include "stm32f10x.h"  
#include "Delay.h"
unsigned int j;             // 全局变量 j（原代码中暂未使用，预留）

/* --------------------------------------------------------------------------
 * 函数功能：初始化 4 个 LED 引脚（PA8~PA11）
 * 参数：无　　返回值：无
 * 说明：
 *   GPIO_Mode_Out_PP 是"推挽输出"模式 —— 既可以输出高电平也可以输出
 *   低电平，且驱动电流比开漏模式大，适合直接驱动 LED
 *   GPIO_Speed_50MHz 是输出速度上限，LED 点亮要求不高，50MHz 足够
 *   初始化后把所有 LED 引脚置 1 → 熄灭（防止上电瞬间 LED 全亮）
 * ------------------------------------------------------------------------ */
void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   // 开启 GPIOA 的时钟（GPIO 必须先开时钟）
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); // （备用写法：开 GPIOB 时钟，已注释）
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // 推挽输出模式
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;   // 用到的引脚：PA8~PA11
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 输出速度 50MHz
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);   // 把配置写入 GPIOA 寄存器
	
	//GPIO_InitTypeDef GPIOB_InitStructure; // （备用写法：PA0、PA1 接 LED 的旧版本，
	//GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //  已注释，可忽略）
	//GPIOB_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1;
	//GPIOB_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	//GPIO_Init(GPIOB,&GPIOB_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);   // 全部置 1 → LED 全灭
	//GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);   // （备用写法，已注释）
}

/* --------------------------------------------------------------------------
 * 函数功能：流水灯效果 —— 4 个 LED 依次点亮、再依次熄灭（从左向右跑一遍）
 * 参数：无　　返回值：无
 * 实现原理：
 *   LEDx = 0 → 点亮该 LED（低电平点亮）；LEDx = 1 → 熄灭
 *   每个 LED 点亮后延时 Dms(100)（约 100ms），再灭掉点亮下一个，
 *   这样人眼就能看到"灯光从左往右流水一样跑"的效果
 * 注意：这里的延时用的是软件延时函数 Dms()（空循环），
 *       延时期间 CPU 什么事都不做，属于"阻塞式"写法
 * ------------------------------------------------------------------------ */
void LED_Run(void)
{
	
		LED0 = 0;          // 点亮第 1 个 LED（PA9 输出低电平）
		Dms(100);          // 停留约 100ms
		LED0 = 1;          // 熄灭第 1 个 LED
		LED1 = 0;          // 点亮第 2 个 LED（PA10）
		Dms(100);          // 停留约 100ms
		LED1 = 1;          // 熄灭第 2 个 LED
		LED2 = 0;          // 点亮第 3 个 LED（PA11）
		Dms(100);          // 停留约 100ms
		LED2 = 1;          // 熄灭第 3 个 LED
		LED3= 0;           // 点亮第 4 个 LED（PA8）
		Dms(100);          // 停留约 100ms
		LED3 = 1;          // 熄灭第 4 个 LED —— 一轮流水结束
	
	
}




