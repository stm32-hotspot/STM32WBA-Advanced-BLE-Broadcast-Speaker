/**
******************************************************************************
* @file    stm32_audio_pdm2pcm.c
* @author  MCD Application Team
* @brief   Manage sai setup for PDM
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32_audio_pdm2pcm.h"
#include "pdm2pcm.h"

#ifdef UTIL_AUDIO_IN_PDM2PCM_USED


/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#ifndef UTIL_AUDIO_CAPTURE_PDM2PCM_LIB_GAIN
  #define UTIL_AUDIO_CAPTURE_PDM2PCM_LIB_GAIN 25
#endif

#ifndef UTIL_AUDIO_CAPTURE_PDM2PCM_LIB_HP_TAP
  #define UTIL_AUDIO_CAPTURE_PDM2PCM_LIB_HP_TAP 2122358088UL
#endif

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  PDM2PCM_Handler_t   hPdm2Pcm[UTIL_AUDIO_MAX_IN_CH_NB];
} Context_t;


/* Private variables ---------------------------------------------------------*/
static Context_t G_Context;

/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/

/**
* @brief  Initialise PDM2PCM library.
* @retval status
*/
int32_t UTIL_AUDIO_PDM2PCM_Init(void)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  static bool PDMToPCM_Init_Done = false;

  if (!PDMToPCM_Init_Done)
  {
    uint8_t const nbMicPdm = (uint8_t)UTIL_AUDIO_IN_HW_CH_NB;

    PDMToPCM_Init_Done = true;

    /* Enable CRC peripheral to unlock the PDM library */
    __HAL_RCC_CRC_CLK_ENABLE();

    for (uint16_t ch = 0U; (error == 0) && (ch < nbMicPdm); ch++)
    {
      G_Context.hPdm2Pcm[ch].bit_order        = PDM2PCM_BIT_ORDER_LSB;
      G_Context.hPdm2Pcm[ch].endianness       = PDM2PCM_ENDIANNESS_LE;
      G_Context.hPdm2Pcm[ch].high_pass_tap    = UTIL_AUDIO_CAPTURE_PDM2PCM_LIB_HP_TAP;
      G_Context.hPdm2Pcm[ch].in_ptr_channels  = (uint8_t)UTIL_AUDIO_IN_HW_CH_NB;
      G_Context.hPdm2Pcm[ch].out_ptr_channels = (uint8_t)UTIL_AUDIO_IN_HW_CH_NB;
      error = (int32_t)PDM2PCM_init(&G_Context.hPdm2Pcm[ch]);

      if (error == UTIL_AUDIO_ERROR_NONE)
      {
        PDM2PCM_Config_t PDM2PCM_config;

        PDM2PCM_config.output_samples_number = (uint16_t)UTIL_AUDIO_IN_SAMPLES_NB;
        PDM2PCM_config.mic_gain = UTIL_AUDIO_CAPTURE_PDM2PCM_LIB_GAIN;
        PDM2PCM_config.decimation_factor = (uint16_t)UTIL_AUDIO_CAPTURE_getPdm2PcmLibRatio();
        error = (int32_t)PDM2PCM_setConfig(&G_Context.hPdm2Pcm[ch], &PDM2PCM_config);
      }
    }
  }
  return error;
}

/**
* @brief  Runs PDM2PCM library.
* @param  pBuf      Main buffer pointer for the recorded data storing
* @param  NbrOfBytes Size of the recorded buffer in bytes
* @retval status
*/
int32_t UTIL_AUDIO_PDM2PCM_Decimate(uint8_t *PDMBuf, uint16_t *PCMBuf)
{
  int32_t       error    = UTIL_AUDIO_PDM2PCM_Init();
  uint8_t const nbMicPdm = (uint8_t)UTIL_AUDIO_IN_HW_CH_NB;

  for (uint8_t ch = 0U; (error == UTIL_AUDIO_ERROR_NONE) && (ch < nbMicPdm); ch++)
  {
    error = (int32_t)PDM2PCM_process(&G_Context.hPdm2Pcm[ch], &PDMBuf[ch], &PCMBuf[ch]);
  }
  return error;
}

#endif // UTIL_AUDIO_IN_PDM2PCM_USED
