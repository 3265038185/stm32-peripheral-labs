#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define LED0 PAout(9)
#define LED1 PAout(10)
#define LED2 PAout(11)
#define LED3 PAout(8)



void LED_Init(void);
void LED_Run(void);
		 				    
#endif

















