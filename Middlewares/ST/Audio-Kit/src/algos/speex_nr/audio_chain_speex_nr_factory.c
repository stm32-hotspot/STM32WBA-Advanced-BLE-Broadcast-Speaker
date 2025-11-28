/**
  ******************************************************************************
  * @file    audio_chain_speex_nr_factory.c
  * @author  MCD Application Team
  * @brief   wrapper of speex_nr_factory algo to match usage inside audio_chain.c
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
/* Includes ------------------------------------------------------------------*/
#include "speex_nr/audio_chain_speex_nr.h"


/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_speex_nr_staticParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue    = tRamTypeKeyValue,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_TCM),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName        = "ramType",
    AUDIO_DESC_PARAM_U8(speex_nr_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  }
};

static const audio_descriptor_param_t s_speex_nr_dynamicParamsDesc[] =
{
  {
    .pName        = "agc",
    .pDefault     = "0.1",
    .pDescription = AUDIO_ALGO_OPT_STR("AGC (Automatic Gain Control) effect : from 0 (no AGC effect) to 1 (max AGC effect)"),
    .pControl     = AUDIO_ALGO_OPT_STR("slidershort"),
    AUDIO_DESC_PARAM_F(speex_nr_dynamic_config_t, agc, 0.0f, 1.0f)
  },
  {
    .pName        = "noise_suppress",
    .pDefault     = "-15",
    .pDescription = AUDIO_ALGO_OPT_STR("noise_suppress (dB): see Speex.org"),
    .pControl     = AUDIO_ALGO_OPT_STR("slider"),
    AUDIO_DESC_PARAM_S8(speex_nr_dynamic_config_t, noise_suppress, -100, 0)
  }
};
END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/


static const audio_descriptor_params_t s_speex_nr_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_speex_nr_staticParamsDesc,
  .nbParams          = sizeof(s_speex_nr_staticParamsDesc) / sizeof(s_speex_nr_staticParamsDesc[0]),
  .szBytes           = sizeof(speex_nr_static_config_t)
};

static const audio_descriptor_params_t s_speex_nr_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_speex_nr_dynamicParamsDesc,
  .nbParams          = sizeof(s_speex_nr_dynamicParamsDesc) / sizeof(s_speex_nr_dynamicParamsDesc[0]),
  .szBytes           = sizeof(speex_nr_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_speex_nr_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_speex_nr_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_speex_nr_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_speex_nr_common,
  .pExecutionCbs         = &AudioChainWrp_speex_nr_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_speex_nr_factory);

