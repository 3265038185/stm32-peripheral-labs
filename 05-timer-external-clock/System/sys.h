/*===============================================================================
 * 文件名称：sys.h（系统级头文件：位带操作宏定义）
 * 模块功能：利用 Cortex-M3 内核的"位带操作（Bit-Band）"特性，
 *           实现"像操作变量一样操作 GPIO 的某一位"：
 *           例：PBout(0) = 0  → 把 PB0 输出寄存器写成 0（引脚输出低电平）
 *               if(PBin(12))  → 读 PB12 输入电平（1 高 / 0 低）
 * 位带操作原理（重点理解）：
 *   1. STM32 的存储空间中有一块"位带区"（GPIO 外设属于其中一部分），
 *      位带区的每一个"位"都对应位带别名区里的一个"32 位地址"。
 *   2. 对别名区地址做"读/写"，等价于对原寄存器的那个"位"做读/写，
 *      并且是原子操作（不会被中断打断），比"读-改-写"整个寄存器更安全高效。
 *   3. BITBAND(addr, bitnum) 宏就是"地址换算"公式：
 *      别名地址 = 位带区基址(0x22000000) + (位所在寄存器地址-0x20000000)*32 + 位号*4
 *   4. MEM_ADDR(addr) 把地址强转成"volatile 指针"再解引用，
 *      volatile 防止编译器把读操作优化掉（因为寄存器值随时会变）
 *   5. 于是 BIT_ADDR(寄存器地址, 位号) 就等价于"该寄存器的某一位"这个变量
 * 每个 GPIO 端口有两类寄存器：
 *   - ODR（输出数据寄存器，基址+12）：写它控制引脚输出电平 → PAout/PBout...
 *   - IDR（输入数据寄存器，基址+8）：读它获取引脚输入电平 → PAin/PBin...
 * 用法示例：
 *   LED0 = 0;      // 等价于把 GPIOA 第 0 位输出寄存器写 0
 *   if(Key1==0)    // 等价于读 GPIOB 第 12 位输入电平
 *===============================================================================
 */
#ifndef __SYS_H
#define __SYS_H	  
#include <stm32f10x.h>   // 引入 STM32 寄存器地址定义（GPIOA_BASE 等）

/* 位带地址换算：addr 是寄存器地址，bitnum 是位号（0~15） */
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
/* 把地址强转成 volatile 指针并解引用，得到一个"可以读写的变量" */
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
/* 组合宏：BIT_ADDR(寄存器地址,位号) = 该寄存器某一位对应的"位带变量" */
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

/* ODR（输出数据寄存器）地址：GPIO 基地址 + 12 字节偏移 */
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

/* IDR（输入数据寄存器）地址：GPIO 基地址 + 8 字节偏移 */
#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 

/* 端口位操作宏：PAout(n) = 写 PA 端口第 n 位（输出）；PAin(n) = 读 PA 端口第 n 位（输入） */
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)   

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











