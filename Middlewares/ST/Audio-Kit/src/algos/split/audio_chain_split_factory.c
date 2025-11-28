/**
******************************************************************************
* @file    audio_chain_split_factory.c
* @author  MCD Application Team
* @brief   factory of split algo (input chunk split in several output chunks) to match usage inside audio_chain.c
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

/* Includes ------------------------------------------------------------------*/
#include "audio_chain_split.h"

const audio_algo_factory_t AudioChainWrp_split_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_split_common,
  .pExecutionCbs         = &AudioChainWrp_split_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_split_factory);
