#ifndef __74HC165_H
#define __74HC165_H

#include "main.h"

// 74HC165引脚定义
#define PL_PIN         GPIO_PIN_7  // 并行加载引脚 (PA7)
#define CP_PIN         GPIO_PIN_3  // 时钟引脚 (PA3)
#define Q7_PIN         GPIO_PIN_2  // 串行输出引脚 (PA2)
#define HC165_GPIO_PORT GPIOA      // 使用GPIOA端口

// 函数声明
void HC165_Init(void);
uint16_t HC165_Read16Bits(void);

#endif // !__74HC165_H