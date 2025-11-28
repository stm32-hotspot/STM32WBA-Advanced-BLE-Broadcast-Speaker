/**
  ******************************************************************************
  * @file    audio_chain_acousticBF_factory.c
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
#include "audio_chain_acousticBF.h"


#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_key_value_t s_type[] =
{
  {"basic cardioid",                       ACOUSTIC_BF_TYPE_CARDIOID_BASIC},
  {"cardioid with denoise",                ACOUSTIC_BF_TYPE_CARDIOID_DENOISE},
  {"highly directive for ASR",             ACOUSTIC_BF_TYPE_ASR_READY},
  {"highly directive beam",                ACOUSTIC_BF_TYPE_STRONG},
  #ifdef ACOUSTICBF_AIC_USED
  {"interference canceler",                ACOUSTIC_BF_TYPE_AIC},
  {"interference canceler with denoise",   ACOUSTIC_BF_TYPE_AIC_DENOISE},
  #endif
  {0}
};

static const audio_descriptor_key_value_t s_direction[] =
{
  {"front",       AC_BF_DIR_FRONT},
  {"rear",        AC_BF_DIR_REAR},
  {0}
};

static const audio_descriptor_key_value_t s_mixer[] =
{
  {"enable",       ACOUSTIC_BF_MIXER_ENABLE},
  {"disable",      ACOUSTIC_BF_MIXER_DISABLE},
  {0}
};

static const audio_descriptor_key_value_t s_refSelect[] =
{
  {"none",                              ACOUSTIC_BF_REF_DISABLE},
  {"omni-directional",                  ACOUSTIC_BF_REF_RAW_MICROPHONE},
  {"opposite antenna",                  ACOUSTIC_BF_REF_OPPOSITE_ANTENNA},
  {0}
};

#define ACOUSTIC_BF_MIN_MIC_DISTANCE 30
#define ACOUSTIC_BF_MAX_MIC_DISTANCE 212

static const audio_descriptor_param_t s_acousticBF_staticParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue    = tRamTypeKeyValue,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName        = "ramType",
    AUDIO_DESC_PARAM_U8(acousticBF_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("microphone distance (tenths of a millimeter). In case of PCM input, mic distance must be in the range [210-212]!"),
    .pControl     = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault     = "210",
    .pName        = "mic_distance",
    AUDIO_DESC_PARAM_U16(acousticBF_static_config_t, mic_distance, ACOUSTIC_BF_MIN_MIC_DISTANCE, ACOUSTIC_BF_MAX_MIC_DISTANCE)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("type of beamforming"),
    .pName        = "type",
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = s_type,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(ACOUSTIC_BF_TYPE_CARDIOID_BASIC),
    #ifdef ACOUSTICBF_AIC_USED
    AUDIO_DESC_PARAM_U8(acousticBF_static_config_t, type, ACOUSTIC_BF_TYPE_CARDIOID_BASIC, ACOUSTIC_BF_TYPE_AIC_DENOISE)
    #else
    AUDIO_DESC_PARAM_U8(acousticBF_static_config_t, type, ACOUSTIC_BF_TYPE_CARDIOID_BASIC, ACOUSTIC_BF_TYPE_STRONG)
    #endif
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("Mixing with omni microphone as function of ambiant noise level. Not applicable for Basic Cardioid."),
    .pName        = "mixer_enable",
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = s_mixer,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(ACOUSTIC_BF_MIXER_DISABLE),
    AUDIO_DESC_PARAM_U8(acousticBF_static_config_t, mixer_enable, ACOUSTIC_BF_MIXER_DISABLE, ACOUSTIC_BF_MIXER_ENABLE)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("Low threshold for mixing with omni microphone. If inferior, then only omni microphone in output signal. Unit is dB"),
    .pControl     = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault     = "0.0",
    .pName        = "tLowDb",
    AUDIO_DESC_PARAM_F(acousticBF_static_config_t, tLowDb, -100.0f, 0.0f)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("High threshold for mixing with omni microphone. If superior, then only processed data in output signal. Unit is dB"),
    .pControl     = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault     = "0.0",
    .pName        = "tHighDb",
    AUDIO_DESC_PARAM_F(acousticBF_static_config_t, tHighDb, -100.0f, 0.0f)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("control activation, 0 control is not active, if n will be call every n*frame size "),
    .pControl     = AUDIO_ALGO_OPT_STR("default"),
    .pDefault     = "0",
    .pName        = "control_refresh",
    AUDIO_DESC_PARAM_U16(acousticBF_static_config_t, control_refresh, 0U, 125U)
  }
};

static const audio_descriptor_param_t s_acousticBF_dynamicParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("Overall gain of the algorithm. It specifies the amount of gain added to the microphones, in dB. It's used only when PDM input is chosen."),
    .pControl     = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault     = "12",
    .pName        = "volume",
    AUDIO_DESC_PARAM_S16(acousticBF_dynamic_config_t, volume, -12, 51)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("chose direction of the beam"),
    .pName        = "direction",
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = s_direction,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(AC_BF_DIR_FRONT),
    AUDIO_DESC_PARAM_U8(acousticBF_dynamic_config_t, direction, AC_BF_DIR_FRONT, AC_BF_DIR_REAR)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("chose a reference signal in right output channel"),
    .pName        = "ref_select",
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = s_refSelect,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(ACOUSTIC_BF_REF_DISABLE),
    AUDIO_DESC_PARAM_U8(acousticBF_dynamic_config_t, ref_select, ACOUSTIC_BF_REF_DISABLE, ACOUSTIC_BF_REF_OPPOSITE_ANTENNA)
  }
};

static const audio_algo_control_param_t s_acousticBF_controlParamsDesc[] =
{
  {
    .pName        = "energy_mic_db",
    .pDescription = AUDIO_ALGO_OPT_STR("energy of microphone data"),
    AUDIO_DESC_CONTROL_F(AcousticBF_Control_t, energy_mic_db)
  },
};


END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_acousticBF_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_acousticBF_staticParamsDesc,
  .nbParams = sizeof(s_acousticBF_staticParamsDesc) / sizeof(s_acousticBF_staticParamsDesc[0]),
  .szBytes  = sizeof(acousticBF_static_config_t)
};

static const audio_descriptor_params_t s_acousticBF_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_acousticBF_dynamicParamsDesc,
  .nbParams = sizeof(s_acousticBF_dynamicParamsDesc) / sizeof(s_acousticBF_dynamicParamsDesc[0]),
  .szBytes  = sizeof(acousticBF_dynamic_config_t)
};

static const audio_algo_control_params_descr_t s_acousticBF_controlParamTemplate =
{
  .pParams  = s_acousticBF_controlParamsDesc,
  .nbParams = sizeof(s_acousticBF_controlParamsDesc) / sizeof(s_acousticBF_controlParamsDesc[0])
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_acousticBF_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_acousticBF_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_acousticBF_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(&s_acousticBF_controlParamTemplate),
  .pCapabilities         = &AudioChainWrp_acousticBF_common,
  .pExecutionCbs         = &AudioChainWrp_acousticBF_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_acousticBF_factory);
