/**
 * @file simple_timer.h
 * @brief 简单软件定时器头文件
 */
 
#ifndef __SIMPLE_TIMER_H
#define __SIMPLE_TIMER_H

#include "py32f0xx_hal.h"
#include <stdint.h>

/**
 * @brief 初始化软件定时器
 * @return None
 */
void SoftTimer_Init(void);

/**
 * @brief 获取系统时间（以微秒为单位）
 * @return 当前系统时间（微秒）
 */
uint32_t SoftTimer_GetSystemTime(void);

/**
 * @brief 微秒级延时
 * @param us 要延时的微秒数
 * @return None
 */
void SoftTimer_DelayUs(uint32_t us);

/**
 * @brief 毫秒级延时
 * @param ms 要延时的毫秒数
 * @return None
 */
void SoftTimer_DelayMs(uint32_t ms);

#endif /* __SIMPLE_TIMER_H */