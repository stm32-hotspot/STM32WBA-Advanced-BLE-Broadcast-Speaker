/**
******************************************************************************
* @file    audio_chain_route_factory.c
* @author  MCD Application Team
* @brief   factory of router algo to match usage inside audio_chain.c
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
#include "audio_chain_route.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_value_t tInputChunkId[] =
{
  ALGO_KEY_VALUE_STRINGIFY(0),
  ALGO_KEY_VALUE_STRINGIFY(1),
  ALGO_KEY_VALUE_STRINGIFY(2),
  ALGO_KEY_VALUE_STRINGIFY(3),
  {0}
};

static const audio_descriptor_key_value_t tInputChannelId[] =
{
  ALGO_KEY_VALUE_STRINGIFY(0),
  ALGO_KEY_VALUE_STRINGIFY(1),
  ALGO_KEY_VALUE_STRINGIFY(2),
  ALGO_KEY_VALUE_STRINGIFY(3),
  ALGO_KEY_VALUE_STRINGIFY(4),
  ALGO_KEY_VALUE_STRINGIFY(5),
  ALGO_KEY_VALUE_STRINGIFY(6),
  ALGO_KEY_VALUE_STRINGIFY(7),
  {"muted", AC_CHANNEL_MUTED},
  {0}
};


/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_routeChOutConfig[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("input id"),
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = tInputChunkId,
    .pDefault     = "0",
    .pName        = "inputId",
    AUDIO_DESC_PARAM_U8(router_index_t, inputId, 0U, 3U)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("input channel id"),
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = tInputChannelId,
    .pDefault     = "0",
    .pName        = "chId",
    AUDIO_DESC_PARAM_U8(router_index_t, chId, 0U, 255U)
  },
  {0}
};

static const audio_descriptor_param_t s_route_dynamicParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("router output channel config"),
    .pControl     = AUDIO_ALGO_OPT_STR("structarray"),
    .pDefArray    = s_routeChOutConfig, // first parameter of this link is number of struct instance (uint32_t), second is the table of these struct instances
    .pDefault     = "{}",
    .pName        = "routeChOutConfig",
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY,
    AUDIO_DESC_PARAM_OBJ(router_dynamic_config_t, routeChOutConfig, AUDIOCHAINWRP_ROUTE_NB_MAX, sizeof(router_index_t))
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_route_dynamicParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_route_dynamicParamsDesc,
  .nbParams          = sizeof(s_route_dynamicParamsDesc) / sizeof(s_route_dynamicParamsDesc[0]),
  .szBytes           = sizeof(router_dynamic_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_route_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_route_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_route_common,
  .pExecutionCbs         = &AudioChainWrp_route_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_route_factory);
