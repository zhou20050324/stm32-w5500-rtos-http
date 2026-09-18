#include "timer3.h"

void Timer3_Init(uint16_t freq)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    uint16_t period, prescaler;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // 计算分频和周期，使 TRGO 频率 = freq
    prescaler = 7200 - 1;      // 72M / 7200 = 10 kHz
    period = 10000 / freq - 1; // 例如 freq=1 -> period=9999

    TIM_TimeBaseStructure.TIM_Period = period;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // 选择更新事件作为触发输出 (TRGO)
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);

    TIM_Cmd(TIM3, DISABLE); // 初始不启动
}

void Timer3_Start(void)
{
    TIM_Cmd(TIM3, ENABLE);
}

void Timer3_Stop(void)
{
    TIM_Cmd(TIM3, DISABLE);
}