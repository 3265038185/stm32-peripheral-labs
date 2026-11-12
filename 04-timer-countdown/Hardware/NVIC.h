#ifndef __NVIC_H__
#define __NVIC_H__

/* NVIC 中断优先级模块头文件。
 * 注意：本工程 main.c 调用了 NVICKey_Init()（给 EXTI 按键中断
 * 配置 NVIC 优先级），但目前 NVIC.c 中只有 NVICtim1_Init()，
 * NVICKey_Init() 的实现缺失会导致链接报错 —— 建议参考
 * NVICtim1_Init 的写法，在 NVIC.c 中补充 NVICKey_Init 的实现。 */
void NVICKey_Init(void);

#endif
