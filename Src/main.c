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
#include "bsp_24c02.h"  /* 添加24C02头文件 */
#include "bsp_at.h"     /* 添加AT命令处理头文件 */

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_GpioConfig(void);
void LED_Toggle_Callback(void *param);
void IO_Status_Read_Task(void *param); /* 添加IO状态读取任务的函数声明 */

/**
 * @brief         设置系统时钟为48Mhz，必须在HAL_Init之后调用
 * 
 */
static void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
 
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE
                                    | RCC_OSCILLATORTYPE_HSI
                                    | RCC_OSCILLATORTYPE_LSE
                                    | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                            /* HSI ON */
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                            /* No division */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;   /* HSI = 24MHz */
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                           /* OFF */
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                           /* OFF */
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                           /* OFF */
  // 以上部分和使用HSI作为时钟源是一样的, 以下是PLL相关的设置, 首先是开启PLL
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // 将PLL时钟源设置为内部高速, HSI频率需要高于12MHz
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  // 应用设置
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }
  // 设置系统时钟
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  // 设置PLL为系统时钟源
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // AHB 不分频
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // APB 不分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  // 应用设置
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}


/**
  * @brief  处理串口接收数据任务，作为软件定时器回调函数
  * @param  param 回调函数参数
  * @retval None
  */
void UART_Process_Task(void *param)
{
    uint8_t data;
    
    /* 从环形缓冲区读取数据 */
    while (!RingBuffer_IsEmpty(&uart_rx_ring_buffer))
    {
        RingBuffer_Read(&uart_rx_ring_buffer, &data);
        printf("%c", data);
        // /* 传递数据给AT命令处理模块 */
        // AT_ProcessRxData(data);
    }
}

/**
  * @brief  IO状态读取任务，作为软件定时器回调函数
  * @param  param 回调函数参数
  * @retval None
  */
void IO_Status_Read_Task(void *param)
{
    /* 读取PB0、PB1和PB2的IO状态 */
    GPIO_PinState pb0_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
    GPIO_PinState pb1_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
    GPIO_PinState pb2_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
    
    /* 打印IO状态 */
    printf("IO Status: PB0=%d, PB1=%d, PB2=%d\r\n", 
           pb0_state == GPIO_PIN_SET ? 1 : 0,
           pb1_state == GPIO_PIN_SET ? 1 : 0,
           pb2_state == GPIO_PIN_SET ? 1 : 0);
}

/**
  * @brief  Main program.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Systick */
  HAL_Init();                                  
  APP_SystemClockConfig(); /* Configure the system clock */
  
  /* Initialize GPIO */
  APP_GpioConfig();
  /* Initialize UART1 */
  bsp_usart1_init();
  
  /* 初始化软件定时器 */
  SoftTimer_Init();
  /* 初始化EEPROM */
  EEPROM_Init();
  // /* 初始化AT命令处理模块 */
  // AT_Init();
  
  printf("SystemCoreClock: %d\r\n", SystemCoreClock);
  printf("AT command interface ready, type 'AT+HELP' for help\r\n");

  /* 创建LED闪烁定时器(无限循环) */
  uint8_t timer1 = SoftTimer_Create(1000, 0, LED_Toggle_Callback, NULL);
  printf("LED toggle timer created, ID: %d\r\n", timer1);
  
  /* 创建UART处理定时器，10ms周期执行 */
  uint8_t timer2 = SoftTimer_Create(10, 0, UART_Process_Task, NULL);
  printf("UART process timer created, ID: %d\r\n", timer2);
  
  /* 创建IO状态读取定时器，10ms周期执行 */
  uint8_t timer3 = SoftTimer_Create(1000, 0, IO_Status_Read_Task, NULL);
  printf("IO status read timer created, ID: %d\r\n", timer3);
  
  while (1)
  {
      /* 执行软件定时器 */
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
  // 启用GPIOB时钟
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* Push-pull output */
  GPIO_InitStruct.Pull = GPIO_NOPULL;        /* No pull-up or pull-down */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; /* Low speed */
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); /* Set PA5 to high */
  
  /* 配置PB0, PB1, PB2为输入模式 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;     /* 输入模式 */
  GPIO_InitStruct.Pull = GPIO_PULLUP;         /* 上拉电阻 */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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

