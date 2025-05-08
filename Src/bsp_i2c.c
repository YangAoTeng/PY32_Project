/**
  ******************************************************************************
  * @file    bsp_i2c.c
  * @author  MCU Application Team
  * @brief   硬件I2C驱动实现
  ******************************************************************************
  */

#include "bsp_i2c.h"

/* I2C句柄 */
I2C_HandleTypeDef hi2c1;

/**
  * @brief  初始化I2C外设
  * @param  None
  * @retval HAL状态
  */
HAL_StatusTypeDef BSP_I2C_Init(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* 使能GPIO和I2C时钟 */
  I2C_SCL_GPIO_CLK_ENABLE();
  I2C_SDA_GPIO_CLK_ENABLE();
  I2C_CLK_ENABLE();
  
  /* 配置I2C引脚 */
  GPIO_InitStruct.Pin = I2C_SCL_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;          /* 复用开漏输出 */
  GPIO_InitStruct.Pull = GPIO_PULLUP;              /* 上拉 */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = I2C_SCL_GPIO_AF;     /* 复用为I2C功能 */
  HAL_GPIO_Init(I2C_SCL_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = I2C_SDA_GPIO_PIN;
  GPIO_InitStruct.Alternate = I2C_SDA_GPIO_AF;
  HAL_GPIO_Init(I2C_SDA_GPIO_PORT, &GPIO_InitStruct);
  
  /* 配置I2C参数 - 使用旧版I2C初始化结构体 */
  hi2c1.Instance = I2C_INSTANCE;
  hi2c1.Init.ClockSpeed = I2C_CLOCK_SPEED;         /* 时钟速度，100kHz */
  hi2c1.Init.DutyCycle = I2C_DUTY_CYCLE;           /* 占空比 */
  hi2c1.Init.OwnAddress1 = I2C_OWN_ADDRESS;        /* 自身地址 */
  hi2c1.Init.GeneralCallMode = I2C_GENERAL_CALL;   /* 通用呼叫模式 */
  hi2c1.Init.NoStretchMode = I2C_NO_STRETCH;       /* 时钟拉伸模式 */
  
  /* 初始化I2C */
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    status = HAL_ERROR;
  }
  
  return status;
}

/**
  * @brief  反初始化I2C外设
  * @param  None
  * @retval HAL状态
  */
HAL_StatusTypeDef BSP_I2C_DeInit(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  /* 反初始化I2C外设 */
  status = HAL_I2C_DeInit(&hi2c1);
  
  /* 反初始化GPIO */
  HAL_GPIO_DeInit(I2C_SCL_GPIO_PORT, I2C_SCL_GPIO_PIN);
  HAL_GPIO_DeInit(I2C_SDA_GPIO_PORT, I2C_SDA_GPIO_PIN);
  
  /* 禁用I2C时钟 */
  I2C_CLK_DISABLE();
  
  return status;
}

/**
  * @brief  向I2C设备写入数据
  * @param  DevAddr: 设备地址(7位地址)
  * @param  MemAddr: 内存地址(寄存器地址)
  * @param  pData: 数据指针
  * @param  Size: 数据大小
  * @retval HAL状态
  */
HAL_StatusTypeDef BSP_I2C_WriteReg(uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size)
{
  return HAL_I2C_Mem_Write(&hi2c1, DevAddr, MemAddr, I2C_MEMADD_SIZE_8BIT, pData, Size, 1000);
}

/**
  * @brief  从I2C设备读取数据
  * @param  DevAddr: 设备地址(7位地址)
  * @param  MemAddr: 内存地址(寄存器地址)
  * @param  pData: 数据指针
  * @param  Size: 数据大小
  * @retval HAL状态
  */
HAL_StatusTypeDef BSP_I2C_ReadReg(uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size)
{
  return HAL_I2C_Mem_Read(&hi2c1, DevAddr, MemAddr, I2C_MEMADD_SIZE_8BIT, pData, Size, 1000);
}

/**
  * @brief  检查I2C设备是否就绪
  * @param  DevAddr: 设备地址(7位地址)
  * @param  Trials: 尝试次数
  * @retval HAL状态
  */
HAL_StatusTypeDef BSP_I2C_IsReady(uint16_t DevAddr, uint32_t Trials)
{
  return HAL_I2C_IsDeviceReady(&hi2c1, DevAddr, Trials, 1000);
}
