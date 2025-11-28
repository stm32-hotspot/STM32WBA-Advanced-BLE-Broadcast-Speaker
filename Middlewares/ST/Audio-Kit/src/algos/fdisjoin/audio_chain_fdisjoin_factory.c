/**
  ******************************************************************************
  * @file    audio_chain_fdisjoin_factory.c
  * @author  MCD Application Team
  * @brief   factory of the algorithm
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#include "audio_chain_fdisjoin.h"


#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_fdisjoin_staticParamsDesc[] =
{
  {
    .pDescription             = AUDIO_ALGO_OPT_STR("index of beginning of the band selection"),
    .pControl                 = AUDIO_ALGO_OPT_STR("default"),
    .pDefault                 = "0",
    .pName                    = "begId",
    AUDIO_DESC_PARAM_U16(fdisjoin_static_config_t, begId, 0U, 4096U)
  },
  {
    .pDescription             = AUDIO_ALGO_OPT_STR("index of ending of the band selection"),
    .pControl                 = AUDIO_ALGO_OPT_STR("default"),
    .pDefault                 = "128",
    .pName                    = "endId",
    AUDIO_DESC_PARAM_U16(fdisjoin_static_config_t, endId, 0U, 4096U)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_fdisjoin_staticParamTemplate =
{
  .pParam                     = (audio_descriptor_param_t *)s_fdisjoin_staticParamsDesc,
  .nbParams                   = sizeof(s_fdisjoin_staticParamsDesc) / sizeof(s_fdisjoin_staticParamsDesc[0]),
  .szBytes                    = sizeof(fdisjoin_static_config_t),
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_fdisjoin_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_fdisjoin_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_fdisjoin_common,
  .pExecutionCbs         = &AudioChainWrp_fdisjoin_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_fdisjoin_factory);
