/**
******************************************************************************
* @file    audio_chain_instance_conf.h
* @author  MCD Application Team
* @brief   Common header for audio chain instance, whatever the instance is.
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
#ifndef __AUDIO_CHAIN_INSTANCE_CONF_H
#define __AUDIO_CHAIN_INSTANCE_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "traces.h"
#include "stm32_audio.h"
#include "cycles.h"
#include "audio_persist_config.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#if defined(USE_LIVETUNE_DESIGNER) || defined(AUDIO_CHAIN_RELEASE)
#define NO_DEFAULT_TERMINAL
#endif /* USE_LIVETUNE_DESIGNER || AUDIO_CHAIN_RELEASE */


/* overload the HARD CODED audio params by a value read from the registry */
/* Setting for in & out from the registry */
#define AC_FRAME_MS                   (audio_persist_get_config_cur()->audioMs)

//#define AC_SYSOUT_USB_FORMAT_TYPE     (audio_persist_get_config_cur()->ac_sysUsbBuffType)
//#define AC_SYSIN_USB_FORMAT_TYPE      (audio_persist_get_config_cur()->ac_sysUsbBuffType)

#define AC_LOW_LATENCY                (audio_persist_get_config_cur()->audioChainLowLatency)


//#ifndef AC_LOW_LATENCY
//#define AC_LOW_LATENCY                0U
//#endif

#define AC_N_MS_PER_RUN               UTIL_AUDIO_N_MS_PER_INTERRUPT

#define AC_N_MS_DIV                   UTIL_AUDIO_N_MS_DIV

//#ifndef AC_FRAME_MS
//#define AC_FRAME_MS                   8U
//#endif

#define AC_SYSOUT_NBFRAMES            (3U - AC_LOW_LATENCY)

#define AC_SYSIN_BLE_LC3_CHANNELS_NB      UTIL_AUDIO_IN_CH_NB
#define AC_SYSIN_BLE_LC3_FORMAT_DOMAIN    ABUFF_FORMAT_TIME
#define AC_SYSIN_BLE_LC3_NBFRAMES         2U
#define AC_SYSIN_BLE_LC3_FS               UTIL_AUDIO_IN_FREQUENCY
#define AC_SYSIN_BLE_LC3_INTERLEAVED      ABUFF_FORMAT_INTERLEAVED
#define AC_SYSIN_BLE_LC3_MS               AC_FRAME_MS
#define AC_SYSIN_BLE_LC3_FORMAT_TYPE      ABUFF_FORMAT_FIXED16

#define AC_SYSIN_PDM_CHANNELS_NB      2
#define AC_SYSIN_PDM_FORMAT_DOMAIN    ABUFF_FORMAT_TIME
#define AC_SYSIN_PDM_NBFRAMES         2U
#define AC_SYSIN_PDM_FREQ             16000
#define AC_SYSIN_PDM_FS               ((AC_SYSIN_PDM_FREQ == 48000U) ? 3072000U : ((AC_SYSIN_PDM_FREQ == 16000U) ? 2048000U : 0U))
#define AC_SYSIN_PDM_INTERLEAVED      ABUFF_FORMAT_INTERLEAVED
#define AC_SYSIN_PDM_MS               AC_FRAME_MS
#define AC_SYSIN_PDM_FORMAT_TYPE      ABUFF_FORMAT_PDM_LSB_FIRST

#define AC_SYSIN_CODEC_ADC_CHANNELS_NB       UTIL_AUDIO_IN_CH_NB
#define AC_SYSIN_CODEC_ADC_FORMAT_DOMAIN     ABUFF_FORMAT_TIME
#define AC_SYSIN_CODEC_ADC_NBFRAMES          2U
#define AC_SYSIN_CODEC_ADC_FS                UTIL_AUDIO_IN_FREQUENCY
#define AC_SYSIN_CODEC_ADC_INTERLEAVED       ABUFF_FORMAT_INTERLEAVED
#define AC_SYSIN_CODEC_ADC_MS                AC_FRAME_MS
#define AC_SYSIN_CODEC_ADC_FORMAT_TYPE       ABUFF_FORMAT_FIXED16

#define AC_SYSIN_WAVFILE_CHANNELS_NB  1
#define AC_SYSIN_WAVFILE_NBFRAMES     2U
#define AC_SYSIN_WAVFILE_INTERL       ABUFF_FORMAT_INTERLEAVED
#define AC_SYSIN_WAVFILE_FS           16000
#define AC_SYSIN_WAVEFILE_MS          AC_FRAME_MS

#define AC_SYSOUT_BLE_LC3_CHANNELS_NB     UTIL_AUDIO_IN_CH_NB
#define AC_SYSOUT_BLE_LC3_FORMAT_DOMAIN   ABUFF_FORMAT_TIME
#define AC_SYSOUT_BLE_LC3_NBFRAMES        2u
#define AC_SYSOUT_BLE_LC3_FS              UTIL_AUDIO_OUT_FREQUENCY
#define AC_SYSOUT_BLE_LC3_INTERLEAVED     ABUFF_FORMAT_INTERLEAVED
#define AC_SYSOUT_BLE_LC3_MS              AC_FRAME_MS
#define AC_SYSOUT_BLE_LC3_FORMAT_TYPE     ABUFF_FORMAT_FIXED16


#define AC_SYSOUT_CODEC_DAC_CHANNELS_NB     UTIL_AUDIO_OUT_CH_NB
#define AC_SYSOUT_CODEC_DAC_FORMAT_DOMAIN   ABUFF_FORMAT_TIME
#define AC_SYSOUT_CODEC_DAC_NBFRAMES        2U
#define AC_SYSOUT_CODEC_DAC_FS              UTIL_AUDIO_OUT_FREQUENCY
#define AC_SYSOUT_CODEC_DAC_INTERLEAVED     ABUFF_FORMAT_INTERLEAVED
#define AC_SYSOUT_CODEC_DAC_MS              AC_FRAME_MS
#define AC_SYSOUT_CODEC_DAC_FORMAT_TYPE     ABUFF_FORMAT_FIXED16


#define AC_SYSIN_WAVFILE_NB_ELEMENTS_PER_FRAME (((uint32_t)AC_SYSIN_WAVFILE_FS * AC_FRAME_MS / 1000UL) / AC_N_MS_DIV)
#define AC_SYSIN_BLE_LC3_NB_ELEMENTS_PER_FRAME  (((uint32_t)AC_SYSIN_BLE_LC3_MS  * AC_SYSIN_BLE_LC3_FS  / 1000UL) / AC_N_MS_DIV)
#define AC_SYSIN_CODEC_ADC_NB_ELEMENTS_PER_FRAME   (((uint32_t)AC_SYSIN_CODEC_ADC_MS  * AC_SYSIN_CODEC_ADC_FS  / 1000UL) / AC_N_MS_DIV)
#define AC_SYSIN_PDM_NB_ELEMENTS_PER_FRAME  (((uint32_t)AC_SYSIN_PDM_MS  * AC_SYSIN_PDM_FS  / 1000UL) / AC_N_MS_DIV)
#define AC_SYSOUT_CODEC_DAC_NB_ELEMENTS_PER_FRAME (((uint32_t)AC_SYSOUT_CODEC_DAC_MS * AC_SYSOUT_CODEC_DAC_FS / 1000UL) / AC_N_MS_DIV)
#define AC_SYSOUT_BLE_LC3_NB_ELEMENTS_PER_FRAME AC_SYSIN_BLE_LC3_NB_ELEMENTS_PER_FRAME

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_INSTANCE_CONF_H */

