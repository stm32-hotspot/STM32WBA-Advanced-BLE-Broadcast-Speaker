/**
******************************************************************************
* @file    audio_chain_cic_factory.c
* @author  MCD Application Team
* @brief   factory of cic algo
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
#include "audio_chain_cic.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

//static const audio_descriptor_key_value_t tSyncType[] =
//{
//  ALGO_KEY_VALUE_STRINGIFY(CIC_SYNC_NORMAL),
//  ALGO_KEY_VALUE_STRINGIFY(CIC_SYNC_FAST),
//  {0, 0}
//};


/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_cic_staticParamsDesc[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl        = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue       = tRamTypeKeyValue,
    .pDefault        = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag      = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName           = "ramType",
    AUDIO_DESC_PARAM_U8(cic_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Filter Order"),
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault        = "4",
    .pName           = "order",
    AUDIO_DESC_PARAM_U8(cic_static_config_t, order, 0U, 5U)
  },
  //  {
  //    .pDescription    = AUDIO_ALGO_OPT_STR("Decimation Factor"),
  //    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
  //    .pDefault        = "128",
  //    .pName           = "dec",
  //    AUDIO_DESC_PARAM_U16(cic_static_config_t, dec, 0U, 1024U)
  //  },
  //  {
  //    .pDescription    = AUDIO_ALGO_OPT_STR("Sync type normal vs Type"),
  //    .pControl        = AUDIO_ALGO_OPT_STR("droplist"),
  //    .pDefault        = "0",
  //    .pName           = "fastsync",
  //    .pKeyValue       = tSyncType,
  //    .iParamFlag      = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY, // tell to the designer to use label as define
  //    AUDIO_DESC_PARAM_U8(cic_static_config_t, fastsync, CIC_SYNC_NORMAL, CIC_SYNC_FAST)
  //  },
  //  {
  //    .pDescription    = AUDIO_ALGO_OPT_STR("Integration Length"),
  //    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
  //    .pDefault        = "0",
  //    .pName           = "int_len",
  //    AUDIO_DESC_PARAM_U16(cic_static_config_t, int_len, 0U, 256U)
  //  },
};
static const audio_descriptor_param_t s_cic_dynamicParamsDesc[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Right bit shift for attenuation"),
    .pControl        = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault        = "9",
    .pName           = "rbs",
    AUDIO_DESC_PARAM_U8(cic_dynamic_config_t, rbs, 0U, 31U)
  },
};



END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_cic_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_cic_staticParamsDesc,
  .nbParams          = sizeof(s_cic_staticParamsDesc) / sizeof(s_cic_staticParamsDesc[0]),
  .szBytes           = sizeof(cic_static_config_t)
};

static const audio_descriptor_params_t s_cic_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_cic_dynamicParamsDesc,
  .nbParams          = sizeof(s_cic_dynamicParamsDesc) / sizeof(s_cic_dynamicParamsDesc[0]),
  .szBytes           = sizeof(cic_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_cic_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_cic_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_cic_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_cic_common,
  .pExecutionCbs         = &AudioChainWrp_cic_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_cic_factory);
