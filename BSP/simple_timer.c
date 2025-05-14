/**
 * @file simple_timer.c
 * @brief 简单软件定时器实现
 */

#include "simple_timer.h"

// 全局变量，用于记录系统启动以来的微秒数
static volatile uint32_t g_system_us = 0;
static volatile uint32_t g_system_ms = 0;

/**
 * @brief 初始化软件定时器（使用SysTick配置1ms中断）
 */
void SoftTimer_Init(void)
{
    // 配置SysTick中断周期为1ms
    HAL_SYSTICK_Config(SystemCoreClock / 1000);
    
    // 配置SysTick中断优先级
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
    
    // 重置系统时间计数器
    g_system_us = 0;
    g_system_ms = 0;
}

/**
 * @brief 获取系统时间（以微秒为单位）
 * @return 当前系统时间（微秒）
 * 
 * 注意：这里使用g_system_ms*1000+计算当前SysTick计数值对应的微秒数
 * 精度受限于SysTick时钟分辨率
 */
uint32_t SoftTimer_GetSystemTime(void)
{
    uint32_t ms;
    uint32_t tick;
    
    // 读取毫秒计数和当前SysTick值（注意关中断防止读取期间被更新）
    __disable_irq();
    ms = g_system_ms;
    tick = SysTick->VAL;
    __enable_irq();
    
    // 计算微秒时间：毫秒计数*1000 + (SysTick重载值-当前值)/滴答频率*1000000
    // SysTick->LOAD是重载值，对应1ms时间
    // SysTick时钟频率为SystemCoreClock
    return ms * 1000 + (SysTick->LOAD - tick) * 1000 / (SystemCoreClock / 1000);
}

/**
 * @brief 微秒级延时
 * @param us 要延时的微秒数
 */
void SoftTimer_DelayUs(uint32_t us)
{
    uint32_t start = SoftTimer_GetSystemTime();
    
    // 等待直到经过指定的微秒数
    while ((SoftTimer_GetSystemTime() - start) < us) {
        // 空循环等待
    }
}

/**
 * @brief 毫秒级延时
 * @param ms 要延时的毫秒数
 */
void SoftTimer_DelayMs(uint32_t ms)
{
    SoftTimer_DelayUs(ms * 1000);
}

/**
 * @brief SysTick中断服务函数（在HAL_IncTick中调用）
 * 
 * 注意：HAL库已经定义了SysTick_Handler，会调用HAL_IncTick()
 * 我们需要在HAL_IncTick中增加对UpdateSystemTime的调用
 */
void UpdateSystemTime(void)
{
    // 更新毫秒计数
    g_system_ms++;
}

/**
 * @brief 重写HAL_IncTick函数以更新我们的系统时间
 * 
 * 注意：这个函数会覆盖HAL库中的同名函数
 */
void HAL_IncTick(void)
{
    // 调用原始HAL库的tick计数器更新
    extern __IO uint32_t uwTick;
    uwTick += uwTickFreq;
    
    // 更新我们的系统时间
    UpdateSystemTime();
}