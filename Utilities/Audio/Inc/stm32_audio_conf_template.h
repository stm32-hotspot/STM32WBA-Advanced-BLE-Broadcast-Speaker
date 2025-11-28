/**
  ******************************************************************************
  * @file    stm32_audio_conf.h
  * @author  MCD Application Team
  * @brief   Header for stm32_audio.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018(-2022) STMicroelectronics.
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
// Please include the header of your audio devices here
// for instance for an H735 DK it should be its BSP
// #include "stm32h735g_discovery.h"

#include <stdint.h>
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#ifndef NUMBER_MICS
#define NUMBER_MICS (2)
#endif

#ifndef NUMBER_RENDERING_CHANNELS
#define NUMBER_RENDERING_CHANNELS (2)
#endif

#ifndef UTIL_AUDIO_IN_FREQUENCY
#define UTIL_AUDIO_IN_FREQUENCY AUDIO_FREQUENCY_16K
#endif

#ifndef CONF_AUDIO_IN_DEVICE_DIGITAL_MIC
#define CONF_AUDIO_IN_DEVICE_DIGITAL_MIC AUDIO_IN_DEVICE_DIGITAL_MIC
#endif


#ifdef __cplusplus
}
#endif

#endif /* __STM32_AUDIO_CONF_H */

