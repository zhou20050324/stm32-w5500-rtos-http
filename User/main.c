#include "stm32f10x.h"
#include "Delay.h"
#include "UART1.h"
#include "eth.h"
#include "timer3.h"
#include "dht11.h"
#include "adc.h"
#include "APP_freeRTOS.h"
#include <stdio.h>

int main(void)
{ 
	Delay_Init();
	UART1_Init(115200);
	printf("System Start\r\n");

	// 初始化以太网（W5500复位、MAC、IP）
	ETH_Init();

	// 初始化传感器
	ADC_Sensor_Init();
	DHT11_Init();

	// 启动定时器触发ADC (1Hz)
	Timer3_Init(1);
	Timer3_Start();

	// 初始化FreeRTOS任务并启动调度器
	APP_FreeRTOS_Init();

	while (1)
		;
}