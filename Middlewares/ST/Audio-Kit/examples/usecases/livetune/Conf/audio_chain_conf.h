/**
******************************************************************************
* @file    audio_chain_conf.h
* @author  MCD Application Team
* @brief   Configuration header for audio chain. Helps to enabledisable features
*          with big impact on memory.
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
#ifndef __AUDIO_CHAIN_CONF_H
#define __AUDIO_CHAIN_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_audio_conf.h"
/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/



/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

// Define supported sampling frequencies
#define AC_SUPPORT_FS_8000
#define AC_SUPPORT_FS_12000
#define AC_SUPPORT_FS_16000
#define AC_SUPPORT_FS_24000
#define AC_SUPPORT_FS_32000
#define AC_SUPPORT_FS_48000
#if UTIL_AUDIO_MAX_IN_FREQUENCY > 48000UL
#define AC_SUPPORT_FS_96000
#endif


#define AC_SUPPORT_RATIO_2
#define AC_SUPPORT_RATIO_3
#define AC_SUPPORT_RATIO_4
#define AC_SUPPORT_RATIO_6
#define AC_SUPPORT_RATIO_8
#ifdef AC_SUPPORT_FS_96000
#define AC_SUPPORT_RATIO_12
#endif

//#define USE_RESAMPLE_TYPE_FIR
#define USE_RESAMPLE_TYPE_IIR

//#define USE_HPF_TYPE_FIR
#define USE_HPF_TYPE_IIR
#if defined(TUD_AUDIO_IN_USED) && defined(CFG_TUD_MIDI_IN)
#define USE_SOUND_FONT  // no sense to build this algo without USB midi support
#endif

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_INSTANCE_H */

