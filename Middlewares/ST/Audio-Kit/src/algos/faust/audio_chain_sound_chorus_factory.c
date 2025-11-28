/**
******************************************************************************
* @file    audio_chain_sound_chorus_factory.c
* @author  MCD Application Team
* @brief   factory of chorus effect
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
#include "faust/audio_chain_sound_chorus.h"


/**
* @brief Parameter description
*
*/

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_soundChorus_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8(sound_chorus_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  }
};

static const audio_descriptor_param_t s_soundChorus_dynamicParamsDesc[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("When this is checked, disable the effect"),
    .pControl        = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault        = "0",
    .pName           = "bypass",
    AUDIO_DESC_PARAM_U8(sound_chorus_dynamic_config_t, bypass, 0, 1)
  },

  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Sound feedback"),
    .pControl        = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault        = "1",
    .pName           = "feedback",
    AUDIO_DESC_PARAM_F(sound_chorus_dynamic_config_t, feedback, 0, 1)
  },

  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Variable [0 5HZ]  oscillation generator, used in music to control slow modulations"),
    .pControl        = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault        = "0.4",
    .pName           = "frequency",
    AUDIO_DESC_PARAM_F(sound_chorus_dynamic_config_t, frequency, 0, 1)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("offset between voices onto the LFO"),
    .pControl        = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault        = "0.4",
    .pName           = "preDelay",
    AUDIO_DESC_PARAM_F(sound_chorus_dynamic_config_t, preDelay, 0, 1)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("drives the pitch in frequency"),
    .pControl        = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault        = "0.4",
    .pName           = "chorusAmp",
    AUDIO_DESC_PARAM_F(sound_chorus_dynamic_config_t, chorusAmp, 0, 1)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_soundChorus_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_soundChorus_staticParamsDesc,
  .nbParams          = sizeof(s_soundChorus_staticParamsDesc) / sizeof(s_soundChorus_staticParamsDesc[0]),
  .szBytes           = sizeof(sound_chorus_static_config_t)
};

static const audio_descriptor_params_t s_soundChorus_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_soundChorus_dynamicParamsDesc,
  .nbParams          = sizeof(s_soundChorus_dynamicParamsDesc) / sizeof(s_soundChorus_dynamicParamsDesc[0]),
  .szBytes           = sizeof(sound_chorus_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_sound_chorus_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_soundChorus_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_soundChorus_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_sound_chorus_common,
  .pExecutionCbs         = &AudioChainWrp_sound_chorus_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_soundChorus_factory);
