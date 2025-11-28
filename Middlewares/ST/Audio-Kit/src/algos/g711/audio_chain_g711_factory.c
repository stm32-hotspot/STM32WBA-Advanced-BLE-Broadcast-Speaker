/**
  ******************************************************************************
  * @file    audio_chain_g711_factory.c
  * @author  MCD Application Team
  * @brief   factory of G711 (encode & decode) algo
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
#include "audio_chain_g711_enc.h"
#include "audio_chain_g711_dec.h"

const audio_algo_factory_t AudioChainWrp_g711_dec_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_g711_dec_common,
  .pExecutionCbs         = &AudioChainWrp_g711_dec_cbs
};

const audio_algo_factory_t AudioChainWrp_g711_enc_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_g711_enc_common,
  .pExecutionCbs         = &AudioChainWrp_g711_enc_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_g711Dec_factory);
// ALGO_FACTORY_DECLARE(AudioChainWrp_g711Enc_factory);
