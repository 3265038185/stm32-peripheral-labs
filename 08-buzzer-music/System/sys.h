/*============================================================================
  文件：sys.h（系统级头文件 - 位带操作宏定义）
  ----------------------------------------------------------------------------
  功能：提供"位带操作"（Bit-Band）宏，让单个引脚可以像普通变量一样
        直接读写：
          PBout(n) —— 写 PBn 引脚输出电平（0/1）
          PBin(n)  —— 读 PBn 引脚输入电平（返回 0/1）
        例：LED0 = 0;  相当于 GPIOA->ODR 的 bit0 写 0；
            if(Key1 == 0)  相当于读 GPIOB->IDR 的 bit12。
  ----------------------------------------------------------------------------
  位带操作原理（重点，理解后收获很大）：
    - 普通方式操作某一位："读-改-写"三步，还可能误动其它位；
    - 位带方式：Cortex-M3 把"外设/内存的每一位"都映射到
      "位带别名区的一个 32 位地址"上。想置 1 某位，就往这个地址
      写 1；想读某位，直接读这个地址。硬件自动完成，互不影响。
    - 映射公式：别名地址 = 位带区基地址 + 0x2000000
                           + (字节偏移 << 5) + (位号 << 2)
      其中 <<5 是"一个字节的 8 个位占 32 字节"（8*4=32），
      <<2 是"一位占 4 字节"（32 位地址）。
  ----------------------------------------------------------------------------
  宏展开示例（PBout(1)）：
    PBout(1) -> BIT_ADDR(GPIOB_ODR_Addr, 1)
             -> MEM_ADDR(BITBAND(GPIOB_ODR_Addr, 1))
             -> *(volatile unsigned long *)(...算出的地址...)
    结果就是"一个可读写的 32 位地址"，对它赋值 0/1 即控制 PB1 输出。
  ============================================================================*/
#ifndef __SYS_H
#define __SYS_H	  
#include <stm32f10x.h>   

/* 位带映射核心公式：
   BITBAND(addr, bitnum) 算出"addr 地址第 bitnum 位"在别名区的地址；
   MEM_ADDR(addr) 把地址解释成可读写的变量；
   BIT_ADDR 合起来：直接读/写某一位。  */
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

/* 各 GPIO 端口"输出数据寄存器 ODR"的地址（基地址 + 12）：
   ODR 里每一位对应一个引脚的输出电平。  */
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

/* 各 GPIO 端口"输入数据寄存器 IDR"的地址（基地址 + 8）：
   IDR 里每一位对应一个引脚当前的实际电平（只读）。  */
#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 

/* 输出宏：PAout(n) = 写 PA n 号引脚输出电平（0 或 1）
   输入宏：PAin(n)  = 读 PA n 号引脚输入电平（返回 0 或 1）
   其它端口同理。  */
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
/*  以下两行是"示例"（被注释）：说明用法
    #define LED0 PAout(0)    // 把 PA0 起名为 LED0，以后 LED0=1/0 即控制它
    #define KEY1 PBin(12)    // 把 PB12 起名为 KEY1，读它判断按键  */


#endif











