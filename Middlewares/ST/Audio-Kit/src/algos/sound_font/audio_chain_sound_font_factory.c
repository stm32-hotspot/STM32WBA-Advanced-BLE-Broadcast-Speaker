/**
******************************************************************************
* @file    audio_chain_sound_font_factory.c
* @author  MCD Application Team
* @brief   play a sound font file
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

#include "audio_chain_conf.h"
#ifdef USE_SOUND_FONT
#include "audio_chain_sound_font.h"
#include "st_flash_storage_conf.h"
#include <stdio.h>

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static char tDefaultSoundFontPointer[12];

/**
* @brief change the flash address according to the board at the factory construction
*/

static void AudioChainWrp_sound_font_factory_cb(const audio_algo_factory_t *pFactory)
{
  snprintf(tDefaultSoundFontPointer, sizeof(tDefaultSoundFontPointer), "0x%08X", ST_FLASH_SLOT2_BASE_ADDR);
}


static const audio_descriptor_param_t s_sound_font_staticParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl     = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue    = tRamTypeKeyValue,
    .pDefault     = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMEXT),
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName        = "ramType",
    AUDIO_DESC_PARAM_U8(sound_font_static_config_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("SoundFont file pointer in flash. Notice: For livetune, the address is not modifiable and can be variable according to the board."),
    .pDefault     = (const char *)tDefaultSoundFontPointer,
    .pControl     = AUDIO_ALGO_OPT_STR("address"),
    .pName        = "SoundFont ptr",
    .iParamFlag   = AUDIO_DESC_PARAM_TYPE_FLAG_DISABLED | AUDIO_DESC_PARAM_TYPE_FLAG_ALWAYS_DEFAULT, // tell to the designer to gray this param
    AUDIO_DESC_PARAM_ADDR(sound_font_static_config_t, soundFontPtr)
  }
};

static const audio_descriptor_param_t s_sound_font_dynamicParamsDesc[] =
{
  {
    .pDescription = AUDIO_ALGO_OPT_STR("Play a note on at the button down and a note off at the buuton up "),
    .pControl     = AUDIO_ALGO_OPT_STR("button"),
    .pDefault     = "0",
    .pName        = "gate",
    AUDIO_DESC_PARAM_U8(sound_font_dynamic_config_t, gate, 0U, 1U)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("Select an instrument according to the sf2 file from 0 to 128 ( General Midi), if standard drump is selected, the index is the General MIDI Drum Kit Map (35 to 82)"),
    .pControl     = AUDIO_ALGO_OPT_STR("default"),
    .pDefault     = "51",
    .pName        = "preset",
    AUDIO_DESC_PARAM_U8(sound_font_dynamic_config_t, preset, 0U, 128U)
  },
  {
    .pDescription = AUDIO_ALGO_OPT_STR("The index is the General MIDI Drum Kit Map (35 to 82) "),
    .pControl     = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault     = "0",
    .pName        = "MIDI Drum Kit",
    AUDIO_DESC_PARAM_U8(sound_font_dynamic_config_t, drums, 0U, 128U)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/


static const audio_descriptor_params_t s_sound_font_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_sound_font_staticParamsDesc,
  .nbParams = sizeof(s_sound_font_staticParamsDesc) / sizeof(s_sound_font_staticParamsDesc[0]),
  .szBytes  = sizeof(sound_font_static_config_t)
};

static const audio_descriptor_params_t s_sound_font_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_sound_font_dynamicParamsDesc,
  .nbParams = sizeof(s_sound_font_dynamicParamsDesc) / sizeof(s_sound_font_dynamicParamsDesc[0]),
  .szBytes  = sizeof(sound_font_dynamic_config_t)
};

#endif // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_sound_font_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_sound_font_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_sound_font_dynamicParamTemplate),
  .pControlTemplate      = NULL,
  .pCapabilities         = &AudioChainWrp_sound_font_common,
  .pExecutionCbs         = &AudioChainWrp_sound_font_cbs,
  .pFactoryConstructorCb = &AudioChainWrp_sound_font_factory_cb
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_sound_font_factory);

#endif // USE_SOUND_FONT