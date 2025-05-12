#include "msg_fifo.h"

static MSG_FIFO_T g_tMsg;

/**
 * @brief           消息队列初始化
 * 
 */
void bsp_InitMsg(void)
{
	bsp_ClearMsg();
}

/**
 * @brief               将一个消息压入消息队列
 * 
 * @param _MsgCode      消息代码
 * @param _MsgParam     消息参数，可以是一个uint32的数据，可以为地址。
 */
void bsp_PutMsg(uint16_t _MsgCode, uint32_t _MsgParam)
{
	g_tMsg.Buf[g_tMsg.Write].MsgCode = _MsgCode;
	g_tMsg.Buf[g_tMsg.Write].MsgParam = _MsgParam;

	if (++g_tMsg.Write  >= MSG_FIFO_SIZE)
	{
		g_tMsg.Write = 0;
	}
}


/**
 * @brief               将一个消息弹出消息队列
 * 
 * @param _pMsg         消息代码
 * @return uint8_t      1表示取到消息，0表示没有消息
 */
uint8_t bsp_GetMsg(MSG_T *_pMsg)
{
	MSG_T *p;

	if (g_tMsg.Read == g_tMsg.Write)
	{
		return 0;
	}
	else
	{
		p = &g_tMsg.Buf[g_tMsg.Read];

		if (++g_tMsg.Read >= MSG_FIFO_SIZE)
		{
			g_tMsg.Read = 0;
		}
		
		_pMsg->MsgCode = p->MsgCode;
		_pMsg->MsgParam = p->MsgParam;
		return 1;
	}
}

/**
 * @brief               将一个消息弹出消息队列，第二个read指针，可以用于多线程使用
 * 
 * @param _pMsg         消息代码
 * @return uint8_t      1表示取到消息，0表示没有消息
 */
uint8_t bsp_GetMsg2(MSG_T *_pMsg)
{
	MSG_T *p;

	if (g_tMsg.Read2 == g_tMsg.Write)
	{
		return 0;
	}
	else
	{
		p = &g_tMsg.Buf[g_tMsg.Read2];

		if (++g_tMsg.Read2 >= MSG_FIFO_SIZE)
		{
			g_tMsg.Read2 = 0;
		}
		
		_pMsg->MsgCode = p->MsgCode;
		_pMsg->MsgParam = p->MsgParam;
		return 1;
	}
}

/**
 * @brief           清空消息队列
 * 
 */
void bsp_ClearMsg(void)
{
	g_tMsg.Read = g_tMsg.Write;
}
