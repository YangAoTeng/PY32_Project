#include "msg_fifo.h"

/**
 * @brief           消息队列初始化
 * @param _pFifo    消息队列指针
 */
void bsp_InitMsg(MSG_FIFO_T *_pFifo)
{
	bsp_ClearMsg(_pFifo);
}

/**
 * @brief               将一个消息压入消息队列
 * 
 * @param _pFifo        消息队列指针
 * @param _MsgCode      消息代码
 * @param _MsgParam     消息参数，可以是一个uint32的数据，可以为地址。
 */
void bsp_PutMsg(MSG_FIFO_T *_pFifo, uint16_t _MsgCode, uint32_t _MsgParam)
{
	_pFifo->Buf[_pFifo->Write].MsgCode = _MsgCode;
	_pFifo->Buf[_pFifo->Write].MsgParam = _MsgParam;

	if (++_pFifo->Write >= MSG_FIFO_SIZE)
	{
		_pFifo->Write = 0;
	}
}

/**
 * @brief               将一个消息弹出消息队列
 * 
 * @param _pFifo        消息队列指针
 * @param _pMsg         消息代码
 * @return uint8_t      1表示取到消息，0表示没有消息
 */
uint8_t bsp_GetMsg(MSG_FIFO_T *_pFifo, MSG_T *_pMsg)
{
	MSG_T *p;

	if (_pFifo->Read == _pFifo->Write)
	{
		return 0;
	}
	else
	{
		p = &_pFifo->Buf[_pFifo->Read];

		if (++_pFifo->Read >= MSG_FIFO_SIZE)
		{
			_pFifo->Read = 0;
		}
		
		_pMsg->MsgCode = p->MsgCode;
		_pMsg->MsgParam = p->MsgParam;
		return 1;
	}
}

/**
 * @brief               将一个消息弹出消息队列，第二个read指针，可以用于多线程使用
 * 
 * @param _pFifo        消息队列指针
 * @param _pMsg         消息代码
 * @return uint8_t      1表示取到消息，0表示没有消息
 */
uint8_t bsp_GetMsg2(MSG_FIFO_T *_pFifo, MSG_T *_pMsg)
{
	MSG_T *p;

	if (_pFifo->Read2 == _pFifo->Write)
	{
		return 0;
	}
	else
	{
		p = &_pFifo->Buf[_pFifo->Read2];

		if (++_pFifo->Read2 >= MSG_FIFO_SIZE)
		{
			_pFifo->Read2 = 0;
		}
		
		_pMsg->MsgCode = p->MsgCode;
		_pMsg->MsgParam = p->MsgParam;
		return 1;
	}
}

/**
 * @brief               清空消息队列
 * @param _pFifo        消息队列指针
 */
void bsp_ClearMsg(MSG_FIFO_T *_pFifo)
{
	_pFifo->Read = _pFifo->Write;
	_pFifo->Read2 = _pFifo->Write;
}
