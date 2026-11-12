/* ============================================================================
 * 文件说明：sys.h —— 系统头文件：位带操作宏定义（正点原子风格）
 * ----------------------------------------------------------------------------
 * 【位带操作是什么？】
 *   Cortex-M3 内核把一部分内存（SRAM 区 + 外设区）映射了一块"位带别名区"：
 *   位带区的每一个"位"，在别名区对应一个"32 位的地址"。
 *   于是读/写这个 32 位地址，就等于读/写原来那个位，而且操作是原子性的。
 *
 * 【有什么用？】
 *   用标准库读一个引脚要调用 GPIO_ReadInputDataBit()，比较啰嗦。
 *   用了位带宏之后，可以直接写：
 *     PBout(0) = 0;    // 相当于把 GPIOB 的 ODR 寄存器 bit0 清零（输出低电平）
 *     if(PBin(12)){...} // 读 GPIOB 的 IDR 寄存器 bit12 的电平
 *   写起来像操作普通变量一样，非常方便！
 *
 * 【宏公式解释】
 *   BITBAND(addr, bitnum)：
 *     位带区地址 addr 的第 bitnum 位 → 别名区地址：
 *     别名地址 = 位带区基地址(0x42000000) + (addr 在位带区内的偏移 × 32) + bitnum × 4
 *     公式里的 0x2000000 是"位带区基地址 - 位带别名区基地址"的差值换算，
 *     展开：(addr & 0xF0000000) + 0x2000000 算出别名区基址，
 *           ((addr & 0xFFFFF) << 5) 是偏移 × 32，(bitnum << 2) 是位号 × 4。
 *   MEM_ADDR(addr)  ：把 addr 强转成"可读写的 32 位指针"并解引用。
 *   BIT_ADDR(addr, bitnum)：组合上面两个宏，得到最终的可读写位。
 *
 * 【GPIO 各寄存器地址】
 *   GPIOx_ODR 是"输出数据寄存器"（引脚输出电平写这里），偏移 +12；
 *   GPIOx_IDR 是"输入数据寄存器"（读引脚电平读这里），偏移 +8。
 *   例如 GPIOA_BASE = 0x40010800，所以 GPIOA_ODR = 0x4001080C。
 * ========================================================================== */

#ifndef __SYS_H              // 头文件保护，防止重复包含
#define __SYS_H	  
#include <stm32f10x.h>       // STM32F103 标准库头文件（提供 GPIOA_BASE 等地址定义）

/* —— 位带操作核心宏（见文件头详细解释）—— */
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
// 位带地址换算：把"位带区某一位"转换成"别名区一个 32 位地址"
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
// 把地址强转成指针并解引用：读/写该地址即读/写该位（volatile 防止编译器优化跳过）
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
// 组合宏：BIT_ADDR(寄存器地址, 位号) = 直接操作那一位

/* —— GPIO 输出寄存器(ODR)地址：GPIO 基地址 + 12 —— */
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

/* —— GPIO 输入寄存器(IDR)地址：GPIO 基地址 + 8 —— */
#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 

/* —— 最终提供给用户使用的位操作宏 ——
 * 用法：
 *   PAout(0) = 1;   → PA0 输出高电平（写 ODR 第 0 位）
 *   PBin(12)        → 读 PB12 电平（读 IDR 第 12 位），按下返回 0 */
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)    // PA0~PA15 输出控制
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)    // PA0~PA15 电平读取

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)    // PB0~PB15 输出控制
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)    // PB0~PB15 电平读取

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)    // PC0~PC15 输出控制
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)    // PC0~PC15 电平读取

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)    // PD0~PD15 输出控制
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)    // PD0~PD15 电平读取

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)    // PE0~PE15 输出控制
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)    // PE0~PE15 电平读取

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)    // PF0~PF15 输出控制
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)    // PF0~PF15 电平读取

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)    // PG0~PG15 输出控制
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)    // PG0~PG15 电平读取
/////////////////////////////////////////////////////////////////

/* 说明：本工程用到的是 PAout/PBin（LED 和按键），
 * 其余宏是为其他工程/后续扩展预留的，用法完全一样 */

#endif











