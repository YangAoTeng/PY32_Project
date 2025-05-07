#ifndef __BSP_USART_H
#define __BSP_USART_H
#include "py32f0xx_hal.h"
#include "stdio.h"

extern UART_HandleTypeDef huart1;

void bsp_usart1_init(void);
#endif // !__BSP_USART_H