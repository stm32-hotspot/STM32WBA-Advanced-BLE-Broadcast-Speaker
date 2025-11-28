/**
******************************************************************************
* @file    audio_chain_linear_mix_factory.c
* @author  MCD Application Team
* @brief   factory of mix algo
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
#include "audio_chain_linear_mix.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

#define LINEAR_MIX_PARAM_DESCR(i)                                               \
  {                                                                             \
    .pDescription    = AUDIO_ALGO_OPT_STR("linear gain for entry "#i),          \
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),                       \
    .pDefault        = "1.0",                                                   \
    .pName           = "gain"#i,                                                \
    AUDIO_DESC_PARAM_F(linear_mix_dynamic_config_t, gain##i, -10.0f, +10.0f)    \
  }

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_linear_mix_dynamicParamsDesc[LINEAR_MIX_NB_INPUTS] =
{
  #if LINEAR_MIX_NB_INPUTS >= 1U
  LINEAR_MIX_PARAM_DESCR(0),
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 2U
  LINEAR_MIX_PARAM_DESCR(1),
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 3U
  LINEAR_MIX_PARAM_DESCR(2),
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 4U
  LINEAR_MIX_PARAM_DESCR(3),
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 5U
  LINEAR_MIX_PARAM_DESCR(4),
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 6U
  LINEAR_MIX_PARAM_DESCR(5),
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 7U
  LINEAR_MIX_PARAM_DESCR(6),
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 8U
  LINEAR_MIX_PARAM_DESCR(7),
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 9U
  LINEAR_MIX_PARAM_DESCR(8),
  #endif
  #if LINEAR_MIX_NB_INPUTS >= 10U
  LINEAR_MIX_PARAM_DESCR(9),
  #endif
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_linear_mix_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_linear_mix_dynamicParamsDesc,
  .nbParams = sizeof(s_linear_mix_dynamicParamsDesc) / sizeof(s_linear_mix_dynamicParamsDesc[0]),
  .szBytes  = sizeof(linear_mix_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_linear_mix_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_linear_mix_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_linear_mix_common,
  .pExecutionCbs         = &AudioChainWrp_linear_mix_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_linear_mix_factory);
