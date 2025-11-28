/**
******************************************************************************
* @file    audio_chain_faust_distortion_factory.c
* @author  MCD Application Team
* @brief   factory of noise faust-distortion effect
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
#include "audio_chain_faust_distortion.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type*/
BEGIN_IGNORE_DIV0_WARNING



static const audio_descriptor_param_t s_faust_distortion_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8(faust_distortion_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
};

static const audio_descriptor_param_t s_faust_distortion_dynamicParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("When this is checked, the nonlinearity has no effect"),
    .pControl           = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault           = "0.0",
    .pName              = "bypass",
    AUDIO_DESC_PARAM_F(faust_distortion_dynamic_config_t, bypass, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Amount of distortion"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.0",
    .pName              = "drive",
    AUDIO_DESC_PARAM_F(faust_distortion_dynamic_config_t, drive, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Brings in even harmonics"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.0",
    .pName              = "offset",
    AUDIO_DESC_PARAM_F(faust_distortion_dynamic_config_t, offset, 0.0f, 1.0f)
  },

};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_faust_distortion_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_distortion_staticParamsDesc,
  .nbParams          = sizeof(s_faust_distortion_staticParamsDesc) / sizeof(s_faust_distortion_staticParamsDesc[0]),
  .szBytes           = sizeof(faust_distortion_static_config_t)
};

static const audio_descriptor_params_t s_faust_distortion_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_distortion_dynamicParamsDesc,
  .nbParams          = sizeof(s_faust_distortion_dynamicParamsDesc) / sizeof(s_faust_distortion_dynamicParamsDesc[0]),
  .szBytes           = sizeof(faust_distortion_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_faust_distortion_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_faust_distortion_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_faust_distortion_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_faust_distortion_common,
  .pExecutionCbs         = &AudioChainWrp_faust_distortion_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_faust_distortion_factory);

