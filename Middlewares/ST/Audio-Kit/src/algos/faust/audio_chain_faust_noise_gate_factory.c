/**
******************************************************************************
* @file    audio_chain_faust_noise_gate_factory.c
* @author  MCD Application Team
* @brief   factory of noise faust-noise-gate effect
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
#include "audio_chain_faust_noise_gate.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type*/
BEGIN_IGNORE_DIV0_WARNING



static const audio_descriptor_param_t s_faust_noise_gate_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8(faust_noise_gate_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
};

static const audio_descriptor_param_t s_faust_noise_gate_dynamicParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("When this is checked, the gate has no effect"),
    .pControl           = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault           = "0.0",
    .pName              = "bypass",
    AUDIO_DESC_PARAM_F(faust_noise_gate_dynamic_config_t, bypass, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("When the signal level falls below the Threshold (expressed in dB), the signal is muted"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "-3e+01",
    .pName              = "threshold",
    AUDIO_DESC_PARAM_F(faust_noise_gate_dynamic_config_t, threshold, -1.2e+02f, 0.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Time constant in MICROseconds (1/e smoothing time) for the gate gain to go (exponentially) from 0 (muted) to 1 (unmuted)"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "1e+01",
    .pName              = "attack",
    AUDIO_DESC_PARAM_F(faust_noise_gate_dynamic_config_t, attack, 1e+01f, 1e+04f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Time in ms to keep the gate open (no muting) after the signal level falls below the Threshold"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "2e+02",
    .pName              = "hold",
    AUDIO_DESC_PARAM_F(faust_noise_gate_dynamic_config_t, hold, 1.0f, 1e+03f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Time constant in ms (1/e smoothing time) for the gain to go (exponentially) from 1 (unmuted) to 0 (muted)"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "1e+02",
    .pName              = "release",
    AUDIO_DESC_PARAM_F(faust_noise_gate_dynamic_config_t, release, 1.0f, 1e+03f)
  },

};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_faust_noise_gate_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_noise_gate_staticParamsDesc,
  .nbParams          = sizeof(s_faust_noise_gate_staticParamsDesc) / sizeof(s_faust_noise_gate_staticParamsDesc[0]),
  .szBytes           = sizeof(faust_noise_gate_static_config_t)
};

static const audio_descriptor_params_t s_faust_noise_gate_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_faust_noise_gate_dynamicParamsDesc,
  .nbParams          = sizeof(s_faust_noise_gate_dynamicParamsDesc) / sizeof(s_faust_noise_gate_dynamicParamsDesc[0]),
  .szBytes           = sizeof(faust_noise_gate_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_faust_noise_gate_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_faust_noise_gate_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_faust_noise_gate_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_faust_noise_gate_common,
  .pExecutionCbs         = &AudioChainWrp_faust_noise_gate_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_faust_noise_gate_factory);

