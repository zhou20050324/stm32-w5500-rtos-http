#ifndef __TIMER3_H__
#define __TIMER3_H__

#include "stm32f10x.h"

void Timer3_Init(uint16_t freq); // freq 触发频率(Hz)
void Timer3_Start(void);
void Timer3_Stop(void);

#endif