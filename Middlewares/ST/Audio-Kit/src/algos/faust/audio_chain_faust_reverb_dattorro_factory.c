/**
******************************************************************************
* @file    audio_chain_faust_reverb_dattorro_factory.c
* @author  MCD Application Team
* @brief   factory of noise faust-reverb-dattorro effect
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
#include "audio_chain_faust_reverb_dattorro.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type*/
BEGIN_IGNORE_DIV0_WARNING



static const audio_descriptor_param_t s_faust_reverb_dattorro_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8(faust_reverb_dattorro_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
};

static const audio_descriptor_param_t s_faust_reverb_dattorro_dynamicParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("lowpass-like filter, 0 = no signal, 1 = no filtering"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.7",
    .pName              = "prefilter",
    AUDIO_DESC_PARAM_F(faust_reverb_dattorro_dynamic_config_t, prefilter, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("diffusion factor, influences reverb color and density"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.625",
    .pName              = "diffusion_1",
    AUDIO_DESC_PARAM_F(faust_reverb_dattorro_dynamic_config_t, diffusion_1, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("diffusion factor, influences reverb color and density"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.625",
    .pName              = "diffusion_wall2",
    AUDIO_DESC_PARAM_F(faust_reverb_dattorro_dynamic_config_t, diffusion_wall2, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("diffusion factor, influences reverb color and density"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.625",
    .pName              = "diffusion_wall3",
    AUDIO_DESC_PARAM_F(faust_reverb_dattorro_dynamic_config_t, diffusion_wall3, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("diffusion factor, influences reverb color and density"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.625",
    .pName              = "diffusion_wall4",
    AUDIO_DESC_PARAM_F(faust_reverb_dattorro_dynamic_config_t, diffusion_wall4, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("decay length, 1 = infinite"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.7",
    .pName              = "decay_rate",
    AUDIO_DESC_PARAM_F(faust_reverb_dattorro_dynamic_config_t, decay_rate, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("dampening in feedback network"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.625",
    .pName              = "damping",
    AUDIO_DESC_PARAM_F(faust_reverb_dattorro_dynamic_config_t, damping, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("-1 = dry, 1 = wet"),
    .pControl           = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault           = "0.0",
    .pName              = "dry_wet_mix",
    AUDIO_DESC_PARAM_F(faust_reverb_dattorro_dynamic_config_t, dry_wet_mix, -1.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Output Gain"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "-6.0",
    .pName              = "level",
    AUDIO_DESC_PARAM_F(faust_reverb_dattorro_dynamic_config_t, level, -7e+01f, 4e+01f)
  },

};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_faust_reverb_dattorro_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_reverb_dattorro_staticParamsDesc,
  .nbParams          = sizeof(s_faust_reverb_dattorro_staticParamsDesc) / sizeof(s_faust_reverb_dattorro_staticParamsDesc[0]),
  .szBytes           = sizeof(faust_reverb_dattorro_static_config_t)
};

static const audio_descriptor_params_t s_faust_reverb_dattorro_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_reverb_dattorro_dynamicParamsDesc,
  .nbParams          = sizeof(s_faust_reverb_dattorro_dynamicParamsDesc) / sizeof(s_faust_reverb_dattorro_dynamicParamsDesc[0]),
  .szBytes           = sizeof(faust_reverb_dattorro_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_faust_reverb_dattorro_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_faust_reverb_dattorro_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_faust_reverb_dattorro_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_faust_reverb_dattorro_common,
  .pExecutionCbs         = &AudioChainWrp_faust_reverb_dattorro_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_faust_reverb_dattorro_factory);

