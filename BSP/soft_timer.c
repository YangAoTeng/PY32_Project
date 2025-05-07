/**
 * @file soft_timer.c
 * @brief 软件定时器模块实现文件
 * @author Copilot AND PlayerPencil
 * @date 2025/5/7
 */

#include "soft_timer.h"

/* 定义最大支持的软件定时器数量 */
#define MAX_SOFT_TIMERS 10

/* 定义软件定时器结构体 */
typedef struct {
    uint32_t interval;       /* 定时器周期(ms) */
    uint32_t elapsed;        /* 已经流逝的时间(ms) */
    uint32_t repeat_count;   /* 重复次数, 0表示无限重复 */
    uint32_t executed_count; /* 已执行次数 */
    void (*callback)(void*); /* 回调函数 */
    void* param;             /* 回调函数参数 */
    uint8_t active;          /* 定时器是否激活 */
} SoftTimer_t;

/* 软件定时器池 */
static SoftTimer_t timer_pool[MAX_SOFT_TIMERS];

/* 系统计时器，单位ms */
static volatile uint32_t system_time = 0;

/**
 * @brief 初始化软件定时器模块
 * @return 无
 */
void SoftTimer_Init(void)
{
    uint8_t i;
    
    system_time = 0;
    
    /* 初始化所有定时器为非激活状态 */
    for (i = 0; i < MAX_SOFT_TIMERS; i++) {
        timer_pool[i].active = 0;
    }
}

/**
 * @brief 创建并添加一个软件定时器
 * @param interval    定时器周期(ms)
 * @param repeat      重复次数, 0表示无限重复
 * @param callback    超时回调函数
 * @param param       传递给回调函数的参数
 * @return 返回定时器ID，失败返回SOFT_TIMER_INVALID_ID
 */
uint8_t SoftTimer_Create(uint32_t interval, uint32_t repeat, void (*callback)(void*), void* param)
{
    uint8_t i;
    
    if (callback == NULL || interval == 0) {
        return SOFT_TIMER_INVALID_ID;
    }
    
    /* 查找可用的定时器槽 */
    for (i = 0; i < MAX_SOFT_TIMERS; i++) {
        if (timer_pool[i].active == 0) {
            timer_pool[i].interval = interval;
            timer_pool[i].elapsed = 0;
            timer_pool[i].repeat_count = repeat;
            timer_pool[i].executed_count = 0;
            timer_pool[i].callback = callback;
            timer_pool[i].param = param;
            timer_pool[i].active = 1;
            
            return i;
        }
    }
    
    /* 没有可用的定时器槽 */
    return SOFT_TIMER_INVALID_ID;
}

/**
 * @brief 删除指定的软件定时器
 * @param timer_id 要删除的定时器ID
 * @return 成功返回1，失败返回0
 */
uint8_t SoftTimer_Delete(uint8_t timer_id)
{
    if (timer_id >= MAX_SOFT_TIMERS || timer_pool[timer_id].active == 0) {
        return 0;
    }
    
    timer_pool[timer_id].active = 0;
    return 1;
}

/**
 * @brief 在SysTick中断中调用，更新系统时间
 * @param tick_ms SysTick中断间隔(ms)
 * @return 无
 */
void SoftTimer_UpdateTick(uint32_t tick_ms)
{
    uint8_t i;
    
    system_time += tick_ms;
    
    /* 更新每个激活的定时器的计时 */
    for (i = 0; i < MAX_SOFT_TIMERS; i++) {
        if (timer_pool[i].active) {
            timer_pool[i].elapsed += tick_ms;
        }
    }
}

/**
 * @brief 获取系统时间
 * @return 系统时间(ms)
 */
uint32_t SoftTimer_GetSystemTime(void)
{
    return system_time;
}

/**
 * @brief 检查并执行到期的定时器任务
 * @return 无
 */
void SoftTimer_Execute(void)
{
    uint8_t i;
    
    for (i = 0; i < MAX_SOFT_TIMERS; i++) {
        if (timer_pool[i].active && timer_pool[i].elapsed >= timer_pool[i].interval) {
            /* 执行回调函数 */
            if (timer_pool[i].callback != NULL) {
                timer_pool[i].callback(timer_pool[i].param);
            }
            
            timer_pool[i].executed_count++;
            timer_pool[i].elapsed = 0; /* 重置计时 */
            
            /* 检查是否需要删除定时器 */
            if (timer_pool[i].repeat_count > 0 && 
                timer_pool[i].executed_count >= timer_pool[i].repeat_count) {
                timer_pool[i].active = 0;
            }
        }
    }
}

/**
 * @brief 重置所有软件定时器
 * @return 无
 */
void SoftTimer_ResetAll(void)
{
    uint8_t i;
    
    for (i = 0; i < MAX_SOFT_TIMERS; i++) {
        if (timer_pool[i].active) {
            timer_pool[i].elapsed = 0;
            timer_pool[i].executed_count = 0;
        }
    }
}

/**
 * @brief 删除所有软件定时器
 * @return 无
 */
void SoftTimer_DeleteAll(void)
{
    uint8_t i;
    
    for (i = 0; i < MAX_SOFT_TIMERS; i++) {
        timer_pool[i].active = 0;
    }
}