/**
  ******************************************************************************
  * @file    fnlms_config.h
  * @author  MCD Application Team
  * @brief    frequency nlms algo
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

#ifndef __FNLMS_CONFIG_H
#define __FNLMS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint16_t nbTaps;
  uint8_t  ramType;
}
fnlms_static_config_t;

typedef struct
{
  float mu;
}
fnlms_dynamic_config_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif // __FNLMS_CONFIG_H
