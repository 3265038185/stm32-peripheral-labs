/*****************************************************************************
 * 文件名：Function.c（System 系统层）
 * 所属工程：1-LED闪烁
 *
 * 【文件功能】
 *   用"标准库函数"的方式驱动 10 个 LED（PA0~PA7 + PB0~PB1），三种玩法：
 *     Function(1)：全亮 500ms → 全灭 500ms，循环；
 *     Function(2)：流水灯——PB1、PB0、PA7……PA0 依次单独点亮 500ms；
 *     Function(3)：后 2 个(PB0~PB1)+PA7~PA5 一组，与 PA4~PA0 一组交替亮灭。
 *
 * 【与另外两种写法的对比】
 *   - 本文件：调用标准外设库函数 GPIO_ResetBits/GPIO_SetBits/GPIO_Write，
 *     由库函数内部去操作寄存器，代码易读、不易错；
 *   - Register.c：直接写 GPIOx->ODR 寄存器；
 *   - Bit.c：用位带操作宏（LED0~LED9）。
 *   三份代码效果类似，方便初学者对比"库函数 vs 寄存器 vs 位带"。
 *
 * 【常用库函数说明】
 *   GPIO_ResetBits(GPIOx, Pin)：把指定引脚清零 → 输出低电平 → LED 亮；
 *   GPIO_SetBits(GPIOx, Pin)  ：把指定引脚置一 → 输出高电平 → LED 灭；
 *   GPIO_Write(GPIOx, 值)     ：把整个端口的 ODR 寄存器一次写入一个 16 位值。
 *****************************************************************************/
#include "stm32f10x.h"     
#include "DELAY.h"
#include "led.h"
/*****************************************************************************
 * 函数名：Function
 * 功能：  用库函数控制 10 个 LED 演示三种玩法
 * 参数：  x = 1：全部 LED 一起闪烁；x = 2：流水灯；x = 3：两组交替亮灭
 * 返回值：无
 *
 * 【原理说明】
 *   - 引脚已在 LED_Init() 里配置为推挽输出，这里只管"写电平"；
 *   - LED 为低电平点亮，所以"清零 = 亮，置一 = 灭"；
 *   - GPIO_Pin_All 是 0xFFFF，代表端口的全部 16 个引脚；
 *   - GPIO_Pin_0|GPIO_Pin_1 是"按位或"，表示同时选中多个引脚；
 *   - 每个分支都是 while(1) 死循环，调用后程序永远停在当前玩法。
 *****************************************************************************/
void Function(int x)
{	
	//======== 模式 1：全部 LED 一起闪烁 ========
	if(x==1)
	{
		
		
		
		//GPIO_ResetBits：清零指定的引脚位（输出低电平 = LED 亮）
	//GPIO_Pin_All = 0xFFFF：选中 GPIOA 的全部 16 个引脚（本工程只用了 PA0~PA7）
		while(1)                        //死循环：全亮→全灭→全亮……无限重复
		{
			
			GPIO_ResetBits(GPIOA,GPIO_Pin_All);      //GPIOA 全部引脚输出低电平 → PA0~PA7 全亮
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1); //PB0、PB1 输出低电平 → LED8、LED9 亮
			Delay_ms(500);                            //保持全亮 500ms
			GPIO_SetBits(GPIOA,GPIO_Pin_All);         //GPIOA 全部引脚输出高电平 → PA0~PA7 全灭
			GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1);//PB0、PB1 输出高电平 → LED8、LED9 灭
			Delay_ms(500);                            //保持全灭 500ms，回到循环开头
		}
	}
	//======== 模式 2：流水灯（从 PB1 开始，往 PA0 方向逐个亮） ========
	//原理：点亮当前灯 → 延时 → 熄灭它 → 点亮下一个 → 延时……
	//顺序是 PB1 → PB0 → PA7 → PA6 → … → PA0，光点依次移动。
	else if(x==2)
	{
		while(1)                        //死循环：流水过程反复播放
		{
			
			
			GPIO_ResetBits(GPIOB,GPIO_Pin_1);   //点亮 LED9（PB1 输出低电平）
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOB,GPIO_Pin_1);     //熄灭 LED9

			GPIO_ResetBits(GPIOB,GPIO_Pin_0);   //点亮 LED8（PB0 输出低电平）
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOB,GPIO_Pin_0);     //熄灭 LED8
		
			GPIO_ResetBits(GPIOA,GPIO_Pin_7);   //点亮 LED7（PA7 输出低电平）
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOA,GPIO_Pin_7);     //熄灭 LED7
		
			GPIO_ResetBits(GPIOA,GPIO_Pin_6);   //点亮 LED6
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOA,GPIO_Pin_6);     //熄灭 LED6

			GPIO_ResetBits(GPIOA,GPIO_Pin_5);   //点亮 LED5
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOA,GPIO_Pin_5);     //熄灭 LED5

			GPIO_ResetBits(GPIOA,GPIO_Pin_4);   //点亮 LED4
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOA,GPIO_Pin_4);     //熄灭 LED4

			GPIO_ResetBits(GPIOA,GPIO_Pin_3);   //点亮 LED3
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOA,GPIO_Pin_3);     //熄灭 LED3

			GPIO_ResetBits(GPIOA,GPIO_Pin_2);   //点亮 LED2
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOA,GPIO_Pin_2);     //熄灭 LED2

			GPIO_ResetBits(GPIOA,GPIO_Pin_1);   //点亮 LED1
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOA,GPIO_Pin_1);     //熄灭 LED1

			GPIO_ResetBits(GPIOA,GPIO_Pin_0);   //点亮 LED0
			Delay_ms(500);                      //亮 500ms
			GPIO_SetBits(GPIOA,GPIO_Pin_0);     //熄灭 LED0，回到循环开头从 PB1 再来
		}
	}
	//======== 模式 3：两组 LED 交替亮灭 ========
	//原理：第一组（PB0、PB1、PA7~PA5 共 5 个）亮 500ms，然后
	//      第二组（PA4~PA0 共 5 个）亮 500ms，两组交替出现。
	//这里用到了 GPIO_Write()：它可以直接把整个端口的 ODR 寄存器
	//一次写成任意 16 位值，比逐个引脚操作更高效。
		else if(x==3)
		{
			while(1)                        //死循环：交替过程反复播放
			{
			GPIO_ResetBits(GPIOB,GPIO_Pin_1);   //PB1 亮
			GPIO_ResetBits(GPIOB,GPIO_Pin_0);   //PB0 亮
			GPIO_Write(GPIOA,~0x00E0);          //把 GPIOA 的 ODR 写成 ~0x00E0：
			                                    //  0x00E0 = 二进制 0000 0000 1110 0000，
			                                    //  对应 PA5、PA6、PA7 三位置 1；
			                                    //  取反 ~ 后这三位置 0（亮），其余位置 1（灭）。
			                                    //  → 第一组 5 个 LED 全亮（PB0+PB1+PA5~PA7）

			Delay_ms(500);                      //第一组保持亮 500ms
		
			GPIO_SetBits(GPIOB,GPIO_Pin_1);     //PB1 灭
			GPIO_SetBits(GPIOB,GPIO_Pin_0);     //PB0 灭
			GPIO_Write(GPIOA,~0x0000);          //~0x0000 = 0xFFFF：全部位置 1 → PA0~PA7 全灭
		
			Delay_ms(500);                      //全灭 500ms

				
			GPIO_Write(GPIOA,~0x001F);          //0x001F = 二进制 0000 0000 0001 1111，
			                                    //对应 PA0~PA4 五位置 1；取反后这 5 位置 0（亮）
			                                    //→ 第二组 5 个 LED（PA0~PA4）全亮
			Delay_ms(500);                      //第二组保持亮 500ms
			GPIO_Write(GPIOA,~0x0000);          //再全灭
			Delay_ms(500);                      //全灭 500ms，回到循环开头

				
			
			}
			
		}
		
	
}
