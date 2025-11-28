/**
  ******************************************************************************
  * @file    audio_faust_flanger.h
  * @author  MCD Application Team
  * @brief   produce sound faust-flanger
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
#ifndef __AUDIO_FAUST_FLANGER_H
#define __AUDIO_FAUST_FLANGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

/* Exported types ------------------------------------------------------------*/
/**
* @brief  Library dynamic configuration handler. It contains dynamic parameters.
*/

typedef struct
{
  uint8_t ramType;
} faust_flanger_static_config_t;

typedef struct
{
  float  bypass;
  float  invert_flange_sum;
  float  speed;
  float  depth;
  float  feedback;
  float  flange_delay;
  float  delay_offset;
  float  flanger_output_level;

} faust_flanger_dynamic_config_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_FAUST_FLANGER_H */
