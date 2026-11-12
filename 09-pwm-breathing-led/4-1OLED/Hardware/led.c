/* ============================================================================
 * 文件说明：led.c —— LED 驱动（初始化 10 个 LED：PA0~PA7 + PB0~PB1）
 * ----------------------------------------------------------------------------
 * 【硬件连接】
 *   LED 阳极接电源正极，阴极接 IO 口 → IO 输出低电平(0)点亮，输出高电平(1)熄灭。
 *   注意：本文件只初始化了 GPIOA 的 PA0~PA3（见 GPIO_Pin 参数），
 *   led.h 里定义的 LED4~LED9（PA4~PA7、PB0~PB1）对应的引脚需要时需自行补充初始化。
 *
 * 【用到的知识点】
 *   - GPIO 输出模式：推挽输出 GPIO_Mode_Out_PP（强驱动，能直接点亮 LED）
 *   - GPIO_SetBits / GPIO_ResetBits：置位/清零 ODR 寄存器的某一位
 *   - 初始化后把全部 LED 熄灭（默认灭，避免上电全亮）
 * ========================================================================== */

#include "led.h"           // 本头文件（含 LED0~LED9 宏定义）
#include "stm32f10x.h"                  // STM32F103 标准库头文件


void LED_Init(void)        // LED 引脚初始化
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	// 开启 GPIOA 的时钟（LED 引脚在 PA0~PA3 等，挂在 APB2 总线）
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	// 上面这行被注释了：如果要用 PB0/PB1 的 LED，需要把这一行取消注释
	// （当前初始化没有配置 GPIOB，所以 LED8/LED9 无法点亮！）
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	// 推挽输出模式：IO 既能输出高也能输出低，驱动能力强，
	// 适合直接驱动 LED（低电平点亮）。注意不是开漏输出。
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	// 初始化 PA0~PA3 四个引脚（LED0~LED3）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// 输出速度 50MHz：IO 翻转最快 50MHz，控制 LED 绰绰有余
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);   // 配置写入 GPIOA 的 CRL 寄存器，生效
	
	//GPIO_InitTypeDef GPIOB_InitStructure;
	//GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//GPIOB_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1;
	//GPIOB_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	//GPIO_Init(GPIOB,&GPIOB_InitStructure);
	/* 上面被注释的代码：GPIOB 版 LED 初始化（PB0/PB1），
	 * 需要点亮 LED8/LED9 时取消注释即可 */
	GPIO_SetBits(GPIOA,GPIO_Pin_All);
	// 把所有 PA 引脚置 1（输出高电平）→ 全部 LED 熄灭。
	// 注意：LED 是低电平点亮，所以初始状态要让引脚为高 = 灯灭。
	// GPIO_SetBits 就是把 ODR 寄存器的对应位写成 1
	//GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);
}






