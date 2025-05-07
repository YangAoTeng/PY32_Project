#include "bsp_usart.h"

UART_HandleTypeDef huart1;
void APP_ErrorHandler(void);

/**
 * @brief           usart1 初始化  PF0 RX  PF1 TX
 * 
 */
void bsp_usart1_init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

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
}


int fputc(int ch, FILE *f)
{
   while (!(USART1->SR & USART_SR_TXE)); 
   USART1->DR = (uint8_t)ch; 
   return ch;
}
