/**
  ******************************************************************************
  * @file    audio_chain_disto_factory.c
  * @author  MCD Application Team
  * @brief   wrapper of disto_factory algo to match usage inside audio_chain.c
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
#include "disto/audio_chain_disto.h"


/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_value_t tDistoTypeKeyValue[] =
{
  ALGO_KEY_VALUE_STRINGIFY(DISTO_CLIPPING_SOFT),
  ALGO_KEY_VALUE_STRINGIFY(DISTO_CLIPPING_CUBIC),
  {0, 0}
};


/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING
static const audio_descriptor_param_t s_disto_staticParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue    = tRamTypeKeyValue,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName        = "ramType",
    AUDIO_DESC_PARAM_U8(disto_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("Type of clipping"),
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue    = tDistoTypeKeyValue,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(DISTO_CLIPPING_SOFT),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName        = "type",
    AUDIO_DESC_PARAM_U8(disto_static_config_t, type, 0U, (uint8_t)DISTO_TYPE_NB - 1U)
  },

  {
    .pName        = "drive",
    .pDefault     = "10.0",
    .pDescription = AUDIO_ALGO_OPT_STR("Drive "),
    .pControl     = AUDIO_ALGO_OPT_STR("slidershort"),
    AUDIO_DESC_PARAM_F(disto_static_config_t, drive, 0.0f, 1000.0f)
  },
  {
    .pName        = "dry_wet",
    .pDefault     = "1.0",
    .pDescription = AUDIO_ALGO_OPT_STR("Dry wet mix "),
    .pControl     = AUDIO_ALGO_OPT_STR("slidershort"),
    AUDIO_DESC_PARAM_F(disto_static_config_t, dry_wet, 0.0f, 1.0f)
  },
  {
    .pName        = "off_axis",
    .pDefault     = "0.0",
    .pDescription = AUDIO_ALGO_OPT_STR("Add DC to tune symmetry/asymmetry to enhance even harmonics"),
    .pControl     = AUDIO_ALGO_OPT_STR("slidershort"),
    AUDIO_DESC_PARAM_F(disto_static_config_t, off_axis, 0.0f, 10.0f)
  },

  {
    .pName        = "dry_gain",
    .pDefault     = "0.0",
    .pDescription = AUDIO_ALGO_OPT_STR("Apply gain on dry to fine tune dry_wet mixing "),
    .pControl     = AUDIO_ALGO_OPT_STR("slidershort"),
    AUDIO_DESC_PARAM_F(disto_static_config_t, dry_gain, -100.0f, 60.0f)
  },
  {
    .pName        = "post_gain",
    .pDefault     = "0.0",
    .pDescription = AUDIO_ALGO_OPT_STR("Final gain "),
    .pControl     = AUDIO_ALGO_OPT_STR("slidershort"),
    AUDIO_DESC_PARAM_F(disto_static_config_t, post_gain, -100.0f, 60.0f)
  }
};
END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/


static const audio_descriptor_params_t s_disto_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_disto_staticParamsDesc,
  .nbParams          = sizeof(s_disto_staticParamsDesc) / sizeof(s_disto_staticParamsDesc[0]),
  .szBytes           = sizeof(disto_static_config_t)
};


#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_disto_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_disto_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_disto_common,
  .pExecutionCbs         = &AudioChainWrp_disto_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_disto_factory);

