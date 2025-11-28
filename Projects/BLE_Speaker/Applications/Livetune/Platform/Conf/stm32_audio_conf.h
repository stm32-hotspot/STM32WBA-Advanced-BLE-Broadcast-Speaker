/**
  ******************************************************************************
  * @file    stm32_audio_conf.h
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
#ifndef __STM32_AUDIO_CONF_H
#define __STM32_AUDIO_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "platform_def.h"
#include "stm32_audio_capture_conf.h"
#include "BLE_Speaker_H5_bsp_audio.h"
#include "irq_utils.h"

#ifdef USE_THREADS
#define UTIL_AUDIO_RTOS_USED
#endif

#define UTIL_AUDIO_IN_PCM_USED
#define UTIL_AUDIO_IN_PDM_USED
//#define UTIL_AUDIO_SAI_PDM_USED

#ifndef UTIL_AUDIO_IN_INSTANCE
#define UTIL_AUDIO_IN_INSTANCE (1U) /* SAI PDM */
#endif


#ifndef UTIL_AUDIO_IN_CH_NB
#ifdef UTIL_AUDIO_MICONBOARD
#define UTIL_AUDIO_IN_CH_NB                   1U
#else
#define UTIL_AUDIO_IN_CH_NB                   2U
#endif
#endif

#ifndef UTIL_AUDIO_OUT_CH_NB
#define UTIL_AUDIO_OUT_CH_NB                  2U
#endif
#ifndef UTIL_AUDIO_USE_MIC_PDM
#define UTIL_AUDIO_USE_MIC_PDM                1U
#endif

#ifndef UTIL_AUDIO_IN_HW_CH_NB
#ifdef UTIL_AUDIO_MICONBOARD
#define UTIL_AUDIO_IN_HW_CH_NB                1U
#else
#define UTIL_AUDIO_IN_HW_CH_NB                2U
#endif
#endif
#ifndef UTIL_AUDIO_IN_DEVICE_SELECT
#define UTIL_AUDIO_IN_DEVICE_SELECT           AUDIO_IN_DEVICE_DIGITAL_MIC
#endif

#define UTIL_AUDIO_MEMORY_SECTION_HW_BUFFERS  ".noncacheable"
#define UTIL_AUDIO_OUT_DEVICE                 AUDIO_OUT_DEVICE_HEADPHONE

#ifndef UTIL_AUDIO_IN_BIT_RESOLUTION
#define UTIL_AUDIO_IN_BIT_RESOLUTION          AUDIO_RESOLUTION_16B
#endif

#ifndef UTIL_AUDIO_IN_BIT_RESOLUTION
#define UTIL_AUDIO_IN_DMA_BIT_RESOLUTION      AUDIO_RESOLUTION_8B
#endif

#ifndef UTIL_AUDIO_IN_VOLUME
#define UTIL_AUDIO_IN_VOLUME                  80
#endif


#ifndef UTIL_AUDIO_OUT_BIT_RESOLUTION
#define UTIL_AUDIO_OUT_BIT_RESOLUTION         AUDIO_RESOLUTION_16B
#endif

#ifndef UTIL_AUDIO_OUT_VOLUME
#define UTIL_AUDIO_OUT_VOLUME                 95
#endif

#define UTIL_AUDIO_DISABLE_IRQ disable_irq_with_cnt
#define UTIL_AUDIO_ENABLE_IRQ  enable_irq_with_cnt

//#define STM32_AUDIO_USE_EXTERN_BUFF

#define USE_WAVEFILE_IN


/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __STM32_AUDIO_CONF_H */

