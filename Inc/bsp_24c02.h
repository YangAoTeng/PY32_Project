#ifndef __BSP_24C02_H__
#define __BSP_24C02_H__

#include "main.h"
#include "bsp_i2c.h"

/* 24C02参数定义 */
#define EEPROM_DEV_ADDR          0xA0      /* 24C02设备地址: 1010 000x b (左移1位后为0xA0) */
#define EEPROM_PAGE_SIZE         8         /* 24C02的页大小，每页8字节 */
#define EEPROM_SIZE              256       /* 24C02总容量为256字节 */
#define EEPROM_WRITE_TIMEOUT     5         /* 写周期延迟，单位ms */

/* 函数声明 */
HAL_StatusTypeDef EEPROM_Init(void);
HAL_StatusTypeDef EEPROM_WriteByte(uint16_t addr, uint8_t data);
uint8_t EEPROM_ReadByte(uint16_t addr);
HAL_StatusTypeDef EEPROM_WriteBytes(uint16_t addr, uint8_t *data, uint16_t len);
HAL_StatusTypeDef EEPROM_ReadBytes(uint16_t addr, uint8_t *data, uint16_t len);
HAL_StatusTypeDef EEPROM_PageWrite(uint16_t addr, uint8_t *data, uint8_t len);
HAL_StatusTypeDef EEPROM_WaitEepromStandbyState(void);

#endif /* __BSP_24C02_H__ */
