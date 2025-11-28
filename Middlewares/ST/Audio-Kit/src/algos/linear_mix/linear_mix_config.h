/**
  ******************************************************************************
  * @file    linear_mix_config.h
  * @author  MCD Application Team
  * @brief   config of linear_mix algo
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
#ifndef __LINEAR_MIX_CONFIG_H
#define __LINEAR_MIX_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

/* Exported constants --------------------------------------------------------*/
#define LINEAR_MIX_NB_INPUTS 4U // 10U maxi

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  #if LINEAR_MIX_NB_INPUTS >= 1U
  float gain0;
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 2U
  float gain1;
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 3U
  float gain2;
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 4U
  float gain3;
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 5U
  float gain4;
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 6U
  float gain5;
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 7U
  float gain6;
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 8U
  float gain7;
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 9U
  float gain8;
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 10U
  float gain9;
  #endif
}
linear_mix_dynamic_config_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __LINEAR_MIX_CONFIG_H */
