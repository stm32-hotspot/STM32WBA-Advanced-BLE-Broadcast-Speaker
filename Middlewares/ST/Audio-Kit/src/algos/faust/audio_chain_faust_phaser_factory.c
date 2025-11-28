/**
******************************************************************************
* @file    audio_chain_faust_phaser_factory.c
* @author  MCD Application Team
* @brief   factory of noise faust-phaser effect
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
#include "audio_chain_faust_phaser.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type*/
BEGIN_IGNORE_DIV0_WARNING



static const audio_descriptor_param_t s_faust_phaser_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8(faust_phaser_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
};

static const audio_descriptor_param_t s_faust_phaser_dynamicParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("When this is checked, the phaser has no effect"),
    .pControl           = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault           = "0.0",
    .pName              = "bypass",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, bypass, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault           = "0.0",
    .pName              = "invert_internal_phaser_sum",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, invert_internal_phaser_sum, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault           = "0.0",
    .pName              = "vibrato_mode",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, vibrato_mode, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault           = "0.5",
    .pName              = "speed",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, speed, 0.0f, 1e+01f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "1.0",
    .pName              = "notch_depth__intensity_",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, notch_depth__intensity_, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault           = "0.0",
    .pName              = "feedback_gain",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, feedback_gain, -0.999f, 0.999f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "1e+03",
    .pName              = "notch_width",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, notch_width, 1e+01f, 5e+03f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "1e+02",
    .pName              = "min_notch1_freq",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, min_notch1_freq, 2e+01f, 5e+03f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "8e+02",
    .pName              = "max_notch1_freq",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, max_notch1_freq, 2e+01f, 1e+04f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault           = "1.5",
    .pName              = "notch_freq_ratio",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, notch_freq_ratio, 1.1f, 4.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR(""),
    .pControl           = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault           = "0.0",
    .pName              = "phaser_output_level",
    AUDIO_DESC_PARAM_F(faust_phaser_dynamic_config_t, phaser_output_level, -6e+01f, 1e+01f)
  },

};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_faust_phaser_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_phaser_staticParamsDesc,
  .nbParams          = sizeof(s_faust_phaser_staticParamsDesc) / sizeof(s_faust_phaser_staticParamsDesc[0]),
  .szBytes           = sizeof(faust_phaser_static_config_t)
};

static const audio_descriptor_params_t s_faust_phaser_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_phaser_dynamicParamsDesc,
  .nbParams          = sizeof(s_faust_phaser_dynamicParamsDesc) / sizeof(s_faust_phaser_dynamicParamsDesc[0]),
  .szBytes           = sizeof(faust_phaser_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_faust_phaser_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_faust_phaser_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_faust_phaser_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_faust_phaser_common,
  .pExecutionCbs         = &AudioChainWrp_faust_phaser_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_faust_phaser_factory);

