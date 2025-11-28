/**
  ******************************************************************************
  * @file    stm32xx_hal_timebase_tim_conf.h
  * @author  MCD Application Team
  * @brief   Header for stm32xx_hal_timebase_tim.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32XX_HAL_TIMEBASE_TIM_CONF_H
#define __STM32XX_HAL_TIMEBASE_TIM_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Exported constants --------------------------------------------------------*/
#define TIMER_BASE_IRQHandler TIM6_IRQHandler
#define TIMER_BASE_IRQn       TIM6_IRQn

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#define HAL_RCC_GetClockConfig_nl(a)     {uint32_t pFLatency;HAL_RCC_GetClockConfig(a,&pFLatency);UNUSED(pFLatency);}


/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __STM32XX_HAL_TIMEBASE_TIM_CONF_H */

