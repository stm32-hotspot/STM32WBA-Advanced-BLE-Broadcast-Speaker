/**
******************************************************************************
* @file    audio_chain_faust_compressor_factory.c
* @author  MCD Application Team
* @brief   factory of noise faust-compressor effect
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
#include "audio_chain_faust_compressor.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type*/
BEGIN_IGNORE_DIV0_WARNING



static const audio_descriptor_param_t s_faust_compressor_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8(faust_compressor_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
};

static const audio_descriptor_param_t s_faust_compressor_dynamicParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("When this is checked, the compressor has no effect"),
    .pControl           = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault           = "0.0",
    .pName              = "bypass",
    AUDIO_DESC_PARAM_F(faust_compressor_dynamic_config_t, bypass, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("A compression Ratio of N means that for each N dB increase in input signal level above Threshold, the output level goes up 1 dB"),
    .pControl           = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault           = "5.0",
    .pName              = "ratio",
    AUDIO_DESC_PARAM_F(faust_compressor_dynamic_config_t, ratio, 1.0f, 2e+01f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("When the signal level exceeds the Threshold (in dB), its level is compressed according to the Ratio"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "-3e+01",
    .pName              = "threshold",
    AUDIO_DESC_PARAM_F(faust_compressor_dynamic_config_t, threshold, -1e+02f, 1e+01f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Time constant in ms (1/e smoothing time) for the compression gain to approach (exponentially) a new lower target level (the compression 'kicking in')"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "5e+01",
    .pName              = "attack",
    AUDIO_DESC_PARAM_F(faust_compressor_dynamic_config_t, attack, 1.0f, 1e+03f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Time constant in ms (1/e smoothing time) for the compression gain to approach (exponentially) a new higher target level (the compression 'releasing')"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "5e+02",
    .pName              = "release",
    AUDIO_DESC_PARAM_F(faust_compressor_dynamic_config_t, release, 1.0f, 1e+03f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("The compressed-signal output level is increased by this amount (in dB) to make up for the level lost due to compression"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "4e+01",
    .pName              = "makeup_gain",
    AUDIO_DESC_PARAM_F(faust_compressor_dynamic_config_t, makeup_gain, -96.0f, 96.0f)
  },

};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_faust_compressor_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_compressor_staticParamsDesc,
  .nbParams          = sizeof(s_faust_compressor_staticParamsDesc) / sizeof(s_faust_compressor_staticParamsDesc[0]),
  .szBytes           = sizeof(faust_compressor_static_config_t)
};

static const audio_descriptor_params_t s_faust_compressor_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_compressor_dynamicParamsDesc,
  .nbParams          = sizeof(s_faust_compressor_dynamicParamsDesc) / sizeof(s_faust_compressor_dynamicParamsDesc[0]),
  .szBytes           = sizeof(faust_compressor_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_faust_compressor_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_faust_compressor_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_faust_compressor_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_faust_compressor_common,
  .pExecutionCbs         = &AudioChainWrp_faust_compressor_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_faust_compressor_factory);

