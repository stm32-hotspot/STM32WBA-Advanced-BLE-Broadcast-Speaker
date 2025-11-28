/**
******************************************************************************
* @file    audio_chain_hpf_factory.c
* @author  MCD Application Team
* @brief   factory of dc removing
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
#include "audio_chain_hpf.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_value_t tTypeKeyValue[] =
{
  #ifdef USE_HPF_TYPE_FIR
  ALGO_KEY_VALUE_STRINGIFY(HPF_TYPE_PARKSMCCLELLAN),
  ALGO_KEY_VALUE_STRINGIFY(HPF_TYPE_KAISERWINDOW),
  #endif
  #ifdef USE_HPF_TYPE_IIR
  ALGO_KEY_VALUE_STRINGIFY(HPF_TYPE_BUTTERWORTH),
  ALGO_KEY_VALUE_STRINGIFY(HPF_TYPE_CHEBYSHEV1),
  ALGO_KEY_VALUE_STRINGIFY(HPF_TYPE_CHEBYSHEV2),
  ALGO_KEY_VALUE_STRINGIFY(HPF_TYPE_ELLIPTIC),
  #endif
  {0, 0}
};

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_hpf_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8(hpf_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("Filter type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tTypeKeyValue,
    #ifdef USE_HPF_TYPE_IIR
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(HPF_TYPE_ELLIPTIC),
    #elif defined(USE_HPF_TYPE_FIR)
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(HPF_TYPE_PARKSMCCLELLAN),
    #else
    .pDefault         = "0",
    #endif
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "filterType",
    AUDIO_DESC_PARAM_U8(hpf_static_config_t, filterType, 0U, (uint8_t)HPF_TYPE_NB - 1U)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_hpf_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_hpf_staticParamsDesc,
  .nbParams = sizeof(s_hpf_staticParamsDesc) / sizeof(s_hpf_staticParamsDesc[0]),
  .szBytes  = sizeof(hpf_static_config_t),
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_hpf_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_hpf_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_hpf_common,
  .pExecutionCbs         = &AudioChainWrp_hpf_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_hpf_factory);
