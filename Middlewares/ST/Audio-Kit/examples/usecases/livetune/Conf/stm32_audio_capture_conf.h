/**
  ******************************************************************************
  * @file    stm32_audio_capture_conf.h
  * @author  MCD Application Team
  * @brief   Header for stm32_audio_*.c module
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
#ifndef __STM32_AUDIO_CAPTURE_CONF_H
#define __STM32_AUDIO_CAPTURE_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_persist_config.h"

/* Exported constants --------------------------------------------------------*/
/* Setting for usb from the registry */
#define UTIL_AUDIO_USB_BIT_RESOLUTION         (audio_persist_get_config_cur()->usbResolution)

/* Setting for audio out from the registry */
#define UTIL_AUDIO_OUT_VOLUME                 (audio_persist_get_config_cur()->audioOutVolume)
#define UTIL_AUDIO_OUT_FREQUENCY              (audio_persist_get_config_cur()->audioOutFreq)
#define UTIL_AUDIO_OUT_CH_NB                  (audio_persist_get_config_cur()->audioOutCh)
#define UTIL_AUDIO_OUT_BIT_RESOLUTION         (audio_persist_get_config_cur()->audioOutResolution)

#define UTIL_AUDIO_IN_FREQUENCY               (audio_persist_get_config_cur()->audioInFreq)
#define UTIL_AUDIO_IN_CH_NB                   (audio_persist_get_config_cur()->audioInCh)
#define UTIL_AUDIO_IN_IS_DMA_DATA_INTERLEAVED (audio_persist_get_config_cur()->audioInHwInterleave)

/* Setting for audio in from the registry */
#define UTIL_AUDIO_USE_MIC_PDM                (audio_persist_get_config_cur()->audioInUsePdm)

#define UTIL_AUDIO_IN_VOLUME                  (audio_persist_get_config_cur()->audioInVolume)
#define UTIL_AUDIO_IN_BIT_RESOLUTION          (audio_persist_get_config_cur()->audioInResolution)
#define UTIL_AUDIO_IN_DMA_BIT_RESOLUTION      (audio_persist_get_config_cur()->audioInHwResolution)
#define UTIL_AUDIO_IN_INSTANCE                (audio_persist_get_config_cur()->audioInDeviceOutFormat)
#define UTIL_AUDIO_IN_DEVICE_SELECT           (audio_persist_get_config_cur()->audioInDeviceSelect)
#define UTIL_AUDIO_IN_HW_CH_NB                (audio_persist_get_config_cur()->audioInHwCh)
#define UTIL_AUDIO_IN_PDM2PCM_USED            (audio_persist_get_config_cur()->audioInUsePdm2Pcm)

/* Set max value possible for this application (for allocation purposes in nonCache zone to avoid maintenance) */
#define UTIL_AUDIO_MAX_IN_FREQUENCY           48000UL
#define UTIL_AUDIO_MAX_OUT_FREQUENCY          48000UL
#define UTIL_AUDIO_MAX_IN_CH_NB               2U
#define UTIL_AUDIO_MAX_OUT_CH_NB              2U

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __STM32_AUDIO_CAPTURE_CONF_H */

