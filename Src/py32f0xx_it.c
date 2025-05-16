/**
  ******************************************************************************
  * @file    py32f0xx_it.c
  * @author  MCU Application Team
  * @brief   Interrupt Service Routines.
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
#include "py32f0xx_it.h"
#include "soft_timer.h"
#include "bsp_usart.h"
/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*          Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

extern __IO uint64_t g_motor_system_time;

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  static uint32_t tick = 0;

  tick++;

  if (tick > 200)           // 200KHZ -> 1000HZ
  {
    HAL_IncTick();
    SoftTimer_UpdateTick(1);
    tick = 0;
  }
  g_motor_system_time+= 5; // 更新系统时间(单位:5us)
  
  // /* 更新软件定时器系统时间，HAL库默认SysTick是1ms中断 */

  // // PA4 m1  PB3 M2 PA0 M3 PA1 M4
  // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4); // 切换PA4引脚状态
  // HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3); // 切换PB3引脚状态
  // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0); // 切换PA0引脚状态
  // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1); // 切换PA1引脚状态
}

void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1); // 调用HAL库的UART中断处理函数
}

/******************************************************************************/
/* PY32F0xx Peripheral Interrupt Handlers                                     */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file.                                          */
/******************************************************************************/

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
