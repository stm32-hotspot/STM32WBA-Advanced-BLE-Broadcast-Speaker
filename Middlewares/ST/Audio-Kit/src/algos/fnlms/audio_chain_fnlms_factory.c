/**
******************************************************************************
* @file    audio_chain_fnlms_factory.c
* @author  MCD Application Team
* @brief   factory of frequency version of nlms
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
#include "audio_chain_fnlms.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING
static const audio_descriptor_param_t s_fnlms_staticParamsDesc[] =
{
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl          = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue         = tRamTypeKeyValue,
    .pDefault          = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_TCM),
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName             = "ramType",
    AUDIO_DESC_PARAM_U8(fnlms_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("number of taps"),
    .pControl          = AUDIO_ALGO_OPT_STR("default"),
    .pDefault          = "5",
    .pName             = "nbTaps",
    AUDIO_DESC_PARAM_U16(fnlms_static_config_t, nbTaps, 1U, 20U)
  }
};

static const audio_descriptor_param_t s_fnlms_dynamicParamsDesc[] =
{
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("adaptation step (the biggest, the fastest convergence but the less accurate)"),
    .pControl          = AUDIO_ALGO_OPT_STR("default"),
    .pDefault          = "0.1",
    .pName             = "mu",
    AUDIO_DESC_PARAM_F(fnlms_dynamic_config_t, mu, 0.0f, 1.0f)
  }/*,
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("with or without normalization"),
    .pControl          = AUDIO_ALGO_OPT_STR("default"),
    .pDefault          = "1",
    .pName             = "normalize",
    AUDIO_DESC_PARAM_U8(fnlms_dynamic_config_t, normalize, 0U, 1U)
  }*/

};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_fnlms_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_fnlms_staticParamsDesc,
  .nbParams          = sizeof(s_fnlms_staticParamsDesc) / sizeof(s_fnlms_staticParamsDesc[0]),
  .szBytes           = sizeof(fnlms_static_config_t)
};

static const audio_descriptor_params_t s_fnlms_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_fnlms_dynamicParamsDesc,
  .nbParams          = sizeof(s_fnlms_dynamicParamsDesc) / sizeof(s_fnlms_dynamicParamsDesc[0]),
  .szBytes           = sizeof(fnlms_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_fnlms_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_fnlms_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_fnlms_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_fnlms_common,
  .pExecutionCbs         = &AudioChainWrp_fnlms_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_fnlms_factory);
