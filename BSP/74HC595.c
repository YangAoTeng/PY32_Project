// filepath: c:\Users\q8979\Desktop\PY32F0xx_Firmware-master\Projects\PY32F030-STK\Example\GPIO\PY32_Project\BSP\74HC595.c

#include "74HC595.h"

/**
  * @brief  初始化74HC595相关的IO口
  * @param  无
  * @retval 无
  */
void HC595_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* 使能GPIOB时钟 */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  /* 配置74HC595控制引脚为输出模式 */
  GPIO_InitStruct.Pin = SER_IN_PIN | ST_CP_PIN | SH_CP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(HC595_GPIO_PORT, &GPIO_InitStruct);
  
  /* 初始化控制引脚为低电平 */
  HAL_GPIO_WritePin(HC595_GPIO_PORT, SER_IN_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(HC595_GPIO_PORT, ST_CP_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(HC595_GPIO_PORT, SH_CP_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  向三个级联的74HC595发送24位数据
  * @param  data: 要发送的24位数据(每个74HC595负责8位)
  * @retval 无
  */
void HC595_Send24Bits(uint32_t data)
{
  uint8_t i;
  
  /* 禁止输出 */
  HAL_GPIO_WritePin(HC595_GPIO_PORT, ST_CP_PIN, GPIO_PIN_RESET);
  
  /* 移位发送24位数据(从高位到低位) */
  for (i = 0; i < 24; i++)
  {
    /* 设置数据位 */
    if (data & 0x800000)
      HAL_GPIO_WritePin(HC595_GPIO_PORT, SER_IN_PIN, GPIO_PIN_SET);
    else
      HAL_GPIO_WritePin(HC595_GPIO_PORT, SER_IN_PIN, GPIO_PIN_RESET);
    
    //
    /* 产生移位时钟上升沿 */
    HAL_GPIO_WritePin(HC595_GPIO_PORT, SH_CP_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HC595_GPIO_PORT, SH_CP_PIN, GPIO_PIN_SET);
    
    /* 数据左移一位 */
    data <<= 1;
  }
  
  /* 产生存储时钟上升沿，将数据锁存到输出寄存器 */
  HAL_GPIO_WritePin(HC595_GPIO_PORT, ST_CP_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(HC595_GPIO_PORT, ST_CP_PIN, GPIO_PIN_SET);
}