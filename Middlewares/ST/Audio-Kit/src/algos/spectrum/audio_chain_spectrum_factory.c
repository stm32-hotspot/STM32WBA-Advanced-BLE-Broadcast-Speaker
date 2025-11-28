/**
******************************************************************************
* @file    audio_chain_spectrum_factory.c
* @author  MCD Application Team
* @brief   factory of spectrum algo (spectrum estimation from input time samples) to match usage inside audio_chain.c
*          remark: this algo has no output samples
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
#include "audio_chain_spectrum.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_value_t tFftLenghtKeyValue[] =
{
  ALGO_KEY_VALUE_STRINGIFY(64),
  ALGO_KEY_VALUE_STRINGIFY(128),
  ALGO_KEY_VALUE_STRINGIFY(256),
  ALGO_KEY_VALUE_STRINGIFY(512),
  ALGO_KEY_VALUE_STRINGIFY(1024),
  ALGO_KEY_VALUE_STRINGIFY(2048),
  {0, 0}
};

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_spectrum_staticParamsDesc[] =
{
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl          = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue         = tRamTypeKeyValue,
    .pDefault          = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName             = "ramType",
    AUDIO_DESC_PARAM_U8(spectrum_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("Fast Fourier Transformation length (64, 128, 256, 512, 1024, 2048 or 4096)"),
    .pControl          = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue         = tFftLenghtKeyValue,
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pDefault          = "1024",
    .pName             = "fftLength",
    AUDIO_DESC_PARAM_U32(spectrum_static_config_t, fftLength, 64U, 4096U)
  }
};

static const audio_algo_control_param_t s_spectrum_controlParamsDesc[] =
{
  {
    .pName        = "pSquareMag",
    .pDescription = AUDIO_ALGO_OPT_STR("float array with the magnitude of each band, nbBands = ((fftLength / 2) + 1)"),
    AUDIO_DESC_CONTROL_ADDR(spectrumCtrl_t, pSquareMag)
  },
  {
    .pName        = "normalizationCoef",
    .pDescription = AUDIO_ALGO_OPT_STR("float : square magnitude normalization coefficient"),
    AUDIO_DESC_CONTROL_F(spectrumCtrl_t, normalizationCoef)
  },
  {
    .pName        = "nbAccSpectrum",
    .pDescription = AUDIO_ALGO_OPT_STR("uint32_t : number of accumulated spectrums (must be set to 0 by user to acknowledge reading of square magnitude for spectrum to restart accumulation)"),
    AUDIO_DESC_CONTROL_U32(spectrumCtrl_t, nbAccSpectrum)
  },
  {
    .pName        = "fs",
    .pDescription = AUDIO_ALGO_OPT_STR("uint32_t : sample rate"),
    AUDIO_DESC_CONTROL_U32(spectrumCtrl_t, fs)
  },
  {
    .pName        = "nbChannels",
    .pDescription = AUDIO_ALGO_OPT_STR("uint8_t : number of channels"),
    AUDIO_DESC_CONTROL_U8(spectrumCtrl_t, nbChannels)
  },
  {
    .pName        = "fftLength",
    .pDescription = AUDIO_ALGO_OPT_STR("uint32_t : FFT length"),
    AUDIO_DESC_CONTROL_U32(spectrumCtrl_t, fftLength)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_spectrum_staticParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_spectrum_staticParamsDesc,
  .nbParams          = sizeof(s_spectrum_staticParamsDesc) / sizeof(s_spectrum_staticParamsDesc[0]),
  .szBytes           = sizeof(spectrum_static_config_t)
};

static const audio_algo_control_params_descr_t s_spectrum_controlParamTemplate =
{
  .pParams           = s_spectrum_controlParamsDesc,
  .nbParams          = sizeof(s_spectrum_controlParamsDesc) / sizeof(s_spectrum_controlParamsDesc[0])
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_spectrum_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_spectrum_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(&s_spectrum_controlParamTemplate),
  .pCapabilities         = &AudioChainWrp_spectrum_common,
  .pExecutionCbs         = &AudioChainWrp_spectrum_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_spectrum_factory);
