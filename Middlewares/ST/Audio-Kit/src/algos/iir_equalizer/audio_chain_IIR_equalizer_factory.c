/**
******************************************************************************
* @file    audio_chain_IIR_equalizer_factory.c
* @author  MCD Application Team
* @brief   factory of IIR equalizer
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
#include "audio_chain_IIR_equalizer.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_value_t tIirTypeKeyValue[] =
{
  ALGO_KEY_VALUE_STRINGIFY(NOTCH),
  ALGO_KEY_VALUE_STRINGIFY(PEAKING),
  ALGO_KEY_VALUE_STRINGIFY(LOWPASS),
  ALGO_KEY_VALUE_STRINGIFY(HIGHPASS),
  ALGO_KEY_VALUE_STRINGIFY(BANDPASS),
  ALGO_KEY_VALUE_STRINGIFY(LOWSHELF),
  ALGO_KEY_VALUE_STRINGIFY(HIGHSHELF),
  {0, 0}
};

static const audio_descriptor_key_value_t tIirQorBWKeyValue[] =
{
  ALGO_KEY_VALUE_STRINGIFY(STEEPNESS_Q),
  ALGO_KEY_VALUE_STRINGIFY(STEEPNESS_BW),
  {0, 0}
};

static const audio_descriptor_key_value_t tIirBiquadSortKeyValue[] =
{
  ALGO_KEY_VALUE_STRINGIFY(BIQUAD_NO_SORT),
  ALGO_KEY_VALUE_STRINGIFY(BIQUAD_UP_SORT),
  ALGO_KEY_VALUE_STRINGIFY(BIQUAD_DOWN_SORT),
  {0, 0}
};

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_iir_equalizer_biquadConfig[] =
{
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("biquad type (low-pass, high-pass, band-pass, notch, peaking, etc...)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tIirTypeKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(LOWPASS),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName                            = "iirType",
    AUDIO_DESC_PARAM_U8(biquadConfig_t, iirType, 0U, (uint8_t)IIREQ_TYPE_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("biquad steepness defined by Q factor or by band-width"),
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tIirQorBWKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(STEEPNESS_Q),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName                            = "steepnessByQorBW",
    AUDIO_DESC_PARAM_U8(biquadConfig_t, steepnessByQorBW, 0U, (uint8_t)IIREQ_STEEPNESS_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("biquad steepness value according to steepnessByQorBW: Q factor (typically 0.707 for -3dB/octave) or band width (Hz)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("default"),
    .pDefault                         = "0.707",
    .pName                            = "steepness",
    AUDIO_DESC_PARAM_F(biquadConfig_t, steepness, 0.1f, 20000.0f)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("center cutoff frequency (between 0.0025*sampling rate and 0.4975*sampling rate)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("default"),
    .pDefault                         = "1000",
    .pName                            = "centerCutoffFrequency",
    AUDIO_DESC_PARAM_F(biquadConfig_t, centerCutoffFrequency, 20.0f, 23880.0f)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("gain in dB (not used for low-pass, high-pass, band-pass, notch filters)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault                         = "10",
    .pName                            = "gain_dB",
    AUDIO_DESC_PARAM_F(biquadConfig_t, gain_dB, 0.0f, 40.0f)
  },
  {0}
};

static const audio_descriptor_param_t s_iir_equalizer_staticParamsDesc[] =
{
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tRamTypeKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_TCM),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName                            = "ramType",
    AUDIO_DESC_PARAM_U8(iir_equalizer_staticConfig_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("sort biquad poles in order of decreasing (up sort) or increasing (down sort) distance to unit circle: see matlab zp2sos order parameter"),
    .pControl                         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue                        = tIirBiquadSortKeyValue,
    .pDefault                         = AUDIOCHAINFACTORY_INT2STR(BIQUAD_NO_SORT),
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName                            = "biquadSort",
    AUDIO_DESC_PARAM_U8(iir_equalizer_staticConfig_t, biquadSort, 0U, (uint8_t)BIQUAD_SORT_NB - 1U)
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("IIR config (cascade of biquad filters)"),
    .pControl                         = AUDIO_ALGO_OPT_STR("structarray"),
    .pDefArray                        = s_iir_equalizer_biquadConfig, // first parameter of this link is number of struct instance (uint32_t), second is the table of these struct instances
    .pDefault                         = "{}",
    .pName                            = "iirEqStructArray",
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY,
    AUDIO_DESC_PARAM_OBJ(iir_equalizer_staticConfig_t, iirEqStructArray, IIREQ_MAX_BIQUAD_CELLS, sizeof(biquadConfig_t))
  }
};

static const audio_algo_control_param_t s_iir_equalizer_controlParamsDesc[] =
{
  {
    .pName        = "nbBands",
    .pDescription = AUDIO_ALGO_OPT_STR("number points of IIR response curve"),
    AUDIO_DESC_CONTROL_U32(iir_equalizer_ctrl_t, nbBands)
  },
  {
    .pName        = "samplingRate",
    .pDescription = AUDIO_ALGO_OPT_STR("sampling rate (Hz)"),
    AUDIO_DESC_CONTROL_F(iir_equalizer_ctrl_t, samplingRate)
  },
  {
    .pName        = "pResponse_dB",
    .pDescription = AUDIO_ALGO_OPT_STR("array with the magnitude in dB of each band, nbBands points"),
    AUDIO_DESC_CONTROL_ADDR(iir_equalizer_ctrl_t, pResponse_dB)
  },
  {
    .pName        = "modMin_dB",
    .pDescription = AUDIO_ALGO_OPT_STR("min magnitude in dB of filter response"),
    AUDIO_DESC_CONTROL_F(iir_equalizer_ctrl_t, modMin_dB)
  },
  {
    .pName        = "modMax_dB",
    .pDescription = AUDIO_ALGO_OPT_STR("max magnitude in dB of filter response"),
    AUDIO_DESC_CONTROL_F(iir_equalizer_ctrl_t, modMax_dB)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_iir_equalizer_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_iir_equalizer_staticParamsDesc,
  .nbParams = sizeof(s_iir_equalizer_staticParamsDesc) / sizeof(s_iir_equalizer_staticParamsDesc[0]),
  .szBytes  = sizeof(iir_equalizer_staticConfig_t)
};

static const audio_algo_control_params_descr_t s_iir_equalizer_controlParamTemplate =
{
  .pParams  = s_iir_equalizer_controlParamsDesc,
  .nbParams = sizeof(s_iir_equalizer_controlParamsDesc) / sizeof(s_iir_equalizer_controlParamsDesc[0])
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_iir_equalizer_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_iir_equalizer_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(&s_iir_equalizer_controlParamTemplate),
  .pCapabilities         = &AudioChainWrp_iir_equalizer_common,
  .pExecutionCbs         = &AudioChainWrp_iir_equalizer_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_iir_equalizer_factory);
