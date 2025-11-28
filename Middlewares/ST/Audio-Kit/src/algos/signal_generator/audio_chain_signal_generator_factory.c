/**
******************************************************************************
* @file    audio_chain_signal_generator_factory.c
* @author  MCD Application Team
* @brief   factory of signal_generator algo to match usage inside audio_chain.c
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
#include "audio_chain_signal_generator.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_value_t tSignalTypeKeyValue[] =
{
  ALGO_KEY_VALUE_STRINGIFY(WHITE_NOISE),
  ALGO_KEY_VALUE_STRINGIFY(PINK_NOISE),
  ALGO_KEY_VALUE_STRINGIFY(SINE),
  ALGO_KEY_VALUE_STRINGIFY(SQUARE),
  ALGO_KEY_VALUE_STRINGIFY(TRIANGLE),
  {0, 0}
};

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_signal_generator_staticParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl           = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue          = tRamTypeKeyValue,
    .pDefault           = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_TCM),
    .iParamFlag         = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName              = "ramType",
    AUDIO_DESC_PARAM_U8(signal_generator_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  }
};

static const audio_descriptor_param_t s_signal_generator_dynamicParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("Signal Type"),
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(WHITE_NOISE),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pName            = "signalType",
    .pKeyValue        = tSignalTypeKeyValue,
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY, // tell to the designer to use label as define
    AUDIO_DESC_PARAM_U8(signal_generator_dynamic_config_t, signalType, WHITE_NOISE, TRIANGLE)
  },
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("Signal frequency in Hz for SINE, SQUARE & TRIANGLE (should be < fs/2, else frequency aliasing will occur)"),
    .pName             = SIGNAL_GENERATOR_FREQUENCY,
    .pControl          = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault          = "1000",
    AUDIO_DESC_PARAM_U32(signal_generator_dynamic_config_t, frequency, SIGNAL_GENERATOR_FREQ_MARGIN, 24000UL - SIGNAL_GENERATOR_FREQ_MARGIN)
  },
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("Gain in dB"),
    .pControl          = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault          = "-6",
    .pName             = "gain",
    AUDIO_DESC_PARAM_F(signal_generator_dynamic_config_t, gain, -100.0f, 0.0f)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_signal_generator_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_signal_generator_staticParamsDesc,
  .nbParams = sizeof(s_signal_generator_staticParamsDesc) / sizeof(s_signal_generator_staticParamsDesc[0]),
  .szBytes  = sizeof(signal_generator_static_config_t),
};

static const audio_descriptor_params_t s_signal_generator_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_signal_generator_dynamicParamsDesc,
  .nbParams = sizeof(s_signal_generator_dynamicParamsDesc) / sizeof(s_signal_generator_dynamicParamsDesc[0]),
  .szBytes  = sizeof(signal_generator_dynamic_config_t),
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_signal_generator_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_signal_generator_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_signal_generator_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_signal_generator_common,
  .pExecutionCbs         = &AudioChainWrp_signal_generator_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_signal_generator_factory);
