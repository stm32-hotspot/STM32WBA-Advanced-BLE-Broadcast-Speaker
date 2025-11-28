/**
  ******************************************************************************
  * @file    audio_chain_acousticBF.h
  * @author  MCD Application Team
  * @brief   wrapper of acoustic beamforming cardiod algo to match usage inside audio_chain.c
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
#ifndef __AUDIO_CHAIN_ACOUSTICBF_CARDOID_H
#define __AUDIO_CHAIN_ACOUSTICBF_CARDOID_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "acoustic_bf_cardoid.h"


#define ACOUSTIC_BF_CARDOID_DIRECTION_FRONT      0x00U
#define ACOUSTIC_BF_CARDOID_DIRECTION_REAR       0x01U


typedef struct
{
  uint16_t mic_distance;
  uint8_t  rear_enable;
  uint8_t  delay_enable;
  uint8_t  ramType;
} acousticBF_cardoid_static_config_t;


typedef struct
{
  uint8_t  ref_select;
  int16_t  volume;
  uint8_t  direction;
} acousticBF_cardoid_dynamic_config_t;

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_acousticBF_cardoid_factory;
extern const audio_algo_common_t  AudioChainWrp_acousticBF_cardoid_common;
extern       audio_algo_cbs_t     AudioChainWrp_acousticBF_cardoid_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_ACOUSTICBF_CARDOID_H */
