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
#include "stm32_audio_capture_conf.h"
#include "stm32wba65i_discovery.h"

#define UTIL_AUDIO_IN_PCM_USED
//#define UTIL_AUDIO_IN_PDM_USED
//#define UTIL_AUDIO_SAI_PDM_USED

#define UTIL_AUDIO_N_MS_PER_INTERRUPT         10U

#define UTIL_AUDIO_DISABLE_IRQ disable_irq_with_cnt
#define UTIL_AUDIO_ENABLE_IRQ  enable_irq_with_cnt

#define STM32_AUDIO_USE_EXTERN_BUFF

//#define USE_WAVEFILE_IN

#endif /* __STM32_AUDIO_CONF_H */

