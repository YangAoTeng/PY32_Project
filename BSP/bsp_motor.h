#ifndef __BSP_MOTOR_H
#define __BSP_MOTOR_H

#include "py32f0xx_hal.h"
#include <stdint.h>

// 步进电机状态定义
typedef enum {
    STEPPER_STATE_IDLE = 0,     // 空闲状态
    STEPPER_STATE_RUNNING,      // 运行状态
    STEPPER_STATE_ACCELERATING, // 加速状态
    STEPPER_STATE_DECELERATING, // 减速状态
    STEPPER_STATE_STOPPING      // 停止状态
} StepperState_t;

// 步进电机方向定义
typedef enum {
    STEPPER_DIR_CW = 0,  // 顺时针
    STEPPER_DIR_CCW = 1  // 逆时针
} StepperDirection_t;

// 步进电机结构体
typedef struct StepperMotor {
    // 引脚配置
    GPIO_TypeDef* PWM_Port;    // PWM端口
    uint16_t PWM_Pin;          // PWM引脚
    GPIO_TypeDef* DIR_Port;    // 方向端口
    uint16_t DIR_Pin;          // 方向引脚
    GPIO_TypeDef* EN_Port;     // 使能端口
    uint16_t EN_Pin;           // 使能引脚
    
    // 运行状态
    StepperState_t state;      // 电机状态
    StepperDirection_t dir;    // 运行方向
    uint32_t position;         // 当前位置（步数）
    uint32_t target_position;  // 目标位置
    
    // 速度参数
    uint32_t step_delay;       // 步进延时(us)
    uint32_t min_step_delay;   // 最小步进延时(最大速度)
    uint32_t max_step_delay;   // 最大步进延时(启动速度)
    uint32_t accel_steps;      // 加速步数
      // 时间控制
    uint32_t last_step_time;   // 上次步进时间
    uint8_t pulse_state;       // PWM脉冲状态
    uint32_t pulse_width;      // PWM脉冲宽度(us)
    
    // 限位开关标志
    uint8_t limit_enabled;     // 限位开关使能标志
    uint8_t cw_limit;          // 正转限位开关状态(1=触发)
    uint8_t ccw_limit;         // 反转限位开关状态(1=触发)
    
    // 链表指针，用于管理多个电机
    struct StepperMotor* next;
} StepperMotor_t;

/**
 * @brief 初始化步进电机
 * @param motor 步进电机结构体指针
 * @param pwm_port PWM端口
 * @param pwm_pin PWM引脚
 * @param dir_port 方向端口
 * @param dir_pin 方向引脚
 * @param en_port 使能端口
 * @param en_pin 使能引脚
 * @return None
 */
void Stepper_Init(StepperMotor_t* motor, 
                 GPIO_TypeDef* pwm_port, uint16_t pwm_pin,
                 GPIO_TypeDef* dir_port, uint16_t dir_pin,
                 GPIO_TypeDef* en_port, uint16_t en_pin);

/**
 * @brief 设置步进电机速度参数
 * @param motor 步进电机结构体指针
 * @param max_speed 最大速度(步/秒)
 * @param start_speed 启动速度(步/秒)
 * @param accel 加速度(步/秒^2)
 * @return None
 */
void Stepper_SetSpeed(StepperMotor_t* motor, uint32_t max_speed, uint32_t start_speed, uint32_t accel);

/**
 * @brief 设置步进电机目标位置(相对运动)
 * @param motor 步进电机结构体指针
 * @param steps 步数
 * @param dir 方向
 * @return None
 */
void Stepper_Move(StepperMotor_t* motor, uint32_t steps, StepperDirection_t dir);

/**
 * @brief 设置步进电机目标位置(绝对运动)
 * @param motor 步进电机结构体指针
 * @param position 目标位置
 * @return None
 */
void Stepper_MoveTo(StepperMotor_t* motor, uint32_t position);

/**
 * @brief 停止步进电机
 * @param motor 步进电机结构体指针
 * @param immediate 是否立即停止
 * @return None
 */
void Stepper_Stop(StepperMotor_t* motor, uint8_t immediate);

/**
 * @brief 使能步进电机
 * @param motor 步进电机结构体指针
 * @param enable 是否使能
 * @return None
 */
void Stepper_Enable(StepperMotor_t* motor, uint8_t enable);

/**
 * @brief 步进电机周期性处理函数(应该在主循环或定时器中调用)
 * @param motor 步进电机结构体指针
 * @return None
 */
void Stepper_Handler(StepperMotor_t* motor);

/**
 * @brief 获取步进电机状态
 * @param motor 步进电机结构体指针
 * @return 电机状态
 */
StepperState_t Stepper_GetState(StepperMotor_t* motor);

/**
 * @brief 获取步进电机位置
 * @param motor 步进电机结构体指针
 * @return 当前位置
 */
uint32_t Stepper_GetPosition(StepperMotor_t* motor);

/**
 * @brief 复位步进电机位置计数器
 * @param motor 步进电机结构体指针
 * @return None
 */
void Stepper_ResetPosition(StepperMotor_t* motor);

/**
 * @brief 管理多个步进电机的处理函数(在主循环中调用)
 * @return None
 */
void Stepper_ProcessAllMotors(void);

/**
 * @brief 添加步进电机到管理列表
 * @param motor 步进电机结构体指针
 * @return None
 */
void Stepper_AddMotor(StepperMotor_t* motor);

/**
 * @brief 从管理列表移除步进电机
 * @param motor 步进电机结构体指针
 * @return None
 */
void Stepper_RemoveMotor(StepperMotor_t* motor);

/**
 * @brief 根据指定转速匀速运行步进电机(不需要目标位置)
 * @param motor 步进电机结构体指针
 * @param speed 速度(步/秒)，正值为顺时针，负值为逆时针
 * @return None
 */
void Stepper_RunSpeed(StepperMotor_t* motor, int32_t speed);

/**
 * @brief 设置限位开关状态
 * @param motor 步进电机结构体指针
 * @param cw_limit 正转限位状态(1=触发)
 * @param ccw_limit 反转限位状态(1=触发)
 * @return None
 */
void Stepper_SetLimitSwitches(StepperMotor_t* motor, uint8_t cw_limit, uint8_t ccw_limit);

/**
 * @brief 使能或禁用限位开关功能
 * @param motor 步进电机结构体指针
 * @param enable 是否使能限位开关
 * @return None
 */
void Stepper_EnableLimitSwitches(StepperMotor_t* motor, uint8_t enable);

/**
 * @brief 回归零点（寻找原点）
 * @param motor 步进电机结构体指针
 * @param speed 回归速度(步/秒)，使用正值（内部会转为反转方向）
 * @return None
 * @note 该函数会使电机反转直到触发反转限位开关，然后将该位置设置为0
 */
void Stepper_GoHome(StepperMotor_t* motor, uint32_t speed);

#endif // !__BSP_MOTOR_H