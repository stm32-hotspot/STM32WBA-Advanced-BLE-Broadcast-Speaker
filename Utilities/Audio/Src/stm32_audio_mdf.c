/**
******************************************************************************
* @file    stm32_audio_mdf.c
* @author  MCD Application Team
* @brief   Manage MDF setup for PCM capture
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
#include "stm32_audio_mdf.h"
#include "stm32_audio_dma.h"

#ifdef UTIL_AUDIO_MDF_USED

#if (USE_HAL_MDF_REGISTER_CALLBACKS == 1)
  #error "This driver is not ready for USE_HAL_MDF_REGISTER_CALLBACKS set to 1"
#endif
#if (USE_HAL_ADF_REGISTER_CALLBACKS == 1)
  #error "This driver is not ready for USE_HAL_ADF_REGISTER_CALLBACKS set to 1"
#endif

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#define SaturaLH(N, L, H) (((N)<(L))?(L):(((N)>(H))?(H):(N)))



#define MDF_DECIMATION_RATIO(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))   ? (64U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K))  ? (64U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K))  ? (32U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K))  ? (32U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K))  ? (16U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K))  ? (16U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K))  ? (16U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K))  ? (16U) : (4U)

/* Careful: Filter gain is in step of around 3db (from -48db to 72dB).
* This parameter must be a number between Min_Data = -16 and Max_Data = 24 */

/* This settings apply a gain of 18 dB (3-bit extra) to match other board of livetune project.
The samples delivered are 24-bit and must be shifted by SW */
#define MDF_GAIN(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))   ? (0) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K))  ? (0) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K))  ? (-2)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K))  ? (-2)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K))  ? (8) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K))  ? (8) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K))  ? (8) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K))  ? (18) : (24)


/* Setting HPF to cut max 200 Hz*/
#define MDF_HPF(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))   ? (MDF_HPF_CUTOFF_0_0095FPCM)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K))  ? (MDF_HPF_CUTOFF_0_0095FPCM) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K))  ? (MDF_HPF_CUTOFF_0_0095FPCM) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K))  ? (MDF_HPF_CUTOFF_0_0095FPCM) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K))  ? (MDF_HPF_CUTOFF_0_0025FPCM)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K))  ? (MDF_HPF_CUTOFF_0_0025FPCM)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K))  ? (MDF_HPF_CUTOFF_0_0025FPCM)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K))  ? (MDF_HPF_CUTOFF_0_00125FPCM) : (MDF_HPF_CUTOFF_0_000625FPCM)

#define MDF_CIC_MODE(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))   ? (MDF_ONE_FILTER_SINC4) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K))  ? (MDF_ONE_FILTER_SINC4) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K))  ? (MDF_ONE_FILTER_SINC5) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K))  ? (MDF_ONE_FILTER_SINC5) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K))  ? (MDF_ONE_FILTER_SINC5) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K))  ? (MDF_ONE_FILTER_SINC5) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K))  ? (MDF_ONE_FILTER_SINC5) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K))  ? (MDF_ONE_FILTER_SINC5) : (MDF_ONE_FILTER_SINC4)

#define MDF_PROC_CLOCK_DIVIDER(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))   ? (2U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K))  ? (1U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K))  ? (2U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K))  ? (1U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K))  ? (2U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K))  ? (1U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K))  ? (2U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K))  ? (2U) : (1U)

#define MDF_OUTPUT_CLOCK_DIVIDER(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))   ? (12U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K))  ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K))  ? (12U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K))  ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K))  ? (12U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K))  ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K))  ? (8U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K))  ? (16U) : (16U)

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  UTIL_AUDIO_params_t       conf;
  MDF_HandleTypeDef         hMdf;
  MDF_FilterConfigTypeDef   hFilterConfig;
  UTIL_AUDIO_DMA_t          hDma;
  UTIL_AUDIO_MDF_User_t     user;
} Context_t;



/* Private variables ---------------------------------------------------------*/
static Context_t G_Context;


/* Private function prototypes -----------------------------------------------*/
void UTIL_AUDIO_MDF_DMA_IRQHandler(void);

static void s_gpio_init(void);
static void s_gpio_deinit(void);
static int32_t s_dmaInit(MDF_HandleTypeDef *hMdf);


/* Functions Definition ------------------------------------------------------*/
/**
* @brief  Initialize MDF.
* @param  AudioInit Init structure
* @retval Status
*/
int32_t UTIL_AUDIO_MDF_Init(UTIL_AUDIO_params_t *AudioInit)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  /* Save in context user config */
  memcpy(&G_Context.conf, AudioInit, sizeof(UTIL_AUDIO_params_t));

  if (UTIL_AUDIO_MDF_ClockConfig(AudioInit->SampleRate) != UTIL_AUDIO_ERROR_NONE)
  {
    error = BSP_ERROR_CLOCK_FAILURE;
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    G_Context.hMdf.Instance = UTIL_AUDIO_MDF_INSTANCE;
    s_gpio_init();
    error = s_dmaInit(&G_Context.hMdf);
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    UTIL_AUDIO_MDF_User_t *pUser = &G_Context.user;

    pUser->pHdle = &G_Context.hMdf;
    pUser->pFilterHdle = &G_Context.hFilterConfig;
    //    pUser->pAudioConf = AudioInit;

    pUser->hMdfConf.Gain               = MDF_GAIN(AudioInit->SampleRate);
    pUser->hMdfConf.Hpf                = MDF_HPF(AudioInit->SampleRate);
    pUser->hMdfConf.DecimationRatio    = MDF_DECIMATION_RATIO(AudioInit->SampleRate);
    pUser->hMdfConf.CicMode            = MDF_CIC_MODE(AudioInit->SampleRate);
    pUser->hMdfConf.ProcClockDivider   = MDF_PROC_CLOCK_DIVIDER(AudioInit->SampleRate);
    pUser->hMdfConf.OutputClockDivider = MDF_OUTPUT_CLOCK_DIVIDER(AudioInit->SampleRate);

    /* Prepare a default filter configuration but User can overwrite thru UTIL_AUDIO_MDF_MX_Init redefinition */
    G_Context.hFilterConfig.DataSource                     = MDF_DATA_SOURCE_BSMX;
    G_Context.hFilterConfig.Delay                          = 0U;
    G_Context.hFilterConfig.CicMode                        = pUser->hMdfConf.CicMode;
    G_Context.hFilterConfig.DecimationRatio                = pUser->hMdfConf.DecimationRatio;
    G_Context.hFilterConfig.Gain                           = pUser->hMdfConf.Gain;
    G_Context.hFilterConfig.ReshapeFilter.Activation       = ENABLE;
    G_Context.hFilterConfig.ReshapeFilter.DecimationRatio  = MDF_RSF_DECIMATION_RATIO_4;
    G_Context.hFilterConfig.HighPassFilter.Activation      = ENABLE;
    G_Context.hFilterConfig.HighPassFilter.CutOffFrequency = pUser->hMdfConf.Hpf; // MDF_HPF_CUTOFF_0_000625FPCM;
    G_Context.hFilterConfig.SoundActivity.Activation       = DISABLE;
    G_Context.hFilterConfig.AcquisitionMode                = MDF_MODE_ASYNC_CONT;
    G_Context.hFilterConfig.FifoThreshold                  = MDF_FIFO_THRESHOLD_NOT_EMPTY;
    G_Context.hFilterConfig.DiscardSamples                 = 0U;


    /* MDF default peripheral initialization but User can overwrite thru UTIL_AUDIO_MDF_MX_Init redefinition */
    G_Context.hMdf.Init.CommonParam.ProcClockDivider               = pUser->hMdfConf.ProcClockDivider;
    G_Context.hMdf.Init.CommonParam.OutputClock.Activation         = ENABLE;
    G_Context.hMdf.Init.CommonParam.OutputClock.Pins               = MDF_OUTPUT_CLOCK_0;
    G_Context.hMdf.Init.CommonParam.OutputClock.Divider            = pUser->hMdfConf.OutputClockDivider;
    G_Context.hMdf.Init.CommonParam.OutputClock.Trigger.Activation = DISABLE;
    G_Context.hMdf.Init.SerialInterface.Activation                 = ENABLE;
    G_Context.hMdf.Init.SerialInterface.Mode                       = MDF_SITF_NORMAL_SPI_MODE;
    G_Context.hMdf.Init.SerialInterface.ClockSource                = MDF_SITF_CCK0_SOURCE;
    G_Context.hMdf.Init.SerialInterface.Threshold                  = 31U;
    G_Context.hMdf.Init.FilterBistream                             = MDF_BITSTREAM0_FALLING;


    if (UTIL_AUDIO_MDF_MX_Init(pUser) != UTIL_AUDIO_ERROR_NONE)
    {
      error = UTIL_AUDIO_ERROR;
    }

  }

  return error;
}

/* This weak can be overwritten to modify periph and filter config from pointer
*   in UTIL_AUDIO_MDF_User_t*/
__weak int32_t UTIL_AUDIO_MDF_MX_Init(UTIL_AUDIO_MDF_User_t *pUser)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;

  if (HAL_MDF_Init(pUser->pHdle) != HAL_OK)
  {
    error = UTIL_AUDIO_ERROR;
  }
  return error;
}


/**
* @brief  DeInitialize MDF.
* @param  None
* @retval Status
*/
int32_t UTIL_AUDIO_MDF_DeInit(void)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;

  error = UTIL_AUDIO_DMA_DeInit(&G_Context.hDma);
  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    s_gpio_deinit();

    if (HAL_MDF_DeInit(&G_Context.hMdf) != HAL_OK)
    {
      error = UTIL_AUDIO_ERROR;
    }
  }
  /* Reset context */
  memset(&G_Context, 0, sizeof(Context_t));
  return error;
}


/**
* @brief  Weak function for clock config (either calling existing function from BSP or write your own). It is called at init.
* @param  Sampling Frequency
* @retval Status
*/
__weak int32_t UTIL_AUDIO_MDF_ClockConfig(uint32_t SampleRate)
{
  return UTIL_AUDIO_ERROR_NONE;
}


/**
* @brief  Starts audio recording with DFSDM.
* @param  pBuf      Main buffer pointer for the recorded data storing
* @param  NbrOfBytes Size of the recorded buffer in bytes
* @retval status
*/
int32_t UTIL_AUDIO_MDF_Start(uint8_t *pBuf, uint32_t NbrOfBytes)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;

  UTIL_AUDIO_DISABLE_IRQ();
  /* Start Main channel with DMA*/
  /* TODO fix me
  something wrong in the MDF DMA management, we need to disable caches to start the the IP
  */
  SCB_DisableDCache();
  MDF_DmaConfigTypeDef dmaConfig;
  dmaConfig.Address                    = (uint32_t)pBuf;
  dmaConfig.DataLength                   = NbrOfBytes;

  if (G_Context.conf.BitsPerSample == AUDIO_RESOLUTION_32B)
  {
    dmaConfig.MsbOnly                    = DISABLE;
  }
  else
  {
    dmaConfig.MsbOnly                    = ENABLE;  /* DMA will only transfer the 16MSB of the acquistion data */
  }

  if (HAL_MDF_AcqStart_DMA(&G_Context.hMdf, &G_Context.hFilterConfig, &dmaConfig) != HAL_OK)
  {
    error = UTIL_AUDIO_ERROR;
  }
  SCB_EnableDCache();
  UTIL_AUDIO_ENABLE_IRQ();

  return error;
}


/**
* @brief  Stops audio recording with DFSDM.
* @param  pBuf      Main buffer pointer for the recorded data storing
* @param  NbrOfBytes Size of the recorded buffer in bytes
* @retval status
*/
int32_t UTIL_AUDIO_MDF_Stop(void)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  UTIL_AUDIO_DISABLE_IRQ();

  if (HAL_MDF_AcqStop_DMA(&G_Context.hMdf) != HAL_OK)
  {
    error = UTIL_AUDIO_ERROR;
  }

  UTIL_AUDIO_ENABLE_IRQ();
  return error;
}


/**
* @brief  Initialize GPIO.
* @param  None
* @retval None
*/
static void s_gpio_init(void)
{
  GPIO_InitTypeDef  gpio_init_structure;

  /* Enable MDF clock */
  UTIL_AUDIO_MDF_CLK_ENABLE();

  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  UTIL_AUDIO_MDF_DATA_IN_ENABLE();
  gpio_init_structure.Pin = UTIL_AUDIO_MDF_DATA_IN_PIN;
  gpio_init_structure.Alternate = UTIL_AUDIO_MDF_DATA_IN_AF;
  HAL_GPIO_Init(UTIL_AUDIO_MDF_DATA_IN_PORT, &gpio_init_structure);

  UTIL_AUDIO_MDF_CLK_OUT_ENABLE();
  gpio_init_structure.Pin = UTIL_AUDIO_MDF_CLK_OUT_PIN;
  gpio_init_structure.Alternate = UTIL_AUDIO_MDF_CLK_OUT_AF;
  HAL_GPIO_Init(UTIL_AUDIO_MDF_CLK_OUT_PORT, &gpio_init_structure);


}


/**
* @brief  Initialize GPIO.
* @param  None
* @retval None
*/
static void s_gpio_deinit(void)
{
  /* De-initialize SD pin */
  HAL_GPIO_DeInit(UTIL_AUDIO_MDF_DATA_IN_PORT, UTIL_AUDIO_MDF_DATA_IN_PIN);
  HAL_GPIO_DeInit(UTIL_AUDIO_MDF_CLK_OUT_PORT, UTIL_AUDIO_MDF_CLK_OUT_PIN);

  /* Disable MDF clock */
  UTIL_AUDIO_MDF_CLK_DISABLE();
}


/**
* @brief  Initialize DMA.
* @param  hMdf  MDF handle
* @retval None
*/
static int32_t s_dmaInit(MDF_HandleTypeDef *hMdf)
{
  /* Enable the DMA clock */
  UTIL_AUDIO_MDF_DMA_CLK_ENABLE();

  G_Context.hDma.pIpHdle         = hMdf;
  G_Context.hDma.type            = UTIL_AUDIO_DMA_IP_TYPE_MDF;
  G_Context.hDma.request         = UTIL_AUDIO_MDF_DMA_REQUEST;
  G_Context.hDma.preemptPriority = UTIL_AUDIO_MDF_IT_PRIORITY;
  G_Context.hDma.pInstance       = UTIL_AUDIO_MDF_DMA_INSTANCE;
  G_Context.hDma.irqn            = UTIL_AUDIO_MDF_DMA_IRQ;
  G_Context.hDma.direction       = DMA_PERIPH_TO_MEMORY;

  if (G_Context.conf.BitsPerSample == AUDIO_RESOLUTION_32B)
  {
    G_Context.hDma.srcDataWidth  = DMA_SRC_DATAWIDTH_WORD;
    G_Context.hDma.destDataWidth = DMA_DEST_DATAWIDTH_WORD;
  }
  else
  {
    G_Context.hDma.srcDataWidth  = DMA_SRC_DATAWIDTH_HALFWORD;
    G_Context.hDma.destDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;
  }
  return UTIL_AUDIO_DMA_Init(&G_Context.hDma);
}


/**
  * @brief  Half reception complete callback.
  * @param  hMdf   MDF handle.
  * @retval None
  */
void MDF_AcqHalfCpltCallback(MDF_HandleTypeDef *hMdf);
void MDF_AcqHalfCpltCallback(MDF_HandleTypeDef *hMdf)
{
  /* Call the record update function to get the first half */
  if (hMdf->Instance == G_Context.hMdf.Instance)
  {
    UTIL_AUDIO_CAPTURE_TxComplete_cb(0UL, 1); /* 1 is PDM */
  }
}


/**
  * @brief  Reception complete callback.
  * @param  hMdf   MDF handle.
  * @retval None
  */
void MDF_AcqCpltCallback(MDF_HandleTypeDef *hMdf);
void MDF_AcqCpltCallback(MDF_HandleTypeDef *hMdf)
{
  /* Call the record update function to get the second half */
  if (hMdf->Instance == G_Context.hMdf.Instance)
  {
    UTIL_AUDIO_CAPTURE_TxComplete_cb(UTIL_AUDIO_CAPTURE_getHalfBuffOffsetBytes(), 1); /* 1 is PDM */
  }
}


void UTIL_AUDIO_MDF_DMA_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(G_Context.hMdf.hdma);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}

#endif // UTIL_AUDIO_MDF_USED
