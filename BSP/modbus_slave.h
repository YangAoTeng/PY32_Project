#ifndef __MODBUY_SLAVE_H
#define __MODBUY_SLAVE_H
#include "main.h"
#include "msg_fifo.h"


/*
	01 读取线圈寄存器（可以当作输出）			05 写单个线圈寄存器（）
	02 读取输入状态寄存器（可以当作输入）		
	03 读取保持寄存器（可以当作输入）			06 写单个保持寄存器（）   10 写多个保持寄存器
	04 读取输入寄存器（模拟信号）
*/
// #define SADDR485	01
// #define SBAUD485	115200

/* 01H 读强制单线圈 */
/* 05H 写强制单线圈 */
#define D_COIL_SIZE    32   /* 定义线圈数组大小 */
#define REG_D_START    0x00 /* 线圈起始地址 */
#define REG_D_END     (REG_D_START + D_COIL_SIZE - 1) /* 线圈结束地址 */

/* 02H 读输入状态 */
#define T_INPUT_SIZE    32   /* 定义输入状态数组大小 */
#define REG_T_START    0x00 /* 输入状态起始地址 */
#define REG_T_END     (REG_T_START + T_INPUT_SIZE - 1) /* 输入状态结束地址 */

/* 03H 读保持寄存器 */
/* 06H 写保持寄存器 */
/* 10H 写多个保存寄存器 */
#define P_REG_SIZE    32   /* 定义保持寄存器数组大小 */
#define REG_P_START    0x00 /* 保持寄存器起始地址 */
#define REG_P_END     (REG_P_START + P_REG_SIZE - 1) /* 保持寄存器结束地址 */

/* 04H 读取输入寄存器(模拟信号) */
#define A_REG_SIZE    32   /* 定义模拟量寄存器数组大小 */
#define REG_A_START    0x00 /* 模拟量寄存器起始地址 */
#define REG_A_END     (REG_A_START + A_REG_SIZE - 1) /* 模拟量寄存器结束地址 */


/* RTU 应答代码 */
#define RSP_OK				0		/* 成功 */
#define RSP_ERR_CMD			0x01	/* 不支持的功能码 */
#define RSP_ERR_REG_ADDR	0x02	/* 寄存器地址错误 */
#define RSP_ERR_VALUE		0x03	/* 数据值域错误 */
#define RSP_ERR_WRITE		0x04	/* 写入失败 */

#define S_RX_BUF_SIZE		30
#define S_TX_BUF_SIZE		128

typedef struct
{
	uint8_t Addr;		

	uint8_t RxBuf[S_RX_BUF_SIZE];
	uint8_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t RspCode;

	uint8_t TxBuf[S_TX_BUF_SIZE];
	uint8_t TxCount;
}MODS_T;



typedef struct
{	/* 03H 06H 读写保持寄存器 */
	uint16_t P[P_REG_SIZE];    /* 支持最多32个保持寄存器 */

	/* 04H 读取模拟量寄存器 */
	int16_t A[A_REG_SIZE];    /* 支持最多32个模拟量寄存器 */

	/* 01H 05H 读写单个强制线圈 */
	uint16_t D[D_COIL_SIZE];    /* 支持最多32个线圈状态 */
	
	/* 02H 读取输入状态 */
	uint8_t T[T_INPUT_SIZE];    /* 支持最多32个输入状态 */

}VAR_T;

extern MSG_FIFO_T g_tModS_Fifo;
extern VAR_T g_tVar;
extern SystemParam_t g_tSysParam;
enum{
	MSG_MODS_NONE = 0,
	MSG_MODS_01H = 0x01,
	MSG_MODS_02H,
	MSG_MODS_03H,
	MSG_MODS_04H,
	MSG_MODS_05H,
	MSG_MODS_06H,
	MSG_MODS_10H,
	MSG_MODS_ERR,
	MSG_MODS_END,

};

void MODS_Poll(void);
void MODS_Init(void);
extern MODS_T g_tModS;
extern VAR_T g_tVar;

uint8_t MODS_ReadRegister(uint8_t reg_type, uint16_t index, void *value);
uint8_t MODS_WriteRegister(uint8_t reg_type, uint16_t index, uint16_t value);
#endif

