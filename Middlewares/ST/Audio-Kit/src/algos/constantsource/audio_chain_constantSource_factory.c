/**
  ******************************************************************************
  * @file    audio_chain_constantSource_factory.c
  * @author  MCD Application Team
  * @brief   factory of constantSource algo
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
#include "audio_chain_constantSource.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_constantSource_dynamicParams[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Constant source, set as a float but converted & clamped if output is fixed point (real part of complex value)"),
    .pControl        = AUDIO_ALGO_OPT_STR("default"),
    .pDefault        = "0.0",
    .pName           = "re",
    AUDIO_DESC_PARAM_F(constantSource_dynamic_config_t, re, -1.0f, 1.0f)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Constant source, set as a float but converted & clamped if output is fixed point (imaginary part of complex value, only for frequencty signal, not used for time signal)"),
    .pControl        = AUDIO_ALGO_OPT_STR("default"),
    .pDefault        = "0.0",
    .pName           = "im",
    AUDIO_DESC_PARAM_F(constantSource_dynamic_config_t, im, -1.0f, 1.0f)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_constantSource_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_constantSource_dynamicParams,
  .nbParams          = sizeof(s_constantSource_dynamicParams) / sizeof(s_constantSource_dynamicParams[0]),
  .szBytes           = sizeof(constantSource_dynamic_config_t),
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_constantSource_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_constantSource_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_constantSource_common,
  .pExecutionCbs         = &AudioChainWrp_constantSource_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_constantSource_factory);
