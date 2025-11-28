/**
******************************************************************************
* @file    audio_chain_switch_factory.c
* @author  MCD Application Team
* @brief   factory of switch algo (select 1 input chunk) to match usage inside audio_chain.c
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
#include "audio_chain_switch.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_value_t tInputId[] =
{
  ALGO_KEY_VALUE_STRINGIFY(0),
  ALGO_KEY_VALUE_STRINGIFY(1),
  ALGO_KEY_VALUE_STRINGIFY(2),
  ALGO_KEY_VALUE_STRINGIFY(3),
  {0}
};

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_switch_dynamicParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("selected input"),
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = tInputId,
    .pDefault     = "0",
    .pName        = SWITCH_INPUT_ID,
    AUDIO_DESC_PARAM_U8(switch_dynamic_config_t, inputId, 0U, SWITCH_NB_INPUT - 1U)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_switch_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_switch_dynamicParamsDesc,
  .nbParams = sizeof(s_switch_dynamicParamsDesc) / sizeof(s_switch_dynamicParamsDesc[0]),
  .szBytes  = sizeof(switch_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_switch_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_switch_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_switch_common,
  .pExecutionCbs         = &AudioChainWrp_switch_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_switch_factory);
