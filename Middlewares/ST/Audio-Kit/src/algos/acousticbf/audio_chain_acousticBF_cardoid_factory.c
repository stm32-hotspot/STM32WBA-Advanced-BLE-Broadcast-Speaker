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
#include "audio_chain_acousticBF_cardoid.h"


#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_key_value_t s_direction[] =
{
  {"front",            ACOUSTIC_BF_CARDOID_DIRECTION_FRONT},
  {"rear",             ACOUSTIC_BF_CARDOID_DIRECTION_REAR},
  {0}
};

static const audio_descriptor_key_value_t s_ref_select[] =
{
  {"none (mono out)",  ACOUSTIC_BF_CARDOID_REF_DISABLE},
  {"raw microphone",   ACOUSTIC_BF_CARDOID_REF_RAW_MICROPHONE},
  {"rear antenna",     ACOUSTIC_BF_CARDOID_REF_OPPOSITE_ANTENNA},
  {0}
};

static const audio_descriptor_key_value_t s_rear_enable[] =
{
  {"front only",       ACOUSTIC_BF_CARDOID_REAR_DISABLE},
  {"front & rear",     ACOUSTIC_BF_CARDOID_REAR_ENABLE},
  {0}
};


static const audio_descriptor_key_value_t s_delay_enable[] =
{
  {"delay managed by user",       ACOUSTIC_BF_CARDOID_DELAY_DISABLE},
  {"delay managed by algorithm",  ACOUSTIC_BF_CARDOID_DELAY_ENABLE},
  {0}
};


#define ACOUSTIC_BF_MIN_MIC_DISTANCE 30
#define ACOUSTIC_BF_MAX_MIC_DISTANCE 212

static const audio_descriptor_param_t s_acousticBF_cardoid_staticParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue    = tRamTypeKeyValue,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName        = "ramType",
    AUDIO_DESC_PARAM_U8(acousticBF_cardoid_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("microphone distance (tenths of a millimeter). In case of PCM input, mic distance must be in the range [210-212]!"),
    .pControl     = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault     = "210",
    .pName        = "mic_distance",
    AUDIO_DESC_PARAM_U16(acousticBF_cardoid_static_config_t, mic_distance, ACOUSTIC_BF_MIN_MIC_DISTANCE, ACOUSTIC_BF_MAX_MIC_DISTANCE)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("chose if beamformer should output one front antenna or two opposite ones"),
    .pName        = "rear_enable",
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = s_rear_enable,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(ACOUSTIC_BF_CARDOID_REAR_ENABLE),
    AUDIO_DESC_PARAM_U8(acousticBF_cardoid_static_config_t, rear_enable, ACOUSTIC_BF_CARDOID_REAR_DISABLE, ACOUSTIC_BF_CARDOID_REAR_ENABLE)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("beamformer can manage delaying of samples internally or not; if disable, the user must align microphone before sending data"),
    .pName        = "delay_enable",
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = s_delay_enable,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(ACOUSTIC_BF_CARDOID_DELAY_ENABLE),
    AUDIO_DESC_PARAM_U8(acousticBF_cardoid_static_config_t, delay_enable, ACOUSTIC_BF_CARDOID_DELAY_DISABLE, ACOUSTIC_BF_CARDOID_DELAY_ENABLE)
  }
};

static const audio_descriptor_param_t s_acousticBF_cardoid_dynamicParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("chose direction of the beam"),
    .pName        = "direction",
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = s_direction,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(ACOUSTIC_BF_CARDOID_DIRECTION_FRONT),
    AUDIO_DESC_PARAM_U8(acousticBF_cardoid_dynamic_config_t, direction, ACOUSTIC_BF_CARDOID_DIRECTION_FRONT, ACOUSTIC_BF_CARDOID_DIRECTION_REAR)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("chose signal in right output channel"),
    .pName        = "ref_select",
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue    = s_ref_select,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(ACOUSTIC_BF_CARDOID_REF_DISABLE),
    AUDIO_DESC_PARAM_U8(acousticBF_cardoid_dynamic_config_t, ref_select, ACOUSTIC_BF_CARDOID_REF_DISABLE, ACOUSTIC_BF_CARDOID_REF_OPPOSITE_ANTENNA)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("Overall gain of the algorithm. It specifies the amount of gain added to the microphones, in dB. It's used only when PDM input is chosen."),
    .pControl     = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault     = "12",
    .pName        = "volume",
    AUDIO_DESC_PARAM_S16(acousticBF_cardoid_dynamic_config_t, volume, -12, 51)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_acousticBF_cardoid_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_acousticBF_cardoid_staticParamsDesc,
  .nbParams = sizeof(s_acousticBF_cardoid_staticParamsDesc) / sizeof(s_acousticBF_cardoid_staticParamsDesc[0]),
  .szBytes  = sizeof(acousticBF_cardoid_static_config_t)
};

static const audio_descriptor_params_t s_acousticBF_cardoid_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_acousticBF_cardoid_dynamicParamsDesc,
  .nbParams = sizeof(s_acousticBF_cardoid_dynamicParamsDesc) / sizeof(s_acousticBF_cardoid_dynamicParamsDesc[0]),
  .szBytes  = sizeof(acousticBF_cardoid_dynamic_config_t)
};


#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_acousticBF_cardoid_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_acousticBF_cardoid_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_acousticBF_cardoid_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_acousticBF_cardoid_common,
  .pExecutionCbs         = &AudioChainWrp_acousticBF_cardoid_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_acousticBF_cardoid_factory);
