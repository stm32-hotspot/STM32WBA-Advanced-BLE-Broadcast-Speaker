/**
  ******************************************************************************
  * @file    audio_faust_phaser.h
  * @author  MCD Application Team
  * @brief   produce sound faust-phaser
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
#ifndef __AUDIO_FAUST_PHASER_H
#define __AUDIO_FAUST_PHASER_H

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
} faust_phaser_static_config_t;

typedef struct
{
  float  bypass;
  float  invert_internal_phaser_sum;
  float  vibrato_mode;
  float  speed;
  float  notch_depth__intensity_;
  float  feedback_gain;
  float  notch_width;
  float  min_notch1_freq;
  float  max_notch1_freq;
  float  notch_freq_ratio;
  float  phaser_output_level;

} faust_phaser_dynamic_config_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_FAUST_PHASER_H */
