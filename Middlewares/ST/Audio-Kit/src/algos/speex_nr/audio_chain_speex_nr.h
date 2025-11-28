/**
  ******************************************************************************
  * @file    audio_chain_speex_nr.h
  * @author  MCD Application Team
  * @brief   wrapper of speex_nr algo to match usage inside audio_chain.c
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
#ifndef __AUDIO_CHAIN_SPEEX_NR_H
#define __AUDIO_CHAIN_SPEEX_NR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

typedef struct
{
  uint8_t ramType;
} speex_nr_static_config_t;

typedef struct
{
  float  agc; /*AGC (Automatic Gain Control) effect : from 0 (no AGC effect) to 1 (max AGC effect)*/
  int8_t noise_suppress; /*noise_suppress (dB): see Speex.org*/
} speex_nr_dynamic_config_t;


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_speex_nr_factory;
extern const audio_algo_common_t  AudioChainWrp_speex_nr_common;
extern       audio_algo_cbs_t     AudioChainWrp_speex_nr_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_SPEEX_NR_H */
