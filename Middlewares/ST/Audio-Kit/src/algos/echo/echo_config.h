/**
  ******************************************************************************
  * @file    echo_config.h
  * @author  MCD Application Team
  * @brief   produce echo
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
#ifndef __ECHO_CONFIG_H
#define __ECHO_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

/* Exported constants --------------------------------------------------------*/
//#define ECHO_CONFIG_WITH_STEREO_PARAM

/* Exported types ------------------------------------------------------------*/
/**
* @brief  Library dynamic configuration handler. It contains dynamic parameters.
*/
typedef struct
{
  float   delay;
  uint8_t ramType;
} echoStaticConfig_t;

typedef struct
{
  float feedback;
  float level;
  #ifdef ECHO_CONFIG_WITH_STEREO_PARAM
  float stereo;
  #endif
} echoDynamicConfig_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __ECHO_CONFIG_H */
