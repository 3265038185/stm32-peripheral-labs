/*==============================================================================
 * 文件：sys.h（系统头文件——"位带操作"宏定义大全）
 * 功能：利用Cortex-M3内核的"位带(Bit-Band)区"特性，把"某个寄存器的某一位"
 *       映射到SRAM中的一个普通地址，让我们可以直接用 "宏名(位号)" 来读写
 *       任意一个GPIO引脚，非常方便。
 * 原理：STM32的SRAM区（0x20000000~0x200FFFFF）和片内外设区
 *       （0x40000000~0x400FFFFF）各有一块"位带别名区"，
 *       位带区1个位 对应 别名区4个字节（1个地址）。
 *       公式：别名地址 = 位带区基地址+0x2000000 + (位所在字节偏移<<5) + (位号<<2)
 *       宏BITBAND(addr, bitnum) 就是在算这个"别名地址"，
 *       MEM_ADDR(addr) 把这个地址当成一个volatile指针去读写，
 *       于是 BIT_ADDR(addr, bitnum) 就实现了"读/写addr寄存器的第bitnum位"。
 * 用法示例：PBout(12)=1  → 让PB12输出高电平；
 *           PBin(12)     → 读取PB12引脚电平。
 * 注意：使用这些宏之前，必须先把对应GPIO的时钟和模式初始化好！
 * 知识点：位带操作、volatile关键字、寄存器地址映射
 *============================================================================*/
#ifndef __SYS_H
#define __SYS_H	  
#include <stm32f10x.h>   /* STM32核心头文件：GPIOA_BASE等基地址都从这里来 */

/* 位带公式宏：addr=寄存器地址，bitnum=第几位。
   算法分解：
     (addr & 0xF0000000)     → 取最高4位（区分SRAM区还是外设区，基地址不一样）
     + 0x2000000             → 加上位带别名区的偏移量
     + ((addr & 0xFFFFF)<<5) → 该位所在字节在区内的偏移×32（每字节对应32个地址）
     + (bitnum<<2)           → 位号×4（每一位对应4个地址） */
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
/* 把地址强制转换成"volatile无符号长整型指针"再解引用，
   volatile告诉编译器：这个地址的值随时可能变化（被硬件改），
   每次使用都必须重新去内存读，不能被编译器优化缓存 */
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
/* 组合宏：BIT_ADDR(addr, bitnum) = 读写addr寄存器的第bitnum位 */
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

/* 各GPIO端口"输出数据寄存器"（ODR）的地址：
   基地址+12 就是ODR寄存器（GPIO_BASE + 0x0C）。
   如 GPIOA_BASE=0x40010800，则 GPIOA_ODR 的地址是 0x4001080C */
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

/* 各GPIO端口"输入数据寄存器"（IDR）的地址：基地址+8 就是IDR寄存器 */
#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 

/* 输出宏：PAout(n) = 写PA端口ODR寄存器的第n位 = 控制PA口第n引脚输出电平。
   比如 PAout(0)=1 让PA0输出高电平，PAout(0)=0 让PA0输出低电平 */
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  
/* 输入宏：PAin(n) = 读PA端口IDR寄存器的第n位 = 读取PA口第n引脚的电平 */
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)   

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  /* 控制PB口输出（同上） */
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  /* 读取PB口输入（同上） */

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  /* 控制PC口输出 */
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  /* 读取PC口输入 */

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  /* 控制PD口输出 */
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  /* 读取PD口输入 */

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  /* 控制PE口输出 */
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  /* 读取PE口输入 */

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  /* 控制PF口输出 */
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  /* 读取PF口输入 */

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  /* 控制PG口输出 */
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  /* 读取PG口输入 */
/////////////////////////////////////////////////////////////////

/* 使用提示：
   1. 必须先开GPIO时钟并初始化好引脚模式，宏才能正常工作；
   2. 这些宏常用于按键读取(PBin)、LED控制(PAout)等场景，
      比标准库函数GPIO_ReadInputDataBit更简洁，但可读性稍差。 */

#endif	/* 头文件保护结束 */










