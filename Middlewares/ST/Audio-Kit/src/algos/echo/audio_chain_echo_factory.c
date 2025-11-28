/**
******************************************************************************
* @file    audio_chain_echo_factory.c
* @author  MCD Application Team
* @brief   factory of echo effect
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
#include "audio_chain_echo.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_echo_staticParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl           = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue          = tRamTypeKeyValue,
    .pDefault           = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMEXT),
    .iParamFlag         = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName              = "ramType",
    AUDIO_DESC_PARAM_U8(echoStaticConfig_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("delay length in second"),
    .pControl           = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault           = "0.25",
    .pName              = "delay",
    AUDIO_DESC_PARAM_F(echoStaticConfig_t, delay, 0.0f, 1.0f)
  }
};

static const audio_descriptor_param_t s_echo_dynamicParamsDesc[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("feedback level"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.5",
    .pName              = "feedback",
    AUDIO_DESC_PARAM_F(echoDynamicConfig_t, feedback, 0.0f, 1.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("echo level"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.5",
    .pName              = "level",
    AUDIO_DESC_PARAM_F(echoDynamicConfig_t, level, 0.0f, 1.0f)
  }
  #ifdef ECHO_CONFIG_WITH_STEREO_PARAM
  ,
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("stereo effect (unused for mono signal)"),
    .pControl           = AUDIO_ALGO_OPT_STR("percent"),
    .pDefault           = "0.2",
    .pName              = "stereo",
    AUDIO_DESC_PARAM_F(echoDynamicConfig_t, stereo, 0.0f, 1.0f)
  }
  #endif
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_echo_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_echo_staticParamsDesc,
  .nbParams          = sizeof(s_echo_staticParamsDesc) / sizeof(s_echo_staticParamsDesc[0]),
  .szBytes           = sizeof(echoStaticConfig_t)
};

static const audio_descriptor_params_t s_echo_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_echo_dynamicParamsDesc,
  .nbParams          = sizeof(s_echo_dynamicParamsDesc) / sizeof(s_echo_dynamicParamsDesc[0]),
  .szBytes           = sizeof(echoDynamicConfig_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_echo_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_echo_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_echo_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_echo_common,
  .pExecutionCbs         = &AudioChainWrp_echo_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_echo_factory);

