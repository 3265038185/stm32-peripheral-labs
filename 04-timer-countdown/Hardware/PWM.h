#ifndef __PWM_H__
#define __PWM_H__

/* PWM 模块头文件：声明 PWM 初始化和呼吸灯函数的原型。
 * 硬件：TIM4 通道3 → PB8。详见 PWM.c 中的详细注释。 */

void PWM_Init(void);
void PWM_LED(void);
#endif
