#ifndef __DELAY_H
#define __DELAY_H

#include <stdint.h>

void Delay_Init(void); // 可选的显式初始化（留空亦可，首次调用自动初始化）
void Delay_us(uint32_t nus);
void Delay_ms(uint32_t nms);
void Delay_s(uint32_t ns);

#endif