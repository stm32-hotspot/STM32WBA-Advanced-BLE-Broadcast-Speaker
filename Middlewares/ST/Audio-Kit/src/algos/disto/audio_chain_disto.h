/**
  ******************************************************************************
  * @file    audio_chain_disto.h
  * @author  MCD Application Team
  * @brief   wrapper of disto algo to match usage inside audio_chain.c
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
#ifndef __AUDIO_CHAIN_DISTO_H
#define __AUDIO_CHAIN_DISTO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

typedef enum
{
  DISTO_CLIPPING_SOFT,
  DISTO_CLIPPING_CUBIC,
  DISTO_TYPE_NB
} clippingType_t;

typedef struct
{
  uint8_t ramType;
  uint8_t type;         /* Type of clipping*/
  float   drive;        /* Drive */
  float   off_axis;     /* Add DC to tune symmetry/asymmetry to enhance even harmonics */
  float   dry_wet;      /* Dry wet mix */
  float   dry_gain;     /* Apply gain on dry to fine tune dry_wet mixing */
  float   post_gain;    /* Final gain */
} disto_static_config_t;


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_disto_factory;
extern const audio_algo_common_t  AudioChainWrp_disto_common;
extern       audio_algo_cbs_t     AudioChainWrp_disto_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_DISTO_H */
