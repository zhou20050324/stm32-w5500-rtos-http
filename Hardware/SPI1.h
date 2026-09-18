#ifndef __SPI1_H__
#define __SPI1_H__

#include "stm32f10x.h"


void SPI1_Init(void);
void SPI1_Start(void);
void SPI1_Stop(void);
uint8_t SPI1_SwapByte(uint8_t byte);


#endif