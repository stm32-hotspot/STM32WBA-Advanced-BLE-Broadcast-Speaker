/**
  ******************************************************************************
  * @file    stm32_errors.h
  * @author  MCD Application Team
  * @brief   Header for stm32_error.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_ERRORS_H
#define __STM32_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#define UTIL_ERROR_NONE        (0)
#define UTIL_ERROR             (1)
#define UTIL_ERROR_BUSY        (2)
#define UTIL_ERROR_TIMEOUT_HAL (3)
#define UTIL_ERROR_ALLOC       (4)
#define UTIL_ERROR_OVERRUN     (5)
#define UTIL_ERROR_UNDERRUN    (6)
#define UTIL_ERROR_TIMEOUT     (-1) /*Negative value because often used as returned error code in functions that returns nb_data upon success*/


/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __STM32_ERRORS_H */
