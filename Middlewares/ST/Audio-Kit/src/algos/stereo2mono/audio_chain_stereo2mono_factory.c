/**
******************************************************************************
* @file    audio_chain_stereo2mono_factory.c
* @author  MCD Application Team
* @brief   factory of gain algo to match usage inside audio_chain.c
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
#include "audio_chain_stereo2mono.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_stereo2mono_dynamicParamsDesc[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("left/right balance"),
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault        = "0",
    .pName           = "balance",
    AUDIO_DESC_PARAM_F(stereo2mono_dynamic_config_t, balance, -1.0f, +1.0f)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_stereo2mono_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_stereo2mono_dynamicParamsDesc,
  .nbParams          = sizeof(s_stereo2mono_dynamicParamsDesc) / sizeof(s_stereo2mono_dynamicParamsDesc[0]),
  .szBytes           = sizeof(stereo2mono_dynamic_config_t),
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_stereo2mono_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_stereo2mono_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_stereo2mono_common,
  .pExecutionCbs         = &AudioChainWrp_stereo2mono_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_stereo2mono_factory);
