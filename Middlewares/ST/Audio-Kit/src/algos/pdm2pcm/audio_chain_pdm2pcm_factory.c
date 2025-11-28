/**
  ******************************************************************************
  * @file    audio_chain_pdm2pcm_factory.c
  * @author  MCD Application Team
  * @brief   factory of pdm2pcm algo
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
#include "audio_chain_pdm2pcm.h"


#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_pdm2pcm_staticParamsDesc[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl        = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue       = tRamTypeKeyValue,
    .pDefault        = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag      = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName           = "ramType",
    AUDIO_DESC_PARAM_U8(pdm2pcm_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("PDM samples coming from I2S. I2S data register is 16-bit. In case of mono signal, changing the bit order on the I2S configuration will result in a temporal switch of two consecutive samples. The library needs to be aware so it performs a byte reverting operation. There is no such trouble with SAIPDM HW IP"),
    .pName           = "from_i2s",
    .pControl        = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault        = "0",
    AUDIO_DESC_PARAM_U8(pdm2pcm_static_config_t, from_i2s, 0U, 1U)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("0: no high-pass; the closest to 1, the most accurate and stable for DC removal but slower to converge"),
    .pControl        = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault        = "0.9883",
    .pName           = "high_pass_tap",
    AUDIO_DESC_PARAM_F(pdm2pcm_static_config_t, high_pass_tap, 0.0f, 1.0f)
  }
};

static const audio_descriptor_param_t s_pdm2pcm_dynamicParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("gain"),
    .pName            = "gain",
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "12",
    AUDIO_DESC_PARAM_S16(pdm2pcm_dynamic_config_t, gain, -12, 51)
  },
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_pdm2pcm_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_pdm2pcm_staticParamsDesc,
  .nbParams = sizeof(s_pdm2pcm_staticParamsDesc) / sizeof(s_pdm2pcm_staticParamsDesc[0]),
  .szBytes  = sizeof(pdm2pcm_static_config_t)
};

static const audio_descriptor_params_t s_pdm2pcm_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_pdm2pcm_dynamicParamsDesc,
  .nbParams = sizeof(s_pdm2pcm_dynamicParamsDesc) / sizeof(s_pdm2pcm_dynamicParamsDesc[0]),
  .szBytes  = sizeof(pdm2pcm_dynamic_config_t)
};


#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_pdm2pcm_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_pdm2pcm_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_pdm2pcm_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_pdm2pcm_common,
  .pExecutionCbs         = &AudioChainWrp_pdm2pcm_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_pdm2pcm_factory);
