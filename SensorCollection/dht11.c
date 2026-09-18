#include "dht11.h"
#include "Delay.h"

// 设置GPIO为输出
static void DHT11_Mode_Out(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

// 设置GPIO为输入
static void DHT11_Mode_In(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

// 读一个字节
static uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, val = 0;
    for (i = 0; i < 8; i++)
    {
        while (!GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN))
            ; // 等待高电平
        Delay_us(40);
        if (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN))
            val |= (1 << (7 - i));
        while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN))
            ; // 等待低电平结束
    }
    return val;
}

void DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(DHT11_RCC, ENABLE);
    DHT11_Mode_Out();
    GPIO_SetBits(DHT11_PORT, DHT11_PIN); // 空闲高电平
}

uint8_t DHT11_Read(float *temperature, float *humidity)
{
    uint8_t buf[5];
    uint8_t i;
    uint16_t timeout;

    // 1. 主机发送起始信号：拉低 >18ms，再拉高 20-40us
    DHT11_Mode_Out();
    GPIO_ResetBits(DHT11_PORT, DHT11_PIN);
    Delay_ms(20);
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);
    Delay_us(30);

    // 2. 切换输入，等待从机响应
    DHT11_Mode_In();

    // 2.1 等从机拉低总线（响应开始，约80us）
    timeout = 10000;
    while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN))
    {
        if (--timeout == 0)
            return 0;
    }
    // 2.2 等从机释放总线（变为高电平，约80us）
    timeout = 10000;
    while (!GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN))
    {
        if (--timeout == 0)
            return 0;
    }
    // 2.3 等从机再次拉低总线（开始发送数据）
    timeout = 10000;
    while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN))
    {
        if (--timeout == 0)
            return 0;
    }

    // 3. 接收 40 位数据
    for (i = 0; i < 5; i++)
    {
        buf[i] = DHT11_Read_Byte();
    }

    // 4. 校验
    if ((uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
    {
        *humidity = (float)buf[0] + (float)buf[1] * 0.1f;
        *temperature = (float)buf[2] + (float)buf[3] * 0.1f;
        return 1;
    }
    return 0;
}