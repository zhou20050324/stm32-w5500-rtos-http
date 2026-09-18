#ifndef __UART1_H__
#define __UART1_H__ 

#include "stm32f10x.h"
#include "stdio.h"

void UART1_Init(uint32_t baudrate);
void UART1_SendByte(uint8_t byte);

extern volatile uint8_t UART1_ReceivedByte;
extern volatile uint8_t UART1_ReceivedFlag;
#endif