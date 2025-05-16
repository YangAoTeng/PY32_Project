/**
 * @file bsp_motor.c
 * @brief 步进电机驱动库实现
 */

#include "bsp_motor.h"
#include <stdlib.h>

// 系统时间(单位:1us)
uint64_t __IO g_motor_system_time = 0;            

// 全局链表头指针，用于管理所有电机
static StepperMotor_t* g_stepper_list = NULL;

// 私有函数声明
static void Stepper_SetDirection(StepperMotor_t* motor, StepperDirection_t dir);
static void Stepper_TogglePulse(StepperMotor_t* motor);

/**
 * @brief 注册步进电机引脚控制回调函数
 */
void Stepper_RegisterPinControl(StepperMotor_t* motor, PinControlFunc_t pinControlFunc)
{
    if (motor != NULL) {
        motor->PinControl = pinControlFunc;
    }
}

/**
 * @brief 初始化步进电机
 */
void Stepper_Init(StepperMotor_t* motor, PinControlFunc_t pinControlFunc)
{
    // 初始化电机状态
    motor->state = STEPPER_STATE_IDLE;
    motor->dir = STEPPER_DIR_CW;
    motor->position = 0;
    motor->target_position = 0;
    
    // 注册引脚控制回调函数
    motor->PinControl = pinControlFunc;
    
    // 设置默认速度参数
    motor->step_delay = 1000;      // 默认延时1ms
    motor->min_step_delay = 500;   // 最小延时0.5ms (最大速度2000步/秒)
    motor->max_step_delay = 2000;  // 最大延时2ms (启动速度500步/秒)
    motor->accel_steps = 100;      // 加速步数
    
    // 初始化时间控制
    motor->last_step_time = 0;
    motor->pulse_state = 0;
    // 移除 pulse_width 初始化
    
    // 初始化限位开关
    motor->limit_enabled = 0;      // 默认禁用限位开关
    motor->cw_limit = 0;
    motor->ccw_limit = 0;
    
    // 初始化链表指针
    motor->next = NULL;
    
    // 设置引脚状态(如果已注册回调函数)
    if (motor->PinControl != NULL) {
        motor->PinControl(PIN_TYPE_PWM, 0);  // PWM引脚低电平
        motor->PinControl(PIN_TYPE_DIR, (motor->dir == STEPPER_DIR_CW) ? 0 : 1);  // 方向引脚
        motor->PinControl(PIN_TYPE_EN, 1);  // 使能引脚高电平(禁用电机)
    }
    
    // 添加到电机管理列表
    Stepper_AddMotor(motor);
}

/**
 * @brief 设置步进电机速度参数
 */
void Stepper_SetSpeed(StepperMotor_t* motor, uint32_t max_speed, uint32_t start_speed, uint32_t accel)
{
    // 速度转换为延时
    if (max_speed > 0) {
        motor->min_step_delay = 1000000 / max_speed; // 将步/秒转换为us延时
    }
    
    if (start_speed > 0) {
        motor->max_step_delay = 1000000 / start_speed; // 将步/秒转换为us延时
    }
    
    // 设置当前步进延时为最大延时(启动速度)
    motor->step_delay = motor->max_step_delay;
    
    // 设置加速步数
    if (accel > 0) {
        // 计算所需加速步数: 加速度 = 速度差/时间，步数 = 速度差^2/(2*加速度)
        uint32_t speed_diff = max_speed - start_speed;
        motor->accel_steps = (speed_diff * speed_diff) / (2 * accel);
        
        // 确保至少有一个加速步骤
        if (motor->accel_steps < 1) {
            motor->accel_steps = 1;
        }
    } else {
        motor->accel_steps = 0; // 不进行加减速
    }
}

/**
 * @brief 设置步进电机目标位置(相对运动)
 */
void Stepper_Move(StepperMotor_t* motor, uint32_t steps, StepperDirection_t dir)
{
    // 设置方向
    Stepper_SetDirection(motor, dir);
    
    // 计算目标位置
    if (dir == STEPPER_DIR_CW) {
        motor->target_position = motor->position + steps;
    } else {
        motor->target_position = motor->position - steps;
    }
    
    // 设置初始速度为启动速度
    motor->step_delay = motor->max_step_delay;
    
    // 更新电机状态
    if (motor->accel_steps > 0 && steps > motor->accel_steps * 2) {
        motor->state = STEPPER_STATE_ACCELERATING;
    } else {
        motor->state = STEPPER_STATE_RUNNING;
    }
    
    // 重要：重置脉冲状态，确保从低电平开始
    motor->pulse_state = 0;
    if (motor->PinControl != NULL) {
        motor->PinControl(PIN_TYPE_PWM, 0);
    }
    
    // 使能电机
    Stepper_Enable(motor, 1);
}

/**
 * @brief 设置步进电机目标位置(绝对运动)
 */
void Stepper_MoveTo(StepperMotor_t* motor, uint32_t position)
{
    // 计算移动方向和步数
    StepperDirection_t dir;
    uint32_t steps;
    
    if (position > motor->position) {
        dir = STEPPER_DIR_CW;
        steps = position - motor->position;
    } else {
        dir = STEPPER_DIR_CCW;
        steps = motor->position - position;
    }
    
    // 执行相对运动
    Stepper_Move(motor, steps, dir);
}

/**
 * @brief 停止步进电机
 */
void Stepper_Stop(StepperMotor_t* motor, uint8_t immediate)
{
    if (immediate) {
        // 立即停止
        motor->state = STEPPER_STATE_IDLE;
        motor->target_position = motor->position;
    } else {
        // 减速停止
        if (motor->state != STEPPER_STATE_IDLE) {
            motor->state = STEPPER_STATE_DECELERATING;
            
            // 根据当前位置和减速度计算新的目标位置
            if (motor->dir == STEPPER_DIR_CW) {
                motor->target_position = motor->position + motor->accel_steps;
            } else {
                motor->target_position = motor->position - motor->accel_steps;
            }
        }
    }
}

/**
 * @brief 使能步进电机
 */
void Stepper_Enable(StepperMotor_t* motor, uint8_t enable)
{
    if (motor->PinControl != NULL) {
        if (enable) {
            motor->PinControl(PIN_TYPE_EN, 0); // 低电平使能
        } else {
            motor->PinControl(PIN_TYPE_EN, 1); // 高电平禁用
            motor->state = STEPPER_STATE_IDLE; // 禁用时设为空闲状态
        }
    }
}

/**
 * @brief 步进电机周期性处理函数
 */
void Stepper_Handler(StepperMotor_t* motor)
{
    uint64_t current_time = g_motor_system_time; // 获取当前系统时间
    
    // 处理脉冲信号
    if (motor->state != STEPPER_STATE_IDLE) {
        // 检查限位开关状态
        if (motor->limit_enabled) {
            // 检查当前方向的限位开关是否触发
            if ((motor->dir == STEPPER_DIR_CW && motor->cw_limit) ||
                (motor->dir == STEPPER_DIR_CCW && motor->ccw_limit)) {
                // 限位开关触发，立即停止
                motor->state = STEPPER_STATE_IDLE;
                
                // 如果是反转方向触发限位开关，并且是回归零点操作，则重置位置为0
                if (motor->dir == STEPPER_DIR_CCW && motor->ccw_limit) {
                    motor->position = 0;
                    motor->target_position = 0;
                }
                
                return;
            }
        }
        
        // 计算半个周期的时间（50%占空比）
        uint32_t half_period = motor->step_delay / 2;
        
        // 检查是否需要切换脉冲状态
        if (current_time - motor->last_step_time >= half_period) {
            motor->last_step_time = current_time;
            
            // 切换脉冲状态
            if (motor->pulse_state == 0) {
                // 从低电平切换到高电平（脉冲开始）
                if (motor->PinControl != NULL) {
                    motor->PinControl(PIN_TYPE_PWM, 1);  // PWM引脚高电平
                }
                motor->pulse_state = 1;
            } else {
                // 从高电平切换到低电平（脉冲结束，完成一个步进）
                if (motor->PinControl != NULL) {
                    motor->PinControl(PIN_TYPE_PWM, 0);  // PWM引脚低电平
                }
                motor->pulse_state = 0;
                
                // 更新位置
                if (motor->dir == STEPPER_DIR_CW) {
                    motor->position++;
                } else {
                    motor->position--;
                }
                
                // 检查是否达到目标位置
                if ((motor->dir == STEPPER_DIR_CW && motor->position >= motor->target_position) ||
                    (motor->dir == STEPPER_DIR_CCW && motor->position <= motor->target_position)) {
                    // 达到目标位置，停止
                    motor->state = STEPPER_STATE_IDLE;
                    return;
                }
                
                // 根据当前状态计算下一步的延时(为整个周期的延时)
                uint32_t next_delay = motor->step_delay; // 默认保持当前延时
                
                switch (motor->state) {
                    case STEPPER_STATE_ACCELERATING:
                    {
                        // 计算已经走过的加速距离（从加速开始算起）
                        static uint32_t accel_steps_taken = 0;
                        
                        // 每完成一步加速计数增加
                        accel_steps_taken++;
                        
                        // 计算剩余距离 - 用于判断是否需要提前进入减速阶段
                        uint32_t remain_distance;
                        if (motor->dir == STEPPER_DIR_CW) {
                            remain_distance = motor->target_position - motor->position;
                        } else {
                            remain_distance = motor->position - motor->target_position;
                        }
                        // 更新加速阶段的延迟
                        if (accel_steps_taken < motor->accel_steps) {
                            // 仍在加速
                            next_delay = motor->max_step_delay - 
                                ((motor->max_step_delay - motor->min_step_delay) * accel_steps_taken) / motor->accel_steps;
                                
                            // 确保延时不小于最小延时
                            if (next_delay < motor->min_step_delay) {
                                next_delay = motor->min_step_delay;
                            }
                            
                            // 检查是否需要提前进入减速阶段（剩余距离小于等于加速步数）
                            if (remain_distance <= motor->accel_steps) {
                                motor->state = STEPPER_STATE_DECELERATING;
                                accel_steps_taken = 0; // 重置加速步数计数
                            }
                        } else {
                            // 加速完成，进入匀速阶段
                            next_delay = motor->min_step_delay;
                            motor->state = STEPPER_STATE_RUNNING;
                            accel_steps_taken = 0; // 重置加速步数计数
                            
                            // 检查是否需要立即开始减速
                            if (remain_distance <= motor->accel_steps) {
                                motor->state = STEPPER_STATE_DECELERATING;
                            }
                        }
                        break;
                    }
                    
                    case STEPPER_STATE_RUNNING:
                    {
                        // 匀速运动，检查是否需要开始减速
                        next_delay = motor->min_step_delay; // 保持最大速度
                        
                        uint32_t remain_distance;
                        if (motor->dir == STEPPER_DIR_CW) {
                            remain_distance = motor->target_position - motor->position;
                        } else {
                            remain_distance = motor->position - motor->target_position;
                        }
                        
                        if (remain_distance <= motor->accel_steps) {
                            motor->state = STEPPER_STATE_DECELERATING;
                        }
                        break;
                    }
                    
                    case STEPPER_STATE_DECELERATING:
                    {
                        // 计算剩余距离（用于减速）
                        uint32_t decel_distance;
                        if (motor->dir == STEPPER_DIR_CW) {
                            decel_distance = motor->target_position - motor->position;
                        } else {
                            decel_distance = motor->position - motor->target_position;
                        }
                        
                        // 更新减速阶段的延迟
                        if (decel_distance > 0 && motor->accel_steps > 0) {
                            next_delay = motor->min_step_delay + 
                                ((motor->max_step_delay - motor->min_step_delay) * (motor->accel_steps - decel_distance)) / motor->accel_steps;
                                
                            // 确保延时不大于最大延时
                            if (next_delay > motor->max_step_delay) {
                                next_delay = motor->max_step_delay;
                            }
                        } else {
                            // 减速完成，使用启动速度
                            next_delay = motor->max_step_delay;
                        }
                        break;
                    }
                    
                    default:
                        // 保持当前延迟
                        break;
                }
                
                // 更新步进延时
                motor->step_delay = next_delay;
            }
        }
    }
}

/**
 * @brief 获取步进电机状态
 */
StepperState_t Stepper_GetState(StepperMotor_t* motor)
{
    return motor->state;
}

/**
 * @brief 获取步进电机位置
 */
uint32_t Stepper_GetPosition(StepperMotor_t* motor)
{
    return motor->position;
}

/**
 * @brief 复位步进电机位置计数器
 */
void Stepper_ResetPosition(StepperMotor_t* motor)
{
    motor->position = 0;
    motor->target_position = 0;
}

/**
 * @brief 管理多个步进电机的处理函数
 */
void Stepper_ProcessAllMotors(void)
{
    StepperMotor_t* motor = g_stepper_list;
    
    // 遍历所有电机并处理
    while (motor != NULL) {
        Stepper_Handler(motor);
        motor = motor->next;
    }
}

/**
 * @brief 添加步进电机到管理列表
 */
void Stepper_AddMotor(StepperMotor_t* motor)
{
    if (motor == NULL) {
        return;
    }
    
    // 已经在列表中，不重复添加
    StepperMotor_t* temp = g_stepper_list;
    while (temp != NULL) {
        if (temp == motor) {
            return;
        }
        temp = temp->next;
    }
    
    // 添加到链表头部
    motor->next = g_stepper_list;
    g_stepper_list = motor;
}

/**
 * @brief 从管理列表移除步进电机
 */
void Stepper_RemoveMotor(StepperMotor_t* motor)
{
    if (motor == NULL || g_stepper_list == NULL) {
        return;
    }
    
    // 如果是头节点
    if (g_stepper_list == motor) {
        g_stepper_list = motor->next;
        motor->next = NULL;
        return;
    }
    
    // 查找并移除
    StepperMotor_t* temp = g_stepper_list;
    while (temp != NULL && temp->next != NULL) {
        if (temp->next == motor) {
            temp->next = motor->next;
            motor->next = NULL;
            return;
        }
        temp = temp->next;
    }
}

/**
 * @brief 设置步进电机方向
 */
static void Stepper_SetDirection(StepperMotor_t* motor, StepperDirection_t dir)
{
    motor->dir = dir;
    
    // 设置方向引脚
    if (motor->PinControl != NULL) {
        motor->PinControl(PIN_TYPE_DIR, (dir == STEPPER_DIR_CW) ? 0 : 1);
    }
}

/**
 * @brief 切换步进电机脉冲状态
 * @note 此函数为辅助函数，目前未在内部使用，但保留供将来可能的扩展使用
 */
static void Stepper_TogglePulse(StepperMotor_t* motor)
{
    if (motor->PinControl != NULL) {
        if (motor->pulse_state == 0) {
            motor->PinControl(PIN_TYPE_PWM, 1);  // PWM引脚高电平
            motor->pulse_state = 1;
        } else {
            motor->PinControl(PIN_TYPE_PWM, 0);  // PWM引脚低电平
            motor->pulse_state = 0;
        }
    }
}

/**
 * @brief 根据指定转速匀速运行步进电机(不需要目标位置)
 */
void Stepper_RunSpeed(StepperMotor_t* motor, int32_t speed)
{
    // 检查速度是否为0，如果是则停止电机
    if (speed == 0) {
        Stepper_Stop(motor, 1); // 立即停止
        return;
    }
    
    // 设置方向
    StepperDirection_t dir = (speed > 0) ? STEPPER_DIR_CW : STEPPER_DIR_CCW;
    Stepper_SetDirection(motor, dir);
    
    // 计算步进延时(微秒)
    uint32_t step_delay = 1000000 / abs(speed); // 将步/秒转换为微秒延时
    
    // 设置电机参数
    motor->step_delay = step_delay;
    motor->target_position = (dir == STEPPER_DIR_CW) ? 0xFFFFFFFF : 0; // 设置极端值使电机持续运行
    motor->state = STEPPER_STATE_RUNNING; // 设置为匀速运行状态
    
    // 重置脉冲状态
    motor->pulse_state = 0;
    if (motor->PinControl != NULL) {
        motor->PinControl(PIN_TYPE_PWM, 0);
    }
    
    // 使能电机
    Stepper_Enable(motor, 1);
}

/**
 * @brief 设置限位开关状态
 */
void Stepper_SetLimitSwitches(StepperMotor_t* motor, uint8_t cw_limit, uint8_t ccw_limit)
{
    if (motor == NULL) {
        return;
    }
    
    // 更新限位开关状态
    motor->cw_limit = cw_limit ? 1 : 0;  // 确保值为0或1
    motor->ccw_limit = ccw_limit ? 1 : 0;
}

/**
 * @brief 使能或禁用限位开关功能
 */
void Stepper_EnableLimitSwitches(StepperMotor_t* motor, uint8_t enable)
{
    if (motor == NULL) {
        return;
    }
    
    // 使能或禁用限位开关功能
    motor->limit_enabled = enable ? 1 : 0;
}

/**
 * @brief 回归零点（寻找原点）
 */
void Stepper_GoHome(StepperMotor_t* motor, uint32_t speed)
{
    if (motor == NULL || speed == 0) {
        return;
    }
    
    // 确保使能限位开关功能
    Stepper_EnableLimitSwitches(motor, 1);
    
    // 反转方向以寻找原点开关
    // 注意：使用负速度值，使电机反转
    Stepper_RunSpeed(motor, -((int32_t)speed));
    
    // 注意：此函数仅启动向原点方向的运动
    // 电机会一直运行直到触发反转限位开关
    // 在 Stepper_Handler 中，当检测到限位开关触发时会停止运动
    
    // 当移动停止后（需要在外部检测状态），应用程序应该调用 Stepper_ResetPosition
    // 将当前位置设置为0
}

