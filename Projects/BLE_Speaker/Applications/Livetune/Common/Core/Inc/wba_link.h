/**
******************************************************************************
* @file
* @author  MCD Application Team
* @brief
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
#include "stdint.h"

typedef uint8_t audio_mode_t;
#define MODE_MEDIA_48k   0
#define MODE_MEDIA_24k   1
#define MODE_TELEPHONY   2

void EXTI13_IRQHandler(void);

/* returns the audio configuration asked by the WBA */
audio_mode_t WBA_link_init(void);

void app_sleep(void);

void Error_Handler(void);