#include "adc.h"
#include "Delay.h"

volatile uint16_t adc_buffer[ADC_CH_NUM] = {0}; // [0]=光敏, [1]=温度传感器, [2]=Vrefint
volatile uint8_t adc_data_ready = 0;

void ADC_Sensor_Init(void)
{
    // 时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // PA0 模拟输入（光敏）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    DMA_InitTypeDef DMA_InitStructure;

    // DMA1_Channel1 配置
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = ADC_CH_NUM; // 3个数据
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // 循环
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    // 使能 DMA 传输完成中断
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    ADC_InitTypeDef ADC_InitStructure;

    // ADC1 配置：扫描模式，外部触发
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;                           // 扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                    // 单次扫描，由触发启动
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO; // TIM3 TRGO
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = ADC_CH_NUM; // 3个通道

    ADC_Init(ADC1, &ADC_InitStructure);

    // 规则序列配置：先光敏(Ch0)，再温度(Ch16)，再Vrefint(Ch17)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 2, ADC_SampleTime_55Cycles5); // 温度传感器
    ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 3, ADC_SampleTime_55Cycles5); // Vrefint

    // 使能内部温度和参考电压
    ADC_TempSensorVrefintCmd(ENABLE);

    // DMA 使能
    ADC_DMACmd(ADC1, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE);

    // ADC 校准
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1))
        ;

    // 使能外部触发
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);
}

/* 实际 ADC 参考电压 (V) */
float Get_ADC_Reference_Voltage(void)
{
    return 1.20f * 4096.0f / (float)adc_buffer[ADC_CH_VREFINT];
}

/* 获取内部温度 (℃) */
float Get_Internal_Temperature(void)
{
    float vref = Get_ADC_Reference_Voltage(); // 当前实际参考电压
    uint16_t adc_ts = adc_buffer[ADC_CH_TEMP_SENSOR];
    // 温度传感器电压
    float v_sensor = (adc_ts * vref) / 4096.0f;
    // STM32内部温度计算公式：T = (V25 - v_sensor) / Avg_Slope + 25
    // 典型值 V25 = 1.43V，Avg_Slope = 4.3 mV/℃
    float temp = (1.43f - v_sensor) / 0.0043f + 25.0f;
    return temp;
}

/* 获取光敏传感器电压 (V) */
float Get_Light_Voltage(void)
{
    float vref = Get_ADC_Reference_Voltage();
    uint16_t adc_light = adc_buffer[ADC_CH_LIGHT];
    return (adc_light * vref) / 4096.0f;
}

/* DMA传输完成中断（一轮3个数据全部传输完） */
void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC1);
        adc_data_ready = 1; // 主循环读取标志
    }
}