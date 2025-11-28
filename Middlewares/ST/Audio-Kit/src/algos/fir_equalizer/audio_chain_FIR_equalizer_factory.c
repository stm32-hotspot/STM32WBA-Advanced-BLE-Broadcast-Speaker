/**
******************************************************************************
* @file    audio_chain_FIR_equalizer_factory.c
* @author  MCD Application Team
* @brief   factory of FIR equalizer
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
#include "audio_chain_FIR_equalizer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_string_t s_FirPreset[] =
{
  {"Acoustic - Folk", "firType|0||firSize|50||gainInterpolLinear|0||firResponseCurve|{'0':[0,0],'1':[40,1.5],'2':[200,-1],'3':[5000,3],'4':[20000,2]}"},
  {"Classic",         "firType|0||firSize|50||gainInterpolLinear|0||firResponseCurve|{'0':[0,0],'1':[60,3],'2':[300,0],'3':[2000,0],'4':[8000,1.5],'5':[20000,3]}"},
  {"Electronic",      "firType|0||firSize|50||gainInterpolLinear|0||firResponseCurve|{'0':[0,0],'1':[60,3],'2':[150,1],'3':[300,-2],'4':[1500,3],'5':[3000,1.5],'6':[8000,2],'7':[20000,3]}"},
  {"Hip Hop",         "firType|0||firSize|50||gainInterpolLinear|0||firResponseCurve|{'0':[0,0],'1':[64,3],'2':[300,0],'3':[1500,3],'4':[5000,0]}"},
  {"Pop",             "firType|0||firSize|50||gainInterpolLinear|0||firResponseCurve|{'0':[0,0],'1':[60,3],'2':[250,0],'3':[700,0],'4':[1200,2.5],'5':[3000,0],'6':[9000,1.5],'7':[20000,3]}"},
  {"Rock",            "firType|0||firSize|50||gainInterpolLinear|0||firResponseCurve|{'0':[0,0],'1':[40,1.5],'2':[200,-1],'3':[5000,3],'4':[20000,2]}"},
  {0}
};

static const audio_descriptor_key_value_t tFirTypeKeyValue[] =
{
  {"linear phase",  FIREQ_LINEAR_PHASE},
  {"minimum phase", FIREQ_MINIMUM_PHASE},
  {0, 0}
};

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_fir_equalizer_frequencyResponse[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("response frequency"),
    .pControl        = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault        = "0",
    .pName           = "frequency",
    AUDIO_DESC_PARAM_F(firEqResponsePoint_t, frequency, 0.0f, 24000.0f)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("response gain in dB"),
    .pControl        = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault        = "0",
    .pName           = "gain_dB",
    AUDIO_DESC_PARAM_F(firEqResponsePoint_t, gain_dB, -80.0f, +20.0f)
  },
  {0}
};

static const audio_descriptor_param_t s_fir_equalizer_staticParamsDesc[] =
{
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tRamTypeKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_TCM),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName                            = "ramType",
    AUDIO_DESC_PARAM_U8(fir_equalizer_staticConfig_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("Applies a quick setting"),
    .pName                            = "preset",
    .pControl                         = AUDIO_ALGO_OPT_STR("preset"),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_STRING | AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    .pKeyString                       = AUDIO_ALGO_OPT_TUNING(s_FirPreset),
    .pDefault                         = "0", /* ignored for this control */
    AUDIO_DESC_PARAM_U8(fir_equalizer_staticConfig_t, preset, 0U, 0U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("FIR type (linear phase: same delay at every frequency, minimum phase: minimum delay but variable delay per frequency)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tFirTypeKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(FIREQ_LINEAR_PHASE),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName                            = "firType",
    AUDIO_DESC_PARAM_U8(fir_equalizer_staticConfig_t, firType, 0U, (uint8_t)FIREQ_TYPE_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("number of FIR coefficients (the bigger, the most accurate filter response at the expense of increased CPU load)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault                         = "100",
    .pName                            = "firSize",
    AUDIO_DESC_PARAM_U16(fir_equalizer_staticConfig_t, firSize, 0U, 200U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("gain interpolation in linear (enabled) or in dB (disabled)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault                         = "0",
    .pName                            = "gainInterpolLinear",
    AUDIO_DESC_PARAM_U8(fir_equalizer_staticConfig_t, gainInterpolLinear, 0U, 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("FIR response curve (frequency, gain in dB); Shift click to create a point, Del to remove a point"),
    .pControl                         = AUDIO_ALGO_OPT_STR("grapharray"),
    .pCustom                          = AUDIO_ALGO_OPT_STR("{\"xAxis\":\"Hz\",\"yAxis\":\"dB\",\"ScaleX\":\"log\"}"),
    .pDefArray                        = s_fir_equalizer_frequencyResponse, // first parameter of this link is number of struct instance (uint32_t), second is the table of these struct instances
    .pDefault                         = "{}",
    .pName                            = FIR_RESPONSE_CURVE,
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY,
    AUDIO_DESC_PARAM_OBJ(fir_equalizer_staticConfig_t, firResponse, FIREQ_MAX_RESPONSE_POINTS, sizeof(firEqResponsePoint_t))
  }
};

static const audio_algo_control_param_t s_fir_equalizer_controlParamsDesc[] =
{
  {
    .pName        = "nbBands",
    .pDescription = AUDIO_ALGO_OPT_STR("number points of FIR response curve"),
    AUDIO_DESC_CONTROL_U32(fir_equalizer_ctrl_t, nbBands)
  },
  {
    .pName        = "samplingRate",
    .pDescription = AUDIO_ALGO_OPT_STR("sampling rate (Hz)"),
    AUDIO_DESC_CONTROL_F(fir_equalizer_ctrl_t, samplingRate)
  },
  {
    .pName        = "pResponse_dB",
    .pDescription = AUDIO_ALGO_OPT_STR("array with the magnitude in dB of each band, nbBands points"),
    AUDIO_DESC_CONTROL_ADDR(fir_equalizer_ctrl_t, pResponse_dB)
  },
  {
    .pName        = "modMin_dB",
    .pDescription = AUDIO_ALGO_OPT_STR("min magnitude in dB of filter response"),
    AUDIO_DESC_CONTROL_F(fir_equalizer_ctrl_t, modMin_dB)
  },
  {
    .pName        = "modMax_dB",
    .pDescription = AUDIO_ALGO_OPT_STR("max magnitude in dB of filter response"),
    AUDIO_DESC_CONTROL_F(fir_equalizer_ctrl_t, modMax_dB)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_fir_equalizer_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_fir_equalizer_staticParamsDesc,
  .nbParams = sizeof(s_fir_equalizer_staticParamsDesc) / sizeof(s_fir_equalizer_staticParamsDesc[0]),
  .szBytes  = sizeof(fir_equalizer_staticConfig_t)
};

static const audio_algo_control_params_descr_t s_fir_equalizer_controlParamTemplate =
{
  .pParams  = s_fir_equalizer_controlParamsDesc,
  .nbParams = sizeof(s_fir_equalizer_controlParamsDesc) / sizeof(s_fir_equalizer_controlParamsDesc[0])
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_fir_equalizer_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_fir_equalizer_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(&s_fir_equalizer_controlParamTemplate),
  .pCapabilities         = &AudioChainWrp_fir_equalizer_common,
  .pExecutionCbs         = &AudioChainWrp_fir_equalizer_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_fir_equalizer_factory);
