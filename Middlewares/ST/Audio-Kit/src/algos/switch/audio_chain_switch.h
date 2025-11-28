/**
  ******************************************************************************
  * @file    audio_chain_switch.h
  * @author  MCD Application Team
  * @brief   wrapper of switch algo (select 1 input chunk) to match usage inside audio_chain.c
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
#ifndef __AUDIO_CHAIN_SWITCH_H
#define __AUDIO_CHAIN_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "switch_config.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define SWITCH_NB_INPUT 4U
#define SWITCH_INPUT_ID "inputId"

/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_switch_factory;
extern const audio_algo_common_t  AudioChainWrp_switch_common;
extern       audio_algo_cbs_t     AudioChainWrp_switch_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_SWITCH_H */
