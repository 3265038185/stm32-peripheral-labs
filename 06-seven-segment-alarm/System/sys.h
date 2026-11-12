/* ============================================================================
 * 文件说明（sys.h —— 位带操作宏定义头文件）
 * ----------------------------------------------------------------------------
 * 本文件实现 Cortex-M3 内核的"位带（Bit-Band）"操作：
 *   普通方式操作某一位：读 ODR 整个寄存器 → 改某一位 → 再写回去
 *   （麻烦且不安全：读改写之间若被中断打断，其他位可能被误改）
 *   位带方式：每一个"位"都映射到一个独立的 32 位地址上，
 *   直接往那个地址写 0/1，就只影响那一位，安全又高效！
 *
 * 用法举例（在 led.h 中）：
 *   #define LED0 PAout(9)   → 之后写 "LED0 = 1/0" 就是单独控制 PA9 输出
 *   PAout(n) 控制输出寄存器 ODR 的位 n；PAin(n) 读取输入寄存器 IDR 的位 n
 * ========================================================================== */
#ifndef __SYS_H
#define __SYS_H	  
#include <stm32f10x.h>   

/* ---- 位带运算的三个基础宏（由低级到高级依次调用） ---- */
// BITBAND：算出"地址 addr 的第 bitnum 位"对应的位带别名地址
// 公式说明：位带区起始 0x40000000（外设区）→ 别名区 0x42000000，偏移放大了 32 倍，
//           加上每个位占 4 个字节（bitnum<<2）
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
// MEM_ADDR：把地址转成"可读写的内存指针"，volatile 防止编译器优化掉访问
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
// BIT_ADDR：组合上面两个宏 → 直接访问某个位的"别名地址"
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

/* ---- 各 GPIO 端口的寄存器基地址 ---- */
// ODR（Output Data Register，输出数据寄存器）：引脚电平输出，地址=端口基地址+12
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

// IDR（Input Data Register，输入数据寄存器）：读引脚电平，地址=端口基地址+8
#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 

/* ---- 用户使用的高级宏 ---- */
// PAout(n)：给 PA 端口的第 n 号引脚"输出"电平（写 0 或 1）
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  
// PAin(n)：读取 PA 端口第 n 号引脚的"输入"电平（返回 0 或 1）
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)   

// 以下宏依此类推：PBout/PBin 是 B 口，PCout/PCin 是 C 口……
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)   

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)   
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)   

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)   
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)   
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)   
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  
/////////////////////////////////////////////////////////////////


#endif











