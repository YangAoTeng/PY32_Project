/**
 * @file soft_timer.h
 * @brief 软件定时器模块头文件
 * @author Copilot AND PlayerPencil
 * @date 2025/5/7
 */

#ifndef __SOFT_TIMER_H
#define __SOFT_TIMER_H

#include "main.h"

/* 无效定时器ID */
#define SOFT_TIMER_INVALID_ID 0xFF

/**
 * @brief 初始化软件定时器模块
 * @return 无
 */
void SoftTimer_Init(void);

/**
 * @brief 创建并添加一个软件定时器
 * @param interval    定时器周期(ms)
 * @param repeat      重复次数, 0表示无限重复
 * @param callback    超时回调函数
 * @param param       传递给回调函数的参数
 * @return 返回定时器ID，失败返回SOFT_TIMER_INVALID_ID
 */
uint8_t SoftTimer_Create(uint32_t interval, uint32_t repeat, void (*callback)(void*), void* param);

/**
 * @brief 删除指定的软件定时器
 * @param timer_id 要删除的定时器ID
 * @return 成功返回1，失败返回0
 */
uint8_t SoftTimer_Delete(uint8_t timer_id);

/**
 * @brief 在SysTick中断中调用，更新系统时间
 * @param tick_ms SysTick中断间隔(ms)
 * @return 无
 */
void SoftTimer_UpdateTick(uint32_t tick_ms);

/**
 * @brief 获取系统时间
 * @return 系统时间(ms)
 */
uint32_t SoftTimer_GetSystemTime(void);

/**
 * @brief 检查并执行到期的定时器任务
 * @return 无
 */
void SoftTimer_Execute(void);

/**
 * @brief 重置所有软件定时器
 * @return 无
 */
void SoftTimer_ResetAll(void);

/**
 * @brief 删除所有软件定时器
 * @return 无
 */
void SoftTimer_DeleteAll(void);

#endif /* __SOFT_TIMER_H */