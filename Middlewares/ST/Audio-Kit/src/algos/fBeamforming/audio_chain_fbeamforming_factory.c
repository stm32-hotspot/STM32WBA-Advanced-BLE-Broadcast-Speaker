/**
  ******************************************************************************
  * @file    audio_chain_fBeamforming_factory.c
  * @author  MCD Application Team
  * @brief   factory of frequency beamforming algo
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#include "audio_chain_fbeamforming.h"


#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_fBeamforming_coef_descr[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("real part of coefficient"),
    .pControl        = AUDIO_ALGO_OPT_STR("static"),
    .pDefault        = "0",
    .pName           = "real",
    AUDIO_DESC_PARAM_F(fbf_coef, real, -1000.0f, 1000.0f)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("imaginary part of coefficient"),
    .pControl        = AUDIO_ALGO_OPT_STR("static"),
    .pDefault        = "0",
    .pName           = "imag",
    AUDIO_DESC_PARAM_F(fbf_coef, imag, -1000.0f, 1000.0f)
  },
  {0}
};


static const audio_descriptor_param_t s_fBeamforming_speech_source_descr[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Speech sources"),
    .pControl        = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault        = "0",
    .pName           = "value",
    AUDIO_DESC_PARAM_U16(fbf_src_value, value, 0, 360)
  },
  {0}
};


static const audio_descriptor_param_t s_fBeamforming_interference_sources_descr[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Interference sources"),
    .pControl        = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault        = "0",
    .pName           = "value",
    AUDIO_DESC_PARAM_U16(fbf_src_value, value, 0, 360)
  },
  {0}
};

static const audio_descriptor_key_value_t tNbBandsKeyValue[] =
{
  ALGO_KEY_VALUE_STRINGIFY(65),
  ALGO_KEY_VALUE_STRINGIFY(129),
  ALGO_KEY_VALUE_STRINGIFY(257),
  {0, 0}
};

static const audio_descriptor_key_value_t tSampleFreqKeyValue[] =
{
  ALGO_KEY_VALUE_STRINGIFY(8000),
  ALGO_KEY_VALUE_STRINGIFY(16000),
  {0, 0}
};

static const audio_descriptor_param_t s_fBeamforming_staticParamsDesc[] =
{
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl          = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue         = tRamTypeKeyValue,
    .pDefault          = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName             = "ramType",
    AUDIO_DESC_PARAM_U8(fbf_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("When we push the button, compute all coefs and update the device. Microphones axis is the 90->270 degrees axis"),
    .pControl          = AUDIO_ALGO_OPT_STR("bfapply"),
    .pDefault          = "0",
    .pName             = "apply",
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_TOOL | AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    AUDIO_DESC_PARAM_U8(fbf_static_config_t, is_legacy, 0, 0)
  },
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("True if units are in DB"),
    .pControl          = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault          = "1",
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    .pName             = "unitDb",
    AUDIO_DESC_PARAM_U8(fbf_static_config_t, unitDb, 0, 1)
  },
  {
    .pGroup            = AUDIO_ALGO_OPT_STR("General"),
    .pDescription      = AUDIO_ALGO_OPT_STR("plot_freq"),
    .pControl          = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault          = "2000",
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    .pName             = "plot_freq",
    AUDIO_DESC_PARAM_F(fbf_static_config_t, plot_freq, 10.0f, 8000.0f)
  },
  {
    .pGroup            = AUDIO_ALGO_OPT_STR("General"),
    .pDescription      = AUDIO_ALGO_OPT_STR("Sample_freq"),
    .pControl          = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue           = tSampleFreqKeyValue,
    .pDefault          = "16000",
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName             = "sample_freq",
    AUDIO_DESC_PARAM_U32(fbf_static_config_t, sample_freq, 8000UL, 16000UL)
  },
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("Microphones number"),
    .pControl          = AUDIO_ALGO_OPT_STR("static"),
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DISABLED | AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    .pDefault          = "2",
    .pName             = "mic_nb",
    AUDIO_DESC_PARAM_U8(fbf_static_config_t, mic_nb, 2, 4)
  },
  {
    .pDescription      = AUDIO_ALGO_OPT_STR("Bands number "),
    .pControl          = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue         = tNbBandsKeyValue,
    .pDefault          = "129",
    .iParamFlag        = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName             = "bands_nb",
    AUDIO_DESC_PARAM_U32(fbf_static_config_t, bands_nb, 1, 1025)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Microphone distance in mm"),
    .pControl           = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault           = "36",
    .pName              = "mic_dist",
    .iParamFlag         = AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    AUDIO_DESC_PARAM_U8(fbf_static_config_t, mic_dist, 0, 128)
  },
  {
    .pDescription       = AUDIO_ALGO_OPT_STR("Standard noise"),
    .pControl           = AUDIO_ALGO_OPT_STR("slidershort"),
    .pDefault           = "0.000001",
    .pName              = "noise_std",
    .iParamFlag         = AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    AUDIO_DESC_PARAM_F(fbf_static_config_t, noise_std, 0.00000000001, 0.1)
  },
  {
    .pGroup                         = AUDIO_ALGO_OPT_STR("Sources"),
    .pDescription                     = AUDIO_ALGO_OPT_STR("Speech sources, list of sources (in degre)"),
    .pDefault                         = "{0:[0]}",
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    .pControl                         = AUDIO_ALGO_OPT_STR("structarray"),
    .pDefArray                        = s_fBeamforming_speech_source_descr,
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY  | AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    .pName                            = "speech_sources",
    AUDIO_DESC_PARAM_OBJ(fbf_static_config_t, speech_sources, BF_MAX_SRC_ARRAY /* Must be confirmed we can have several items */, sizeof(fbf_src_value))
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("Interference sources, list of interferences to avoid (in degre)"),
    .pDefault                         = "{0:[20]}",
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY | AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    .pControl                         = AUDIO_ALGO_OPT_STR("structarray"),
    .pDefArray                        = s_fBeamforming_interference_sources_descr,
    .pName                            = "interference_sources",
    AUDIO_DESC_PARAM_OBJ(fbf_static_config_t, interference_sources, BF_MAX_SRC_ARRAY /* Must be confirmed we can have several items */, sizeof(fbf_src_value))
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("Antennas coefs list"),
    .pDefault                         = "{}",
    .pControl                         = AUDIO_ALGO_OPT_STR("structarray"),
    .pDefArray                        = s_fBeamforming_coef_descr,
    .pName                            = "tAntennaCoef1",
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DISABLED | AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY | AUDIO_DESC_PARAM_TYPE_FLAG_STOP_GRAPH,
    AUDIO_DESC_PARAM_OBJ(fbf_static_config_t, tAntennaCoef1, BF_MAX_ANTENNA_COEFS, sizeof(fbf_coef))
  },
  {
    .pDescription                     = AUDIO_ALGO_OPT_STR("Antennas coefs list"),
    .pDefault                         = "{}",
    .pControl                         = AUDIO_ALGO_OPT_STR("structarray"),
    .pDefArray                        = s_fBeamforming_coef_descr,
    .pName                            = "tAntennaCoef2",
    .iParamFlag                       = AUDIO_DESC_PARAM_TYPE_FLAG_DISABLED | AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY | AUDIO_DESC_PARAM_TYPE_FLAG_STOP_GRAPH,
    AUDIO_DESC_PARAM_OBJ(fbf_static_config_t, tAntennaCoef2, BF_MAX_ANTENNA_COEFS, sizeof(fbf_coef))
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_fBeamforming_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_fBeamforming_staticParamsDesc,
  .nbParams = sizeof(s_fBeamforming_staticParamsDesc) / sizeof(s_fBeamforming_staticParamsDesc[0]),
  .szBytes  = sizeof(fbf_static_config_t)
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_fBeamforming_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_fBeamforming_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(NULL),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_fBeamforming_common,
  .pExecutionCbs         = &AudioChainWrp_fBeamforming_cbs
};

//ALGO_FACTORY_DECLARE(AudioChainWrp_fBeamforming_factory);
