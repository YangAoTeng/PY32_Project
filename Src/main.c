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
#include "hardware_timr.h" /* 添加硬件定时器头文件 */
#include "modbus_slave.h" /* 添加Modbus从站头文件 */
#include "bsp_flash.h" /* 添加Flash操作头文件 */
#include "74HC595.h" /* 添加74HC595头文件 */
#include "74HC165.h" /* 添加74HC165头文件 */
#include "bsp_motor.h"
// #include "msg_fifo.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_GpioConfig(void);
void LED_Toggle_Callback(void *param);
void IO_Status_Read_Task(void *param); /* 添加IO状态读取任务的函数声明 */

MSG_T g_tMsg;

StepperMotor_t g_tMotor1; // 步进电机结构体实例
StepperMotor_t g_tMotor2; // 步进电机结构体实例
StepperMotor_t g_tMotor3; // 步进电机结构体实例
StepperMotor_t g_tMotor4; // 步进电机结构体实例

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
  * @brief  IO状态读取任务，作为软件定时器回调函数
  * @param  param 回调函数参数
  * @retval None
  */
void IO_Status_Read_Task(void *param)
{
  uint16_t data = HC165_Read16Bits();
  // printf("Read data: %04X\r\n", data);
  // 将16位数据的每一位分别存入g_tVar.T数组
  for (int i = 0; i < 16; i++)
  {
    g_tVar.T[i] = (data >> i) & 0x01;  // 提取data的第i位，存入T[i]
  }
}

// 当前IO的状态 
uint32_t g_u32IOStatus = 0; // 当前IO状态

void IO_Status_Write_Task(void *param)
{
  // 1. 读取 g_tVar.D数组中线圈的状态，按照位放在 g_u32IOStatus中

  // 遍历D数组（线圈状态），组合成一个32位整数
  for (int i = 0; i < D_COIL_SIZE && i < 16; i++) // 只处理前24位，因为74HC595接收24位
  {
    if (g_tVar.D[i]) // 如果线圈状态为1
    {
      g_u32IOStatus |= (1UL << i); // 将对应位置1
    }else{
      g_u32IOStatus &= ~(1UL << i); // 将对应位置0
    }
  }

  // 2. 将 g_u32IOStatus 中的状态写入到74HC595中
  HC595_Send24Bits(g_u32IOStatus); // 发送24位数据到74HC595

}

void Motor_Control_Task(void *param)
{
  if (g_tMotor1.state == STEPPER_STATE_IDLE)
  {

    /*
        // 读取g_tVar.P[]
byte10          默认状态                0         
                电机移动到目标位置       1
                电机移动相对位置         2
                电机匀速运动             3    （会按照 byte18）
                电机急停                4
                电机停止（减速停止）      5


                byte11          电机速度寄存器              只能为正值
                byte12          电机最大速度寄存器          只能为正值
                byte13          电机启动速度寄存器          只能为正值
                byte14          电机加速度寄存器            只能为正值
                byte15          电机当前位置                只读
                byte16          电机移动到目标位置（绝对位置）           只能为正值
                byte17          电机移动到相对位置           正值为正转，负值为反转。绝对位置不能突破到0以下。
                byte18          电机匀速运动寄存器           正值为正转，负值为反转。
                byte19          是否启动限位开关             1 启动限位开关  0不启用限位
                byte20          正转限位开关编号            
                byte21          反转限位开关编号
                byte22          电机当前运行状态
    */
    if (g_tVar.P[10] == 1)
    {
      Stepper_SetSpeed(&g_tMotor1,g_tVar.P[12], g_tVar.P[13], g_tVar.P[14]); // 设置电机速度
      Stepper_MoveTo(&g_tMotor1, g_tVar.P[16]); // 移动到目标位置
    }else if (g_tVar.P[10] == 2)
    {
      Stepper_SetSpeed(&g_tMotor1, g_tVar.P[12], g_tVar.P[13], g_tVar.P[14]); // 设置电机速度
      if (g_tVar.P[17] > 0)
      {
        Stepper_Move(&g_tMotor1, g_tVar.P[17], STEPPER_DIR_CW); // 移动相对位置
      }else if (g_tVar.P[17] < 0)
      {
        Stepper_Move(&g_tMotor1, -g_tVar.P[17], STEPPER_DIR_CCW); // 移动相对位置
      }
    }else if (g_tVar.P[10] == 3)
    {
      Stepper_RunSpeed(&g_tMotor1, g_tVar.P[18]); // 匀速运动
    }
    g_tVar.P[10] = 0; // 清除命令
  }else{
    // 如果电机正在运行，检查是否需要停止
    if (g_tVar.P[10] == 4)
    {
      Stepper_Stop(&g_tMotor1, 1); // 立即停止
      g_tVar.P[10] = 0; // 清除命令
    }else if (g_tVar.P[10] == 0)
    {
      Stepper_Stop(&g_tMotor1, 0); // 减速停止
      g_tVar.P[10] = 0; // 清除命令
    }

  }
  g_tVar.P[22] = g_tMotor1.state; // 更新电机状态
  g_tVar.P[15] = g_tMotor1.position; // 更新电机当前位置
}

void ModbusPoll_Task(void *param)
{
  MODS_Poll();
}


SystemParam_t g_tSysParam = {0}; // 系统参数结构体

void SystemParam_Init(void)
{
  BSP_Flash_Read((uint8_t *)&g_tSysParam, sizeof(SystemParam_t)); // 读取Flash
  if (g_tSysParam.init != 0x10)
  {
    g_tSysParam.init = 0x10; // 设置初始化标志
    g_tSysParam.baud = 6; // 设置默认波特率
    g_tSysParam.modbusId = 1; // 设置默认Modbus ID
    BSP_Flash_Write((uint8_t *)&g_tSysParam, sizeof(SystemParam_t)); // 写入Flash
  }
}



void SystemSoftTime_init(void)
{
  SoftTimer_Create(1000, 0, LED_Toggle_Callback, NULL);     // LED闪烁定时器
  SoftTimer_Create(100, 0, IO_Status_Read_Task, NULL);      // IO状态读取定时器
  SoftTimer_Create(100, 0, IO_Status_Write_Task, NULL);     // IO状态写入定时器
  SoftTimer_Create(10, 0, ModbusPoll_Task, NULL);           // Modbus从站轮询定时器
  // SoftTimer_Create(100, 0, Motor_Control_Task, NULL);        // 电机控制定时器
}

void Motor1PinControl(StepperPinType_t pinType, uint8_t state)
{
  switch (pinType)
  {
    case PIN_TYPE_PWM:
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, state ? GPIO_PIN_SET : GPIO_PIN_RESET); // PA4
      break;
    case PIN_TYPE_DIR:
      // HC959 BIT 16  DIR 
      if (state)
        g_u32IOStatus |= (1UL << 17);  // 设置第16位为1
      else
        g_u32IOStatus &= ~(1UL << 17); // 设置第16位为0
        HC595_Send24Bits(g_u32IOStatus); // 更新74HC595的状态
      break;
    case PIN_TYPE_EN:
      // HC959 BIT 17  EN
      if (state)
        g_u32IOStatus |= (1UL << 16);  // 设置第17位为1
      else
        g_u32IOStatus &= ~(1UL << 16); // 设置第17位为0
        HC595_Send24Bits(g_u32IOStatus); // 更新74HC595的状态
      break;
    default:
      break;
  }

}

void Motor2PinControl(StepperPinType_t pinType, uint8_t state)
{
  switch (pinType)
  {
    case PIN_TYPE_PWM:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, state ? GPIO_PIN_SET : GPIO_PIN_RESET); // PB3
      break;
    case PIN_TYPE_DIR:
      // HC959 BIT 16  DIR 
      if (state)
        g_u32IOStatus |= (1UL << 19);  // 设置第16位为1
      else
        g_u32IOStatus &= ~(1UL << 19); // 设置第16位为0
        HC595_Send24Bits(g_u32IOStatus); // 更新74HC595的状态
      break;
    case PIN_TYPE_EN:
      // HC959 BIT 17  EN
      if (state)
        g_u32IOStatus |= (1UL << 18);  // 设置第17位为1
      else
        g_u32IOStatus &= ~(1UL << 18); // 设置第17位为0
        HC595_Send24Bits(g_u32IOStatus); // 更新74HC595的状态
      break;
    default:
      break;
  }
}

void Motor3PinControl(StepperPinType_t pinType, uint8_t state)
{
  switch (pinType)
  {
    case PIN_TYPE_PWM:
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, state ? GPIO_PIN_SET : GPIO_PIN_RESET); // PA0
      break;
    case PIN_TYPE_DIR:
      // HC959 BIT 16  DIR 
      if (state)
        g_u32IOStatus |= (1UL << 21);  
      else
        g_u32IOStatus &= ~(1UL << 21); 
        HC595_Send24Bits(g_u32IOStatus); // 更新74HC595的状态
      break;
    case PIN_TYPE_EN:
      // HC959 BIT 17  EN
      if (state)
        g_u32IOStatus |= (1UL << 20);  
      else
        g_u32IOStatus &= ~(1UL << 20); 
        HC595_Send24Bits(g_u32IOStatus); // 更新74HC595的状态
      break;
    default:
      break;
  }
}

void Motor4PinControl(StepperPinType_t pinType, uint8_t state)
{
  switch (pinType)
  {
    case PIN_TYPE_PWM:
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, state ? GPIO_PIN_SET : GPIO_PIN_RESET); // PA1
      break;
    case PIN_TYPE_DIR:
      // HC959 BIT 16  DIR 
      if (state)
        g_u32IOStatus |= (1UL << 23);  
      else
        g_u32IOStatus &= ~(1UL << 23); 
        HC595_Send24Bits(g_u32IOStatus); // 更新74HC595的状态
      break;
    case PIN_TYPE_EN:
      // HC959 BIT 17  EN
      if (state)
        g_u32IOStatus |= (1UL << 22);  
      else
        g_u32IOStatus &= ~(1UL << 22); 
        HC595_Send24Bits(g_u32IOStatus); // 更新74HC595的状态
      break;
    default:
      break;
  }
}

void Motor_io_init(void)
{
  // PA4 m1  PB3 M2 PA0 M3 PA1 M4
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // 启用GPIOA时钟
  __HAL_RCC_GPIOA_CLK_ENABLE();
  // 启用GPIOB时钟
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_0 | GPIO_PIN_1; // PA4, PA0, PA1
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* Push-pull output */
  GPIO_InitStruct.Pull = GPIO_NOPULL;        /* No pull-up or pull-down */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; /* Low speed */
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_3; // PB3
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* Push-pull output */
  GPIO_InitStruct.Pull = GPIO_NOPULL;        /* No pull-up or pull-down */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; /* Low speed */
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  // 设置初始状态
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_RESET); // PA4, PA0, PA1
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET); // PB3
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
  HAL_SYSTICK_Config(SystemCoreClock /200000);
  SystemParam_Init(); // 初始化系统参数


  /* Initialize GPIO */
  APP_GpioConfig();
  /* Initialize UART1 */
  bsp_usart1_init(g_tSysParam.baud);
  
  /* 初始化软件定时器 */
  SoftTimer_Init();

  bsp_InitHardTimer();  // 初始化硬件定时器
  
  HC595_Init();       // 输出初始化
  HC165_Init();       // 输入初始化
  Motor_io_init();    // 电机IO初始化

  MODS_Init();        // Modbus从站初始化

  SystemSoftTime_init(); // 初始化软件定时器

  Stepper_Init(&g_tMotor1, Motor1PinControl); // 初始化步进电机

  g_tVar.P[12] = 6000; // 清除保持寄存器
  g_tVar.P[13] = 800; // 清除命令
  g_tVar.P[14] = 500; // 清除命令


  Stepper_SetSpeed(&g_tMotor1, 6000, 800, 500); // 设置电机速度
  // Stepper_Move(&g_tMotor1, 100000, STEPPER_DIR_CW); // 向前移动1000步
  Stepper_MoveTo(&g_tMotor1, 10000); // 移动到目标位置

  while (1)
  {
      /* 执行软件定时器 */
      SoftTimer_Execute();
      Stepper_ProcessAllMotors(); 
  }
}

/**
  * @brief  LED闪烁回调函数
  * @param  param 回调函数参数
  * @retval None
  */
void LED_Toggle_Callback(void *param)
{
  
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6); 
  // 获取电机1的位置
  printf("Motor1 position: %d\r\n", g_tMotor1.position);
  // 获取电机1的状态
  printf("Motor1 state: %d\r\n", g_tMotor1.state);
  // uint16_t data = 0;
  // data = HC165_Read16Bits();
  // printf("Read data: %04X\r\n", data);

  // uint16_t led = 0;
  // if (bsp_GetMsg(&g_tModS_Fifo, &g_tMsg) == 0)
  // {
  //   return; /* 没有消息 */
  // }
  // if (g_tMsg.MsgCode == MSG_MODS_05H)
  // {
  //     MODS_ReadRegister(0, 0, &led); /* 读取线圈状态 */
  //     if (led == 0xFF00)
  //     {
  //         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); /* Set LED to low */
  //     }
  //     else
  //     {
  //         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); /* Set LED to high */
  //     }
  // }else if (g_tMsg.MsgCode == MSG_MODS_06H)
  // {
  //   // 读取保持寄存器的值
  //   MODS_ReadRegister(2, g_tMsg.MsgParam, &led);
  //   if (g_tMsg.MsgParam == 30)
  //   {
  //     g_tSysParam.baud = led; // 设置波特率
  //     if (led > 0 && led < 9)
  //     {
  //       BSP_Flash_Write((uint8_t *)&g_tSysParam, sizeof(SystemParam_t)); // 写入Flash
  //     }
  //   }else if (g_tMsg.MsgParam == 31)
  //   {
  //     g_tSysParam.modbusId = led; // 设置Modbus ID
  //     if (led > 0 && led < 256)
  //     {
  //       BSP_Flash_Write((uint8_t *)&g_tSysParam, sizeof(SystemParam_t)); // 写入Flash
  //     }
  //   }
  // }
  // else
  // {

  // }
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

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; /* Push-pull output */
  GPIO_InitStruct.Pull = GPIO_NOPULL;        /* No pull-up or pull-down */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; /* Low speed */
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); /* Set PA5 to high */
  
  // /* 配置PB0, PB1, PB2为输入模式 */
  // GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
  // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;     /* 输入模式 */
  // GPIO_InitStruct.Pull = GPIO_PULLUP;         /* 上拉电阻 */
  // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  // HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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

