/**
  ******************************************************************************
  * @file    bsp_at.h
  * @author  MCU Application Team
  * @brief   AT命令解析处理模块头文件
  ******************************************************************************
  */

#ifndef __BSP_AT_H__
#define __BSP_AT_H__

#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* AT命令缓冲区大小定义 */
#define AT_CMD_BUFFER_SIZE     128
#define AT_MAX_COMMANDS        20
#define AT_CMD_HEADER          "AT+"
#define AT_CMD_TERMINATOR      "\r\n"

/* AT命令处理结果 */
typedef enum {
    AT_RESULT_OK = 0,          /* 命令执行成功 */
    AT_RESULT_ERROR,           /* 命令执行错误 */
    AT_RESULT_UNKNOWN_CMD,     /* 未知命令 */
    AT_RESULT_INVALID_PARAM,   /* 参数错误 */
    AT_RESULT_BUSY             /* 系统忙 */
} AT_Result_t;

/* AT命令回调函数类型定义 */
typedef AT_Result_t (*AT_CmdHandler_t)(char *cmd);

/* AT命令结构体定义 */
typedef struct {
    char *cmd;                 /* 命令名称（不包含AT头） */
    AT_CmdHandler_t handler;   /* 命令处理函数 */
    char *help;                /* 命令帮助信息 */
} AT_Command_t;

/* 函数声明 */
void AT_Init(void);
void AT_RegisterCommand(const char *cmd, AT_CmdHandler_t handler, const char *help);
void AT_ProcessRxData(uint8_t data);
void AT_SendResponse(AT_Result_t result, const char *info);

#endif /* __BSP_AT_H__ */
