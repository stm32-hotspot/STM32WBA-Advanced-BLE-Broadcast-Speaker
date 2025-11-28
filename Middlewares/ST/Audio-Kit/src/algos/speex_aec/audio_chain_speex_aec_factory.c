/**
******************************************************************************
* @file    audio_chain_speex_aec_factory.c
* @author  MCD Application Team
* @brief   factory of ST acoustic echo canceler algo
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
#include "audio_chain_speex_aec.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

#ifdef CONF_DEBUG_ACTIVE
static const audio_descriptor_key_value_t tDebugDelayKeyValue[] =
{
  {"NONE",      0},
  {"DELAYREF",  1},
  {"PROCESSED", 2},
  {0, 0}
};
#endif

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_speexEc_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_TCM),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8(aec_speex_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("Max delay length in second (between Spk & Mic signals: Spk with this delay must precede Mic); permit to limit echoLength in case of physical delay between Spk & Mic and thus to limit MIPS consumption"),
    .pName            = "delay_length",
    .pDefault         = "0",
    .pControl         = AUDIO_ALGO_OPT_STR("slidershort"),
    AUDIO_DESC_PARAM_F(aec_speex_static_config_t, delay_length, 0.0f, 1.0f)
  },
  {
    .pName            = "echo_length",
    .pDescription     = AUDIO_ALGO_OPT_STR("Max echo length in second"),
    .pDefault         = "0.02",
    .pControl         = AUDIO_ALGO_OPT_STR("slidershort"),
    AUDIO_DESC_PARAM_F(aec_speex_static_config_t, echo_length, 0.001f, 0.1f)
  },
  {
    .pName            = "preprocess_init",
    .pDescription     = AUDIO_ALGO_OPT_STR("preprocess init (in fact in current implementation, it is a post AEC process): flag use for memory allocation purpose (if not set, preprocess_state, residual_echo_remove, AGC_value, noise_suppress, echo_suppress & echo_suppress_active config parameters have no impact)"),
    .pControl         = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault         = "0",
    AUDIO_DESC_PARAM_U8(aec_speex_static_config_t, preprocess_init, 0U, 1U)
  },
  #ifdef CONF_DEBUG_DELAY
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("debug_delay (replace RX with a pulsed sine test signal for echo delay estimation, delay estimation is sent on UART trace log)"),
    .pName            = "debug_delay",
    .pDefault         = "0",
    .pControl         = AUDIO_ALGO_OPT_STR("checkbox"),
    AUDIO_DESC_PARAM_U8(aec_speex_static_config_t, debug_delay, 0U, 1U)
  }
  #endif
};

static const audio_descriptor_param_t s_speexEc_dynamicParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("preprocess_state : enables preprocess if set"),
    .pName            = "preprocess_state",
    .pDefault         = "0",
    .pControl         = AUDIO_ALGO_OPT_STR("checkbox"),
    AUDIO_DESC_PARAM_U8(aec_speex_dynamic_config_t, preprocess_state, 0U, 1U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("residual_echo_remove : remove more (than with AEC alone) residual echo remove if set"),
    .pName            = "residual_echo_remove",
    .pDefault         = "0",
    .pControl         = AUDIO_ALGO_OPT_STR("checkbox"),
    AUDIO_DESC_PARAM_U8(aec_speex_dynamic_config_t, residual_echo_remove, 0U, 1U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("AGC (Automatic Gain Control) effect : from 0 (no AGC effect) to 1 (max AGC effect)"),
    .pName            = "AGC_value",
    .pDefault         = "0.1",
    .pControl         = AUDIO_ALGO_OPT_STR("slidershort"),
    AUDIO_DESC_PARAM_F(aec_speex_dynamic_config_t, AGC_value, 0.0f, 1.0f)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("noise_suppress (dB): see Speex.org"),
    .pName            = "noise_suppress",
    .pDefault         = "-15",
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    AUDIO_DESC_PARAM_S8(aec_speex_dynamic_config_t, noise_suppress, -100, 0)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("echo_suppress (dB): see Speex.org"),
    .pName            = "echo_suppress",
    .pDefault         = "-40",
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    AUDIO_DESC_PARAM_S8(aec_speex_dynamic_config_t, echo_suppress, -100, 0)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("echo_suppress_active (dB): see Speex.org"),
    .pName            = "echo_suppress_active",
    .pDefault         = "-15",
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    AUDIO_DESC_PARAM_S8(aec_speex_dynamic_config_t, echo_suppress_active, -100, 0)
  },
  #ifdef CONF_DEBUG_ACTIVE
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("debug output"),
    .pName            = "debug_output",
    .pDefault         = "0",
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue        = tDebugDelayKeyValue,
    AUDIO_DESC_PARAM_U8(aec_speex_dynamic_config_t, debug_output, AC_SPEEX_AEC_DEBUG_OUTPUT_NONE, AC_SPEEX_AEC_DEBUG_OUTPUT_PROCESSED)
  }
  #endif
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_speexEc_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_speexEc_staticParamsDesc,
  .nbParams          = sizeof(s_speexEc_staticParamsDesc) / sizeof(s_speexEc_staticParamsDesc[0]),
  .szBytes           = sizeof(aec_speex_static_config_t)
};

static const audio_descriptor_params_t s_speexEc_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_speexEc_dynamicParamsDesc,
  .nbParams          = sizeof(s_speexEc_dynamicParamsDesc) / sizeof(s_speexEc_dynamicParamsDesc[0]),
  .szBytes           = sizeof(aec_speex_dynamic_config_t)
};

#endif // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_speex_aec_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_speexEc_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_speexEc_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_speex_aec_common,
  .pExecutionCbs         = &AudioChainWrp_speex_aec_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_speex_aec_factory);
