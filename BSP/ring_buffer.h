/**
 * @file ring_buffer.h
 * @brief 环形缓冲区模块头文件
 * @author Copilot AND PlayerPencil
 * @date 2025/5/7
 */

#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include "main.h"

/* 环形缓冲区结构体定义 */
typedef struct {
    uint8_t *buffer;      /* 缓冲区数组指针 */
    uint16_t size;        /* 缓冲区大小 */
    uint16_t read_index;  /* 读指针 */
    uint16_t write_index; /* 写指针 */
    uint16_t count;       /* 当前数据计数 */
} RingBuffer_t;

/**
 * @brief 初始化环形缓冲区
 * @param rb      环形缓冲区结构体指针
 * @param buffer  用户提供的缓冲区数组
 * @param size    缓冲区大小
 * @return 成功返回1，失败返回0
 */
uint8_t RingBuffer_Init(RingBuffer_t *rb, uint8_t *buffer, uint16_t size);

/**
 * @brief 向环形缓冲区写入一个字节
 * @param rb      环形缓冲区结构体指针
 * @param data    要写入的数据
 * @return 成功返回1，失败返回0
 */
uint8_t RingBuffer_Write(RingBuffer_t *rb, uint8_t data);

/**
 * @brief 从环形缓冲区读取一个字节
 * @param rb      环形缓冲区结构体指针
 * @param data    读取的数据存放位置
 * @return 成功返回1，失败返回0
 */
uint8_t RingBuffer_Read(RingBuffer_t *rb, uint8_t *data);

/**
 * @brief 向环形缓冲区写入多个字节
 * @param rb      环形缓冲区结构体指针
 * @param data    要写入的数据数组
 * @param len     要写入的数据长度
 * @return 实际写入的字节数
 */
uint16_t RingBuffer_WriteMultiple(RingBuffer_t *rb, const uint8_t *data, uint16_t len);

/**
 * @brief 从环形缓冲区读取多个字节
 * @param rb      环形缓冲区结构体指针
 * @param data    读取的数据存放数组
 * @param len     要读取的数据长度
 * @return 实际读取的字节数
 */
uint16_t RingBuffer_ReadMultiple(RingBuffer_t *rb, uint8_t *data, uint16_t len);

/**
 * @brief 检查环形缓冲区是否为空
 * @param rb      环形缓冲区结构体指针
 * @return 为空返回1，否则返回0
 */
uint8_t RingBuffer_IsEmpty(RingBuffer_t *rb);

/**
 * @brief 检查环形缓冲区是否已满
 * @param rb      环形缓冲区结构体指针
 * @return 已满返回1，否则返回0
 */
uint8_t RingBuffer_IsFull(RingBuffer_t *rb);

/**
 * @brief 获取环形缓冲区中的数据量
 * @param rb      环形缓冲区结构体指针
 * @return 数据量
 */
uint16_t RingBuffer_GetCount(RingBuffer_t *rb);

/**
 * @brief 获取环形缓冲区中的空闲空间
 * @param rb      环形缓冲区结构体指针
 * @return 空闲空间大小
 */
uint16_t RingBuffer_GetFree(RingBuffer_t *rb);

/**
 * @brief 清空环形缓冲区
 * @param rb      环形缓冲区结构体指针
 */
void RingBuffer_Clear(RingBuffer_t *rb);


/**
 * @brief 环形缓冲区测试回调函数
 * @param param 回调函数参数
 */
void RingBuffer_Test_Callback(void *param);

#endif /* __RING_BUFFER_H */