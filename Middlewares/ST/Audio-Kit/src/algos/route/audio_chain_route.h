/**
  ******************************************************************************
  * @file    audio_chain_route.h
  * @author  MCD Application Team
  * @brief   wrapper of router algo to match usage inside audio_chain.c
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
#ifndef __AUDIO_CHAIN_ROUTE_H
#define __AUDIO_CHAIN_ROUTE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "route/route_config.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_route_factory;
extern const audio_algo_common_t  AudioChainWrp_route_common;
extern       audio_algo_cbs_t     AudioChainWrp_route_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_ROUTE_H */
