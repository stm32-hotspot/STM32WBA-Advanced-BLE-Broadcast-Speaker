/**
  ******************************************************************************
  * @file    nlms_config.h
  * @author  MCD Application Team
  * @brief   config of cmsis nlms
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
#ifndef __NLMS_CONFIG_H
#define __NLMS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/**
* @brief  Library dynamic configuration handler. It contains dynamic parameters.
*/

typedef struct
{
  float   echo_length;
  uint8_t ramType;
}
nlms_static_config_t;

typedef struct
{
  float mu;
}
nlms_dynamic_config_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __NLMS_CONFIG_H */
