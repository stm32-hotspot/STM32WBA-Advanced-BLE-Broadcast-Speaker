/**
  ******************************************************************************
  * @file    audio_chain_signal_generator.h
  * @author  MCD Application Team
  * @brief   wrapper of signal_generator algo to match usage inside audio_chain.c
  *          remark: this algo has no input samples
  *******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ********************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_CHAIN_SIGNAL_GENERATOR_H
#define __AUDIO_CHAIN_SIGNAL_GENERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "common/biquad.h"
#include "signal_generator_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define SIGNAL_GENERATOR_FREQ_MARGIN 10UL
#define SIGNAL_GENERATOR_FREQUENCY "frequency"

/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_signal_generator_factory;
extern const audio_algo_common_t  AudioChainWrp_signal_generator_common;
extern       audio_algo_cbs_t     AudioChainWrp_signal_generator_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_SIGNAL_GENERATOR_H */
