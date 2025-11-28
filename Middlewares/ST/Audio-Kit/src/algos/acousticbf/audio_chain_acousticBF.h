/**
  ******************************************************************************
  * @file    audio_chain_acousticBF.h
  * @author  MCD Application Team
  * @brief   wrapper of acoustic beamforming algo to match usage inside audio_chain.c
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
#ifndef __AUDIO_CHAIN_ACOUSTICBF_H
#define __AUDIO_CHAIN_ACOUSTICBF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "acoustic_bf.h"


/* Exported constants --------------------------------------------------------*/
#define AC_BF_DIR_FRONT                  0x00U
#define AC_BF_DIR_REAR                   0x01U

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint16_t control_refresh; /* if 0 control is not active, if n will be call every n*frame size */
  uint16_t mic_distance;
  uint8_t  type;
  uint8_t  mixer_enable;
  float    tLowDb;
  float    tHighDb;
  uint8_t  ramType;
} acousticBF_static_config_t;


typedef struct
{
  int16_t  volume;
  uint8_t  direction;
  uint8_t  ref_select;   /*!< Select a stream in the reference channel; none
  or the omnidirectional microphone reference or the opposite antenna. */
} acousticBF_dynamic_config_t;

//typedef struct
//{
//  float    energy_mic_db;    /*!< Energy of first raw microphone */
//}
//acousticBF_control_t;

/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_acousticBF_factory;
extern const audio_algo_common_t  AudioChainWrp_acousticBF_common;
extern       audio_algo_cbs_t     AudioChainWrp_acousticBF_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_ACOUSTICBF_H */
