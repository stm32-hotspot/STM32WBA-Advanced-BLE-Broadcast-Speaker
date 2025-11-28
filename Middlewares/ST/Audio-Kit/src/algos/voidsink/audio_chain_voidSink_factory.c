/**
  ******************************************************************************
  * @file    audio_chain_voidSink_factory.c
  * @author  MCD Application Team
  * @brief   void sink factory
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
#include "audio_chain_voidSink.h"

const audio_algo_factory_t AudioChainWrp_voidSink_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_voidSink_common,
  .pExecutionCbs         = &AudioChainWrp_voidSink_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_voidSink_factory);
