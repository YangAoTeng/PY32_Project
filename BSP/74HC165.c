// filepath: c:\Users\q8979\Desktop\PY32F0xx_Firmware-master\Projects\PY32F030-STK\Example\GPIO\PY32_Project\BSP\74HC165.c

#include "74HC165.h"

/**
  * @brief  初始化74HC165相关的IO口
  * @param  无
  * @retval 无
  */
void HC165_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* 使能GPIOA时钟 */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  /* 配置并行加载和时钟引脚为输出模式 */
  GPIO_InitStruct.Pin = PL_PIN | CP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(HC165_GPIO_PORT, &GPIO_InitStruct);
  
  /* 配置串行输出引脚为输入模式 */
  GPIO_InitStruct.Pin = Q7_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(HC165_GPIO_PORT, &GPIO_InitStruct);
  
  /* 初始化控制引脚 */
  HAL_GPIO_WritePin(HC165_GPIO_PORT, PL_PIN, GPIO_PIN_SET);  // 默认高电平，禁止并行加载
  HAL_GPIO_WritePin(HC165_GPIO_PORT, CP_PIN, GPIO_PIN_RESET); // 默认低电平
}

/**
  * @brief  从两个级联的74HC165读取16位数据
  * @param  无
  * @retval 读取的16位数据(高8位为第2片，低8位为第1片)
  */
uint16_t HC165_Read16Bits(void)
{
  uint16_t data = 0;
  uint8_t i;
  
  /* 1. 将PL引脚拉低，加载并行数据 */
  HAL_GPIO_WritePin(HC165_GPIO_PORT, PL_PIN, GPIO_PIN_RESET);
  HAL_Delay(1); // 延时确保数据稳定
  
  /* 2. 将PL引脚拉高，允许移位操作 */
  HAL_GPIO_WritePin(HC165_GPIO_PORT, PL_PIN, GPIO_PIN_SET);
  HAL_Delay(1); // 延时确保信号稳定
  
  /* 3. 读取16位数据(每个74HC165读取8位) */
  for (i = 0; i < 16; i++)
  {
    /* 先读取第二片74HC165的Q7输出，再读取第一片的 */
    data <<= 1; // 左移一位，为新数据腾出空间
    
    /* 读取Q7引脚状态并存入data的最低位 */
    if (HAL_GPIO_ReadPin(HC165_GPIO_PORT, Q7_PIN) == GPIO_PIN_SET)
    {
      data |= 0x01;
    }
    
    /* 产生时钟上升沿，移出下一位数据 */
    HAL_GPIO_WritePin(HC165_GPIO_PORT, CP_PIN, GPIO_PIN_SET);
    HAL_Delay(1); // 延时确保信号稳定
    HAL_GPIO_WritePin(HC165_GPIO_PORT, CP_PIN, GPIO_PIN_RESET);
    HAL_Delay(1); // 延时确保信号稳定
  }
  
  return data;
}