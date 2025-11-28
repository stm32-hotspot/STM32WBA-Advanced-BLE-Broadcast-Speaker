/**
  ******************************************************************************
  * @file    cycles_conf.h
  * @author  MCD Application Team
  * @brief   Header for configuration of cycles.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CYCLES_CONF_H
#define __CYCLES_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#ifdef STM32L4
#include "stm32l4xx.h"
#elif defined(STM32F7)
#include "stm32f7xx.h"
#elif defined(STM32H7)
#include "stm32h7xx.h"
#elif defined(STM32H573xx)
#include "stm32h5xx.h"
#elif defined(STM32U5)
#include "stm32u5xx.h"
#else
#include "stm32f4xx.h"
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
//#define CYCLES_CNT_PARALLEL_MEASURE_MAX_NB 20U /*Default value is 10U */
#define CYCLES_CNT_TRACES_USED
//#define CYCLES_CNT_PRINTF_USED  /* CYCLES_CNT_TRACES_USED & CYCLES_CNT_PRINTF_USED are exclusive; please select one or the other*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __CYCLES_CONF_H */
