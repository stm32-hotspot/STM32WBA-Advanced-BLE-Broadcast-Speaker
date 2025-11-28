/**
  ******************************************************************************
  * @file    audio_chain_fdisjoin.h
  * @author  MCD Application Team
  * @brief   frequency disjoin allows to select a sub set of contiguous bands
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
#ifndef __AUDIO_CHAIN_FDISJOIN_H
#define __AUDIO_CHAIN_FDISJOIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

/* Exported types ------------------------------------------------------------*/

typedef struct
{
  uint16_t begId;
  uint16_t endId;
}
fdisjoin_static_config_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_fdisjoin_factory;
extern const audio_algo_common_t  AudioChainWrp_fdisjoin_common;
extern       audio_algo_cbs_t     AudioChainWrp_fdisjoin_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_FDISJOIN_H */
