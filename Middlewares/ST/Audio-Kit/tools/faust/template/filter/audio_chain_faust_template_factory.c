/**
******************************************************************************
* @file    audio_chain_$(ALGO_NAME_MINUS_STD)_factory.c
* @author  MCD Application Team
* @brief   factory of noise $(ALGO_NAME) effect
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
#include "audio_chain_$(ALGO_NAME_MINUS_STD).h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type*/
BEGIN_IGNORE_DIV0_WARNING

$(ALGO_GLOBAL_CODE)

static const audio_descriptor_param_t s_$(ALGO_NAME_MINUS_STD)_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8($(ALGO_NAME_MINUS_STD)_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
};

static const audio_descriptor_param_t s_$(ALGO_NAME_MINUS_STD)_dynamicParamsDesc[] =
{
$(ALGO_DESCRIPTION)
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_$(ALGO_NAME_MINUS_STD)_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_$(ALGO_NAME_MINUS_STD)_staticParamsDesc,
  .nbParams          = sizeof(s_$(ALGO_NAME_MINUS_STD)_staticParamsDesc) / sizeof(s_$(ALGO_NAME_MINUS_STD)_staticParamsDesc[0]),
  .szBytes           = sizeof($(ALGO_NAME_MINUS_STD)_static_config_t)
};

static const audio_descriptor_params_t s_$(ALGO_NAME_MINUS_STD)_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_$(ALGO_NAME_MINUS_STD)_dynamicParamsDesc,
  .nbParams          = sizeof(s_$(ALGO_NAME_MINUS_STD)_dynamicParamsDesc) / sizeof(s_$(ALGO_NAME_MINUS_STD)_dynamicParamsDesc[0]),
  .szBytes           = sizeof($(ALGO_NAME_MINUS_STD)_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_$(ALGO_NAME_MINUS_STD)_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_$(ALGO_NAME_MINUS_STD)_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_$(ALGO_NAME_MINUS_STD)_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_$(ALGO_NAME_MINUS_STD)_common,
  .pExecutionCbs         = &AudioChainWrp_$(ALGO_NAME_MINUS_STD)_cbs
};

ALGO_FACTORY_DECLARE(AudioChainWrp_$(ALGO_NAME_MINUS_STD)_factory);

