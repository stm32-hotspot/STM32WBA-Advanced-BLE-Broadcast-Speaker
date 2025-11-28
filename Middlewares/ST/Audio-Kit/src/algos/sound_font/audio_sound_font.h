/**
  ******************************************************************************
  * @file    audio_sound_font.h
  * @author  MCD Application Team
  * @brief   Decode an SF2 file
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
#ifndef __AUDIO_SOUND_FONT_H
#define __AUDIO_SOUND_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

/* Exported types ------------------------------------------------------------*/
/**
* @brief  Library dynamic configuration handler. It contains dynamic parameters.
*/

typedef struct
{
  uint8_t *soundFontPtr;    /* raw soundFont file address in flash */
  uint8_t ramType;
} sound_font_static_config_t;

typedef struct
{
  uint8_t  gate;
  uint8_t  preset;
  uint8_t  drums;
} sound_font_dynamic_config_t;


/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_SOUND_FONT_H */
