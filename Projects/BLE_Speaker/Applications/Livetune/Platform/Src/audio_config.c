/**
******************************************************************************
* @file        audio_config.c
* @author      MCD Application Team
* @brief       audio config
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
* All rights reserved.</center></h2>
*
* This software component is licensed by ST under Ultimate Liberty license
* SLA0044, the "License"; You may not use this file except in compliance with
* the License. You may obtain a copy of the License at:
*                             www.st.com/SLA0044
*
******************************************************************************
*/
#include "audio_persist_config.h"
#include "stm32_audio_conf.h"



/* config for the main path : alternate path does not change */
static const audio_persist_config tAudioConfig[] =
{
  {
    .pConfigName            = "media48kHz",
    .audioMs                = 8U,
    .audioInFreq            = 48000U,
    .audioInCh              = 2U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 48000U,
    .audioOutCh             = 2U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 2UL,
    .audioInUsePdm          = 0
  },

  {
    .pConfigName            = "media32kHz",
    .audioMs                = 10U,
    .audioInFreq            = 32000U,
    .audioInCh              = 2U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 32000U,
    .audioOutCh             = 2U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 2UL,
    .audioInUsePdm          = 0
  },

  {
    .pConfigName            = "media24kHz",
    .audioMs                = 8U,
    .audioInFreq            = 24000U,
    .audioInCh              = 2U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 24000U,
    .audioOutCh             = 2U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 2UL,
    .audioInUsePdm          = 0
  },

  {
    .pConfigName            = "media16kHz",
    .audioMs                = 10U,
    .audioInFreq            = 16000U,
    .audioInCh              = 2U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 16000U,
    .audioOutCh             = 2U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 2UL,
    .audioInUsePdm          = 0
  },

  {
    .pConfigName            = "media8kHz",
    .audioMs                = 10U,
    .audioInFreq            = 8000U,
    .audioInCh              = 2U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 8000U,
    .audioOutCh             = 2U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 2UL,
    .audioInUsePdm          = 0
  },

  /*{
    .pConfigName            = "telephony48kHz",
    .audioMs                = 10U,
    .audioInFreq            = 32000U,
    .audioInCh              = 1U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 32000U,
    .audioOutCh             = 1U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 1UL,
    .audioInUsePdm          = 0
  },*/

  {
    .pConfigName            = "telephony32kHz",
    .audioMs                = 10U,
    .audioInFreq            = 32000U,
    .audioInCh              = 1U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 32000U,
    .audioOutCh             = 1U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 1UL,
    .audioInUsePdm          = 0
  },

  {
    .pConfigName            = "telephony24kHz",
    .audioMs                = 10U,
    .audioInFreq            = 24000U,
    .audioInCh              = 1U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 24000U,
    .audioOutCh             = 1U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 1UL,
    .audioInUsePdm          = 0
  },

  {
    .pConfigName            = "telephony16kHz",
    .audioMs                = 10U,
    .audioInFreq            = 16000U,
    .audioInCh              = 1U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 16000U,
    .audioOutCh             = 1U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 1UL,
    .audioInUsePdm          = 0
  },

  {
    .pConfigName            = "telephony8kHz",
    .audioMs                = 10U,
    .audioInFreq            = 8000U,
    .audioInCh              = 1U,
    .audioInHwInterleave    = 1U,
    .audioOutFreq           = 8000U,
    .audioOutCh             = 1U,
    .audioOutResolution     = 16,

    .audioInResolution      = 16,
    .audioInHwResolution    = 16,
    .audioInHwCh            = 1UL,
    .audioInUsePdm          = 0
  },
};



/**
* @brief  return the config struct
*
* @param index config index
* @return the config struct
*/
const audio_persist_config *audio_persist_get_config(int32_t index)
{
  return &tAudioConfig[index];
}

/**
* @brief  return the number of config
*
* @return config number
*/

int32_t audio_persist_get_config_number(void)
{
  return (int32_t)(sizeof(tAudioConfig) / sizeof(tAudioConfig[0]));
}
