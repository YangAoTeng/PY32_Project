/**
 * @file limit_switch_example.c
 * @brief 限位开关使用示例
 */

#include "bsp_motor.h"
#include "py32f0xx_hal.h"

// 定义电机和限位开关引脚
#define CW_LIMIT_PORT        GPIOA
#define CW_LIMIT_PIN         GPIO_PIN_5
#define CCW_LIMIT_PORT       GPIOA
#define CCW_LIMIT_PIN        GPIO_PIN_6

// 声明步进电机
static StepperMotor_t motor;

/**
 * @brief 限位开关GPIO初始化
 */
static void LimitSwitch_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIO时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // 配置限位开关引脚为输入
    GPIO_InitStruct.Pin = CW_LIMIT_PIN | CCW_LIMIT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // 使用上拉电阻，限位开关闭合时接地
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief 示例初始化函数
 */
void LimitSwitch_Example_Init(void)
{
    // 初始化限位开关GPIO
    LimitSwitch_GPIO_Init();
    
    // 初始化步进电机
    Stepper_Init(&motor, 
                GPIOA, GPIO_PIN_0,  // PWM
                GPIOA, GPIO_PIN_1,  // DIR
                GPIOA, GPIO_PIN_2); // EN
    
    // 设置电机速度参数
    Stepper_SetSpeed(&motor, 2000, 500, 1000);
    
    // 使能限位开关功能
    Stepper_EnableLimitSwitches(&motor, 1);
}

/**
 * @brief 读取限位开关状态并更新到电机
 */
void LimitSwitch_Update(void)
{
    // 读取限位开关状态（低电平有效）
    uint8_t cw_limit_state = (HAL_GPIO_ReadPin(CW_LIMIT_PORT, CW_LIMIT_PIN) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t ccw_limit_state = (HAL_GPIO_ReadPin(CCW_LIMIT_PORT, CCW_LIMIT_PIN) == GPIO_PIN_RESET) ? 1 : 0;
    
    // 更新电机限位开关状态
    Stepper_SetLimitSwitches(&motor, cw_limit_state, ccw_limit_state);
}

/**
 * @brief 运行示例（在主循环中调用）
 */
void LimitSwitch_Example_Run(void)
{
    static uint8_t initialized = 0;
    
    if (!initialized) {
        // 第一次运行时，启动电机向正方向运行
        Stepper_RunSpeed(&motor, 1000); // 以1000步/秒的速度正向运行
        initialized = 1;
    }
    
    // 更新限位开关状态
    LimitSwitch_Update();
    
    // 处理电机
    Stepper_Handler(&motor);
    
    // 检查电机状态
    if (motor.state == STEPPER_STATE_IDLE) {
        // 如果电机停止（可能是由于限位开关触发），切换方向继续运行
        static StepperDirection_t last_dir = STEPPER_DIR_CW;
        
        // 等待500ms
        HAL_Delay(500);
        
        // 切换方向
        if (last_dir == STEPPER_DIR_CW) {
            Stepper_RunSpeed(&motor, -1000); // 以1000步/秒的速度反向运行
            last_dir = STEPPER_DIR_CCW;
        } else {
            Stepper_RunSpeed(&motor, 1000);  // 以1000步/秒的速度正向运行
            last_dir = STEPPER_DIR_CW;
        }
    }
}

/**
 * @brief 外部限位开关状态注册示例
 * 当使用外部获取的限位开关状态时，可以这样注册
 * @param cw_limit 正转限位开关状态（1=触发）
 * @param ccw_limit 反转限位开关状态（1=触发）
 */
void Register_External_LimitSwitches(uint8_t cw_limit, uint8_t ccw_limit)
{
    // 直接更新电机的限位开关状态
    Stepper_SetLimitSwitches(&motor, cw_limit, ccw_limit);
}