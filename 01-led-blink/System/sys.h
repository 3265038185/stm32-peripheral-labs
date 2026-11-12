/*****************************************************************************
 * 文件名：sys.h（System 系统层头文件）
 * 所属工程：1-LED闪烁
 *
 * 【文件功能】
 *   定义"位带操作"相关宏，实现"像操作普通变量一样操作某个 IO 引脚"。
 *   例如 PAout(0) 在 led.h 中被定义为 LED0，于是写：
 *       LED0 = 0;   →  PA0 输出低电平（LED 亮）
 *       LED0 = 1;   →  PA0 输出高电平（LED 灭）
 *   读写一个引脚就像读写一个 int 变量那样简单直观！
 *
 * 【位带操作原理（Bit-Band，STM32 的独门绝技）】
 *   普通方式改一个引脚，需要"读-改-写"ODR 寄存器整 32 位，
 *   麻烦且容易误改其他位；位带操作则把"某一个位"映射到
 *   一段独立的 32 位地址空间：
 *   - 芯片把 1MB 的外设区（0x40000000~0x400FFFFF）每个位
 *     都"放大"成 Alias（别名）区里的一个 32 位字（4 字节）；
 *   - 你只要往那个别名地址"写 0 或写 1"，硬件就会自动把
 *     原寄存器里对应的那一位清零/置一，别的位完全不受影响；
 *   - 别名地址 = 原地址每偏移 4 字节，映射到别名区偏移 32 字节。
 *
 * 【三个核心宏的含义】
 *   1. BITBAND(addr, bitnum) ：算出 addr 里第 bitnum 位对应的别名地址；
 *   2. MEM_ADDR(addr)        ：把地址转成"易失性指针"再取值
 *      （volatile 告诉编译器每次都要重新读内存，别优化掉）；
 *   3. BIT_ADDR(addr, bitnum)：合并前两者 = 直接读写那一位！
 *
 * 【各宏分工】
 *   GPIOx_ODR_Addr：x 端口"输出数据寄存器"的地址（ODR 偏移 12）；
 *   GPIOx_IDR_Addr：x 端口"输入数据寄存器"的地址（IDR 偏移 8）；
 *   PAout(n)/PAin(n)：PA 口第 n 引脚的"输出位/输入位"。
 *   led.h 里的 LED0~LED9 就是用 PBout/PAout 组合出来的。
 *****************************************************************************/
#ifndef __SYS_H
#define __SYS_H	  
#include <stm32f10x.h>   

/* ================= 位带操作三大宏 ================= */
//BITBAND：计算"位带别名地址"。
//公式拆解（以 addr=0x40010C0C, bitnum=0 为例，即 PB0 的 ODR 位）：
//   (addr & 0xF0000000)   → 保留最高 4 位：区分外设区(0x40000000)
//                           还是 SRAM 区(0x20000000)，别名区基地址按此偏移；
//   + 0x2000000           → 加上别名区基地址偏移（外设别名区从 0x42000000 开始）；
//   + ((addr & 0xFFFFF)<<5) → 原地址低 20 位 × 32：一个位在别名区占 4 字节(32位)，
//                           所以位号偏移是字节偏移的 32 倍；
//   + (bitnum<<2)         → 加上位号 × 4：第 n 位在别名区里偏移 n*4 字节。
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
//MEM_ADDR：把地址强转成"volatile 无符号长整型指针"并解引用，
//          即"直接读写这个地址里的值"。volatile 防止编译器优化掉重复读写。
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
//BIT_ADDR：一步到位——先算别名地址，再读写该地址 = 读写目标寄存器的那一位。
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

/* ================= 各 GPIO 端口寄存器地址宏 ================= */
//ODR = Output Data Register（输出数据寄存器），偏移地址 = 12。
//向 ODR 的某位写 0/1，对应引脚就输出低/高电平。
//注释里的数值是绝对地址：例如 GPIOB 基址 0x40010C00 + 12 = 0x40010C0C。
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

//IDR = Input Data Register（输入数据寄存器），偏移地址 = 8。
//读取 IDR 的某位 = 读取对应引脚当前是高电平(1)还是低电平(0)。
#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 

/* ================= 方便的引脚读写宏 ================= */
//PAout(n)：写 PA 口第 n 位（输出电平）。例：PAout(0)=0 让 PA0 输出低电平。
//PAin(n) ：读 PA 口第 n 位（输入电平）。例：if(PAin(1)==0) 判断 PA1 是否被按下。
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)   

//PBout(n)/PBin(n)：PB 口的输出/输入位，用法同上。
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  

//PCout(n)/PCin(n)：PC 口的输出/输入位（本工程未使用，预留备用）。
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)   

//PDout(n)/PDin(n)：PD 口的输出/输入位（本工程未使用，预留备用）。
#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)   
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)   

//PEout(n)/PEin(n)：PE 口的输出/输入位（本工程未使用，预留备用）。
#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)   
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  

//PFout(n)/PFin(n)：PF 口的输出/输入位（STM32F103 中容量芯片无此端口）。
#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)   
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  

//PGout(n)/PGin(n)：PG 口的输出/输入位（STM32F103 中容量芯片无此端口）。
#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)   
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  
///////////////////////////////////////////////////////////////
//【小结】本工程真正用到的只有：PAout(0~7) 和 PBout(0~1)，
//        它们就是 led.h 中 LED0~LED9 的真身。
///////////////////////////////////////////////////////////////


#endif











