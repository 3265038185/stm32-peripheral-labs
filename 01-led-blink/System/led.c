/*****************************************************************************
 * 文件名：led.c（System 系统层）
 * 所属工程：1-LED闪烁
 *
 * 【文件功能】
 *   初始化 10 个 LED 所用的 GPIO 引脚：
 *     LED0~LED7  →  PA0~PA7（GPIOA 端口）；
 *     LED8、LED9 →  PB0、PB1（GPIOB 端口）。
 *
 * 【LED 硬件知识】
 *   - 本开发板 LED 采用"低电平点亮"接法：LED 阳极（+）接 3.3V 电源，
 *     阴极（-）经过限流电阻接到单片机的 IO 口；
 *   - IO 输出低电平时，LED 两端产生电压差，电流流过 LED → 点亮；
 *   - IO 输出高电平时，两端都是高电平，没有压差 → 熄灭。
 *   所以后面控制 LED 时："写 0 = 亮，写 1 = 灭"。
 *
 * 【为什么用"推挽输出"模式？】
 *   推挽输出（GPIO_Mode_Out_PP）：内部有两个互补的晶体管，
 *   一个负责拉高、一个负责拉低，输出高/低电平都很"有劲"（驱动能力强），
 *   能提供足够的电流点亮 LED。这是点亮 LED 最常用的输出模式。
 *
 * 【为什么速度选 50MHz？】
 *   GPIO_Speed 是"引脚翻转速度上限"，不是输出电平幅度。
 *   50MHz 是 STM32F103 最快的模式，应对延时闪烁绰绰有余，
 *   也便于以后驱动更快的信号（如 SPI、PWM）。
 *****************************************************************************/
#include "led.h"
#include "stm32f10x.h"                  


/*****************************************************************************
 * 函数名：LED_Init（LED 初始化，在 main 里最先调用）
 * 功能：  把 PA0~PA7、PB0、PB1 配置为推挽输出，并默认全部熄灭
 * 参数：  无
 * 返回值：无
 *
 * 【配置流程】
 *   1. 开时钟：外设必须先"通电"（RCC_APB2PeriphClockCmd）才能配置；
 *   2. 填结构体：把"模式、引脚、速度"写进 GPIO_InitTypeDef 结构体；
 *   3. 调 GPIO_Init：把结构体里的配置写入 GPIO 寄存器（生效）；
 *   4. 默认熄灭：GPIO_SetBits 输出高电平，保证上电瞬间 LED 不误亮。
 *
 * 【库函数配置的两个细节】
 *   - GPIO_Pin_All（=0xFFFF）：把 GPIOA 的 16 个引脚全选了，
 *     本工程只用 PA0~PA7，多配的 PA8~PA15 以后想用也不用再配；
 *   - GPIO_Pin_0|GPIO_Pin_1：用"按位或"一次选中 PB0、PB1 两个引脚。
 *****************************************************************************/
void LED_Init(void)
{
	//【第 1 步】开启 GPIOA、GPIOB 的时钟（挂在 APB2 总线上）：
	//STM32 的外设默认是"断电"状态，不开时钟时对寄存器的读写无效。
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	//【第 2、3 步】配置 GPIOA（PA0~PA7 接 LED0~LED7）：
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //模式：通用推挽输出（能拉高也能拉低，驱动 LED 有力）
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_All;        //引脚：选中全部 16 个（本工程实际用 PA0~PA7）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度：50MHz（最快档，点灯用绰绰有余）
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);             //把配置写入 GPIOA 的寄存器，正式生效
	
	//【第 2、3 步】配置 GPIOB（PB0、PB1 接 LED8、LED9）：
	GPIO_InitTypeDef GPIOB_InitStructure;
	GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      //模式：通用推挽输出
	GPIOB_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1;   //引脚：PB0 和 PB1（按位或同时选中两个）
	GPIOB_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //速度：50MHz
	
	GPIO_Init(GPIOB,&GPIOB_InitStructure);            //把配置写入 GPIOB 的寄存器，正式生效
	//【第 4 步】默认全部熄灭：输出高电平（低电平才亮，高电平 = 灭）
	GPIO_SetBits(GPIOA,GPIO_Pin_All);                 //PA0~PA7 全部输出高电平 → LED0~LED7 全灭
	GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);        //PB0、PB1 输出高电平 → LED8、LED9 灭
}






