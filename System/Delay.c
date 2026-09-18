#include "stm32f10x.h"

// 直接定义 DWT 和调试寄存器地址（无需任何 CMSIS 头文件）
#define DWT_BASE (0xE0001000UL)
#define DWT_CTRL (*(volatile uint32_t *)(DWT_BASE + 0x00))
#define DWT_CYCCNT (*(volatile uint32_t *)(DWT_BASE + 0x04))
#define DWT_CTRL_CYCCNTENA (1UL << 0)

#define DEMCR (*(volatile uint32_t *)(0xE000EDFC))
#define DEMCR_TRCENA (1UL << 24)

static uint8_t dwt_initialized = 0;

static void DWT_Init_If_Needed(void)
{
  if (!dwt_initialized)
  {
    DEMCR |= DEMCR_TRCENA;          // 使能 DWT 访问
    DWT_CYCCNT = 0;                 // 清零计数器
    DWT_CTRL |= DWT_CTRL_CYCCNTENA; // 启动 CYCCNT
    dwt_initialized = 1;
  }
}

void Delay_Init(void)
{
  DWT_Init_If_Needed();
}

void Delay_us(uint32_t nus)
{
  DWT_Init_If_Needed();
  uint32_t start = DWT_CYCCNT;
  uint32_t ticks = nus * (SystemCoreClock / 1000000);
  while ((DWT_CYCCNT - start) < ticks)
    ;
}

void Delay_ms(uint32_t nms)
{
  while (nms--)
  {
    Delay_us(1000);
  }
}

void Delay_s(uint32_t ns)
{
  while (ns--)
  {
    Delay_ms(1000);
  }
}