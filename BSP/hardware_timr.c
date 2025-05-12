#include "hardware_timr.h"


/* 保存 TIM定时中断到后执行的回调函数指针 */
static void (*s_TIM_CallBack1)(void);
static void (*s_TIM_CallBack2)(void);
static void (*s_TIM_CallBack3)(void);
static void (*s_TIM_CallBack4)(void);

/**
 * @brief  初始化硬件定时器
 * 
 */
void bsp_InitHardTimer(void)
{
    TIM_HandleTypeDef  TimHandle = {0};
	uint32_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;
	
    __HAL_RCC_TIM3_CLK_ENABLE();
	
	uiTIMxCLK = SystemCoreClock;

	usPrescaler = uiTIMxCLK / 1000000 - 1;	/* 分频比 = 1 */
	

    usPeriod = 0xFFFF;
	

	/* 
       设置分频为usPrescaler后，那么定时器计数器计1次就是1us
       而参数usPeriod的值是决定了最大计数：
       usPeriod = 0xFFFF 表示最大0xFFFF微秒。
       usPeriod = 0xFFFFFFFF 表示最大0xFFFFFFFF微秒。
    */
	TimHandle.Instance = TIM3;
	TimHandle.Init.Prescaler         = usPrescaler;
	TimHandle.Init.Period            = usPeriod;
	TimHandle.Init.ClockDivision     = 0;
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;
    TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	
	if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
	{
        printf("HAL_TIM_Base_Init error\n");
	}

	/* 配置定时器中断，给CC捕获比较中断使用 */
	{
		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 2);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
    
    /* 启动定时器 */
	HAL_TIM_Base_Start(&TimHandle);
}

/**
 * @brief                   启动硬件定时器
 *                          定时时间到后执行回调函数。可以同时启动4个定时器通道，互不干扰。
 *                          是16位定时器，最大定时时间为65535us，即65.535ms
 * 
 * @param _CC               定时器通道  1~4
 * @param _uiTimeOut        超时时间, 单位 1us. 对于16位定时器，最大 65.5ms; 对于32位定时器，最大 4294秒
 * @param _pCallBack        定时时间到后，被执行的函数
 */
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack)
{
    uint32_t cnt_now;
    uint32_t cnt_tar;
	TIM_TypeDef* TIMx = TIM3;
	
    /* 无需补偿延迟，实测精度正负1us */
    
    cnt_now = TIMx->CNT; 
    cnt_tar = cnt_now + _uiTimeOut;			/* 计算捕获的计数器值 */
    if (_CC == 1)
    {
        // printf("bsp_StartHardTimer: %d, %d\n", cnt_now, cnt_tar);
        s_TIM_CallBack1 = (void (*)(void))_pCallBack;

		TIMx->CCR1 = cnt_tar; 			    /* 设置捕获比较计数器CC1 */
        TIMx->SR = (uint16_t)~TIM_IT_CC1;   /* 清除CC1中断标志 */
		TIMx->DIER |= TIM_IT_CC1;			/* 使能CC1中断 */

	}
    else if (_CC == 2)
    {
		s_TIM_CallBack2 = (void (*)(void))_pCallBack;

		TIMx->CCR2 = cnt_tar;				/* 设置捕获比较计数器CC2 */
        TIMx->SR = (uint16_t)~TIM_IT_CC2;	/* 清除CC2中断标志 */
		TIMx->DIER |= TIM_IT_CC2;			/* 使能CC2中断 */
    }
    else if (_CC == 3)
    {
        s_TIM_CallBack3 = (void (*)(void))_pCallBack;

		TIMx->CCR3 = cnt_tar;				/* 设置捕获比较计数器CC3 */
        TIMx->SR = (uint16_t)~TIM_IT_CC3;	/* 清除CC3中断标志 */
		TIMx->DIER |= TIM_IT_CC3;			/* 使能CC3中断 */
    }
    else if (_CC == 4)
    {
        s_TIM_CallBack4 = (void (*)(void))_pCallBack;

		TIMx->CCR4 = cnt_tar;				/* 设置捕获比较计数器CC4 */
        TIMx->SR = (uint16_t)~TIM_IT_CC4;	/* 清除CC4中断标志 */
		TIMx->DIER |= TIM_IT_CC4;			/* 使能CC4中断 */
    }
	else
    {
        return;
    }
}

/**
 * @brief                       TIM3中断处理函数
 * 
 */
void TIM3_IRQHandler(void)
{
	uint16_t itstatus = 0x0, itenable = 0x0;
	TIM_TypeDef* TIMx = TIM3;
    
  	itstatus = TIMx->SR & TIM_IT_CC1;
	itenable = TIMx->DIER & TIM_IT_CC1;
	if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
	{
		TIMx->SR = (uint16_t)~TIM_IT_CC1;
		TIMx->DIER &= (uint16_t)~TIM_IT_CC1;		/* 禁能CC1中断 */	
        // DEBUG_PRINTF(LEVEL_DEBUG, "TIM3->CCR1: %d\n", TIMx->CCR1);
        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack1();
    }

	itstatus = TIMx->SR & TIM_IT_CC2;
	itenable = TIMx->DIER & TIM_IT_CC2;
	if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
	{
		TIMx->SR = (uint16_t)~TIM_IT_CC2;
		TIMx->DIER &= (uint16_t)~TIM_IT_CC2;		/* 禁能CC2中断 */	

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack2();
    }

	itstatus = TIMx->SR & TIM_IT_CC3;
	itenable = TIMx->DIER & TIM_IT_CC3;
	if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
	{
		TIMx->SR = (uint16_t)~TIM_IT_CC3;
		TIMx->DIER &= (uint16_t)~TIM_IT_CC3;		/* 禁能CC2中断 */	

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack3();
    }

	itstatus = TIMx->SR & TIM_IT_CC4;
	itenable = TIMx->DIER & TIM_IT_CC4;
	if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
	{
		TIMx->SR = (uint16_t)~TIM_IT_CC4;
		TIMx->DIER &= (uint16_t)~TIM_IT_CC4;		/* 禁能CC4中断 */	

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack4();
    }	
}

/**
 * @brief                       TIM1更新、断路、触发和换相中断处理函数
 * 
 */
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
    TIM_TypeDef* TIMx = TIM1;
    
    /* 检查是否发生了更新中断 */
    if ((TIMx->SR & TIM_IT_UPDATE) != 0 && (TIMx->DIER & TIM_IT_UPDATE) != 0)
    {
        TIMx->SR = (uint16_t)~TIM_IT_UPDATE;  /* 清除更新中断标志 */
        /* 这里可以添加更新中断的处理代码 */
    }
    
    /* 其他中断（断路、触发、换相）的处理可以根据需要添加 */
}

