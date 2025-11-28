/**
******************************************************************************
* @file    audio_chain_wola_fwd_factory.c
* @author  MCD Application Team
* @brief   factory of wola (Weighted Overlap and Add) algo to match usage inside audio_chain.c
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
#include "audio_chain_wola.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_wola_fwd_staticParamsDesc[] =
{
  {
    .pDescription             = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl                 = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                = tRamTypeKeyValue,
    .pDefault                 = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag               = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName                    = "ramType",
    AUDIO_DESC_PARAM_U8(wola_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription             = AUDIO_ALGO_OPT_STR("with or without synthesis window (synthesis window is necessary in case of reconstruction of combination of different signals, for instance in case of beam-forming antennas), must be consistent with wola_inv config"),
    .pControl                 = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault                 = "1",
    .pName                    = "synthesisWindow",
    AUDIO_DESC_PARAM_U8(wola_static_config_t, synthesisWindow, 0U, 1U)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_wola_fwd_staticParamTemplate =
{
  .pParam                     = (audio_descriptor_param_t *)s_wola_fwd_staticParamsDesc,
  .nbParams                   = sizeof(s_wola_fwd_staticParamsDesc) / sizeof(s_wola_fwd_staticParamsDesc[0]),
  .szBytes                    = sizeof(wola_static_config_t),
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_wola_fwd_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_wola_fwd_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_wola_fwd_common,
  .pExecutionCbs         = &AudioChainWrp_wola_fwd_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_wola_fwd_factory);
