#ifndef __74HC595_H
#define __74HC595_H

#include "main.h"

// 74HC595引脚定义
#define SER_IN_PIN      GPIO_PIN_0  // 数据输入引脚 (PB0)
#define ST_CP_PIN       GPIO_PIN_1  // 存储寄存器时钟引脚 (PB1)
#define SH_CP_PIN       GPIO_PIN_2  // 移位寄存器时钟引脚 (PB2)
#define HC595_GPIO_PORT GPIOB       // 使用GPIOB端口

// 函数声明
void HC595_Init(void);
void HC595_Send24Bits(uint32_t data);

#endif // !__74HC595_H