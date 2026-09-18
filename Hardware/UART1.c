#include "UART1.h"

volatile uint8_t UART1_ReceivedByte = 0;
volatile uint8_t UART1_ReceivedFlag = 0;


/**
  * @brief  USART1 初始化（含接收中断配置）
  * @param  baud: 波特率，如 115200
  */
void UART1_Init(uint32_t baud)
{
    
    /* 使能时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);


    /* PA9(TX) 复用推挽输出 */
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA10(RX) 浮空输入 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART 基本参数 */
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate            = baud;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    /* 使能接收中断 (RXNE) */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    /* NVIC 中断优先级配置（与你的 DMA 版类似） */
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel              = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority   = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd           = ENABLE;
    
    NVIC_Init(&NVIC_InitStructure);

    /* 使能 USART1 */
    USART_Cmd(USART1, ENABLE);
}

/**
  * @brief  轮询发送一个字节
  */
void UART1_SendByte(uint8_t byte)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, byte);
}



/* ========= 重定向 printf 的两个底层入口 ========= */
/**
  * @brief  ARM 半主机 / MicroLIB 标准输出底层
  */
int __io_putchar(int ch)
{
    UART1_SendByte((uint8_t)ch);
    return ch;
}



/**
  * @brief  标准 C 库 printf 最终字符输出
  */
int fputc(int ch, FILE *f)
{
    return __io_putchar(ch);
}


/**
  * @brief  USART1 接收中断服务函数
  */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        UART1_ReceivedByte = USART_ReceiveData(USART1);    // 读取数据会自动清除 RXNE 标志
        UART1_ReceivedFlag = 1;                            // 通知主循环有新数据
    }
}
