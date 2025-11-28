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


/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define AC_SYSIOS_MS 1UL
#define AC_SYSIOS_FS 16000UL
#define AC_SYSIOS_CH 2U
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
  /* microphones pcm format */
  sys_in_ios[AC_SYSIN_MIC].conf.chunkType        = (uint8_t)AUDIO_CHUNK_TYPE_SYS_IN;
  sys_in_ios[AC_SYSIN_MIC].conf.nbChannels       = AC_SYSIOS_CH;
  sys_in_ios[AC_SYSIN_MIC].conf.fs               = AC_SYSIOS_FS;
  sys_in_ios[AC_SYSIN_MIC].conf.nbElements       = AC_SYSIOS_MS  * AC_SYSIOS_FS  / 1000UL;
  sys_in_ios[AC_SYSIN_MIC].conf.nbFrames         = 2U;
  sys_in_ios[AC_SYSIN_MIC].conf.timeFreq         = (uint8_t)ABUFF_FORMAT_TIME;
  sys_in_ios[AC_SYSIN_MIC].conf.bufferType       = (uint8_t)ABUFF_FORMAT_FIXED16;
  sys_in_ios[AC_SYSIN_MIC].conf.interleaved      = (uint8_t)ABUFF_FORMAT_INTERLEAVED;
  sys_in_ios[AC_SYSIN_MIC].conf.pName            = AC_SYSIN_MIC_NAME;


  /*
  * ... Add & configure all your other inputs here
  */

  /* Local audio playback to audio codec */
  sys_out_ios[AC_SYSOUT_SPK].conf.chunkType      = (uint8_t)AUDIO_CHUNK_TYPE_SYS_OUT;
  sys_out_ios[AC_SYSOUT_SPK].conf.nbChannels     = AC_SYSIOS_CH;
  sys_out_ios[AC_SYSOUT_SPK].conf.fs             = AC_SYSIOS_FS;
  sys_out_ios[AC_SYSOUT_SPK].conf.nbElements     = AC_SYSIOS_MS  * AC_SYSIOS_FS  / 1000UL;;
  sys_out_ios[AC_SYSOUT_SPK].conf.nbFrames       = 3U;
  sys_out_ios[AC_SYSOUT_SPK].conf.timeFreq       = (uint8_t)ABUFF_FORMAT_TIME;
  sys_out_ios[AC_SYSOUT_SPK].conf.bufferType     = (uint8_t)ABUFF_FORMAT_FIXED16;
  sys_out_ios[AC_SYSOUT_SPK].conf.interleaved    = (uint8_t)ABUFF_FORMAT_INTERLEAVED;
  sys_out_ios[AC_SYSOUT_SPK].conf.pName          = AC_SYSOUT_SPK_NAME;

  /*
  * ... Add & configure all your other outputs here
  */


  return &sys_ios;
}
