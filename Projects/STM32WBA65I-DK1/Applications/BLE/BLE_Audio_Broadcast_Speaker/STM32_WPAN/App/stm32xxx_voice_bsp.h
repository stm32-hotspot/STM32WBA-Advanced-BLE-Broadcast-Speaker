/**
  ******************************************************************************
  * @file    stm32xxx_voice_bsp.h
  * @author  MCD Application Team
  * @brief   Header for cube_hal.c module
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
#ifndef __STM32XXX_VOICE_BSP__
#define __STM32XXX_VOICE_BSP__

#ifdef __cplusplus
extern "C" {
#endif


/* Conf ----------------------------------------------------------------------*/
#define STM32_NAME_STRING  "STM32WBA65I-DK"

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32wbaxx.h"



/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

#define HAL_GetCpuClockFreq()     HAL_RCC_GetSysClockFreq()


/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif //__STM32XXX_VOICE_BSP__
