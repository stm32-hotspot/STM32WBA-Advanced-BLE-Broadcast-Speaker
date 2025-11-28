/**
  ******************************************************************************
  * @file    audio_chain_capture.h
  * @author  MCD Application Team
  * @brief   wrapper of capture algo (capture estimation from input time samples) to match usage inside audio_chain.c
  *          remark: this algo has no output samples
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
#ifndef __AUDIO_CHAIN_CAPTURE_H
#define __AUDIO_CHAIN_CAPTURE_H

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "capture/capture_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_capture_factory;
extern const audio_algo_common_t  AudioChainWrp_capture_common;
extern       audio_algo_cbs_t     AudioChainWrp_capture_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_SPECTRUM_H */
