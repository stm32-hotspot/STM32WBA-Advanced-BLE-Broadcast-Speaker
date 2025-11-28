/**
******************************************************************************
* @file    audio_chain_FIR_graphic_equalizer_factory.c
* @author  MCD Application Team
* @brief   factory of FIR graphic equalizer
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
#include "audio_chain_FIR_graphic_equalizer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_value_t tFirGrEqTypeKeyValue[] =
{
  {"linear phase",  FIREQ_LINEAR_PHASE},
  {"minimum phase", FIREQ_MINIMUM_PHASE},
  {0, 0}
};

static const audio_descriptor_key_value_t tFirGrEqModeKeyValue[] =
{
  {"full-automatic center frequency mode", FIRGREQ_MODE_AUTO},
  {"semi-automatic center frequency mode", FIRGREQ_MODE_SEMI_AUTO},
  {"manual center frequency mode",         FIRGREQ_MODE_MANUAL},
  {0, 0}
};

static const audio_descriptor_key_value_t tFirGrEqBandStepKeyValue[] =
{
  {"1/6 octave", FIRGREQ_BAND_STEP_1DIV6_OCTAVE},
  {"1/5 octave", FIRGREQ_BAND_STEP_1DIV5_OCTAVE},
  {"1/4 octave", FIRGREQ_BAND_STEP_1DIV4_OCTAVE},
  {"1/3 octave", FIRGREQ_BAND_STEP_1DIV3_OCTAVE},
  {"1/2 octave", FIRGREQ_BAND_STEP_1DIV2_OCTAVE},
  {"1 octave",   FIRGREQ_BAND_STEP_1_OCTAVE},
  {"2 octaves",  FIRGREQ_BAND_STEP_2_OCTAVES},
  {"3 octaves",  FIRGREQ_BAND_STEP_3_OCTAVES},
  {"4 octaves",  FIRGREQ_BAND_STEP_4_OCTAVES},
  {"5 octaves",  FIRGREQ_BAND_STEP_5_OCTAVES},
  {"6 octaves",  FIRGREQ_BAND_STEP_6_OCTAVES},
  {0, 0}
};


/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_fir_graphic_equalizer_frequencyResponse[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("center frequency"),
    .pControl        = AUDIO_ALGO_OPT_STR("default"),
    .pDefault        = "0",
    .pName           = "centerFreq",
    AUDIO_DESC_PARAM_F(firGrEqResponsePoint_t, centerFreq, 0.0f, 24000.0f)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("response gain in dB"),
    .pControl        = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault        = "0",
    .pName           = "gain_dB",
    AUDIO_DESC_PARAM_F(firGrEqResponsePoint_t, gain_dB, -20.0f, +20.0f)
  },
  {0}
};

static const audio_descriptor_param_t s_fir_graphic_equalizer_staticParamsDesc[] =
{
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tRamTypeKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_TCM),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName                            = "ramType",
    AUDIO_DESC_PARAM_U8(fir_graphic_equalizer_staticConfig_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("FIR type (linear phase: same delay at every frequency, minimum phase: minimum delay but variable delay per frequency)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tFirGrEqTypeKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(FIREQ_LINEAR_PHASE),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName                            = "firType",
    AUDIO_DESC_PARAM_U8(fir_graphic_equalizer_staticConfig_t, firType, 0U, (uint8_t)FIREQ_TYPE_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("number of FIR coefficients (the bigger, the most accurate filter response at the expense of increased CPU load)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault                         = "100",
    .pName                            = "firSize",
    AUDIO_DESC_PARAM_U16(fir_graphic_equalizer_staticConfig_t, firSize, 0U, 200U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("gain interpolation in linear (enabled) or in dB (disabled)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault                         = "0",
    .pName                            = "gainInterpolLinear",
    AUDIO_DESC_PARAM_U8(fir_graphic_equalizer_staticConfig_t, gainInterpolLinear, 0U, 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("center frequencies computation mode (center frequencies are updated only when audio is started); full-automatic: computed from sampling frequency & number of bands; semi-automatic: computed from firstBandCenterFrequency & bandCenterFrequencyStep; manual: set by user"),
    .pName                            = "centerFrequencyMode",
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tFirGrEqModeKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(FIRGREQ_MODE_AUTO),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    AUDIO_DESC_PARAM_U8(fir_graphic_equalizer_staticConfig_t, centerFrequencyMode, 0U, (uint8_t)FIRGREQ_MODE_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("first band center frequency; used only in semi-automatic center frequency mode"),
    .pControl                         = AUDIO_ALGO_OPT_STR("default"),
    .pDefault                         = "10",
    .pName                            = "firstBandCenterFrequency",
    AUDIO_DESC_PARAM_F(fir_graphic_equalizer_staticConfig_t, firstBandCenterFrequency, 0.0f, 24000.0f)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("band center frequency step; used only in semi-automatic center frequency mode"),
    .pName                            = "bandCenterFrequencyStep",
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tFirGrEqBandStepKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(FIRGREQ_BAND_STEP_1_OCTAVE),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    AUDIO_DESC_PARAM_U8(fir_graphic_equalizer_staticConfig_t, bandCenterFrequencyStep, 0U, (uint8_t)FIRGREQ_BAND_STEP_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("gains per band"),
    .pControl                         = AUDIO_ALGO_OPT_STR("structarray"),
    .pDefArray                        = s_fir_graphic_equalizer_frequencyResponse, // first parameter of this link is number of struct instance (uint32_t), second is the table of these struct instances
    .pDefault                         = "{}",
    .pName                            = "firGrEqResponse",
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY,
    AUDIO_DESC_PARAM_OBJ(fir_graphic_equalizer_staticConfig_t, firGrEqResponse, FIRGREQ_MAX_NB_BANDS, sizeof(firGrEqResponsePoint_t))
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("reorder bands in manual mode"),
    .pControl                         = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault                         = "0",
    .pName                            = "reorderBands",
    AUDIO_DESC_PARAM_U8(fir_graphic_equalizer_staticConfig_t, reorderBands, 0U, 1U)
  }
};

static const audio_algo_control_param_t s_fir_graphic_equalizer_controlParamsDesc[] =
{
  {
    .pName        = "nbBands",
    .pDescription = AUDIO_ALGO_OPT_STR("number points of FIR response curve"),
    AUDIO_DESC_CONTROL_U32(fir_graphic_equalizer_ctrl_t, nbBands)
  },
  {
    .pName        = "samplingRate",
    .pDescription = AUDIO_ALGO_OPT_STR("sampling rate (Hz)"),
    AUDIO_DESC_CONTROL_F(fir_graphic_equalizer_ctrl_t, samplingRate)
  },
  {
    .pName        = "pResponse_dB",
    .pDescription = AUDIO_ALGO_OPT_STR("array with the magnitude in dB of each band, nbBands points"),
    AUDIO_DESC_CONTROL_ADDR(fir_graphic_equalizer_ctrl_t, pResponse_dB)
  },
  {
    .pName        = "modMin_dB",
    .pDescription = AUDIO_ALGO_OPT_STR("min magnitude in dB of filter response"),
    AUDIO_DESC_CONTROL_F(fir_graphic_equalizer_ctrl_t, modMin_dB)
  },
  {
    .pName        = "modMax_dB",
    .pDescription = AUDIO_ALGO_OPT_STR("max magnitude in dB of filter response"),
    AUDIO_DESC_CONTROL_F(fir_graphic_equalizer_ctrl_t, modMax_dB)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_fir_graphic_equalizer_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_fir_graphic_equalizer_staticParamsDesc,
  .nbParams = sizeof(s_fir_graphic_equalizer_staticParamsDesc) / sizeof(s_fir_graphic_equalizer_staticParamsDesc[0]),
  .szBytes  = sizeof(fir_graphic_equalizer_staticConfig_t)
};

static const audio_algo_control_params_descr_t s_fir_graphic_equalizer_controlParamTemplate =
{
  .pParams  = s_fir_graphic_equalizer_controlParamsDesc,
  .nbParams = sizeof(s_fir_graphic_equalizer_controlParamsDesc) / sizeof(s_fir_graphic_equalizer_controlParamsDesc[0])
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_fir_graphic_equalizer_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_fir_graphic_equalizer_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(&s_fir_graphic_equalizer_controlParamTemplate),
  .pCapabilities         = &AudioChainWrp_fir_graphic_equalizer_common,
  .pExecutionCbs         = &AudioChainWrp_fir_graphic_equalizer_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_fir_graphic_equalizer_factory);
