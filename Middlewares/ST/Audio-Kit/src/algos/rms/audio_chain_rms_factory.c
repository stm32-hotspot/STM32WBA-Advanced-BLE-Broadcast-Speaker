/**
******************************************************************************
* @file    audio_chain_rms_factory.c
* @author  MCD Application Team
* @brief   factory of RMS algo
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
#include "audio_chain_rms.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_rms_staticParamsDesc[] =
{
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("Precision double or float"),
    .pDefault          = "0",
    .pControl          = AUDIO_ALGO_OPT_STR("checkbox"),
    .pName             = "isDoublePrecision",
    AUDIO_DESC_PARAM_U8(rms_stat_config_t, isDoublePrecision, 0U, 1U)
  }
};

static const audio_descriptor_param_t s_rms_dynamicParamsDesc[] =
{
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("SmoothingTime: Time (in ms) to reach 63% of RMS for a step input"),
    .pDefault          = "300",
    .pControl          = AUDIO_ALGO_OPT_STR("slider"),
    .pName             = "smoothingTime",
    AUDIO_DESC_PARAM_U16(rms_dyn_config_t, smoothingTime, 10, 1000)
  },
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("Duration (in ms) between each rms value output"),
    .pDefault          = "1000",
    .pControl          = AUDIO_ALGO_OPT_STR("slider"),
    .pName             = "rmsWindow",
    AUDIO_DESC_PARAM_U16(rms_dyn_config_t, rmsWindow, 100, 10000)
  }
};

static const audio_algo_control_param_t s_rms_controlParamsDesc[] =
{
  {
    .pName        = "rms",
    .pDescription = AUDIO_ALGO_OPT_STR("Table with rms value for all channels, maximum number of channels defined by RMS_MAX_NB_CHANNELS"),
    AUDIO_DESC_CONTROL_OBJ(rmsCtrl_t, rms)
  },
  {
    .pName        = "nbChannels",
    .pDescription = AUDIO_ALGO_OPT_STR("number of channels"),
    AUDIO_DESC_CONTROL_U32(rmsCtrl_t, nbChannels)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_rms_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_rms_staticParamsDesc,
  .nbParams = sizeof(s_rms_staticParamsDesc) / sizeof(s_rms_staticParamsDesc[0]),
  .szBytes  = sizeof(rms_stat_config_t)
};

static const audio_descriptor_params_t s_rms_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_rms_dynamicParamsDesc,
  .nbParams = sizeof(s_rms_dynamicParamsDesc) / sizeof(s_rms_dynamicParamsDesc[0]),
  .szBytes  = sizeof(rms_dyn_config_t)
};

static const audio_algo_control_params_descr_t s_rms_controlParamTemplate =
{
  .pParams  = s_rms_controlParamsDesc,
  .nbParams = sizeof(s_rms_controlParamsDesc) / sizeof(s_rms_controlParamsDesc[0])
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_rms_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_rms_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_rms_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(&s_rms_controlParamTemplate),
  .pCapabilities         = &AudioChainWrp_rms_common,
  .pExecutionCbs         = &AudioChainWrp_rms_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_rms_factory);
