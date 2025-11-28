/**
******************************************************************************
* @file    audio_chain_nr_factory.c
* @author  MCD Application Team
* @brief   factory of NR (Stationnary Noise Reductor) algo to match usage inside audio_chain.c
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
#include "audio_chain_nr.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_key_value_t s_nr_convergenceProfileTypeKeyValue[] =
{
  {"default",                 NR_CONVERGENCE_PROFILE_DEFAULT},
  {"fast",                    NR_CONVERGENCE_PROFILE_FAST},
  {"accurate",                NR_CONVERGENCE_PROFILE_ACCURATE},
  {"user defined with alpha", NR_CONVERGENCE_PROFILE_USER_DEFINED_WITH_ALPHA},
  {0, 0}
};

static const audio_descriptor_param_t s_nr_dynamicParamsDesc[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Max dB attenuation in noise"),
    .pName           = "maxAttInNoise",
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault        = "30",
    AUDIO_DESC_PARAM_F(nr_dynamic_config_t, maxAttInNoise, 10.0f, 50.0f)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Max dB attenuation in speech"),
    .pName           = "maxAttInSpeech",
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault        = "10",
    AUDIO_DESC_PARAM_F(nr_dynamic_config_t, maxAttInSpeech, 0.0f, 20.0f)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Reference noise dB threshold for min gain estimation"),
    .pName           = "noiseMeanRef",
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault        = "-40",
    AUDIO_DESC_PARAM_F(nr_dynamic_config_t, noiseMeanRef, -80.0f, -20.0f)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("\"intelligent\" (active only in case of noise presence) low-pass cut frequency in percent of signal band width (fs/2)"),
    .pName           = "lowPassBandPcent",
    .pControl        = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault        = "0.5",
    AUDIO_DESC_PARAM_F(nr_dynamic_config_t, lowPassBandPcent, 0.0f, 1.0f)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("noise estimation convergence profile"),
    .pName            = "convergenceProfile",
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue        = s_nr_convergenceProfileTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(NR_CONVERGENCE_PROFILE_DEFAULT),
    AUDIO_DESC_PARAM_U8(nr_dynamic_config_t, convergenceProfile, NR_CONVERGENCE_PROFILE_DEFAULT, NR_CONVERGENCE_PROFILE_USER_DEFINED_WITH_ALPHA)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("alpha B (only for user defined convergence profile)"),
    .pName           = "alphaB",
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault        = "0.3",
    AUDIO_DESC_PARAM_F(nr_dynamic_config_t, alphaB, NR_ALPHAB_MIN, NR_ALPHAB_MAX)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("alpha G (only for user defined convergence profile)"),
    .pName           = "alphaG",
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault        = "0.8",
    AUDIO_DESC_PARAM_F(nr_dynamic_config_t, alphaG, NR_ALPHAG_MIN, NR_ALPHAG_MAX)
  },
};

static const audio_descriptor_param_t s_nr_staticParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl           = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue          = tRamTypeKeyValue,
    .pDefault           = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_TCM),
    .iParamFlag         = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName              = "ramType",
    AUDIO_DESC_PARAM_U8(nr_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_nr_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_nr_dynamicParamsDesc,
  .nbParams = sizeof(s_nr_dynamicParamsDesc) / sizeof(s_nr_dynamicParamsDesc[0]),
  .szBytes  = sizeof(nr_dynamic_config_t)
};

static const audio_descriptor_params_t s_nr_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_nr_staticParamsDesc,
  .nbParams = sizeof(s_nr_staticParamsDesc) / sizeof(s_nr_staticParamsDesc[0]),
  .szBytes  = sizeof(nr_static_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_nr_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_nr_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_nr_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_nr_common,
  .pExecutionCbs         = &AudioChainWrp_nr_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_nr_factory);
