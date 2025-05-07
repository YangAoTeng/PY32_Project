/**
 * @file ring_buffer.c
 * @brief 环形缓冲区模块实现文件
 * @author Copilot AND PlayerPencil
 * @date 2025/5/7
 */

#include "ring_buffer.h"

/**
 * @brief 初始化环形缓冲区
 * @param rb      环形缓冲区结构体指针
 * @param buffer  用户提供的缓冲区数组
 * @param size    缓冲区大小
 * @return 成功返回1，失败返回0
 */
uint8_t RingBuffer_Init(RingBuffer_t *rb, uint8_t *buffer, uint16_t size)
{
    if (rb == NULL || buffer == NULL || size == 0)
    {
        return 0;
    }
    
    rb->buffer = buffer;
    rb->size = size;
    rb->read_index = 0;
    rb->write_index = 0;
    rb->count = 0;
    
    return 1;
}

/**
 * @brief 向环形缓冲区写入一个字节
 * @param rb      环形缓冲区结构体指针
 * @param data    要写入的数据
 * @return 成功返回1，失败返回0
 */
uint8_t RingBuffer_Write(RingBuffer_t *rb, uint8_t data)
{
    /* 判断指针是否有效 */
    if (rb == NULL)
    {
        return 0;
    }
    
    /* 判断缓冲区是否已满 */
    if (rb->count >= rb->size)
    {
        return 0;
    }
    
    /* 写入数据 */
    rb->buffer[rb->write_index] = data;
    
    /* 更新写指针，如果到达缓冲区尾部则回到头部 */
    rb->write_index = (rb->write_index + 1) % rb->size;
    
    /* 更新计数值 */
    rb->count++;
    
    return 1;
}

/**
 * @brief 从环形缓冲区读取一个字节
 * @param rb      环形缓冲区结构体指针
 * @param data    读取的数据存放位置
 * @return 成功返回1，失败返回0
 */
uint8_t RingBuffer_Read(RingBuffer_t *rb, uint8_t *data)
{
    /* 判断指针是否有效 */
    if (rb == NULL || data == NULL)
    {
        return 0;
    }
    
    /* 判断缓冲区是否为空 */
    if (rb->count == 0)
    {
        return 0;
    }
    
    /* 读取数据 */
    *data = rb->buffer[rb->read_index];
    
    /* 更新读指针，如果到达缓冲区尾部则回到头部 */
    rb->read_index = (rb->read_index + 1) % rb->size;
    
    /* 更新计数值 */
    rb->count--;
    
    return 1;
}

/**
 * @brief 向环形缓冲区写入多个字节
 * @param rb      环形缓冲区结构体指针
 * @param data    要写入的数据数组
 * @param len     要写入的数据长度
 * @return 实际写入的字节数
 */
uint16_t RingBuffer_WriteMultiple(RingBuffer_t *rb, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint16_t count = 0;
    
    /* 判断指针是否有效 */
    if (rb == NULL || data == NULL)
    {
        return 0;
    }
    
    /* 循环写入数据 */
    for (i = 0; i < len; i++)
    {
        if (RingBuffer_Write(rb, data[i]))
        {
            count++;
        }
        else
        {
            /* 缓冲区已满，退出 */
            break;
        }
    }
    
    return count;
}

/**
 * @brief 从环形缓冲区读取多个字节
 * @param rb      环形缓冲区结构体指针
 * @param data    读取的数据存放数组
 * @param len     要读取的数据长度
 * @return 实际读取的字节数
 */
uint16_t RingBuffer_ReadMultiple(RingBuffer_t *rb, uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint16_t count = 0;
    
    /* 判断指针是否有效 */
    if (rb == NULL || data == NULL)
    {
        return 0;
    }
    
    /* 循环读取数据 */
    for (i = 0; i < len; i++)
    {
        if (RingBuffer_Read(rb, &data[i]))
        {
            count++;
        }
        else
        {
            /* 缓冲区已空，退出 */
            break;
        }
    }
    
    return count;
}

/**
 * @brief 检查环形缓冲区是否为空
 * @param rb      环形缓冲区结构体指针
 * @return 为空返回1，否则返回0
 */
uint8_t RingBuffer_IsEmpty(RingBuffer_t *rb)
{
    if (rb == NULL)
    {
        return 1;
    }
    
    return (rb->count == 0) ? 1 : 0;
}

/**
 * @brief 检查环形缓冲区是否已满
 * @param rb      环形缓冲区结构体指针
 * @return 已满返回1，否则返回0
 */
uint8_t RingBuffer_IsFull(RingBuffer_t *rb)
{
    if (rb == NULL)
    {
        return 1;
    }
    
    return (rb->count >= rb->size) ? 1 : 0;
}

/**
 * @brief 获取环形缓冲区中的数据量
 * @param rb      环形缓冲区结构体指针
 * @return 数据量
 */
uint16_t RingBuffer_GetCount(RingBuffer_t *rb)
{
    if (rb == NULL)
    {
        return 0;
    }
    
    return rb->count;
}

/**
 * @brief 获取环形缓冲区中的空闲空间
 * @param rb      环形缓冲区结构体指针
 * @return 空闲空间大小
 */
uint16_t RingBuffer_GetFree(RingBuffer_t *rb)
{
    if (rb == NULL)
    {
        return 0;
    }
    
    return rb->size - rb->count;
}

/**
 * @brief 清空环形缓冲区
 * @param rb      环形缓冲区结构体指针
 */
void RingBuffer_Clear(RingBuffer_t *rb)
{
    if (rb == NULL)
    {
        return;
    }
    
    rb->read_index = 0;
    rb->write_index = 0;
    rb->count = 0;
}

// /* 定义环形缓冲区结构体 */
// static RingBuffer_t test_buffer;
// /* 定义环形缓冲区数组 */
// static uint8_t buffer_array[RING_BUFFER_SIZE];
// /* 测试数据 */
// static uint8_t test_data[] = "Ring Buffer Test Data";

// /* 定义缓冲区大小 */
// #define RING_BUFFER_SIZE             64

  // /* 初始化环形缓冲区 */
  // if (RingBuffer_Init(&test_buffer, buffer_array, RING_BUFFER_SIZE)) {
  //   printf("Ring buffer initialized successfully!\r\n");
  // } else {
  //   printf("Ring buffer initialization failed!\r\n");
  //   APP_ErrorHandler();
  // }

// /**
//   * @brief  环形缓冲区测试回调函数
//   * @param  param 回调函数参数
//   * @retval None
//   */
//  void RingBuffer_Test_Callback(void *param)
//  {
//    uint8_t read_data[RING_BUFFER_SIZE];
//    uint16_t written, read, i;
   
//    printf("\r\n----- Ring Buffer Test Start -----\r\n");
   
//    /* 清空缓冲区 */
//    RingBuffer_Clear(&test_buffer);
   
//    /* 向环形缓冲区写入测试数据 */
//    written = RingBuffer_WriteMultiple(&test_buffer, test_data, sizeof(test_data)-1);
//    printf("Written %d bytes to ring buffer\r\n", written);
   
//    /* 显示缓冲区状态 */
//    printf("Buffer count: %d, Buffer free: %d\r\n", 
//           RingBuffer_GetCount(&test_buffer), 
//           RingBuffer_GetFree(&test_buffer));
   
//    /* 从环形缓冲区读取数据 */
//    read = RingBuffer_ReadMultiple(&test_buffer, read_data, written);
//    printf("Read %d bytes from ring buffer\r\n", read);
   
//    /* 打印读取的数据 */
//    printf("Read data: ");
//    for (i = 0; i < read; i++) {
//      printf("%c", read_data[i]);
//    }
//    printf("\r\n");
   
//    /* 尝试写入超出缓冲区大小的数据 */
//    printf("\r\nTesting buffer overflow protection:\r\n");
   
//    /* 先填充缓冲区接近满 */
//    for (i = 0; i < RING_BUFFER_SIZE - 5; i++) {
//      RingBuffer_Write(&test_buffer, 'A' + (i % 26));
//    }
   
//    printf("Buffer count after filling: %d\r\n", RingBuffer_GetCount(&test_buffer));
   
//    /* 尝试写入更多数据 */
//    uint8_t overflow_test[] = "OVERFLOW_TEST";
//    written = RingBuffer_WriteMultiple(&test_buffer, overflow_test, sizeof(overflow_test)-1);
//    printf("Attempted to write %d bytes more, actually written: %d\r\n", 
//           sizeof(overflow_test)-1, written);
   
//    /* 检查缓冲区是否已满 */
//    if (RingBuffer_IsFull(&test_buffer)) {
//      printf("Buffer is full now.\r\n");
//    }
   
//    /* 清空缓冲区 */
//    RingBuffer_Clear(&test_buffer);
//    printf("Buffer cleared. Is empty: %d\r\n", RingBuffer_IsEmpty(&test_buffer));
   
//    printf("----- Ring Buffer Test End -----\r\n\r\n");
//  }
 