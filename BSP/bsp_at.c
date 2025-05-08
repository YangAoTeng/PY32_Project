/**
  ******************************************************************************
  * @file    bsp_at.c
  * @author  MCU Application Team
  * @brief   AT命令解析处理模块
  ******************************************************************************
  */
#if  BSP_AT_EN == 1

#include "bsp_at.h"
#include "ring_buffer.h"

/* AT命令数组 */
static AT_Command_t at_commands[AT_MAX_COMMANDS];
static uint8_t at_cmd_count = 0;

/* AT命令接收缓冲区 */
static char at_rx_buffer[AT_CMD_BUFFER_SIZE];
static uint16_t at_rx_index = 0;
static uint8_t is_receiving_cmd = 0;

static void AT_ProcessCommand(char *cmd);

/**
  * @brief  初始化AT命令处理模块
  * @param  None
  * @retval None
  */
void AT_Init(void)
{
    /* 清空命令数组 */
    memset(at_commands, 0, sizeof(at_commands));
    at_cmd_count = 0;
    
    /* 清空接收缓冲区 */
    memset(at_rx_buffer, 0, sizeof(at_rx_buffer));
    at_rx_index = 0;
    is_receiving_cmd = 0;
    
    printf("AT command module initialized\r\n");
}

/**
  * @brief  注册AT命令
  * @param  cmd: 命令名称（不包含AT前缀）
  * @param  handler: 命令处理函数
  * @param  help: 命令帮助信息
  * @retval None
  */
void AT_RegisterCommand(const char *cmd, AT_CmdHandler_t handler, const char *help)
{
    /* 检查命令数量是否已达上限 */
    if (at_cmd_count >= AT_MAX_COMMANDS)
    {
        printf("AT command register failed: command table full\r\n");
        return;
    }
    
    /* 检查参数有效性 */
    if (cmd == NULL || handler == NULL)
    {
        printf("AT command register failed: invalid parameters\r\n");
        return;
    }
    
    /* 注册命令 */
    at_commands[at_cmd_count].cmd = (char *)cmd;
    at_commands[at_cmd_count].handler = handler;
    at_commands[at_cmd_count].help = (char *)help;
    at_cmd_count++;
    
    printf("AT command registered: %s\r\n", cmd);
}

/**
  * @brief  处理接收到的数据
  * @param  data: 接收到的字节数据
  * @retval None
  */
void AT_ProcessRxData(uint8_t data)
{
    /* 缓冲区溢出保护 */
    if (at_rx_index >= AT_CMD_BUFFER_SIZE - 1)
    {
        at_rx_index = 0;
        is_receiving_cmd = 0;
        memset(at_rx_buffer, 0, sizeof(at_rx_buffer));
        return;
    }
    
    /* 保存数据到缓冲区 */
    at_rx_buffer[at_rx_index++] = data;
    at_rx_buffer[at_rx_index] = '\0';  /* 确保字符串结束 */
    
    /* 检查是否开始接收命令（AT+开头） */
    if (at_rx_index >= strlen(AT_CMD_HEADER) && !is_receiving_cmd)
    {
        if (strncmp(at_rx_buffer, AT_CMD_HEADER, strlen(AT_CMD_HEADER)) == 0)
        {
            is_receiving_cmd = 1;
        }
        else
        {
            /* 不是AT+命令，重置缓冲区 */
            at_rx_index = 0;
            memset(at_rx_buffer, 0, sizeof(at_rx_buffer));
        }
    }
    
    /* 检查是否接收到完整命令（以\r\n结尾） */
    if (is_receiving_cmd && at_rx_index >= 2)
    {
        if (at_rx_buffer[at_rx_index - 2] == '\r' && at_rx_buffer[at_rx_index - 1] == '\n')
        {
            /* 处理接收到的命令 */
            AT_ProcessCommand(at_rx_buffer);
            
            /* 重置接收状态 */
            at_rx_index = 0;
            is_receiving_cmd = 0;
            memset(at_rx_buffer, 0, sizeof(at_rx_buffer));
        }
    }
}

/**
  * @brief  处理接收到的AT命令
  * @param  cmd: 完整的AT命令字符串
  * @retval None
  */
static void AT_ProcessCommand(char *cmd)
{
    uint8_t i;
    AT_Result_t result = AT_RESULT_UNKNOWN_CMD;
    char *cmd_body;
    
    /* 检查是否以AT+开头 */
    if (strncmp(cmd, AT_CMD_HEADER, strlen(AT_CMD_HEADER)) != 0)
    {
        AT_SendResponse(AT_RESULT_ERROR, "Not an AT+ command");
        return;
    }
    
    /* 获取命令主体（跳过AT+前缀） */
    cmd_body = cmd + strlen(AT_CMD_HEADER);
    
    /* 去除尾部的\r\n */
    char *end = strstr(cmd_body, AT_CMD_TERMINATOR);
    if (end)
    {
        *end = '\0';
    }
    
    /* 调试输出 */
    printf("\r\nReceived command: '%s'\r\n", cmd_body);
    
    /* 搜索命令 */
    for (i = 0; i < at_cmd_count; i++)
    {
        /* 检查命令主体是否匹配注册的命令，或者命令主体是否以注册的命令加等号开头（带参数情况） */
        if (strcmp(cmd_body, at_commands[i].cmd) == 0 ||
            (strncmp(cmd_body, at_commands[i].cmd, strlen(at_commands[i].cmd)) == 0 &&
             cmd_body[strlen(at_commands[i].cmd)] == '='))
        {
            /* 调用对应的处理函数 */
            result = at_commands[i].handler(cmd_body);
            break;
        }
    }
    
    /* 发送处理结果 */
    if (i == at_cmd_count)
    {
        AT_SendResponse(AT_RESULT_UNKNOWN_CMD, cmd_body);
    }
    else
    {
        AT_SendResponse(result, NULL);
    }
}

/**
  * @brief  发送AT命令响应
  * @param  result: 命令处理结果
  * @param  info: 附加信息（可为NULL）
  * @retval None
  */
void AT_SendResponse(AT_Result_t result, const char *info)
{
    switch (result)
    {
        case AT_RESULT_OK:
            printf("\r\nOK\r\n");
            break;
        
        case AT_RESULT_ERROR:
            if (info)
                printf("\r\nERROR: %s\r\n", info);
            else
                printf("\r\nERROR\r\n");
            break;
        
        case AT_RESULT_UNKNOWN_CMD:
            printf("\r\nERROR: Unknown command");
            if (info)
                printf(" '%s'", info);
            printf("\r\n");
            break;
        
        case AT_RESULT_INVALID_PARAM:
            printf("\r\nERROR: Invalid parameter");
            if (info)
                printf(" '%s'", info);
            printf("\r\n");
            break;
        
        case AT_RESULT_BUSY:
            printf("\r\nERROR: System busy\r\n");
            break;
        
        default:
            printf("\r\nUnknown result code\r\n");
            break;
    }
}

#if 0

  /* 注册AT命令 */
  AT_RegisterCommand("HELP", AT_HandleHelp, "Show available commands");
  AT_RegisterCommand("INFO", AT_HandleInfo, "Show system information");
  AT_RegisterCommand("LED", AT_HandleLed, "Control LED, usage: AT+LED=ON/OFF");

/**
  * @brief  AT+HELP命令处理函数
  * @param  cmd: 命令字符串
  * @retval AT命令处理结果
  */
 static AT_Result_t AT_HandleHelp(char *cmd)
 {
     printf("\r\nAvailable Commands:\r\n");
     printf("  AT+HELP - Show this help\r\n");
     printf("  AT+INFO - Show system information\r\n");
     printf("  AT+LED=ON/OFF - Control on-board LED\r\n");
     return AT_RESULT_OK;
 }
 
 /**
   * @brief  AT+INFO命令处理函数
   * @param  cmd: 命令字符串
   * @retval AT命令处理结果
   */
 static AT_Result_t AT_HandleInfo(char *cmd)
 {
     printf("\r\nSystem Information:\r\n");
     printf("  MCU: PY32 Series\r\n");
     printf("  Clock: %d Hz\r\n", SystemCoreClock);
     printf("  Compile Date: %s\r\n", __DATE__);
     printf("  Compile Time: %s\r\n", __TIME__);
     return AT_RESULT_OK;
 }
 
 /**
   * @brief  AT+LED命令处理函数
   * @param  cmd: 命令字符串
   * @retval AT命令处理结果
   */
 static AT_Result_t AT_HandleLed(char *cmd)
 {
     /* 检查是否有参数 */
     char *param = strchr(cmd, '=');
     if (param == NULL)
     {
         return AT_RESULT_INVALID_PARAM;
     }
     
     /* 跳过等号 */
     param++;
     
     /* 处理参数 */
     if (strcmp(param, "ON") == 0)
     {
         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
         printf("\r\nLED turned ON\r\n");
         return AT_RESULT_OK;
     }
     else if (strcmp(param, "OFF") == 0)
     {
         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
         printf("\r\nLED turned OFF\r\n");
         return AT_RESULT_OK;
     }
     else
     {
         return AT_RESULT_INVALID_PARAM;
     }
 }

 #endif

 #endif /* BSP_AT_EN == 1 */
 