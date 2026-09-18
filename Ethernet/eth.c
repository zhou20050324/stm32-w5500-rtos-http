#include "eth.h"

uint8_t eth_ip[4] = {192, 168, 1, 100};
uint8_t eth_mac[6] = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x33};
uint8_t eth_submask[4] = {255, 255, 255, 0};
uint8_t eth_gateway[4] = {192, 168, 1, 1};


#define ETH_RST_PORT        GPIOA
#define ETH_RST_PIN         GPIO_Pin_1

/**
  * @brief  硬件复位 W5500（PA1 → RST）
  */
static void ETH_Reset(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = ETH_RST_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(ETH_RST_PORT, &GPIO_InitStructure);

    GPIO_ResetBits(ETH_RST_PORT, ETH_RST_PIN);
    Delay_us(800);
    GPIO_SetBits(ETH_RST_PORT, ETH_RST_PIN);

    printf("W5500 reset complete.\n");
}



/**
  * @brief  MAC 地址设置
  */
static void ETH_SetMac(void)
{
    printf("Setting MAC...\n");
    setSHAR(eth_mac);      // 库函数
    printf("MAC: %02X-%02X-%02X-%02X-%02X-%02X\n",
           eth_mac[0], eth_mac[1], eth_mac[2],
           eth_mac[3], eth_mac[4], eth_mac[5]);
}


/**
  * @brief  IP、子网掩码、网关设置
  */
static void ETH_SetIP(void)
{
    printf("Setting IP...\n");

    setSIPR(eth_ip);       // 本地 IP
    setSUBR(eth_submask);  // 子网掩码
    setGAR(eth_gateway);   // 网关

    printf("IP: %d.%d.%d.%d\n", eth_ip[0], eth_ip[1], eth_ip[2], eth_ip[3]);
}


void ETH_Init(void)
{
    SPI1_Init(); // 初始化 SPI1

    user_register_function(); // 注册用户回调

    ETH_Reset(); // 硬件复位

    ETH_SetMac(); // 设置 MAC 地址

    ETH_SetIP(); // 设置 IP、子网掩码、网关
}