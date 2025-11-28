/**
  ******************************************************************************
  * @file    stm32_buff.h
  * @author  MCD Application Team
  * @brief   Header for stm32_buff.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2021) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _STM32_BUFF_CONF_
#define _STM32_BUFF_CONF_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32xxx_voice_bsp.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#define UTIL_BYTE_BUFF_Free(x) (free(x))
#define UTIL_BYTE_BUFF_Alloc(a,x) (malloc(x))
#define UTIL_BYTE_BUFF_ASSERT(...)                 ((void)0)
#define UTIL_BYTE_BUFF_MEM_PULL 0

/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* _STM32_BUFF_CONF_ */

