#ifndef __HARDWARE_TIMR_H
#define __HARDWARE_TIMR_H
#include "main.h"

void bsp_InitHardTimer(void);
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack);

#endif  // __HARDWARE_TIMR_H