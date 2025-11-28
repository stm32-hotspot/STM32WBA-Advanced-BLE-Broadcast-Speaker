/**
  ******************************************************************************
  * @file    audio_chain_faust_phaser.h
  * @author  MCD Application Team
  * @brief   produce a  faust-phaser effect
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
#ifndef __AUDIO_CHAIN_FAUST_PHASER_H
#define __AUDIO_CHAIN_FAUST_PHASER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "faust/audio_faust_phaser.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_faust_phaser_factory;
extern const audio_algo_common_t  AudioChainWrp_faust_phaser_common;
extern       audio_algo_cbs_t     AudioChainWrp_faust_phaser_cbs;
extern       int32_t              AudioChainWrp_faust_phaser_deinit_ext(audio_algo_t *const pAlgo);
extern       int32_t              AudioChainWrp_faust_phaser_init_ext(audio_algo_t *const pAlgo);
extern       int32_t              AudioChainWrp_faust_phaser_process_ext(audio_algo_t *const pAlgo);

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_FAUST_PHASER_H */
