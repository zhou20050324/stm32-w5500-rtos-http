#ifndef __ETH_H__
#define __ETH_H__

#include "w5500.h"
#include <stdio.h>
#include "Delay.h"


/* 网络配置参数（可在 main.c 中修改） */
extern uint8_t eth_ip[4];
extern uint8_t eth_mac[6];
extern uint8_t eth_submask[4];
extern uint8_t eth_gateway[4];

void ETH_Init(void);

#endif
