/**
  ******************************************************************************
  * @file    bsp_24c02.c
  * @author  MCU Application Team
  * @brief   24C02 EEPROM驱动实现
  ******************************************************************************
  */

#include "bsp_24c02.h"

/**
  * @brief  初始化24C02 EEPROM
  * @param  None
  * @retval HAL状态
  */
HAL_StatusTypeDef EEPROM_Init(void)
{
  /* 初始化I2C总线 */
  return BSP_I2C_Init();
}

/**
  * @brief  向24C02指定地址写入一个字节
  * @param  addr: 数据地址(0~255)
  * @param  data: 要写入的数据
  * @retval HAL状态
  */
HAL_StatusTypeDef EEPROM_WriteByte(uint16_t addr, uint8_t data)
{
  HAL_StatusTypeDef status;
  
  /* 使用I2C接口写入一个字节 */
  status = BSP_I2C_WriteReg(EEPROM_DEV_ADDR, addr, &data, 1);
  
  /* 等待写入完成 */
  if (status == HAL_OK)
  {
    HAL_Delay(EEPROM_WRITE_TIMEOUT);
    status = EEPROM_WaitEepromStandbyState();
  }
  
  return status;
}

/**
  * @brief  从24C02指定地址读取一个字节
  * @param  addr: 数据地址(0~255)
  * @retval 读取到的数据
  */
uint8_t EEPROM_ReadByte(uint16_t addr)
{
  uint8_t data = 0;
  
  /* 使用I2C接口读取一个字节 */
  BSP_I2C_ReadReg(EEPROM_DEV_ADDR, addr, &data, 1);
  
  return data;
}

/**
  * @brief  向24C02写入多个字节
  * @param  addr: 起始地址(0~255)
  * @param  data: 数据缓冲区指针
  * @param  len: 数据长度
  * @retval HAL状态
  * @note   此函数会根据页写入限制自动处理跨页写入
  */
HAL_StatusTypeDef EEPROM_WriteBytes(uint16_t addr, uint8_t *data, uint16_t len)
{
  uint16_t i, addr_cur;
  uint8_t page_offset, page_remain;
  HAL_StatusTypeDef status = HAL_OK;
  
  addr_cur = addr;
  
  while (len > 0 && status == HAL_OK)
  {
    /* 计算当前页内偏移 */
    page_offset = addr_cur % EEPROM_PAGE_SIZE;
    
    /* 计算当前页剩余空间 */
    page_remain = EEPROM_PAGE_SIZE - page_offset;
    
    /* 判断是否需要跨页 */
    if (len <= page_remain)
    {
      /* 不需要跨页，一次写完 */
      page_remain = len;
    }
    
    /* 执行页写入 */
    status = EEPROM_PageWrite(addr_cur, data, page_remain);
    
    /* 更新参数，准备下一页写入 */
    if (status == HAL_OK)
    {
      len -= page_remain;
      addr_cur += page_remain;
      data += page_remain;
    }
  }
  
  return status;
}

/**
  * @brief  从24C02读取多个字节
  * @param  addr: 起始地址(0~255)
  * @param  data: 数据缓冲区指针
  * @param  len: 数据长度
  * @retval HAL状态
  */
HAL_StatusTypeDef EEPROM_ReadBytes(uint16_t addr, uint8_t *data, uint16_t len)
{
  /* 使用I2C接口连续读取多个字节 */
  return BSP_I2C_ReadReg(EEPROM_DEV_ADDR, addr, data, len);
}

/**
  * @brief  24C02页写入
  * @param  addr: 页内起始地址
  * @param  data: 数据缓冲区指针
  * @param  len: 数据长度，不能超过页大小
  * @retval HAL状态
  * @note   24C02每页8字节，本函数不检查跨页情况
  */
HAL_StatusTypeDef EEPROM_PageWrite(uint16_t addr, uint8_t *data, uint8_t len)
{
  HAL_StatusTypeDef status;
  
  /* 数据长度检查 */
  if (len > EEPROM_PAGE_SIZE)
  {
    len = EEPROM_PAGE_SIZE;
  }
  
  /* 使用I2C接口写入数据 */
  status = BSP_I2C_WriteReg(EEPROM_DEV_ADDR, addr, data, len);
  
  /* 等待写入完成 */
  if (status == HAL_OK)
  {
    HAL_Delay(EEPROM_WRITE_TIMEOUT);
    status = EEPROM_WaitEepromStandbyState();
  }
  
  return status;
}

/**
  * @brief  等待EEPROM就绪
  * @param  None
  * @retval HAL状态
  */
HAL_StatusTypeDef EEPROM_WaitEepromStandbyState(void)
{
  /* 检查EEPROM是否就绪，最多尝试1000次 */
  return BSP_I2C_IsReady(EEPROM_DEV_ADDR, 1000);
}

// static uint8_t eeprom_test_buf[256];  /* EEPROM测试缓冲区 - 改为256字节 */

// /**
//   * @brief  EEPROM测试函数
//   * @param  None
//   * @retval None
//   */
//  static void APP_EEPROM_Test(void)
//  {
//    uint16_t i;
//    uint8_t write_buf[256];  /* 增加到256字节 */
//    HAL_StatusTypeDef status;
   
//    /* 初始化写入缓冲区，每个字节的值等于其地址 */
//    for (i = 0; i < 256; i++)
//    {
//      write_buf[i] = (uint8_t)i;  /* 值等于地址，0-255 */
//    }
   
//    printf("\r\n========== 24C02 EEPROM Test (Hardware I2C) ==========\r\n");
//    printf("I2C SCL: PA3 (AF12), SDA: PA7 (AF12)\r\n");
   
//    /* 初始化EEPROM */
//    status = EEPROM_Init();
//    if (status != HAL_OK)
//    {
//      printf("EEPROM initialize failed! (status: %d)\r\n", status);
//      return;
//    }
   
//    /* 测试单字节写入和读取 */
//    printf("Test Single Byte Write/Read:\r\n");
//    status = EEPROM_WriteByte(0x00, 0xA5);
//    if (status == HAL_OK)
//    {
//      printf("Write 0xA5 to address 0x00 success\r\n");
     
//      uint8_t read_data = EEPROM_ReadByte(0x00);
//      printf("Read from address 0x00: 0x%02X\r\n", read_data);
     
//      if (read_data == 0xA5)
//      {
//        printf("Single Byte Test PASS!\r\n");
//      }
//      else
//      {
//        printf("Single Byte Test FAIL!\r\n");
//      }
//    }
//    else
//    {
//      printf("Write to EEPROM failed! (status: %d)\r\n", status);
//    }
   
//    /* 测试全部地址范围的写入和读取 */
//    printf("\r\nTest Full EEPROM Write/Read (255 bytes):\r\n");
//    printf("Writing 255 bytes from address 0x00...\r\n");
   
//    /* 打印前16个要写入的数据 */
//    printf("First 16 bytes to write: ");
//    for (i = 0; i < 16; i++)
//    {
//      printf("%02X ", write_buf[i]);
//    }
//    printf("...\r\n");
   
//    /* 执行全字节写入 */
//    status = EEPROM_WriteBytes(0x00, write_buf, 255);
//    if (status == HAL_OK)
//    {
//      printf("Full EEPROM write success!\r\n");
     
//      /* 清空缓冲区 */
//      for (i = 0; i < 256; i++)
//      {
//        eeprom_test_buf[i] = 0;
//      }
     
//      /* 执行全字节读取 */
//      printf("Reading 255 bytes from address 0x00...\r\n");
//      status = EEPROM_ReadBytes(0x00, eeprom_test_buf, 255);
//      if (status == HAL_OK)
//      {
//        /* 打印前16个读取的数据 */
//        printf("First 16 bytes read: ");
//        for (i = 0; i < 16; i++)
//        {
//          printf("%02X ", eeprom_test_buf[i]);
//        }
//        printf("...\r\n");
       
//        /* 验证所有数据是否一致 */
//        uint8_t error = 0;
//        uint16_t error_count = 0;
       
//        for (i = 0; i < 255; i++)
//        {
//          if (eeprom_test_buf[i] != write_buf[i])
//          {
//            error = 1;
//            error_count++;
//            /* 打印前10个错误 */
//            if (error_count <= 10)
//            {
//              printf("Data mismatch at address 0x%02X: expected 0x%02X, got 0x%02X\r\n", 
//                     i, write_buf[i], eeprom_test_buf[i]);
//            }
//          }
//        }
       
//        if (error)
//        {
//          printf("Full EEPROM Test FAIL! %d errors found.\r\n", error_count);
//        }
//        else
//        {
//          printf("Full EEPROM Test PASS! All 255 bytes verified.\r\n");
//        }
//      }
//      else
//      {
//        printf("Read from EEPROM failed! (status: %d)\r\n", status);
//      }
//    }
//    else
//    {
//      printf("Write to EEPROM failed! (status: %d)\r\n", status);
//    }
   
//    printf("======================================\r\n");
//  }
 