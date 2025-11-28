/**
******************************************************************************
* @file    stm32_audio_dfsdm.c
* @author  MCD Application Team
* @brief   Manage dfsdm setup
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
#include "stm32_audio_dfsdm.h"
#include "stm32_audio_dma.h"
/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#ifdef UTIL_AUDIO_DFSDM_USED
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
  #error "This driver is not ready for USE_HAL_DFSDM_REGISTER_CALLBACKS set to 1"
#endif

#ifndef UTIL_AUDIO_DFSDMx_RBS
  #error "UTIL_AUDIO_DFSDMx_RBS not defined"
  #define UTIL_AUDIO_DFSDMx_RBS 15U
#endif
/* Private macros ------------------------------------------------------------*/
#define SaturaLH(N, L, H) (((N)<(L))?(L):(((N)>(H))?(H):(N)))

/*### RECORD ###*/
#define DFSDM_OVER_SAMPLING(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))  ? (256U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K)) ? (256U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K)) ? (128U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K)) ? (128U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K)) ? (64U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K)) ? (64U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K)) ? (64U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K)) ? (32U) : (20U)

#define DFSDM_CLOCK_DIVIDER(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))  ? (24U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K)) ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K)) ? (24U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K)) ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K)) ? (24U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K)) ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K)) ? (16U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K)) ? (16U) : (25U)

#define DFSDM_FILTER_ORDER(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))  ? (DFSDM_FILTER_SINC3_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K)) ? (DFSDM_FILTER_SINC3_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K)) ? (DFSDM_FILTER_SINC4_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K)) ? (DFSDM_FILTER_SINC4_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K)) ? (DFSDM_FILTER_SINC4_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K)) ? (DFSDM_FILTER_SINC4_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K)) ? (DFSDM_FILTER_SINC5_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K)) ? (DFSDM_FILTER_SINC5_ORDER) : (DFSDM_FILTER_SINC5_ORDER)

/* Right Bit Shift is used to shift sample in the right dynamic as function of OVR & Filter order
*  It is used only  in case of 16-bit pcm output. These settings apply a gain and may saturate but
*  are a good compromise to handle far field application and avoid saturation is close field application.
*  Of course, very close field usage may require different setting */

#define DFSDM_MIC_RIGHT_BIT_SHIFT(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))  ? (UTIL_AUDIO_DFSDMx_RBS-6U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K)) ? (UTIL_AUDIO_DFSDMx_RBS-6U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K)) ? (UTIL_AUDIO_DFSDMx_RBS-2U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K)) ? (UTIL_AUDIO_DFSDMx_RBS-2U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K)) ? (UTIL_AUDIO_DFSDMx_RBS-6U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K)) ? (UTIL_AUDIO_DFSDMx_RBS-6U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K)) ? (UTIL_AUDIO_DFSDMx_RBS)    \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K)) ? (UTIL_AUDIO_DFSDMx_RBS - 5U) : (2U)

/* Left Bit Shift is used when 32-bit output is requested. the goal is to keep the
*  same gain settings that the right bit shift applies when 16-bit output is requested */

#define DFSDM_MIC_LEFT_BIT_SHIFT(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))  ? (9U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K)) ? (9U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K)) ? (5U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K)) ? (5U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K)) ? (9U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K)) ? (9U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K)) ? (3U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K)) ? (8U) : (0U)

/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  UTIL_AUDIO_DMA_t            hDma;
  //  DMA_HandleTypeDef           hDma;
  DFSDM_Filter_HandleTypeDef  hFilter;
  DFSDM_Channel_HandleTypeDef hChannel;
} MicContext_t;

#ifdef UTIL_AUDIO_IN_HPF_USED
typedef struct
{
  int32_t Z;
  int32_t oldOut;
  int32_t oldIn;
} HP_FilterState_TypeDef;
#endif

typedef struct
{
  UTIL_AUDIO_params_t     conf;
  MicContext_t            dfsdmMic[UTIL_AUDIO_DFSDM_MIC_STARTED_NB] ; /* dfsdm handlers per microphone */

  #ifdef UTIL_AUDIO_IN_HPF_USED
  HP_FilterState_TypeDef  dcrem[UTIL_AUDIO_MAX_IN_CH_NB];
  #endif
  uint8_t                 lbs; /* Left bit shift for 32-bit sample gain */
} Context_t;

/* Private variables ---------------------------------------------------------*/
Context_t              G_Context;

/* Private function prototypes -----------------------------------------------*/
void UTIL_AUDIO_DFSDM_DMAx_MIC1_IRQHandler(void);
void UTIL_AUDIO_DFSDM_DMAx_MIC2_IRQHandler(void);
#if (UTIL_AUDIO_DFSDM_MIC_STARTED_NB > 2)
  void UTIL_AUDIO_DFSDM_DMAx_MIC3_IRQHandler(void);
  void UTIL_AUDIO_DFSDM_DMAx_MIC4_IRQHandler(void);
#endif

static void    s_initGpios(void);
static int32_t s_initDma(void);

static int32_t s_initOneMic(DFSDM_Filter_HandleTypeDef *hDfsdmFilter, DFSDM_Channel_HandleTypeDef *hDfsdmChannel);

/* Functions Definition ------------------------------------------------------*/
/**
* @brief  Initialize wave recording.
* @param  AudioInit Init structure
* @retval Status
*/
int32_t UTIL_AUDIO_DFSDM_Init(UTIL_AUDIO_params_t *AudioInit)
{
  uint32_t i;
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  /* Save in context user config */
  memcpy(&G_Context.conf, AudioInit, sizeof(UTIL_AUDIO_params_t));


  /* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
  if (UTIL_AUDIO_DFSDM_ClockConfig(AudioInit->SampleRate) != UTIL_AUDIO_ERROR_NONE)
  {
    error = BSP_ERROR_CLOCK_FAILURE;
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    error = s_initDma();
  }
  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    s_initGpios();
    if (error == UTIL_AUDIO_ERROR_NONE)
    {
      for (i = 0; i < UTIL_AUDIO_DFSDM_MIC_STARTED_NB; i ++)
      {
        if (((AudioInit->Device >> i) & UTIL_AUDIO_DFSDMx_MIC_FIRST) == UTIL_AUDIO_DFSDMx_MIC_FIRST)
        {
          /* Default configuration of DFSDM filters and channels */
          if (s_initOneMic(&G_Context.dfsdmMic[i].hFilter, &G_Context.dfsdmMic[i].hChannel) != UTIL_AUDIO_ERROR_NONE)
          {
            /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
            error = BSP_ERROR_PERIPH_FAILURE;
          }
        }
        if (error != UTIL_AUDIO_ERROR_NONE)
        {
          break;
        }
      }
    }
  }
  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    G_Context.lbs = DFSDM_MIC_LEFT_BIT_SHIFT(UTIL_AUDIO_IN_FREQUENCY);
  }
  return error;
}

/**
* @brief  Weak function for clock config (either calling existing function from BSP or write your own). It is called at init.
* @param  Sampling Frequency
* @retval Status
*/
__weak int32_t UTIL_AUDIO_DFSDM_ClockConfig(uint32_t SampleRate)
{
  return UTIL_AUDIO_ERROR_NONE;
}

/**
  * @brief  Starts audio recording with DFSDM.
  * @param  pBuf      Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes Size of the recorded buffer in bytes
  * @retval status
  */
int32_t UTIL_AUDIO_DFSDM_Record(uint8_t **pBuf, uint32_t NbrOfBytes)
{
  uint16_t i;
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  uint32_t mic_init[UTIL_AUDIO_DFSDM_MIC_STARTED_NB] = {0};
  uint32_t micIdentifier = UTIL_AUDIO_DFSDMx_MIC_FIRST, pbuf_index = 0;
  uint32_t enabled_mic = 0;
  uint32_t micIdx = 0;
  /* Get the number of activated microphones */
  for (i = 0U; i < UTIL_AUDIO_DFSDM_MIC_STARTED_NB; i++)
  {
    if ((G_Context.conf.Device & micIdentifier) == micIdentifier)
    {
      enabled_mic++;
    }
    micIdentifier = micIdentifier << 1;
  }


  micIdentifier = UTIL_AUDIO_DFSDMx_MIC_LAST;
  UTIL_AUDIO_DISABLE_IRQ();
  for (i = 0U; i < UTIL_AUDIO_DFSDM_MIC_STARTED_NB; i++)
  {
    micIdx = POS_VAL(micIdentifier);
    if ((mic_init[micIdx] != 1U) && ((G_Context.conf.Device & micIdentifier) == micIdentifier))
    {
      /* Call the Media layer start function for MICx channel */
      if (HAL_DFSDM_FilterRegularStart_DMA(&G_Context.dfsdmMic[micIdx].hFilter, (int32_t *)pBuf[enabled_mic - 1U - pbuf_index], NbrOfBytes) != HAL_OK)
      {
        error = 1;
      }
      else
      {
        mic_init[micIdx] = 1;
        pbuf_index++;
      }
    }
    micIdentifier = micIdentifier >> 1;
  }
  UTIL_AUDIO_ENABLE_IRQ();

  return error;
}

/**
  * @brief  Stops audio recording with DFSDM.
  * @param  pBuf      Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes Size of the recorded buffer in bytes
  * @retval status
  */
int32_t UTIL_AUDIO_DFSDM_Stop(void)
{
  uint16_t i;
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  uint32_t mic_init[UTIL_AUDIO_DFSDM_MIC_STARTED_NB] = {0};
  uint32_t micIdentifier = UTIL_AUDIO_DFSDMx_MIC_FIRST, pbuf_index = 0;
  uint32_t enabled_mic = 0;
  uint32_t micIdx = 0;
  /* Get the number of activated microphones */
  for (i = 0U; i < UTIL_AUDIO_DFSDM_MIC_STARTED_NB; i++)
  {
    if ((G_Context.conf.Device & micIdentifier) == micIdentifier)
    {
      enabled_mic++;
    }
    micIdentifier = micIdentifier << 1;
  }


  micIdentifier = UTIL_AUDIO_DFSDMx_MIC_LAST;
  UTIL_AUDIO_DISABLE_IRQ();
  for (i = 0U; i < UTIL_AUDIO_DFSDM_MIC_STARTED_NB; i++)
  {
    micIdx = POS_VAL(micIdentifier);
    if ((mic_init[micIdx] != 1U) && ((G_Context.conf.Device & micIdentifier) == micIdentifier))
    {
      /* Call the Media layer start function for MICx channel */
      if (HAL_DFSDM_FilterRegularStop_DMA(&G_Context.dfsdmMic[micIdx].hFilter) != HAL_OK)
      {
        error = 1;
      }
      else
      {
        mic_init[micIdx] = 1;
        pbuf_index++;
      }
    }
    micIdentifier = micIdentifier >> 1;
  }
  UTIL_AUDIO_ENABLE_IRQ();

  return error;
}


void UTIL_AUDIO_DFSDM_32bitInt_process(audio_buffer_t *const pBuffInfo, int32_t **pChannelBuffer, uint32_t nbSamples, uint32_t const offsetSpleBytes)
{
  for (uint8_t idxChannel = 0U; idxChannel < UTIL_AUDIO_IN_CH_NB; idxChannel++)
  {
    uint8_t *pChBaseAddr  = (uint8_t *) pChannelBuffer[idxChannel];
    uint8_t *pInSample_u8 = pChBaseAddr + offsetSpleBytes;
    int32_t *pInSample    = (int32_t *)pInSample_u8;
    int32_t *pOutSample   = (int32_t *)AudioBuffer_getSampleAddress(pBuffInfo, idxChannel, 0U);

    #if (UTIL_AUDIO_CPU_CACHE_MAINTENANCE == 1)
    SCB_InvalidateDCache_by_Addr((void *)pInSample, nbSamples * 4UL); /* 4 because 32-bit samples*/
    #endif /* UTIL_AUDIO_CPU_CACHE_MAINTENANCE */

    for (uint32_t idxPcmBuff = 0UL; idxPcmBuff < nbSamples; idxPcmBuff++) /* half of the ping pong buffer*/
    {
      int32_t sple32                       = pInSample[idxPcmBuff] / 256;           /* 24 bits input sample stored in the MSB of a 32 bits word, div 256 removes status register info */
      #ifdef UTIL_AUDIO_IN_HPF_USED
      G_Context.dcrem[idxChannel].Z        = sple32;
      /* Formula is yn = Alpha * (yn-1 + xn - xn-1) ;
      it comes from yn = xn - dcn where dcn = alpha* dcn-1 +(1-alpha) * xn ; going to Z transform
      we get H(z) = alpha * (1-z^(-1))/(1-alpha*z^(-1) )

      H(z) =   Y(z) / X(z)
      (1-alpha* z-1 ) Y(z) =  alpha * (1 - z-1 ) X(z)
      Y(z) = alpha * Y(z) * z-1 + alpha * (X(z) - X(z) * z-1))
      ==> yn = Alpha * (yn-1 + xn - xn-1)
      */
      G_Context.dcrem[idxChannel].oldOut = (0xFC * (G_Context.dcrem[idxChannel].oldOut + G_Context.dcrem[idxChannel].Z - G_Context.dcrem[idxChannel].oldIn)) / 256;
      G_Context.dcrem[idxChannel].oldIn  = G_Context.dcrem[idxChannel].Z;
      sple32                             = G_Context.dcrem[idxChannel].oldOut;
      #endif
      pOutSample[UTIL_AUDIO_IN_CH_NB * idxPcmBuff] = sple32 << G_Context.lbs ;    /* 32 bits output samples*/ /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-10.1_R7 shift on signed data is necessary for performance reason*/
    }
  }

  #if (UTIL_AUDIO_CPU_CACHE_MAINTENANCE == 1)
  SCB_CleanDCache_by_Addr((uint32_t *)AudioBuffer_getPdata(pBuffInfo), AudioBuffer_getBufferSize(pBuffInfo));
  #endif /* UTIL_AUDIO_CPU_CACHE_MAINTENANCE */

}

void UTIL_AUDIO_DFSDM_16bitInt_process(audio_buffer_t *const pBuffInfo, int32_t **pChannelBuffer, uint32_t nbSamples, uint32_t const offsetSpleBytes)
{
  for (uint8_t idxChannel = 0U; idxChannel < UTIL_AUDIO_IN_CH_NB; idxChannel++)
  {
    uint8_t *pChBaseAddr  = (uint8_t *) pChannelBuffer[idxChannel];
    uint8_t *pInSample_u8 = pChBaseAddr + offsetSpleBytes;
    int32_t *pInSample    = (int32_t *)pInSample_u8;
    int16_t *pOutSample   = (int16_t *)AudioBuffer_getSampleAddress(pBuffInfo, idxChannel, 0U);

    #if (UTIL_AUDIO_CPU_CACHE_MAINTENANCE == 1)
    SCB_InvalidateDCache_by_Addr((void *)pInSample, nbSamples * 4UL); /* 4 because 32-bit samples*/
    #endif /* UTIL_AUDIO_CPU_CACHE_MAINTENANCE */

    for (uint32_t idxPcmBuff = 0UL; idxPcmBuff < nbSamples; idxPcmBuff++) /* half of the ping pong buffer*/
    {
      int32_t sple32                     = pInSample[idxPcmBuff] / 256;           /* 24 bits input sample stored in the MSB of a 32 bits word, div 256 removes status register info */
      #ifdef UTIL_AUDIO_IN_HPF_USED
      G_Context.dcrem[idxChannel].Z      = sple32;
      /* Formula is yn = Alpha * (yn-1 + xn - xn-1) ;
      it comes from yn = xn - dcn where dcn = alpha* dcn-1 +(1-alpha) * xn ; going to Z transform
      we get H(z) = alpha * (1-z^(-1))/(1-alpha*z^(-1) )

      H(z) =   Y(z) / X(z)
      (1-alpha* z-1 ) Y(z) =  alpha * (1 - z-1 ) X(z)
      Y(z) = alpha * Y(z) * z-1 + alpha * (X(z) - X(z) * z-1))
      ==> yn = Alpha * (yn-1 + xn - xn-1)
      */
      G_Context.dcrem[idxChannel].oldOut = (0xFC * (G_Context.dcrem[idxChannel].oldOut + G_Context.dcrem[idxChannel].Z - G_Context.dcrem[idxChannel].oldIn)) / 256;
      G_Context.dcrem[idxChannel].oldIn  = G_Context.dcrem[idxChannel].Z;
      sple32                             = G_Context.dcrem[idxChannel].oldOut;
      #endif
      pOutSample[UTIL_AUDIO_IN_CH_NB * idxPcmBuff] = (int16_t)SaturaLH(sple32, -32768, 32767);    /* 16 bits output samples*/
    }
  }

  #if (UTIL_AUDIO_CPU_CACHE_MAINTENANCE == 1)
  SCB_CleanDCache_by_Addr((uint32_t *)AudioBuffer_getPdata(pBuffInfo), AudioBuffer_getBufferSize(pBuffInfo));
  #endif /* UTIL_AUDIO_CPU_CACHE_MAINTENANCE */

}

static void s_initGpios(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  static uint8_t gpiosInitDone = 0U;
  if (gpiosInitDone == 0U)
  {
    /* Deinit DFSDM pins configured by BSP ---------------------------------*/
    GPIO_InitStruct.Pin = UTIL_AUDIO_DFSDMx_CKOUT_PIN;
    HAL_GPIO_DeInit(UTIL_AUDIO_DFSDMx_CKOUT_GPIO_PORT, GPIO_InitStruct.Pin);

    GPIO_InitStruct.Pin = UTIL_AUDIO_DFSDMx_DATIN_MIC1_PIN;
    HAL_GPIO_DeInit(UTIL_AUDIO_DFSDMx_DATIN_MIC1_GPIO_PORT, GPIO_InitStruct.Pin);
    GPIO_InitStruct.Pin = UTIL_AUDIO_DFSDMx_DATIN_MIC2_PIN;
    HAL_GPIO_DeInit(UTIL_AUDIO_DFSDMx_DATIN_MIC2_GPIO_PORT, GPIO_InitStruct.Pin);
    GPIO_InitStruct.Pin = UTIL_AUDIO_DFSDMx_DATIN_MIC3_PIN;
    HAL_GPIO_DeInit(UTIL_AUDIO_DFSDMx_DATIN_MIC3_GPIO_PORT, GPIO_InitStruct.Pin);
    GPIO_InitStruct.Pin = UTIL_AUDIO_DFSDMx_DATIN_MIC4_PIN;
    HAL_GPIO_DeInit(UTIL_AUDIO_DFSDMx_DATIN_MIC4_GPIO_PORT, GPIO_InitStruct.Pin);


    /* DFSDM pins configuration: DFSDM_CKOUT, DMIC_DATIN pins ----------------*/
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    /* Enable DFSDM clock */
    UTIL_AUDIO_DFSDMx_CLK_ENABLE();

    /* Enable GPIO clock */
    UTIL_AUDIO_DFSDMx_CKOUT_GPIO_CLK_ENABLE();
    UTIL_AUDIO_DFSDMx_DATIN_MIC1_GPIO_CLK_ENABLE();
    UTIL_AUDIO_DFSDMx_DATIN_MIC2_GPIO_CLK_ENABLE();
    UTIL_AUDIO_DFSDMx_DATIN_MIC3_GPIO_CLK_ENABLE();
    UTIL_AUDIO_DFSDMx_DATIN_MIC4_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Pin       = UTIL_AUDIO_DFSDMx_CKOUT_PIN;
    GPIO_InitStruct.Alternate = UTIL_AUDIO_DFSDMx_CKOUT_AF;
    HAL_GPIO_Init(UTIL_AUDIO_DFSDMx_CKOUT_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = UTIL_AUDIO_DFSDMx_DATIN_MIC1_PIN;
    GPIO_InitStruct.Alternate = UTIL_AUDIO_DFSDMx_DATIN_MIC1_AF;
    HAL_GPIO_Init(UTIL_AUDIO_DFSDMx_DATIN_MIC1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = UTIL_AUDIO_DFSDMx_DATIN_MIC2_PIN;
    GPIO_InitStruct.Alternate = UTIL_AUDIO_DFSDMx_DATIN_MIC2_AF;
    HAL_GPIO_Init(UTIL_AUDIO_DFSDMx_DATIN_MIC2_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = UTIL_AUDIO_DFSDMx_DATIN_MIC3_PIN;
    GPIO_InitStruct.Alternate = UTIL_AUDIO_DFSDMx_DATIN_MIC3_AF;
    HAL_GPIO_Init(UTIL_AUDIO_DFSDMx_DATIN_MIC3_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = UTIL_AUDIO_DFSDMx_DATIN_MIC4_PIN;
    GPIO_InitStruct.Alternate = UTIL_AUDIO_DFSDMx_DATIN_MIC4_AF;
    HAL_GPIO_Init(UTIL_AUDIO_DFSDMx_DATIN_MIC4_GPIO_PORT, &GPIO_InitStruct);
    gpiosInitDone++;
  }
  else
  {
    /* do nothing */
  }
}


/**
  * @brief  Initializes the DFSDM
  * @param  hDfsdmFilter  DFSDM Filter Handle
  * @param  hDfsdmChannel DFSDM Channel Handle
  * @retval status
  */
int32_t s_initOneMic(DFSDM_Filter_HandleTypeDef *hDfsdmFilter, DFSDM_Channel_HandleTypeDef *hDfsdmChannel)
{
  DFSDM_Filter_TypeDef  *dfsdmFilterInstance[UTIL_AUDIO_DFSDM_MIC_STARTED_NB]  = {UTIL_AUDIO_DFSDMx_MIC1_FILTER, UTIL_AUDIO_DFSDMx_MIC2_FILTER, UTIL_AUDIO_DFSDMx_MIC3_FILTER, UTIL_AUDIO_DFSDMx_MIC4_FILTER};
  DFSDM_Channel_TypeDef *dfsdmChannelInstance[UTIL_AUDIO_DFSDM_MIC_STARTED_NB] = {UTIL_AUDIO_DFSDMx_MIC1_CHANNEL, UTIL_AUDIO_DFSDMx_MIC2_CHANNEL, UTIL_AUDIO_DFSDMx_MIC3_CHANNEL, UTIL_AUDIO_DFSDMx_MIC4_CHANNEL};
  uint32_t               dfsdmDigitalMicPins[UTIL_AUDIO_DFSDM_MIC_STARTED_NB]  = {DFSDM_CHANNEL_SAME_CHANNEL_PINS, DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS, DFSDM_CHANNEL_SAME_CHANNEL_PINS, DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS};
  uint32_t               dfsdmDigitalMicType[UTIL_AUDIO_DFSDM_MIC_STARTED_NB]  = {DFSDM_CHANNEL_SPI_RISING, DFSDM_CHANNEL_SPI_FALLING, DFSDM_CHANNEL_SPI_RISING, DFSDM_CHANNEL_SPI_FALLING};
  uint32_t               dfsdmChannel4Filter[UTIL_AUDIO_DFSDM_MIC_STARTED_NB]  = {UTIL_AUDIO_DFSDMx_MIC1_CHANNEL_FOR_FILTER, UTIL_AUDIO_DFSDMx_MIC2_CHANNEL_FOR_FILTER, UTIL_AUDIO_DFSDMx_MIC3_CHANNEL_FOR_FILTER, UTIL_AUDIO_DFSDMx_MIC4_CHANNEL_FOR_FILTER};

  static uint16_t micCnt = 0U;
  /* MIC filters  initialization */
  s_initGpios();

  /* Retrieve the mic index by comparing pointer of handlers */
  for (uint16_t i = 0U; i < UTIL_AUDIO_DFSDM_MIC_STARTED_NB; i++)
  {
    if (hDfsdmFilter == &G_Context.dfsdmMic[i].hFilter)
    {
      micCnt = i;
    }
  }

  /* Test if mic index is one that should be used */
  if (((UTIL_AUDIO_IN_DEVICE_SELECT >> micCnt) & UTIL_AUDIO_DFSDMx_MIC_FIRST) == UTIL_AUDIO_DFSDMx_MIC_FIRST)
  {
    hDfsdmFilter->Instance                          = dfsdmFilterInstance[micCnt];

    hDfsdmFilter->Init.RegularParam.Trigger         = DFSDM_FILTER_SW_TRIGGER;
    /* MIC2, MIC3 and MIC4 should be synchronized to MIC1 if it's used */
    if (((UTIL_AUDIO_IN_DEVICE_SELECT & UTIL_AUDIO_DFSDMx_MIC_FIRST) == UTIL_AUDIO_DFSDMx_MIC_FIRST) && (micCnt >= 1U) && (micCnt <= 4U))
    {
      hDfsdmFilter->Init.RegularParam.Trigger       = DFSDM_FILTER_SYNC_TRIGGER;
    }
    hDfsdmFilter->Init.RegularParam.FastMode        = ENABLE;
    hDfsdmFilter->Init.RegularParam.DmaMode         = ENABLE;
    hDfsdmFilter->Init.InjectedParam.Trigger        = DFSDM_FILTER_SW_TRIGGER;
    hDfsdmFilter->Init.InjectedParam.ScanMode       = DISABLE;
    hDfsdmFilter->Init.InjectedParam.DmaMode        = DISABLE;
    hDfsdmFilter->Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM8_TRGO;
    hDfsdmFilter->Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_BOTH_EDGES;
    hDfsdmFilter->Init.FilterParam.SincOrder        = DFSDM_FILTER_ORDER(UTIL_AUDIO_IN_FREQUENCY);
    hDfsdmFilter->Init.FilterParam.Oversampling     = DFSDM_OVER_SAMPLING(UTIL_AUDIO_IN_FREQUENCY);
    hDfsdmFilter->Init.FilterParam.IntOversampling  = 1;

    if (HAL_DFSDM_FilterInit(hDfsdmFilter) != HAL_OK)
    {
      return UTIL_AUDIO_ERROR;
    }

    /* MIC channels initialization */
    hDfsdmChannel->Instance                      = dfsdmChannelInstance[micCnt];
    hDfsdmChannel->Init.OutputClock.Activation   = ENABLE;
    hDfsdmChannel->Init.OutputClock.Selection    = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
    hDfsdmChannel->Init.OutputClock.Divider      = DFSDM_CLOCK_DIVIDER(UTIL_AUDIO_IN_FREQUENCY);
    hDfsdmChannel->Init.Input.Multiplexer        = DFSDM_CHANNEL_EXTERNAL_INPUTS;
    hDfsdmChannel->Init.Input.DataPacking        = DFSDM_CHANNEL_STANDARD_MODE;
    hDfsdmChannel->Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
    hDfsdmChannel->Init.Awd.FilterOrder          = DFSDM_CHANNEL_SINC1_ORDER;
    hDfsdmChannel->Init.Awd.Oversampling         = 10;
    hDfsdmChannel->Init.Offset                   = 0;
    hDfsdmChannel->Init.RightBitShift            = (UTIL_AUDIO_IN_BIT_RESOLUTION == UTIL_AUDIO_RESOLUTION_16B) ? DFSDM_MIC_RIGHT_BIT_SHIFT(UTIL_AUDIO_IN_FREQUENCY) : 0UL;
    hDfsdmChannel->Init.Input.Pins               = dfsdmDigitalMicPins[micCnt];
    hDfsdmChannel->Init.SerialInterface.Type     = dfsdmDigitalMicType[micCnt];

    if (HAL_OK != HAL_DFSDM_ChannelInit(hDfsdmChannel))
    {
      return UTIL_AUDIO_ERROR;
    }

    /* Configure injected channel */
    if (HAL_DFSDM_FilterConfigRegChannel(hDfsdmFilter, dfsdmChannel4Filter[micCnt], DFSDM_CONTINUOUS_CONV_ON) != HAL_OK)
    {
      return UTIL_AUDIO_ERROR;
    }
  }
  else
  {
    /* do nothing */
  }

  return UTIL_AUDIO_ERROR_NONE;
}

/**
  * @brief  Initialize the DFSDM filter MSP.
  * @param  hDfsdmFilter DFSDM Filter handle
  * @retval None
  */
static int32_t s_initDma(void)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  uint32_t i, micIdx, mic_init[UTIL_AUDIO_DFSDM_MIC_STARTED_NB] = {0};
  IRQn_Type           dmaMic_IRQHandler[UTIL_AUDIO_DFSDM_MIC_STARTED_NB] = {UTIL_AUDIO_DFSDMx_DMAx_MIC1_IRQ, UTIL_AUDIO_DFSDMx_DMAx_MIC2_IRQ, UTIL_AUDIO_DFSDMx_DMAx_MIC3_IRQ, UTIL_AUDIO_DFSDMx_DMAx_MIC4_IRQ};
  DMA_Stream_TypeDef *dmaMic_stream[UTIL_AUDIO_DFSDM_MIC_STARTED_NB]     = {UTIL_AUDIO_DFSDMx_DMAx_MIC1_STREAM, UTIL_AUDIO_DFSDMx_DMAx_MIC2_STREAM, UTIL_AUDIO_DFSDMx_DMAx_MIC3_STREAM, UTIL_AUDIO_DFSDMx_DMAx_MIC4_STREAM};
  uint32_t            dmaMic_Request[UTIL_AUDIO_DFSDM_MIC_STARTED_NB]    = {UTIL_AUDIO_DFSDMx_DMAx_MIC1_REQUEST, UTIL_AUDIO_DFSDMx_DMAx_MIC2_REQUEST, UTIL_AUDIO_DFSDMx_DMAx_MIC3_REQUEST, UTIL_AUDIO_DFSDMx_DMAx_MIC4_REQUEST};

  /* Prevent unused argument(s) compilation warning */
  /* UNUSED(hDfsdmFilter); */

  /* Enable DFSDM clock */
  UTIL_AUDIO_DFSDMx_CLK_ENABLE();

  /* Enable the DMA clock */
  UTIL_AUDIO_DFSDMx_DMAx_CLK_ENABLE();

  for (i = 0; (i < UTIL_AUDIO_DFSDM_MIC_STARTED_NB) && (error == UTIL_AUDIO_ERROR_NONE); i++)
  {
    if ((mic_init[POS_VAL(UTIL_AUDIO_DFSDMx_MIC1)] != 1U) && ((G_Context.conf.Device & UTIL_AUDIO_DFSDMx_MIC1) == UTIL_AUDIO_DFSDMx_MIC1))
    {
      micIdx = 0U;
      mic_init[micIdx] = 1;
    }
    else if ((mic_init[POS_VAL(UTIL_AUDIO_DFSDMx_MIC2)] != 1U) && ((G_Context.conf.Device & UTIL_AUDIO_DFSDMx_MIC2) == UTIL_AUDIO_DFSDMx_MIC2))
    {
      micIdx = 1U;
      mic_init[micIdx] = 1;
    }
    else if ((mic_init[POS_VAL(UTIL_AUDIO_DFSDMx_MIC3)] != 1U) && ((G_Context.conf.Device & UTIL_AUDIO_DFSDMx_MIC3) == UTIL_AUDIO_DFSDMx_MIC3))
    {
      micIdx = 2U;
      mic_init[micIdx] = 1;
    }
    else if ((mic_init[POS_VAL(UTIL_AUDIO_DFSDMx_MIC4)] != 1U) && ((G_Context.conf.Device & UTIL_AUDIO_DFSDMx_MIC4) == UTIL_AUDIO_DFSDMx_MIC4))
    {
      micIdx = 3U;
      mic_init[micIdx] = 1;
    }
    else
    {
      break;
    }
    //    /* Configure the Context.dfsdmMic.hDma[i] handle parameters */
    //    G_Context.dfsdmMic[micIdx].hDma.Init.Request             = dmaMic_Request[micIdx];
    //    G_Context.dfsdmMic[micIdx].hDma.Instance                 = dmaMic_stream[micIdx];
    //    G_Context.dfsdmMic[micIdx].hDma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    //    G_Context.dfsdmMic[micIdx].hDma.Init.PeriphInc           = DMA_PINC_DISABLE;
    //    G_Context.dfsdmMic[micIdx].hDma.Init.MemInc              = DMA_MINC_ENABLE;
    //    G_Context.dfsdmMic[micIdx].hDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    //    G_Context.dfsdmMic[micIdx].hDma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    //    G_Context.dfsdmMic[micIdx].hDma.Init.Mode                = DMA_CIRCULAR;
    //    G_Context.dfsdmMic[micIdx].hDma.Init.Priority            = DMA_PRIORITY_HIGH;
    //    G_Context.dfsdmMic[micIdx].hDma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    //    G_Context.dfsdmMic[micIdx].hDma.Init.MemBurst            = DMA_MBURST_SINGLE;
    //    G_Context.dfsdmMic[micIdx].hDma.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    //    G_Context.dfsdmMic[micIdx].hDma.State                    = HAL_DMA_STATE_RESET;
    //
    //    /* Associate the DMA handle */
    //    __HAL_LINKDMA(&G_Context.dfsdmMic[micIdx].hFilter, hdmaReg, G_Context.dfsdmMic[micIdx].hDma);
    //
    //    /* Reset DMA handle state */
    //    __HAL_DMA_RESET_HANDLE_STATE(&G_Context.dfsdmMic[micIdx].hDma);
    //
    //    /* Configure the DMA Channel */
    //    (void)HAL_DMA_Init(&G_Context.dfsdmMic[micIdx].hDma);
    //
    //    /* DMA IRQ Channel configuration */
    //    HAL_NVIC_SetPriority(dmaMic_IRQHandler[micIdx], UTIL_AUDIO_DFSDMx_IT_PRIO, 0);
    //    HAL_NVIC_EnableIRQ(dmaMic_IRQHandler[micIdx]);


    G_Context.dfsdmMic[micIdx].hDma.pIpHdle         = &G_Context.dfsdmMic[micIdx].hFilter;
    G_Context.dfsdmMic[micIdx].hDma.type            = UTIL_AUDIO_DMA_IP_TYPE_DFSDM;
    G_Context.dfsdmMic[micIdx].hDma.request         = dmaMic_Request[micIdx];
    G_Context.dfsdmMic[micIdx].hDma.preemptPriority = UTIL_AUDIO_DFSDMx_IT_PRIO;
    G_Context.dfsdmMic[micIdx].hDma.pInstance       = dmaMic_stream[micIdx];
    G_Context.dfsdmMic[micIdx].hDma.irqn            = dmaMic_IRQHandler[micIdx];
    G_Context.dfsdmMic[micIdx].hDma.srcDataWidth    = DMA_PDATAALIGN_WORD;
    G_Context.dfsdmMic[micIdx].hDma.destDataWidth   = DMA_MDATAALIGN_WORD;
    G_Context.dfsdmMic[micIdx].hDma.direction       = DMA_PERIPH_TO_MEMORY;

    error = UTIL_AUDIO_DMA_Init(&G_Context.dfsdmMic[micIdx].hDma);
  }
  return error;
}


/**
  * @brief  Regular conversion complete callback.
  * @note   In interrupt mode, user has to read conversion value in this function
            using HAL_DFSDM_FilterGetRegularValue.
  * @param  hdfsdm_filter   DFSDM filter handle.
  * @retval None
  */
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  UTIL_AUDIO_CAPTURE_TxComplete_cb(UTIL_AUDIO_CAPTURE_getHalfBuffOffsetBytes(), 2);
}

/**
  * @brief  Half regular conversion complete callback.
  * @param  hdfsdm_filter   DFSDM filter handle.
  * @retval None
  */

void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  UTIL_AUDIO_CAPTURE_TxComplete_cb(0UL, 2);
}

/**
* @brief This function handles DMA MIC1 interrupt request.
* @param None
* @retval None
*/
void UTIL_AUDIO_DFSDM_DMAx_MIC1_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(G_Context.dfsdmMic[POS_VAL(UTIL_AUDIO_DFSDMx_MIC1)].hFilter.hdmaReg);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}

/**
* @brief This function handles DMA MIC2 interrupt request.
* @param None
* @retval None
*/
void UTIL_AUDIO_DFSDM_DMAx_MIC2_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(G_Context.dfsdmMic[POS_VAL(UTIL_AUDIO_DFSDMx_MIC2)].hFilter.hdmaReg);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}

#if (UTIL_AUDIO_DFSDM_MIC_STARTED_NB > 2)
/**
* @brief This function handles DMA MIC3 interrupt request.
* @param None
* @retval None
*/
void UTIL_AUDIO_DFSDM_DMAx_MIC3_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(G_Context.dfsdmMic[POS_VAL(UTIL_AUDIO_DFSDMx_MIC3)].hFilter.hdmaReg);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}

/**
* @brief This function handles DMA MIC4 interrupt request.
* @param None
* @retval None
*/
void UTIL_AUDIO_DFSDM_DMAx_MIC4_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(G_Context.dfsdmMic[POS_VAL(UTIL_AUDIO_DFSDMx_MIC4)].hFilter.hdmaReg);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}
#endif

#endif // USE_DFSDM
