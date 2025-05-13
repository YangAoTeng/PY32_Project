#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H
#include "main.h"

/* Flash相关常量定义 */
#define FLASH_BASE_ADDR      0x08000000                  /* Flash基地址 */
#define FLASH_DATA_ADDR      (FLASH_BASE_ADDR + 31*1024) /* 第31K位置的起始地址 */
#define FLASH_DATA_SIZE      1024                        /* 数据区大小：1K */

/* 函数声明 */
uint8_t BSP_Flash_Write(uint8_t *data, uint32_t len);
uint8_t BSP_Flash_Read(uint8_t *data, uint32_t len);

#endif  // __BSP_FLASH_H
