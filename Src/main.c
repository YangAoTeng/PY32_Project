/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by Puya under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp_usart.h"
#include "soft_timer.h"
#include "ring_buffer.h"

/* Private define ------------------------------------------------------------*/
#define LED_GPIO_PIN                 LED3_PIN
#define LED_GPIO_PORT                LED3_GPIO_PORT
#define LED_GPIO_CLK_ENABLE()        LED3_GPIO_CLK_ENABLE()



/* Private variables ---------------------------------------------------------*/


/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_GpioConfig(void);
void LED_Toggle_Callback(void *param);

/**
  * @brief  Main program.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Systick */
  HAL_Init();                                  
  
  /* Initialize GPIO */
  APP_GpioConfig();
  /* Initialize UART1 */
  bsp_usart1_init();
  /* 初始化软件定时器 */
  SoftTimer_Init();
  

  /* 创建LED闪烁定时器(无限循环) */
  uint8_t timer1 = SoftTimer_Create(1000, 0, LED_Toggle_Callback, NULL);
  printf("LED toggle timer created, ID: %d\r\n", timer1);

  while (1)
  {
    SoftTimer_Execute();
  }
}

/**
  * @brief  LED闪烁回调函数
  * @param  param 回调函数参数
  * @retval None
  */
void LED_Toggle_Callback(void *param)
{
  printf("LED Toggle Callback\r\n");
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}


/**
  * @brief  GPIO configuration
  * @param  None
  * @retval None
  */
static void APP_GpioConfig(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0};

  // PA5 RCC
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* Push-pull output */
  GPIO_InitStruct.Pull = GPIO_NOPULL;        /* No pull-up or pull-down */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; /* Low speed */
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); /* Set PA5 to high */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void APP_ErrorHandler(void)
{
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
