#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#include "main.h"

/* I2C接口定义 */
#define I2C_INSTANCE              I2C1
#define I2C_CLK_ENABLE()          __HAL_RCC_I2C_CLK_ENABLE()
#define I2C_CLK_DISABLE()         __HAL_RCC_I2C_CLK_DISABLE()

/* I2C引脚定义 */
#define I2C_SCL_GPIO_PORT         GPIOA
#define I2C_SCL_GPIO_PIN          GPIO_PIN_3
#define I2C_SCL_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define I2C_SCL_GPIO_AF           GPIO_AF12_I2C

#define I2C_SDA_GPIO_PORT         GPIOA
#define I2C_SDA_GPIO_PIN          GPIO_PIN_7
#define I2C_SDA_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define I2C_SDA_GPIO_AF           GPIO_AF12_I2C

/* I2C参数定义 */
#define I2C_CLOCK_SPEED           100000    /* 100KHz速率 */
#define I2C_DUTY_CYCLE            I2C_DUTYCYCLE_2  /* 标准模式占空比 */
#define I2C_OWN_ADDRESS           0x00      /* 主机模式下自身地址 */
#define I2C_GENERAL_CALL          I2C_GENERALCALL_DISABLE
#define I2C_NO_STRETCH            I2C_NOSTRETCH_DISABLE

/* 函数声明 */
HAL_StatusTypeDef BSP_I2C_Init(void);
HAL_StatusTypeDef BSP_I2C_DeInit(void);
HAL_StatusTypeDef BSP_I2C_WriteReg(uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef BSP_I2C_ReadReg(uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef BSP_I2C_IsReady(uint16_t DevAddr, uint32_t Trials);

extern I2C_HandleTypeDef hi2c1;

#endif /* __BSP_I2C_H__ */
