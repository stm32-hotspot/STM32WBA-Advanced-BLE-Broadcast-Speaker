/**
******************************************************************************
* @file    stm32_audio_sai_pdm.c
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
#include "stm32_audio_sai_pdm.h"
#include "stm32_audio_dma.h"

#ifdef UTIL_AUDIO_SAI_PDM_USED

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
  #error "This driver is not ready for USE_HAL_SAI_REGISTER_CALLBACKS set to 1"
#endif

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#ifndef UTIL_AUDIO_SAI_PDM_D2_USED
  #define UTIL_AUDIO_SAI_PDM_D2_USED 0U
#endif

#define UTIL_AUDIO_PLL_N 344UL
#define UTIL_AUDIO_PLL_M 25UL
#define UTIL_AUDIO_PLL_P 7UL
#define UTIL_AUDIO_PLL_Q 2UL
#define UTIL_AUDIO_PLL_R 35UL
#define UTIL_AUDIO_PLL_FS_kHz (UTIL_AUDIO_PLL_N * 1000UL / UTIL_AUDIO_PLL_P)

/* Private macros ------------------------------------------------------------*/

/* This settings are default setting considering Audio kernel clock is the usual 49.14MHz*/
/* @8kHz                   : Set PDM clock to 1024KHz so Mckdiv to 24(Fsai/(Fpdm*2)) ; Mind that MP23 is in low power mode with such setting, not standard mode */
/* @16kHz & 32kHz          : Set PDM clock to 2048KHz so Mckdiv to 12 (Fsai/(Fpdm*2)) */
/* @11kHz & 22kHz & 44kHz  : Set PDM clock to 1411,2KHz so Mckdiv to 4 (Fsai/(Fpdm*2)) */
/* @48kHz & 96kHz & 192kHz : Set PDM clock to 3072KHz so Mckdiv to 8 (Fsai/(Fpdm*2)) */
/* @88kHz & 196kHz         : Set PDM clock to 2822,4KHz so Mckdiv to 2 (Fsai/(Fpdm*2)) */
#define SAI_PDM_MCKDIV(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))   ? (24U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K))  ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K))  ? (12U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K))  ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K))  ? (12U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K))  ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K))  ? (8U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_88K))  ? (2U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_96K))  ? (8U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_192K)) ? (8U)  : (1U)
//      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_196K)) ? (2U) : (1U)

#define SAI_PDM_FRAMELEN(__D2_USED__) \
        ((__D2_USED__) == (0U))   ? (16U) : (32U)

/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  UTIL_AUDIO_params_t       conf;
  SAI_HandleTypeDef         hSai;
  UTIL_AUDIO_DMA_t          hDma;
  UTIL_AUDIO_SAI_PDM_User_t user;
} Context_t;



/* Private variables ---------------------------------------------------------*/
static Context_t G_Context;


/* Private function prototypes -----------------------------------------------*/
void UTIL_AUDIO_SAI_PDM_DMA_IRQHandler(void);

static void s_gpio_init(void);
static void s_gpio_deinit(void);
static int32_t s_dmaInit(SAI_HandleTypeDef *hsai);
static int32_t s_periphInit(SAI_HandleTypeDef *hsai);



/* Functions Definition ------------------------------------------------------*/
/**
* @brief  Initialize SAI PDM.
* @param  AudioInit Init structure
* @retval Status
*/
int32_t UTIL_AUDIO_SAI_PDM_Init(UTIL_AUDIO_params_t *AudioInit)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  /* Save in context user config */
  memcpy(&G_Context.conf, AudioInit, sizeof(UTIL_AUDIO_params_t));

  if (UTIL_AUDIO_SAI_PDM_ClockConfig(AudioInit->SampleRate) != UTIL_AUDIO_ERROR_NONE)
  {
    error = BSP_ERROR_CLOCK_FAILURE;
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    G_Context.hSai.Instance = UTIL_AUDIO_SAI_PDM_INSTANCE;
    s_gpio_init();
    error = s_dmaInit(&G_Context.hSai);
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    G_Context.user.pHdle      = &G_Context.hSai;
    G_Context.user.pAudioConf = AudioInit;
    error = s_periphInit(&G_Context.hSai);
  }

  return error;
}

/* This weak can be overwritten to modify periph and filter config from pointer
*   in UTIL_AUDIO_MDF_User_t*/
__weak int32_t UTIL_AUDIO_SAI_PDM_MX_Init(UTIL_AUDIO_SAI_PDM_User_t *pUser)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;

  if (HAL_SAI_Init(pUser->pHdle) != HAL_OK)
  {
    error = UTIL_AUDIO_ERROR;
  }
  return error;
}


/**
* @brief  DeInitialize SAI PDM.
* @param  None
* @retval Status
*/
int32_t UTIL_AUDIO_SAI_PDM_DeInit(void)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;

  error = UTIL_AUDIO_DMA_DeInit(&G_Context.hDma);
  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    s_gpio_deinit();

    if (HAL_SAI_DeInit(&G_Context.hSai) != HAL_OK)
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
__weak int32_t UTIL_AUDIO_SAI_PDM_ClockConfig(uint32_t SampleRate)
{
  return UTIL_AUDIO_ERROR_NONE;
}


/**
* @brief  Starts audio recording with DFSDM.
* @param  pBuf      Main buffer pointer for the recorded data storing
* @param  NbrOfBytes Size of the recorded buffer in bytes
* @retval status
*/
int32_t UTIL_AUDIO_SAI_PDM_Start(uint8_t *pBuf, uint32_t NbrOfBytes)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;

  UTIL_AUDIO_DISABLE_IRQ();

  /* Start the process receive DMA */
  /* BSP patch here, we shouldn't use BitsPerSample since DMA is always in
  *  Bytes unlike for SAI_I2S nor DFSDM where dma is configured as function
  *  of BitsPerSample
  */
  //    if (HAL_SAI_Receive_DMA(&G_Context.hSai, (uint8_t *)pBuf, (uint16_t)(NbrOfBytes / (Audio_In_Ctx[Instance].BitsPerSample / 8U))) != HAL_OK)
  if (HAL_SAI_Receive_DMA(&G_Context.hSai, pBuf, (uint16_t)NbrOfBytes) != HAL_OK)
  {
    error = UTIL_AUDIO_ERROR;
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
int32_t UTIL_AUDIO_SAI_PDM_Stop(void)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  UTIL_AUDIO_DISABLE_IRQ();

  if (HAL_SAI_DMAStop(&G_Context.hSai) != HAL_OK)
  {
    error = UTIL_AUDIO_ERROR;
  }

  UTIL_AUDIO_ENABLE_IRQ();
  return error;
}

void UTIL_AUDIO_SAI_PDM_RxHalfCpltCallback(void *pHdleSai)
{
  SAI_HandleTypeDef *hsai = (SAI_HandleTypeDef *)pHdleSai;
  /* Call the record update function to get the first half */
  if (hsai->Instance == G_Context.hSai.Instance)
  {
    UTIL_AUDIO_CAPTURE_TxComplete_cb(0UL, 1); /* 1 is PDM */
  }
}

void UTIL_AUDIO_SAI_PDM_RxCpltCallback(void *pHdleSai)
{
  SAI_HandleTypeDef *hsai = (SAI_HandleTypeDef *)pHdleSai;
  /* Call the record update function to get the second half */
  if (hsai->Instance == G_Context.hSai.Instance)
  {
    UTIL_AUDIO_CAPTURE_TxComplete_cb(UTIL_AUDIO_CAPTURE_getHalfBuffOffsetBytes(), 1); /* 1 is PDM */
  }
}


/**
* @brief  Initialize GPIO.
* @param  None
* @retval None
*/
static void s_gpio_init(void)
{
  GPIO_InitTypeDef  gpio_init_structure;

  /* Enable SAI clock */
  UTIL_AUDIO_SAI_PDM_CLK_ENABLE();

  UTIL_AUDIO_SAI_PDM_CLK_OUT_ENABLE();
  UTIL_AUDIO_SAI_PDM_DATA_IN_ENABLE();

  gpio_init_structure.Pin = UTIL_AUDIO_SAI_PDM_CLK_OUT_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = UTIL_AUDIO_SAI_PDM_CLK_OUT_AF;
  HAL_GPIO_Init(UTIL_AUDIO_SAI_PDM_CLK_OUT_PORT, &gpio_init_structure);

  gpio_init_structure.Pin = UTIL_AUDIO_SAI_PDM_DATA_IN_PIN;
  gpio_init_structure.Alternate = UTIL_AUDIO_SAI_PDM_DATA_IN_AF;
  HAL_GPIO_Init(UTIL_AUDIO_SAI_PDM_DATA_IN_PORT, &gpio_init_structure);

}


/**
* @brief  Initialize GPIO.
* @param  None
* @retval None
*/
static void s_gpio_deinit(void)
{
  /* De-initialize SD pin */
  HAL_GPIO_DeInit(UTIL_AUDIO_SAI_PDM_DATA_IN_PORT, UTIL_AUDIO_SAI_PDM_DATA_IN_PIN);
  HAL_GPIO_DeInit(UTIL_AUDIO_SAI_PDM_CLK_OUT_PORT, UTIL_AUDIO_SAI_PDM_CLK_OUT_PIN);

  /* Disable SAI clock */
  UTIL_AUDIO_SAI_PDM_CLK_DISABLE();
}


/**
* @brief  Initialize DMA.
* @param  hsai  SAI handle
* @retval None
*/
static int32_t s_dmaInit(SAI_HandleTypeDef *hsai)
{
  /* Enable the DMA clock */
  UTIL_AUDIO_SAI_PDM_DMA_CLK_ENABLE();

  G_Context.hDma.pIpHdle         = hsai;
  G_Context.hDma.type            = UTIL_AUDIO_DMA_IP_TYPE_SAI;
  G_Context.hDma.request         = UTIL_AUDIO_SAI_PDM_DMA_REQUEST;
  G_Context.hDma.preemptPriority = UTIL_AUDIO_SAI_PDM_IT_PRIORITY;
  G_Context.hDma.pInstance       = UTIL_AUDIO_SAI_PDM_DMA_INSTANCE;
  G_Context.hDma.irqn            = UTIL_AUDIO_SAI_PDM_DMA_IRQ;
  G_Context.hDma.srcDataWidth    = UTIL_AUDIO_SAI_PDM_DMA_SRC_DATAWIDTH;
  G_Context.hDma.destDataWidth   = UTIL_AUDIO_SAI_PDM_DMA_DEST_DATAWIDTH;
  G_Context.hDma.direction       = DMA_PERIPH_TO_MEMORY;

  return UTIL_AUDIO_DMA_Init(&G_Context.hDma);
}

/**
* @brief  Initialize DMA.
* @param  hsai  SAI handle
* @retval None
*/
static int32_t s_periphInit(SAI_HandleTypeDef *hsai)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;
  UTIL_AUDIO_SAI_PDM_User_t *pUser = &G_Context.user;
  uint32_t SlotNumber              = 2UL;
  uint32_t MicPairsNbr             = 1UL;


  /* Prepare a default  configuration but User can overwrite thru UTIL_AUDIO_SAI_PDM_MX_Init redefinition */

  pUser->hSaiPdmConf.MonoStereoMode    = SAI_STEREOMODE;
  pUser->hSaiPdmConf.DataSize          = SAI_DATASIZE_8;
  pUser->hSaiPdmConf.FrameLength       = SAI_PDM_FRAMELEN(UTIL_AUDIO_SAI_PDM_D2_USED);
  pUser->hSaiPdmConf.ActiveFrameLength = 1;
  pUser->hSaiPdmConf.OutputDrive       = SAI_OUTPUTDRIVE_DISABLE;
  pUser->hSaiPdmConf.SlotActive        = (pUser->pAudioConf->ChannelsNbr == 1U) ? SAI_SLOTACTIVE_0 : SAI_SLOTACTIVE_ALL;
  pUser->hSaiPdmConf.AudioFrequency    = pUser->pAudioConf->SampleRate;
  pUser->hSaiPdmConf.AudioMode         = SAI_MODEMASTER_RX;
  pUser->hSaiPdmConf.ClockStrobing     = SAI_CLOCKSTROBING_FALLINGEDGE;
  pUser->hSaiPdmConf.Synchro           = SAI_ASYNCHRONOUS;
  pUser->hSaiPdmConf.SynchroExt        = SAI_SYNCEXT_DISABLE;
  pUser->hSaiPdmConf.MckDiv            = SAI_PDM_MCKDIV(pUser->pAudioConf->SampleRate);

  if (UTIL_AUDIO_SAI_PDM_D2_USED == 1U) /* This is a trick to work around PCB mistake of connecting mic to D2 before D1 */
  {
    uint32_t pdmClk = UTIL_AUDIO_CAPTURE_getMicClk();
    SlotNumber = 4UL;
    MicPairsNbr = 2UL;
    pUser->hSaiPdmConf.SlotActive        = SAI_SLOTACTIVE_2;      // mic connected to D2 so slot can be 2 or 3 (not 0 nor 1 that are from D1)
    pUser->hSaiPdmConf.MckDiv            = (UTIL_AUDIO_PLL_FS_kHz + (pdmClk * SlotNumber / 2UL)) / (pdmClk * SlotNumber);
    /*                                   input SAI clock = 344/7 = 49,14MHz, it is not the exact expected clock (49,152MHz is) so that there is an error on the final PCM rate.
    *                                    Therefore the ratio calculated from it will be wrong if this value is not corrected.
    *                                    That is why we round the value to the closest value before processing the ratio.
    *                                    The expected mic clock is:
    *                                    pdmClk for 4 slots because mic is connected to D2
    *                                    (cannot select 1 or 2 slots). Hence, the SAI internal clock
    *                                    should be SAI_IntClk = 4*pdmClk ; then MCKDIV = 49,14/SAI_IntClk */

  }


  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(&G_Context.hSai);

  /* Configure SAI1_Block_A */
  G_Context.hSai.Init.AudioFrequency         = SAI_AUDIO_FREQUENCY_MCKDIV;
  G_Context.hSai.Init.MonoStereoMode         = pUser->hSaiPdmConf.MonoStereoMode;
  G_Context.hSai.Init.AudioMode              = pUser->hSaiPdmConf.AudioMode;
  G_Context.hSai.Init.NoDivider              = SAI_MASTERDIVIDER_DISABLE;
  G_Context.hSai.Init.Protocol               = SAI_FREE_PROTOCOL;
  G_Context.hSai.Init.DataSize               = pUser->hSaiPdmConf.DataSize;
  G_Context.hSai.Init.FirstBit               = SAI_FIRSTBIT_LSB;
  G_Context.hSai.Init.ClockStrobing          = pUser->hSaiPdmConf.ClockStrobing;
  G_Context.hSai.Init.Synchro                = pUser->hSaiPdmConf.Synchro;
  G_Context.hSai.Init.OutputDrive            = pUser->hSaiPdmConf.OutputDrive;
  G_Context.hSai.Init.FIFOThreshold          = SAI_FIFOTHRESHOLD_1QF;
  G_Context.hSai.Init.SynchroExt             = pUser->hSaiPdmConf.SynchroExt;
  G_Context.hSai.Init.CompandingMode         = SAI_NOCOMPANDING;
  G_Context.hSai.Init.TriState               = SAI_OUTPUT_NOTRELEASED;
  G_Context.hSai.Init.Mckdiv                 = pUser->hSaiPdmConf.MckDiv;
  G_Context.hSai.Init.PdmInit.Activation     = ENABLE;
  G_Context.hSai.Init.PdmInit.MicPairsNbr    = MicPairsNbr;
  G_Context.hSai.Init.PdmInit.ClockEnable    = UTIL_AUDIO_SAI_PDM_CLK_OUT_ID;


  /* Configure SAI_Block_x Frame */
  G_Context.hSai.FrameInit.FrameLength       = pUser->hSaiPdmConf.FrameLength;
  G_Context.hSai.FrameInit.ActiveFrameLength = pUser->hSaiPdmConf.ActiveFrameLength;
  G_Context.hSai.FrameInit.FSDefinition      = SAI_FS_STARTFRAME;
  G_Context.hSai.FrameInit.FSPolarity        = SAI_FS_ACTIVE_HIGH;
  G_Context.hSai.FrameInit.FSOffset          = SAI_FS_FIRSTBIT;

  /* Configure SAI Block_x Slot */
  G_Context.hSai.SlotInit.FirstBitOffset     = 0;
  G_Context.hSai.SlotInit.SlotSize           = SAI_SLOTSIZE_DATASIZE;
  G_Context.hSai.SlotInit.SlotNumber         = SlotNumber;
  G_Context.hSai.SlotInit.SlotActive         = pUser->hSaiPdmConf.SlotActive;

  if (UTIL_AUDIO_SAI_PDM_MX_Init(pUser) != UTIL_AUDIO_ERROR_NONE)
  {
    error = UTIL_AUDIO_ERROR;
  }
  return error;
}



void UTIL_AUDIO_SAI_PDM_DMA_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(G_Context.hSai.hdmarx);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}

#endif // UTIL_AUDIO_SAI_PDM_USED
