# stm32-w5500-rtos-http
设计并实现了一套物联网多传感器数据采集与远程监控系统。STM32 通过FreeRTOS 多任务架构完成三路  ADC 与DHT1 1  的同步采集，经W5500 以太网上传至PC 端Flask 服务器，实现数据持久化存储与Web 实时 可视化。
