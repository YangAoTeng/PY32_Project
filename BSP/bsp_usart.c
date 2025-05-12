#include "bsp_usart.h"


UART_HandleTypeDef huart1;
void APP_ErrorHandler(void);

uint8_t rx_buffer = 0; // 接收缓冲区

/* UART接收环形缓冲区定义 */
#define UART_RX_BUFFER_SIZE 64
RingBuffer_t uart_rx_ring_buffer;
uint8_t uart_rx_buffer_data[UART_RX_BUFFER_SIZE];

/**
 * @brief           usart1 初始化  PF0 RX  PF1 TX
 * 
 */
void bsp_usart1_init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // PF0 RX
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;              /* Alternate function push-pull */
    GPIO_InitStruct.Pull = GPIO_PULLUP;                  /* Enable pull-up */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;        /* GPIO speed */
    GPIO_InitStruct.Alternate = GPIO_AF8_USART1;         /* Alternate function USART1 */
    /* GPIO Initialization */
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    // PF1 TX
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;              /* Alternate function push-pull */
    GPIO_InitStruct.Pull = GPIO_PULLUP;                  /* Enable pull-up */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;        /* GPIO speed */
    GPIO_InitStruct.Alternate = GPIO_AF8_USART1;         /* Alternate function USART1 */
    /* GPIO Initialization */
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
      APP_ErrorHandler();
    }

    /* 配置NVIC以启用USART1中断 */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    
    /* 初始化接收环形缓冲区 */
    RingBuffer_Init(&uart_rx_ring_buffer, uart_rx_buffer_data, UART_RX_BUFFER_SIZE);
    
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    // 开始接收数据
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&rx_buffer, 1);
}


int fputc(int ch, FILE *f)
{
   while (!(USART1->SR & USART_SR_TXE)); 
   USART1->DR = (uint8_t)ch; 
   return ch;
}

extern void MODS_ReciveNew(uint8_t _byte);
/**
  * @brief  UART接收完成回调函数
  * @param  huart: UART句柄
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // /* 将接收到的数据写入环形缓冲区 */
        // if (!RingBuffer_IsFull(&uart_rx_ring_buffer))
        // {
        //     RingBuffer_Write(&uart_rx_ring_buffer, rx_buffer);
        // }
        MODS_ReciveNew(rx_buffer); // 调用MODS_ReciveNew函数处理接收到的数据
        // printf("%c", rx_buffer); // 打印接收到的数据
        /* 重新启动接收以继续接收数据 */
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&rx_buffer, 1);
    }
}