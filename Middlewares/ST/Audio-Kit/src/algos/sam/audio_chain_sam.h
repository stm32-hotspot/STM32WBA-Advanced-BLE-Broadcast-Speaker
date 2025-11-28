/**
  ******************************************************************************
  * @file    audio_chain_sam.h
  * @author  MCD Application Team
  * @brief   frequency sam algorithm
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
#ifndef __AUDIO_CHAIN_SAM_H
#define __AUDIO_CHAIN_SAM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "sam_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern audio_algo_factory_t      AudioChainWrp_sam_factory;
extern const audio_algo_common_t AudioChainWrp_sam_common;
extern       audio_algo_cbs_t    AudioChainWrp_sam_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_SAM_H */
