/**
******************************************************************************
* @file    audio_chain_capture_factory.c
* @author  MCD Application Team
* @brief   factory of sample capture algo
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
#include "capture/audio_chain_capture.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_algo_control_param_t s_capture_controlParamsDesc[] =
{
  {
    .pName        = "nbChannels",
    .pDescription = AUDIO_ALGO_OPT_STR("Number of channels"),
    AUDIO_DESC_CONTROL_U8(captureCtrl_t, nbChannels)
  },
  {
    .pName        = "nbSamplesByChannels",
    .pDescription = AUDIO_ALGO_OPT_STR("Number of sample by channel"),
    AUDIO_DESC_CONTROL_U32(captureCtrl_t, nbSamplesByChannels)
  },
  {
    .pName        = "fs",
    .pDescription = AUDIO_ALGO_OPT_STR("inputFrequency"),
    AUDIO_DESC_CONTROL_U32(captureCtrl_t, fs)
  },
  {
    .pName        = "pSamples",
    .pDescription = AUDIO_ALGO_OPT_STR("Sample pointer as an array of int16_t "),
    AUDIO_DESC_CONTROL_ADDR(captureCtrl_t, pSamples)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_algo_control_params_descr_t s_capture_controlParamTemplate =
{
  .pParams           = s_capture_controlParamsDesc,
  .nbParams          = sizeof(s_capture_controlParamsDesc) / sizeof(s_capture_controlParamsDesc[0])
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_capture_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(NULL),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(&s_capture_controlParamTemplate),
  .pCapabilities         = &AudioChainWrp_capture_common,
  .pExecutionCbs         = &AudioChainWrp_capture_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_capture_factory);
