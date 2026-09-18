# stm32-w5500-rtos-http
基于STM32F103 + W5500 + FreeRTOS 的物联网多传感器采集与远程监控系统

## 项目简介
STM32通过FreeRTOS多任务框架完成三路ADC与DHT11的同步采集，经W5500以太网上传至PC端Flask服务器，实现数据持久化存储与Web实时可视化。



## 核心贡献
1 . FreeRTOS 多任务架构与同步设计
划分ADC 采集、DHT1 1 采集、数据处理、网络上传四个独立任务，使用事件组（EventGroup）实现双源数据
的同步触发，使用任务通知（TaskNotify）实现处理任务对上传任务的异步唤醒，替代裸机轮询模式，任务间
耦合度低、实时性高。

2. 关键时序保护与并发安全
针对DHT1 1  单总线微秒级时序在RTOS 下易被抢占的问题，使用taskENTER_CRITICAL() 在读取期间临时
关闭中断与任务调度，确保通信成功率；引入互斥量（Mutex）解决多任务并发打印串口时的数据乱码问题，
保证日志完整性。

3. W5500 以太网通信与数据上传
基于W5500 硬件TCP/IP 协议栈与Socket API，实现每秒HTTP POST 上传JSON 数据至PC 端Flask；对
 Flask 未运行等网络异常采用“仅首次告警”策略，避免串口日志刷屏，兼顾调试与可读性。

4. 资源优化与SPI总线安全
针对STM32F103C8T6 仅20KB RAM 的资源限制，将Web 服务器从单片机剥离迁移至PC 端，最终Flash 占
用17KB、RAM 占用13.5KB，释放约12KB Flash 和2KB RAM。重写W5500 临界区回调函数对接
 FreeRTOS 中断保护机制，从底层解决SPI总线并发访问的安全问题。

5. 上位机可视化与数据持久化
Python Flask 接收数据后自动添加时间戳并存入CSV 文件；前端集成ECharts 动态图表、实时数据卡片和时
钟，支持局域网内任意设备浏览器访问，实现远程监控与历史数据追溯。

## 硬件清单
- 主控芯片：STM32F103C8T6
- 以太网模块：W5500
- 传感器：DHT11、ADC模拟采集

## 软件环境
- 嵌入式端：Keil MDK5，FreeRTOS
- 上位机：Python + Flask（Web服务，数据可视化）

## 使用说明
1. 使用Keil打开`Project.uvprojx`编译下载固件
2. 运行Python Flask服务
3. 设备上电，接入局域网，浏览器访问Web页面查看传感器实时数据
