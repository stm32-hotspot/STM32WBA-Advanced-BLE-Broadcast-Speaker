/**
  ******************************************************************************
  * @file    audio_chain_pdm2pcm.h
  * @author  MCD Application Team
  * @brief   wrapper of pdm2pcm algo to match usage inside audio_chain.c
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
#ifndef __AUDIO_CHAIN_PDM2PCM_H
#define __AUDIO_CHAIN_PDM2PCM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

#include "pdm2pcm.h"

typedef struct
{
  uint8_t ramType;
  uint8_t from_i2s;
  float   high_pass_tap;
} pdm2pcm_static_config_t;


typedef struct
{
  int16_t gain;
} pdm2pcm_dynamic_config_t;

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_pdm2pcm_factory;
extern const audio_algo_common_t  AudioChainWrp_pdm2pcm_common;
extern       audio_algo_cbs_t     AudioChainWrp_pdm2pcm_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_PDM2PCM_H */
