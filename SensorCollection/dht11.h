#ifndef __DHT11_H__
#define __DHT11_H__

#include "stm32f10x.h"

// 引脚定义，按实际连接修改
#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_Pin_0
#define DHT11_RCC RCC_APB2Periph_GPIOB

void DHT11_Init(void);
uint8_t DHT11_Read(float *temperature, float *humidity); // 返回1成功

#endif