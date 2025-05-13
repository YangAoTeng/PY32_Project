#include "bsp_flash.h"

/**
 * @brief  向Flash写入数据
 * @param  data: 要写入的数据缓冲区指针
 * @param  len: 数据长度(字节)
 * @retval 0: 成功; 1: 失败
 * @note   函数会先擦除目标区域再写入
 */
uint8_t BSP_Flash_Write(uint8_t *data, uint32_t len)
{
    uint32_t addr, i;
    uint32_t word;
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    
    /* 参数检查 */
    if (len > FLASH_DATA_SIZE)
    {
        return 1;  /* 超出范围 */
    }
    
    /* 使用固定的目标地址 */
    addr = FLASH_DATA_ADDR;
    
    /* 解锁Flash */
    HAL_FLASH_Unlock();
    
    /* 配置擦除参数 */
    /* 注意：根据不同的MCU和HAL库版本，Flash擦除参数和API可能有所不同 */
    /* 以下是常见的PY32系列MCU的擦除配置，可能需要根据实际硬件进行调整 */
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGEERASE;
    EraseInitStruct.PageAddress = addr;
    EraseInitStruct.NbPages = 1; /* 擦除一页，具体页大小根据芯片而定 */
    
    /* 擦除Flash */
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 1; /* 擦除失败 */
    }
    
    /* 按字(4字节)写入数据 */
    for (i = 0; i < len; i += 4)
    {
        /* 准备4字节数据 */
        word = 0;
        if (i < len) word |= ((uint32_t)data[i] << 0);
        if (i + 1 < len) word |= ((uint32_t)data[i + 1] << 8);
        if (i + 2 < len) word |= ((uint32_t)data[i + 2] << 16);
        if (i + 3 < len) word |= ((uint32_t)data[i + 3] << 24);
        
        /* 写入数据 */
        if (HAL_FLASH_Program(FLASH_TYPEERASE_PAGEERASE, addr + i, &word) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return 1;  /* 写入失败 */
        }
    }
    
    /* 锁定Flash */
    HAL_FLASH_Lock();
    
    return 0;  /* 成功 */
}

/**
 * @brief  从Flash读取数据
 * @param  data: 存储读取数据的缓冲区指针
 * @param  len: 要读取的数据长度(字节)
 * @retval 0: 成功; 1: 失败
 */
uint8_t BSP_Flash_Read(uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint8_t *flash_addr;
    
    /* 参数检查 */
    if (len > FLASH_DATA_SIZE)
    {
        return 1;  /* 超出范围 */
    }
    
    /* 使用固定的FLASH_DATA_ADDR作为源地址 */
    flash_addr = (uint8_t*)FLASH_DATA_ADDR;
    
    /* 读取数据 */
    for (i = 0; i < len; i++)
    {
        data[i] = flash_addr[i];
    }
    
    return 0;  /* 成功 */
}
