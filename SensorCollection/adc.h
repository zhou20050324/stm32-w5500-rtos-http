#ifndef __ADC_H__
#define __ADC_H__

#include "stm32f10x.h"

/* 通道索引宏，方便外部直接访问 adc_buffer */
#define ADC_CH_LIGHT 0
#define ADC_CH_TEMP_SENSOR 1
#define ADC_CH_VREFINT 2
#define ADC_CH_NUM 3

extern volatile uint16_t adc_buffer[ADC_CH_NUM];
extern volatile uint8_t adc_data_ready;

void ADC_Sensor_Init(void);

/* 直接返回物理量的函数 */
float Get_Internal_Temperature(void);  // 内部温度 (℃)
float Get_Light_Voltage(void);         // 光敏电压 (V)
float Get_ADC_Reference_Voltage(void); // 系统实际参考电压 (V)

#endif