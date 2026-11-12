#ifndef __KEY_H__
#define __KEY_H__
#include "sys.h"
#define Key1 PBin(12)
#define Key2 PBin(13)
#define Key3 PBin(14)
#define Key4 PBin(15)

void Key_Init(void);

unsigned char Key_GetNum(void);
unsigned char Key_GetState(void);
unsigned char Key_One(void);
unsigned char Key_Oneneed(void);



#endif
