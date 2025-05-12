#ifndef __MSG_FIFO_H
#define __MSG_FIFO_H
#include "main.h"

#define MSG_FIFO_SIZE    10	   		/* 消息个数 */


/* 按键FIFO用到变量 */
typedef struct
{
	uint16_t MsgCode;		/* 消息代码 */
	uint32_t MsgParam;		/* 消息的数据体, 也可以是指针（强制转化） */
}MSG_T;

/* 按键FIFO用到变量 */
typedef struct
{
	MSG_T Buf[MSG_FIFO_SIZE];	/* 消息缓冲区 */
	uint8_t Read;					/* 缓冲区读指针1 */
	uint8_t Write;					/* 缓冲区写指针 */
	uint8_t Read2;					/* 缓冲区读指针2 */
}MSG_FIFO_T;

/* 供外部调用的函数声明 */
void bsp_InitMsg(MSG_FIFO_T *_pFifo);
void bsp_PutMsg(MSG_FIFO_T *_pFifo, uint16_t _MsgCode, uint32_t _MsgParam);
uint8_t bsp_GetMsg(MSG_FIFO_T *_pFifo, MSG_T *_pMsg);
uint8_t bsp_GetMsg2(MSG_FIFO_T *_pFifo, MSG_T *_pMsg);
void bsp_ClearMsg(MSG_FIFO_T *_pFifo);

#endif  // __MSG_FIFO_H