/**
******************************************************************************
* @file    audio_chain_sfc_factory.c
* @author  MCD Application Team
* @brief   factory of SFC (Sample Format Converter) algo to match usage inside audio_chain.c
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
#include "audio_chain_sfc.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_sfc_dynamicParamsDesc[] =
{
  {
    .pDescription             = AUDIO_ALGO_OPT_STR("gain in dB"),
    .pControl                 = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault                 = "0.0",
    .pName                    = "gain",
    AUDIO_DESC_PARAM_F(sfc_dynamic_config_t, gain, -100.0f, +100.0f)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_sfc_dynamicParamTemplate =
{
  .pParam                     = (audio_descriptor_param_t *)s_sfc_dynamicParamsDesc,
  .nbParams                   = sizeof(s_sfc_dynamicParamsDesc) / sizeof(s_sfc_dynamicParamsDesc[0]),
  .szBytes                    = sizeof(sfc_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_sfc_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_sfc_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_sfc_common,
  .pExecutionCbs         = &AudioChainWrp_sfc_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_sfc_factory);
