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
#include "hardware_timr.h" /* 添加硬件定时器头文件 */
#include "modbus_slave.h" /* 添加Modbus从站头文件 */
#include "msg_fifo.h" /* 添加消息FIFO头文件 */
#include "bsp_flash.h" /* 添加Flash操作头文件 */

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_GpioConfig(void);
void LED_Toggle_Callback(void *param);
void IO_Status_Read_Task(void *param); /* 添加IO状态读取任务的函数声明 */

MSG_T g_tMsg;

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
    MODS_WriteRegister(1, 0, pb0_state); /* 写入PB0状态 */
    MODS_WriteRegister(1, 1, pb1_state); /* 写入PB1状态 */
    MODS_WriteRegister(1, 2, pb2_state); /* 写入PB2状态 */
}


void HardTimer_Callback(void *param)
{
    printf("TIM3->CNT: %d\r\n", TIM3->CNT);
    printf("Hard Timer Callback\r\n");
    // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}



SystemParam_t g_tSysParam = {0}; // 系统参数结构体

void SystemParam_Init(void)
{
  BSP_Flash_Read((uint8_t *)&g_tSysParam, sizeof(SystemParam_t)); // 读取Flash
  if (g_tSysParam.init != 0x11)
  {
    g_tSysParam.init = 0x11; // 设置初始化标志
    g_tSysParam.baud = 6; // 设置默认波特率
    g_tSysParam.modbusId = 1; // 设置默认Modbus ID
    BSP_Flash_Write((uint8_t *)&g_tSysParam, sizeof(SystemParam_t)); // 写入Flash
  }
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
  SystemParam_Init(); // 初始化系统参数


  /* Initialize GPIO */
  APP_GpioConfig();
  /* Initialize UART1 */
  bsp_usart1_init(g_tSysParam.baud);
  
  /* 初始化软件定时器 */
  SoftTimer_Init();
  /* 初始化EEPROM */
  // EEPROM_Init();
  // /* 初始化AT命令处理模块 */
  // AT_Init();
  printf("SystemParam Read: %d\r\n", g_tSysParam.baud);
  printf("SystemParam Read: %d\r\n", g_tSysParam.modbusId);



  printf("SystemCoreClock: %d\r\n", SystemCoreClock);
  // printf("AT command interface ready, type 'AT+HELP' for help\r\n");



  /* 创建LED闪烁定时器(无限循环) */
  uint8_t timer1 = SoftTimer_Create(100, 0, LED_Toggle_Callback, NULL);
  printf("LED toggle timer created, ID: %d\r\n", timer1);

  SoftTimer_SetUserData(timer1, 12345);
  
  // /* 创建UART处理定时器，10ms周期执行 */
  // uint8_t timer2 = SoftTimer_Create(10, 0, UART_Process_Task, NULL);
  // printf("UART process timer created, ID: %d\r\n", timer2);
  
  // /* 创建IO状态读取定时器，10ms周期执行 */
  uint8_t timer3 = SoftTimer_Create(100, 0, IO_Status_Read_Task, NULL);
  printf("IO status read timer created, ID: %d\r\n", timer3);
  bsp_InitHardTimer();
  MODS_Init();
  // bsp_StartHardTimer(1, 65535, HardTimer_Callback);
  while (1)
  {
      /* 执行软件定时器 */
      SoftTimer_Execute();
      MODS_Poll();
  }
}


/**
  * @brief  LED闪烁回调函数
  * @param  param 回调函数参数
  * @retval None
  */
void LED_Toggle_Callback(void *param)
{
  uint16_t led = 0;
  if (bsp_GetMsg(&g_tModS_Fifo, &g_tMsg) == 0)
  {
    return; /* 没有消息 */
  }
  if (g_tMsg.MsgCode == MSG_MODS_05H)
  {
      MODS_ReadRegister(0, 0, &led); /* 读取线圈状态 */
      if (led == 0xFF00)
      {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); /* Set LED to low */
      }
      else
      {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); /* Set LED to high */
      }

  }else if (g_tMsg.MsgCode == MSG_MODS_06H)
  {
    // 读取保持寄存器的值
    MODS_ReadRegister(2, g_tMsg.MsgParam, &led);
    if (g_tMsg.MsgParam == 30)
    {
      g_tSysParam.baud = led; // 设置波特率
      BSP_Flash_Write((uint8_t *)&g_tSysParam, sizeof(SystemParam_t)); // 写入Flash
    }else if (g_tMsg.MsgParam == 31)
    {
      g_tSysParam.modbusId = led; // 设置Modbus ID
      BSP_Flash_Write((uint8_t *)&g_tSysParam, sizeof(SystemParam_t)); // 写入Flash
    }
  }
  else
  {
      // printf("No MODBUS_MSG\r\n");
  }
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

