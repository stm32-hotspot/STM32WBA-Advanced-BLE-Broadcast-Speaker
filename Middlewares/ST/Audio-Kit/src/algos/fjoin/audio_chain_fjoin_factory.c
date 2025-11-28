/**
******************************************************************************
* @file    audio_chain_fjoin_factory.c
* @author  MCD Application Team
* @brief   factory of frequency join algo
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
#include "audio_chain_fjoin.h"

//#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)
//
//BEGIN_IGNORE_DIV0_WARNING
//
//static const audio_descriptor_param_t s_fjoin_staticParamsDesc[] =
//{
//  {
//    .pDescription             = AUDIO_ALGO_OPT_STR("index of beginning of the band joinion"),
//    .pControl                 = AUDIO_ALGO_OPT_STR("slider"),
//    .pDefault                 = "0",
//    .pName                    = "begId",
//    AUDIO_DESC_PARAM_U16(fjoin_static_config_t, begId, 0U, 4096U)
//  },
//  {
//    .pDescription             = AUDIO_ALGO_OPT_STR("index of ending of the band joinion"),
//    .pControl                 = AUDIO_ALGO_OPT_STR("slider"),
//    .pDefault                 = "128",
//    .pName                    = "endId",
//    AUDIO_DESC_PARAM_U16(fjoin_static_config_t, endId, 0U, 4096U)
//  },
//};
//
//END_IGNORE_DIV0_WARNING
//
//static const audio_descriptor_params_t s_fjoin_staticParamTemplate =
//{
//  .pParam                     = (audio_descriptor_param_t *)s_fjoin_staticParamsDesc,
//  .nbParams                   = sizeof(s_fjoin_staticParamsDesc) / sizeof(s_fjoin_staticParamsDesc[0])
//};
//
//#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_fjoin_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_fjoin_common,
  .pExecutionCbs         = &AudioChainWrp_fjoin_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_fjoin_factory);
