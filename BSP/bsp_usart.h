#ifndef __BSP_USART_H
#define __BSP_USART_H
#include "py32f0xx_hal.h"
#include "ring_buffer.h"
#include "stdio.h"

extern UART_HandleTypeDef huart1;

void bsp_usart1_init(uint8_t baud_idx);
extern RingBuffer_t uart_rx_ring_buffer;
#endif // !__BSP_USART_H