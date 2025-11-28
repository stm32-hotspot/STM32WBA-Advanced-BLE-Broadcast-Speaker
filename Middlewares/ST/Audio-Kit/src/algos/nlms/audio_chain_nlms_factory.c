/**
******************************************************************************
* @file    audio_chain_nlms_factory.c
* @author  MCD Application Team
* @brief   factory of CMSIS nlms algo
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
#include "audio_chain_nlms.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_nlms_staticParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl           = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue          = tRamTypeKeyValue,
    .pDefault           = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag         = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName              = "ramType",
    AUDIO_DESC_PARAM_U8(nlms_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("max removable echo length in second"),
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault        = "0.02",
    .pName           = "echo_length",
    AUDIO_DESC_PARAM_F(nlms_static_config_t, echo_length, 0.001f, 0.1f)
  }
};

static const audio_descriptor_param_t s_nlms_dynamicParamsDesc[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("adaptation step (the biggest, the fastest convergence but the less accurate)"),
    .pControl        = AUDIO_ALGO_OPT_STR("default"),
    .pDefault        = "0.1",
    .pName           = "mu",
    AUDIO_DESC_PARAM_F(nlms_dynamic_config_t, mu, 0.0f, 1.0f)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_nlms_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_nlms_staticParamsDesc,
  .nbParams          = sizeof(s_nlms_staticParamsDesc) / sizeof(s_nlms_staticParamsDesc[0]),
  .szBytes           = sizeof(nlms_static_config_t),
};

static const audio_descriptor_params_t s_nlms_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_nlms_dynamicParamsDesc,
  .nbParams          = sizeof(s_nlms_dynamicParamsDesc) / sizeof(s_nlms_dynamicParamsDesc[0]),
  .szBytes           = sizeof(nlms_dynamic_config_t),
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_nlms_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_nlms_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_nlms_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_nlms_common,
  .pExecutionCbs         = &AudioChainWrp_nlms_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_nlms_factory);
