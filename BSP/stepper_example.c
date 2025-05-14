/**
 * @file stepper_example.c
 * @brief 步进电机驱动库使用示例
 */

#include "bsp_motor.h"
#include "py32f0xx_hal.h"

// 定义系统时间获取函数（这里假设已经实现）
extern uint32_t SoftTimer_GetSystemTime(void);

// 定义两个步进电机
static StepperMotor_t motor1;
static StepperMotor_t motor2;

/**
 * @brief 配置电机控制的GPIO引脚
 */
static void Stepper_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIO时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // 配置电机1的引脚
    // PWM引脚配置
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // DIR引脚配置
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // EN引脚配置
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 配置电机2的引脚
    // PWM引脚配置
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // DIR引脚配置
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // EN引脚配置
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief 步进电机示例初始化
 */
void Stepper_Example_Init(void)
{
    // 配置GPIO引脚
    Stepper_GPIO_Config();
    
    // 初始化电机1
    Stepper_Init(&motor1, 
                GPIOA, GPIO_PIN_0,  // PWM
                GPIOA, GPIO_PIN_1,  // DIR
                GPIOA, GPIO_PIN_2); // EN
    
    // 初始化电机2
    Stepper_Init(&motor2, 
                GPIOB, GPIO_PIN_0,  // PWM
                GPIOB, GPIO_PIN_1,  // DIR
                GPIOB, GPIO_PIN_2); // EN
    
    // 设置电机1速度参数：最大速度2000步/秒，启动速度500步/秒，加速度1000步/秒^2
    Stepper_SetSpeed(&motor1, 2000, 500, 1000);
    
    // 设置电机2速度参数：最大速度1500步/秒，启动速度300步/秒，加速度800步/秒^2
    Stepper_SetSpeed(&motor2, 1500, 300, 800);
}

/**
 * @brief 步进电机示例运行
 */
void Stepper_Example_Run(void)
{
    // 使能电机
    Stepper_Enable(&motor1, 1);
    Stepper_Enable(&motor2, 1);
    
    // 电机1相对运动3200步（顺时针方向）
    Stepper_Move(&motor1, 3200, STEPPER_DIR_CW);
    
    // 电机2相对运动1600步（逆时针方向）
    Stepper_Move(&motor2, 1600, STEPPER_DIR_CCW);
    
    // 电机控制主循环
    while (1) {
        // 处理所有电机
        Stepper_ProcessAllMotors();
        
        // 检查电机1是否到达目标位置
        if (Stepper_GetState(&motor1) == STEPPER_STATE_IDLE &&
            Stepper_GetState(&motor2) == STEPPER_STATE_IDLE) {
            // 两个电机都完成运动，切换方向继续运动
            
            // 等待1秒
            uint32_t start_time = SoftTimer_GetSystemTime();
            while (SoftTimer_GetSystemTime() - start_time < 1000000) {
                // 等待
            }
            
            // 切换方向
            if (motor1.dir == STEPPER_DIR_CW) {
                Stepper_Move(&motor1, 3200, STEPPER_DIR_CCW);
                Stepper_Move(&motor2, 1600, STEPPER_DIR_CW);
            } else {
                Stepper_Move(&motor1, 3200, STEPPER_DIR_CW);
                Stepper_Move(&motor2, 1600, STEPPER_DIR_CCW);
            }
        }
        
        // 可以在这里添加其他控制逻辑，比如按键检测等
    }
}