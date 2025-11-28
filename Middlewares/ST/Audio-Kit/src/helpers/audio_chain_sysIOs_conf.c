/**
******************************************************************************
* @file    audio_chain_sysIOs_conf.c
* @author  MCD Application Team
* @brief   Helper file that creates input/output system chunks
*          parallel audio path
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
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "audio_chain_sysIOs.h"
#include "audio_chain_sysIOs_conf.h"


/* Global variables ----------------------------------------------------------*/
const char *tLiveTuneDefineConv[][2] =
{
  {"AC_SYSIN_PDM_NAME",  AC_SYSIN_PDM_NAME},
  {"AC_SYSIN_BLE_LC3_NAME",  AC_SYSIN_BLE_LC3_NAME},
  {"AC_SYSIN_CODEC_ADC_NAME", AC_SYSIN_CODEC_ADC_NAME},  
  {"AC_SYSIN_WAVFILE_NAME",  AC_SYSIN_WAVFILE_NAME},

  {"AC_SYSOUT_CODEC_DAC_NAME", AC_SYSOUT_CODEC_DAC_NAME},
  {"AC_SYSOUT_BLE_LC3_NAME",  AC_SYSOUT_BLE_LC3_NAME},
  {0, 0}
};

/* Private defines -----------------------------------------------------------*/

/* Default conf for System Chunks:
*  -------------------------------
*/

/* Default conf SysInChunk microphone */
#ifndef AC_SYSIN_CODEC_ADC_CHANNELS_NB
  #define AC_SYSIN_CODEC_ADC_CHANNELS_NB 2U
#endif

#ifndef AC_SYSIN_CODEC_ADC_FORMAT_DOMAIN
  #define AC_SYSIN_CODEC_ADC_FORMAT_DOMAIN ABUFF_FORMAT_TIME
#endif

#ifndef AC_SYSIN_CODEC_ADC_FORMAT_TYPE
  #define AC_SYSIN_CODEC_ADC_FORMAT_TYPE ABUFF_FORMAT_FIXED16
#endif

#ifndef AC_SYSIN_CODEC_ADC_NBFRAMES
  #define AC_SYSIN_CODEC_ADC_NBFRAMES 1U
#endif

#ifndef AC_SYSIN_CODEC_ADC_FS
  #define AC_SYSIN_CODEC_ADC_FS 16000U
#endif

#ifndef AC_SYSIN_CODEC_ADC_INTERLEAVED
  #define AC_SYSIN_CODEC_ADC_INTERLEAVED ABUFF_FORMAT_INTERLEAVED
#endif

#ifndef AC_SYSIN_CODEC_ADC_MS
  #define AC_SYSIN_CODEC_ADC_MS 1U
#endif


/* Default conf SysInChunk USB - all zeros (disactivated by default) */
#ifndef AC_SYSIN_USB_CHANNELS_NB
  #define AC_SYSIN_USB_CHANNELS_NB 0U
#endif

#ifndef AC_SYSIN_USB_FORMAT_DOMAIN
  #define AC_SYSIN_USB_FORMAT_DOMAIN ABUFF_FORMAT_TIME
#endif

#ifndef AC_SYSIN_USB_FORMAT_TYPE
  #define AC_SYSIN_USB_FORMAT_TYPE ABUFF_FORMAT_FIXED16
#endif

#ifndef AC_SYSIN_USB_NBFRAMES
  #define AC_SYSIN_USB_NBFRAMES 0U
#endif

#ifndef AC_SYSIN_USB_FS
  #define AC_SYSIN_USB_FS 0U
#endif

#ifndef AC_SYSIN_USB_INTERLEAVED
  #define AC_SYSIN_USB_INTERLEAVED ABUFF_FORMAT_INTERLEAVED
#endif

#ifndef AC_SYSIN_USB_MS
  #define AC_SYSIN_USB_MS 0U
#endif


/* Default conf SysInChunk ethernet - all zeros (disactivated by default) */
#ifndef AC_SYSIN_ETH_CHANNELS_NB
  #define AC_SYSIN_ETH_CHANNELS_NB 0U
#endif

#ifndef AC_SYSIN_ETH_FORMAT_DOMAIN
  #define AC_SYSIN_ETH_FORMAT_DOMAIN ABUFF_FORMAT_TIME
#endif

#ifndef AC_SYSIN_ETH_FORMAT_TYPE
  #define AC_SYSIN_ETH_FORMAT_TYPE ABUFF_FORMAT_FIXED16
#endif

#ifndef AC_SYSIN_ETH_NBFRAMES
  #define AC_SYSIN_ETH_NBFRAMES 0U
#endif

#ifndef AC_SYSIN_ETH_FS
  #define AC_SYSIN_ETH_FS 0U
#endif

#ifndef AC_SYSIN_ETH_INTERLEAVED
  #define AC_SYSIN_ETH_INTERLEAVED ABUFF_FORMAT_INTERLEAVED
#endif

#ifndef AC_SYSIN_ETH_MS
  #define AC_SYSIN_ETH_MS 0U
#endif


/* Default conf SysInChunk PDM microphone - all zeros (disactivated by default) */
#ifndef AC_SYSIN_PDM_CHANNELS_NB
  #define AC_SYSIN_PDM_CHANNELS_NB 0U
#endif

#ifndef AC_SYSIN_PDM_FORMAT_DOMAIN
  #define AC_SYSIN_PDM_FORMAT_DOMAIN ABUFF_FORMAT_TIME
#endif

#ifndef AC_SYSIN_PDM_FORMAT_TYPE
  #define AC_SYSIN_PDM_FORMAT_TYPE ABUFF_FORMAT_PDM_LSB_FIRST
#endif

#ifndef AC_SYSIN_PDM_NBFRAMES
  #define AC_SYSIN_PDM_NBFRAMES 0U
#endif

#ifndef AC_SYSIN_PDM_FS
  #define AC_SYSIN_PDM_FS 0U
#endif

#ifndef AC_SYSIN_PDM_INTERLEAVED
  #define AC_SYSIN_PDM_INTERLEAVED ABUFF_FORMAT_INTERLEAVED
#endif

#ifndef AC_SYSIN_PDM_MS
  #define AC_SYSIN_PDM_MS 0U
#endif


/* Default conf SysOutChunk USB */
#ifndef AC_SYSOUT_USB_CHANNELS_NB
  #define AC_SYSOUT_USB_CHANNELS_NB 2U
#endif

#ifndef AC_SYSOUT_USB_FORMAT_DOMAIN
  #define AC_SYSOUT_USB_FORMAT_DOMAIN ABUFF_FORMAT_TIME
#endif

#ifndef AC_SYSOUT_USB_FORMAT_TYPE
  #define AC_SYSOUT_USB_FORMAT_TYPE ABUFF_FORMAT_FIXED16
#endif

#ifndef AC_SYSOUT_USB_NBFRAMES
  #define AC_SYSOUT_USB_NBFRAMES 1U
#endif

#ifndef AC_SYSOUT_USB_FS
  #define AC_SYSOUT_USB_FS 16000U
#endif

#ifndef AC_SYSOUT_USB_INTERLEAVED
  #define AC_SYSOUT_USB_INTERLEAVED ABUFF_FORMAT_INTERLEAVED
#endif

#ifndef AC_SYSOUT_USB_MS
  #define AC_SYSOUT_USB_MS 0U
#endif


/* Default conf SysOutChunk speaker - all zeros (disactivated by default) */
#ifndef AC_SYSOUT_CODEC_DAC_CHANNELS_NB
  #define AC_SYSOUT_CODEC_DAC_CHANNELS_NB 0U
#endif

#ifndef AC_SYSOUT_CODEC_DAC_FORMAT_DOMAIN
  #define AC_SYSOUT_CODEC_DAC_FORMAT_DOMAIN ABUFF_FORMAT_TIME
#endif

#ifndef AC_SYSOUT_CODEC_DAC_FORMAT_TYPE
  #define AC_SYSOUT_CODEC_DAC_FORMAT_TYPE ABUFF_FORMAT_FIXED16
#endif

#ifndef AC_SYSOUT_CODEC_DAC_NBFRAMES
  #define AC_SYSOUT_CODEC_DAC_NBFRAMES 0U
#endif

#ifndef AC_SYSOUT_CODEC_DAC_FS
  #define AC_SYSOUT_CODEC_DAC_FS 0U
#endif

#ifndef AC_SYSOUT_CODEC_DAC_INTERLEAVED
  #define AC_SYSOUT_CODEC_DAC_INTERLEAVED ABUFF_FORMAT_INTERLEAVED
#endif

#ifndef AC_SYSOUT_CODEC_DAC_MS
  #define AC_SYSOUT_CODEC_DAC_MS 0U
#endif


/* Default conf SysOutChunk ethernet - all zeros (disactivated by default) */
#ifndef AC_SYSOUT_ETH_CHANNELS_NB
  #define AC_SYSOUT_ETH_CHANNELS_NB 0U
#endif

#ifndef AC_SYSOUT_ETH_FORMAT_DOMAIN
  #define AC_SYSOUT_ETH_FORMAT_DOMAIN ABUFF_FORMAT_TIME
#endif

#ifndef AC_SYSOUT_ETH_FORMAT_TYPE
  #define AC_SYSOUT_ETH_FORMAT_TYPE ABUFF_FORMAT_FIXED16
#endif

#ifndef AC_SYSOUT_ETH_NBFRAMES
  #define AC_SYSOUT_ETH_NBFRAMES 0U
#endif

#ifndef AC_SYSOUT_ETH_FS
  #define AC_SYSOUT_ETH_FS 0U
#endif

#ifndef AC_SYSOUT_ETH_INTERLEAVED
  #define AC_SYSOUT_ETH_INTERLEAVED ABUFF_FORMAT_INTERLEAVED
#endif

#ifndef AC_SYSOUT_ETH_MS
  #define AC_SYSOUT_ETH_MS 0U
#endif


#ifndef AC_SYSIN_CODEC_ADC_NB_ELEMENTS_PER_FRAME
  #define AC_SYSIN_CODEC_ADC_NB_ELEMENTS_PER_FRAME  (((uint32_t)AC_SYSIN_CODEC_ADC_MS  * AC_SYSIN_CODEC_ADC_FS  / 1000UL) /AC_N_MS_DIV)
#endif

#ifndef AC_SYSIN_USB_NB_ELEMENTS_PER_FRAME
  #define AC_SYSIN_USB_NB_ELEMENTS_PER_FRAME  (((uint32_t)AC_SYSIN_USB_MS  * AC_SYSIN_USB_FS  / 1000UL) /AC_N_MS_DIV)
#endif

#ifndef AC_SYSIN_ETH_NB_ELEMENTS_PER_FRAME
  #define AC_SYSIN_ETH_NB_ELEMENTS_PER_FRAME  (((uint32_t)AC_SYSIN_ETH_MS  * AC_SYSIN_ETH_FS  / 1000UL) /AC_N_MS_DIV)
#endif

#ifndef AC_SYSIN_PDM_NB_ELEMENTS_PER_FRAME
  #define AC_SYSIN_PDM_NB_ELEMENTS_PER_FRAME  (((uint32_t)AC_SYSIN_PDM_MS  * AC_SYSIN_PDM_FS  / 1000UL) /AC_N_MS_DIV)
#endif

#ifndef AC_SYSOUT_USB_NB_ELEMENTS_PER_FRAME
  #define AC_SYSOUT_USB_NB_ELEMENTS_PER_FRAME (((uint32_t)AC_SYSOUT_USB_MS * AC_SYSOUT_USB_FS / 1000UL) /AC_N_MS_DIV)
#endif

#ifndef AC_SYSOUT_CODEC_DAC_NB_ELEMENTS_PER_FRAME
  #define AC_SYSOUT_CODEC_DAC_NB_ELEMENTS_PER_FRAME (((uint32_t)AC_SYSOUT_CODEC_DAC_MS * AC_SYSOUT_CODEC_DAC_FS / 1000UL) /AC_N_MS_DIV)
#endif

#ifndef AC_SYSOUT_ETH_NB_ELEMENTS_PER_FRAME
  #define AC_SYSOUT_ETH_NB_ELEMENTS_PER_FRAME (((uint32_t)AC_SYSOUT_ETH_MS * AC_SYSOUT_ETH_FS / 1000UL) /AC_N_MS_DIV)
#endif


/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static ac_io_descr_t sys_in_ios[AC_NB_MAX_SYS_IN];
static ac_io_descr_t sys_out_ios[AC_NB_MAX_SYS_OUT];

static const ac_sys_ios_t sys_ios =
{
  .in =
  {
    .nb    = AC_NB_MAX_SYS_IN,
    .pIos  = sys_in_ios
  },
  .out =
  {
    .nb   = AC_NB_MAX_SYS_OUT,
    .pIos = sys_out_ios
  }
};


/* Functions Definition ------------------------------------------------------*/

const ac_sys_ios_t *AudioChainSysIOs_get(void)
{
  /* microphones PDM format if available - depends on HW used to capture microphones data */
  sys_in_ios[AC_SYSIN_PDM].conf.chunkType        = (uint8_t)AUDIO_CHUNK_TYPE_SYS_IN;
  sys_in_ios[AC_SYSIN_PDM].conf.nbChannels       = (uint8_t)AC_SYSIN_PDM_CHANNELS_NB;
  sys_in_ios[AC_SYSIN_PDM].conf.fs               = AC_SYSIN_PDM_FS;
  sys_in_ios[AC_SYSIN_PDM].conf.nbElements       = AC_SYSIN_PDM_NB_ELEMENTS_PER_FRAME;
  sys_in_ios[AC_SYSIN_PDM].conf.nbFrames         = AC_SYSIN_PDM_NBFRAMES;
  sys_in_ios[AC_SYSIN_PDM].conf.timeFreq         = (uint8_t)AC_SYSIN_PDM_FORMAT_DOMAIN;
  sys_in_ios[AC_SYSIN_PDM].conf.bufferType       = (uint8_t)AC_SYSIN_PDM_FORMAT_TYPE;
  sys_in_ios[AC_SYSIN_PDM].conf.interleaved      = (uint8_t)AC_SYSIN_PDM_INTERLEAVED;
  sys_in_ios[AC_SYSIN_PDM].conf.pName            = AC_SYSIN_PDM_NAME;

  /* BLE LC3 input (decoder) */
  sys_in_ios[AC_SYSIN_BLE_LC3].conf.chunkType        = (uint8_t)AUDIO_CHUNK_TYPE_SYS_IN;
  sys_in_ios[AC_SYSIN_BLE_LC3].conf.nbChannels       = AC_SYSIN_BLE_LC3_CHANNELS_NB;
  sys_in_ios[AC_SYSIN_BLE_LC3].conf.fs               = AC_SYSIN_BLE_LC3_FS;
  sys_in_ios[AC_SYSIN_BLE_LC3].conf.nbElements       = AC_SYSIN_BLE_LC3_NB_ELEMENTS_PER_FRAME;
  sys_in_ios[AC_SYSIN_BLE_LC3].conf.nbFrames         = AC_SYSIN_BLE_LC3_NBFRAMES;
  sys_in_ios[AC_SYSIN_BLE_LC3].conf.timeFreq         = (uint8_t)AC_SYSIN_BLE_LC3_FORMAT_DOMAIN;
  sys_in_ios[AC_SYSIN_BLE_LC3].conf.bufferType       = (uint8_t)AC_SYSIN_BLE_LC3_FORMAT_TYPE;
  sys_in_ios[AC_SYSIN_BLE_LC3].conf.interleaved      = (uint8_t)AC_SYSIN_BLE_LC3_INTERLEAVED;
  sys_in_ios[AC_SYSIN_BLE_LC3].conf.pName            = AC_SYSIN_BLE_LC3_NAME;

  /* local audio recodering from the codec */
  sys_in_ios[AC_SYSIN_CODEC_ADC].conf.chunkType     = (uint8_t)AUDIO_CHUNK_TYPE_SYS_IN;
  sys_in_ios[AC_SYSIN_CODEC_ADC].conf.nbChannels    = AC_SYSIN_CODEC_ADC_CHANNELS_NB;
  sys_in_ios[AC_SYSIN_CODEC_ADC].conf.fs            = AC_SYSIN_CODEC_ADC_FS;
  sys_in_ios[AC_SYSIN_CODEC_ADC].conf.nbElements    = AC_SYSIN_CODEC_ADC_NB_ELEMENTS_PER_FRAME;
  sys_in_ios[AC_SYSIN_CODEC_ADC].conf.nbFrames      = AC_SYSIN_CODEC_ADC_NBFRAMES;
  sys_in_ios[AC_SYSIN_CODEC_ADC].conf.timeFreq      = (uint8_t)AC_SYSIN_CODEC_ADC_FORMAT_DOMAIN;
  sys_in_ios[AC_SYSIN_CODEC_ADC].conf.bufferType    = (uint8_t)AC_SYSIN_CODEC_ADC_FORMAT_TYPE;
  sys_in_ios[AC_SYSIN_CODEC_ADC].conf.interleaved   = (uint8_t)AC_SYSIN_CODEC_ADC_INTERLEAVED;
  sys_in_ios[AC_SYSIN_CODEC_ADC].conf.pName         = AC_SYSIN_CODEC_ADC_NAME;
  
  /* Wavefile Input */
  sys_in_ios[AC_SYSIN_WAVFILE].conf.chunkType    = (uint8_t)AUDIO_CHUNK_TYPE_SYS_IN;
  sys_in_ios[AC_SYSIN_WAVFILE].conf.nbChannels   = AC_SYSIN_WAVFILE_CHANNELS_NB;
  sys_in_ios[AC_SYSIN_WAVFILE].conf.fs           = AC_SYSIN_WAVFILE_FS;
  sys_in_ios[AC_SYSIN_WAVFILE].conf.nbElements   = AC_SYSIN_WAVFILE_NB_ELEMENTS_PER_FRAME;
  sys_in_ios[AC_SYSIN_WAVFILE].conf.nbFrames     = AC_SYSIN_WAVFILE_NBFRAMES;
  sys_in_ios[AC_SYSIN_WAVFILE].conf.timeFreq     = (uint8_t)ABUFF_FORMAT_TIME;
  sys_in_ios[AC_SYSIN_WAVFILE].conf.bufferType   = (uint8_t)ABUFF_FORMAT_FIXED16;
  sys_in_ios[AC_SYSIN_WAVFILE].conf.interleaved  = (uint8_t)AC_SYSIN_WAVFILE_INTERL;
  sys_in_ios[AC_SYSIN_WAVFILE].conf.pName        = AC_SYSIN_WAVFILE_NAME;

  /* Local audio playback to audio codec */
  sys_out_ios[AC_SYSOUT_CODEC_DAC].conf.chunkType      = (uint8_t)AUDIO_CHUNK_TYPE_SYS_OUT;
  sys_out_ios[AC_SYSOUT_CODEC_DAC].conf.nbChannels     = AC_SYSOUT_CODEC_DAC_CHANNELS_NB;
  sys_out_ios[AC_SYSOUT_CODEC_DAC].conf.fs             = AC_SYSOUT_CODEC_DAC_FS;
  sys_out_ios[AC_SYSOUT_CODEC_DAC].conf.nbElements     = AC_SYSOUT_CODEC_DAC_NB_ELEMENTS_PER_FRAME;
  sys_out_ios[AC_SYSOUT_CODEC_DAC].conf.nbFrames       = AC_SYSOUT_CODEC_DAC_NBFRAMES;
  sys_out_ios[AC_SYSOUT_CODEC_DAC].conf.timeFreq       = (uint8_t)AC_SYSOUT_CODEC_DAC_FORMAT_DOMAIN;
  sys_out_ios[AC_SYSOUT_CODEC_DAC].conf.bufferType     = (uint8_t)AC_SYSOUT_CODEC_DAC_FORMAT_TYPE;
  sys_out_ios[AC_SYSOUT_CODEC_DAC].conf.interleaved    = (uint8_t)AC_SYSOUT_CODEC_DAC_INTERLEAVED;
  sys_out_ios[AC_SYSOUT_CODEC_DAC].conf.pName          = AC_SYSOUT_CODEC_DAC_NAME;

  /* BLE LC3 output (encoder) */
  sys_out_ios[AC_SYSOUT_BLE_LC3].conf.chunkType        = (uint8_t)AUDIO_CHUNK_TYPE_SYS_OUT;
  sys_out_ios[AC_SYSOUT_BLE_LC3].conf.nbChannels       = AC_SYSOUT_BLE_LC3_CHANNELS_NB;
  sys_out_ios[AC_SYSOUT_BLE_LC3].conf.fs               = AC_SYSOUT_BLE_LC3_FS;
  sys_out_ios[AC_SYSOUT_BLE_LC3].conf.nbElements       = AC_SYSOUT_BLE_LC3_NB_ELEMENTS_PER_FRAME;
  sys_out_ios[AC_SYSOUT_BLE_LC3].conf.nbFrames         = AC_SYSOUT_BLE_LC3_NBFRAMES;
  sys_out_ios[AC_SYSOUT_BLE_LC3].conf.timeFreq         = (uint8_t)AC_SYSOUT_BLE_LC3_FORMAT_DOMAIN;
  sys_out_ios[AC_SYSOUT_BLE_LC3].conf.bufferType       = (uint8_t)AC_SYSOUT_BLE_LC3_FORMAT_TYPE;
  sys_out_ios[AC_SYSOUT_BLE_LC3].conf.interleaved      = (uint8_t)AC_SYSOUT_BLE_LC3_INTERLEAVED;
  sys_out_ios[AC_SYSOUT_BLE_LC3].conf.pName            = AC_SYSOUT_BLE_LC3_NAME;

  return &sys_ios;
}
