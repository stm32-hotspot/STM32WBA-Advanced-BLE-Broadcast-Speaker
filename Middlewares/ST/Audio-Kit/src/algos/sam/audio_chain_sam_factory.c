/**
******************************************************************************
* @file    audio_chain_sam_factory.c
* @author  MCD Application Team
* @brief   factory of ??? magnifier
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
#include "audio_chain_sam.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

//static const audio_descriptor_key_value_t tMode[] =
//{
//  ALGO_KEY_VALUE_STRINGIFY(SAM_MODE_LOWCONSO),
//  ALGO_KEY_VALUE_STRINGIFY(SAM_MODE_HIGHPERF),
//  {0, 0}
//};

static const audio_descriptor_key_value_t tSelect[] =
{
  ALGO_KEY_VALUE_STRINGIFY(SAM_BEAMSELECT_FRONT),
  ALGO_KEY_VALUE_STRINGIFY(SAM_BEAMSELECT_REAR),
  ALGO_KEY_VALUE_STRINGIFY(SAM_BEAMSELECT_NOCHANGE),
  {0, 0}
};


//static const audio_descriptor_key_value_t tMix[] =
//{
//  ALGO_KEY_VALUE_STRINGIFY(SAM_MIXER_DISABLE),
//  ALGO_KEY_VALUE_STRINGIFY(SAM_MIXER_ENABLE),
//  ALGO_KEY_VALUE_STRINGIFY(SAM_MIXER_NOCHANGE),
//  {0, 0}
//};

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING
static const audio_descriptor_param_t s_sam_staticParams[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl           = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue          = tRamTypeKeyValue,
    .pDefault           = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag         = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName              = "ramType",
    AUDIO_DESC_PARAM_U8(sam_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  //  {
  //    .pDescription       = AUDIO_ALGO_OPT_STR("algo preset that to chose from maximum performance or lower conso"),
  //    .pDefault          = "0",
  //    .pControl          = AUDIO_ALGO_OPT_STR("droplist"),
  //    .pName             = "mode",
  //    .pKeyValue         = tMode,
  //    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY, // tell to the designer to use label as define
  //    AUDIO_DESC_PARAM_U8(sam_static_config_t, mode, SAM_MODE_LOWCONSO, SAM_MODE_HIGHPERF)
  //  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("number of taps"),
    .pControl           = AUDIO_ALGO_OPT_STR("default"),
    .pDefault           = "5",
    .pName              = "nbTaps",
    AUDIO_DESC_PARAM_U16(sam_static_config_t, nbTaps, 1U, 20U)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("control activation, 0 control is not active, if n will be call every n*frame size "),
    .pControl           = AUDIO_ALGO_OPT_STR("default"),
    .pDefault           = "0",
    .pName              = "control",
    AUDIO_DESC_PARAM_U16(sam_static_config_t, control, 0U, 125U)
  },
};

static const audio_descriptor_param_t s_sam_dynamicParams[] =
{
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("adaptation step (the biggest, the fastest convergence but the less accurate)"),
    .pControl           = AUDIO_ALGO_OPT_STR("default"),
    .pDefault           = "0.1",
    .pName              = "mu",
    AUDIO_DESC_PARAM_F(sam_dynamic_config_t, mu, 0.0f, 1.0f)
  },
  //  {
  //    .pDescription       = AUDIO_ALGO_OPT_STR("gain before any processing in dB"),
  //    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
  //    .pDefault           = "0.0",
  //    .pName              = "gain_pre",
  //    AUDIO_DESC_PARAM_F(sam_dynamic_config_t, gain_pre, -100.0f, +100.0f)
  //  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("post gain (after processing) in dB"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "0.0",
    .pName              = "gain",
    AUDIO_DESC_PARAM_F(sam_dynamic_config_t, gain, -100.0f, +100.0f)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("freeze detection of noise"),
    .pControl           = AUDIO_ALGO_OPT_STR("default"),
    .pDefault           = "0",
    .pName              = "freeze",
    AUDIO_DESC_PARAM_U8(sam_dynamic_config_t, freeze, 0U, 1U)
  },
  //  {
  //    .pDescription       = AUDIO_ALGO_OPT_STR("enable smart mixing of omni microphone and sam as function of ambiant noise"),
  //    .pControl           = AUDIO_ALGO_OPT_STR("default"),
  //    .pDefault           = "0",
  //    .pName              = "enable_mixer",
  //    .pKeyValue         = tMix,
  //    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY, // tell to the designer to use label as define
  //    AUDIO_DESC_PARAM_U8(sam_dynamic_config_t, enable_mixer, SAM_MIXER_DISABLE, SAM_MIXER_NOCHANGE)
  //  },
  //  {
  //    .pDescription       = AUDIO_ALGO_OPT_STR("index of lowest frequency band to consider for antenna selection algorithm. Full band if = 0 ; High pass from the given value otherwise "),
  //    .pControl           = AUDIO_ALGO_OPT_STR("default"),
  //    .pDefault           = "54",
  //    .pName              = "idBandStart",
  //    AUDIO_DESC_PARAM_U16(sam_dynamic_config_t, idBandStart, 0U, 1024U)
  //  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Attack smoothing time in ms for antenna selection decision & mixer (the higher the more stable, the lower the more reactive"),
    .pControl           = AUDIO_ALGO_OPT_STR("default"),
    .pDefault           = "1000",
    .pName              = "attackMs",
    AUDIO_DESC_PARAM_U16(sam_dynamic_config_t, attackMs, 0U, 5000U)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Release smoothing time in ms for antenna selection decision & mixer (the higher the more stable, the lower the more reactive"),
    .pControl           = AUDIO_ALGO_OPT_STR("default"),
    .pDefault           = "100",
    .pName              = "releaseMs",
    AUDIO_DESC_PARAM_U16(sam_dynamic_config_t, releaseMs, 0U, 5000U)
  },
  //  {
  //    .pDescription       = AUDIO_ALGO_OPT_STR("Low threshold for mixing with omni microphone. If inferior then only omni microphone in output signal. Unit is dB"),
  //    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
  //    .pDefault           = "0.0",
  //    .pName              = "tLowDb",
  //    AUDIO_DESC_PARAM_F(sam_dynamic_config_t, tLowDb, -100.0f, 0.0f)
  //  },
  //  {
  //    .pDescription       = AUDIO_ALGO_OPT_STR("High threshold for mixing with omni microphone. If superior then sam processing in output signal. Unit is dB"),
  //    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
  //    .pDefault           = "0.0",
  //    .pName              = "tHighDb",
  //    AUDIO_DESC_PARAM_F(sam_dynamic_config_t, tHighDb, -100.0f, 0.0f)
  //  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Selection of the beam used to evaluate the noise"),
    .pControl           = AUDIO_ALGO_OPT_STR("droplist"),
    .pDefault           = "0",
    .pName              = "beamSelect",
    .pKeyValue         = tSelect,
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY, // tell to the designer to use label as define
    AUDIO_DESC_PARAM_U8(sam_dynamic_config_t, beamSelect, SAM_BEAMSELECT_FRONT, SAM_BEAMSELECT_NOCHANGE)
  }
};

static const audio_algo_control_param_t s_sam_controlParamsDesc[] =
{
  {
    .pName        = "beamSelect",
    .pDescription = AUDIO_ALGO_OPT_STR("Index of antenna identified as noise"),
    AUDIO_DESC_CONTROL_U8(sam_control_t, beamSelect)
  },
  //  {
  //    .pName        = "autoBypass",
  //    .pDescription = AUDIO_ALGO_OPT_STR("Automatic switch to non processed data if sam evaluates that noise source is not an interefernce but rather a diffuse noise"),
  //    AUDIO_DESC_CONTROL_U8(sam_control_t, autoBypass)
  //  },
  //  {
  //    .pName        = "psdMicDb",
  //    .pDescription = AUDIO_ALGO_OPT_STR("energy of microphone signal in dB (depends of the parameter idBandStart)"),
  //    AUDIO_DESC_CONTROL_F(sam_control_t, psdMicDb)
  //  },
  //  {
  //    .pName        = "mixerGainLin",
  //    .pDescription = AUDIO_ALGO_OPT_STR("g in the equation: (1-g) x micData + g x processedData"),
  //    AUDIO_DESC_CONTROL_F(sam_control_t, mixerGainLin)
  //  },
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_sam_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_sam_staticParams,
  .nbParams = sizeof(s_sam_staticParams) / sizeof(s_sam_staticParams[0]),
  .szBytes  = sizeof(sam_static_config_t)
};

static const audio_descriptor_params_t s_sam_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_sam_dynamicParams,
  .nbParams = sizeof(s_sam_dynamicParams) / sizeof(s_sam_dynamicParams[0]),
  .szBytes  = sizeof(sam_dynamic_config_t)
};

static const audio_algo_control_params_descr_t s_sam_controlParamTemplate =
{
  .pParams  = s_sam_controlParamsDesc,
  .nbParams = sizeof(s_sam_controlParamsDesc) / sizeof(s_sam_controlParamsDesc[0])
};
#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

audio_algo_factory_t AudioChainWrp_sam_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_sam_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_sam_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(&s_sam_controlParamTemplate),
  .pCapabilities         = &AudioChainWrp_sam_common,
  .pExecutionCbs         = &AudioChainWrp_sam_cbs,
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_sam_factory);
