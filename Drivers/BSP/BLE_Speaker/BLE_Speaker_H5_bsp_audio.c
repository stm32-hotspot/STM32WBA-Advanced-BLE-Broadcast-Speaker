/**
  ******************************************************************************
  * @file    BLE_Speaker_H5_bsp_audio.c
  * @author  MCD Application Team
  * @brief   This file provides the Audio driver for speaker board, derivated from
              stm32h573i_discovery_audio.c .
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "BLE_Speaker_H5_bsp_audio.h"

/**SAI2_B_Block_B GPIO Configuration
PC0     ------> SAI2_FS_B
PA0     ------> SAI2_SD_B
PA2     ------> SAI2_SCK_B
*/

#define SAI_SD_IN_PIN  GPIO_PIN_0
#define SAI_SD_IN_PORT GPIOA

#define SAI_FS_PIN      GPIO_PIN_0
#define SAI_FS_PORT     GPIOC

#define SAI_MCK_PIN     GPIO_PIN_2
#define SAI_MCK_PORT    GPIOA

#define SAI_GPIO_CLK_ENABLE __HAL_RCC_GPIOC_CLK_ENABLE(); \
                            __HAL_RCC_GPIOA_CLK_ENABLE()
/**SAI2_A_Block_A GPIO Configuration
PC1     ------> SAI2_SD_A
*/
#define SAI_SD_OUT_PIN   GPIO_PIN_1
#define SAI_SD_OUT_PORT  GPIOC

/**SAI1_A_Block_A GPIO Configuration for PDM
PE2     ------> SAI1_CK1
PE6     ------> SAI1_D1
*/
#define SAIPDM_SD_PIN  GPIO_PIN_6
#define SAIPDM_SD_PORT GPIOE

#define SAIPDM_CK_PIN  GPIO_PIN_2
#define SAIPDM_CK_PORT GPIOE

#define SAIPDM_GPIO_CLK_ENABLE __HAL_RCC_GPIOE_CLK_ENABLE()


SAI_HandleTypeDef      haudio_in_sai2 = {NULL};
I2S_HandleTypeDef      haudio_out_i2s = {NULL};
SAI_HandleTypeDef      haudio_in_sai1 = {NULL};
SAI_HandleTypeDef      haudio_out_sai2 = {NULL};

static HAL_StatusTypeDef MX_I2S1_Init(I2S_HandleTypeDef *hsai, uint32_t frequency);
static HAL_StatusTypeDef MX_SAI2_Init(SAI_HandleTypeDef *hsai, MX_SAI_Config_t *MXInit);
static HAL_StatusTypeDef MX_SAI1_Init(SAI_HandleTypeDef *hsai, MX_SAI_Config_t *MXInit);

static void SAIPDM_MspInit(SAI_HandleTypeDef *hsai);
static void SAI_IN_MspInit(SAI_HandleTypeDef *hsai);
static void SAI_OUT_MspInit(SAI_HandleTypeDef *hsai);
static void I2S_MspInit(I2S_HandleTypeDef *hi2s);

static void BSP_Error_Handler(void);

static AUDIO_OUT_Ctx_t Audio_I2sOut_Ctx = {0};
static AUDIO_OUT_Ctx_t Audio_SaiOut_Ctx = {0};
static AUDIO_IN_Ctx_t  Audio_SaiIn_Ctx = {0};
static AUDIO_IN_Ctx_t  Audio_MicIn_Ctx = {0};

/* Audio in DMA handles */
static DMA_HandleTypeDef hDmaSai1Rx = {NULL};
static DMA_HandleTypeDef hDmaSai2Rx = {NULL};
static DMA_HandleTypeDef hDmaSai2Tx = {NULL};
static DMA_HandleTypeDef hDmaI2sTx = {NULL};

/* Queue variables declaration */
static DMA_QListTypeDef SAIPDMRxQueue;
static DMA_QListTypeDef SAIRxQueue;
static DMA_QListTypeDef SAITxQueue;
static DMA_QListTypeDef I2STxQueue;

extern void APP_AUDIO_OUT_Tranfert_Callback(uint8_t half);

/*
****************************** BSP IN PDM ******************************
*/

int32_t BSP_AUDIO_IN_InitPDM(uint32_t Instance, BSP_AUDIO_Init_t* AudioInit)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= 1)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Store the audio record context */
    Audio_MicIn_Ctx.Device          = AudioInit->Device;
    Audio_MicIn_Ctx.ChannelsNbr     = AudioInit->ChannelsNbr;
    Audio_MicIn_Ctx.SampleRate      = AudioInit->SampleRate;
    Audio_MicIn_Ctx.BitsPerSample   = AudioInit->BitsPerSample;
    Audio_MicIn_Ctx.Volume          = AudioInit->Volume;
    Audio_MicIn_Ctx.State           = AUDIO_IN_STATE_RESET;

    if (ret == BSP_ERROR_NONE)
    {
      /* SAI1 gets its clock from SAI1_Block_A
      PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
      if(MX_SAI1_ClockConfig(&haudio_in_sai1, AudioInit->SampleRate) != HAL_OK)
      {
        ret = BSP_ERROR_CLOCK_FAILURE;
      }
      else
      {
        haudio_in_sai1.Instance = SAI1_Block_A;

        SAIPDM_MspInit(&haudio_in_sai1);

        MX_SAI_Config_t mx_config;

        /* Prepare haudio_in_sai handle */
        mx_config.MonoStereoMode    = SAI_STEREOMODE;
        mx_config.DataSize          = SAI_DATASIZE_8;
        mx_config.FrameLength       = 16;
        mx_config.ActiveFrameLength = 1;
        mx_config.OutputDrive       = SAI_OUTPUTDRIVE_DISABLE;
        mx_config.SlotActive        = SAI_SLOTACTIVE_ALL;
        mx_config.AudioFrequency    = AudioInit->SampleRate;
        mx_config.AudioMode         = SAI_MODEMASTER_RX;
        mx_config.ClockStrobing     = SAI_CLOCKSTROBING_FALLINGEDGE;
        mx_config.Synchro           = SAI_ASYNCHRONOUS;
        mx_config.SynchroExt        = SAI_SYNCEXT_DISABLE;

        if(MX_SAI1_Init(&haudio_in_sai1, &mx_config) != HAL_OK)
        {
          /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }

    /* Update BSP AUDIO IN state */
    Audio_MicIn_Ctx.State = AUDIO_IN_STATE_STOP;
  }

  /* Return BSP status */
  return ret;
}


int32_t BSP_AUDIO_IN_RecordPDM(uint32_t Instance, uint8_t* pBuf, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Start the process receive DMA */
  if(HAL_SAI_Receive_DMA(&haudio_in_sai1, (uint8_t*)pBuf, (uint16_t)(NbrOfBytes/(Audio_MicIn_Ctx.BitsPerSample/8U))) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }

  /* Update BSP AUDIO IN state */
  Audio_MicIn_Ctx.State = AUDIO_IN_STATE_RECORDING;

  /* Return BSP status */
  return ret;
}

/*
****************************** BSP IN ******************************
*/

/**
  * @brief  Initialize wave recording.
  * @param  AudioInit Init structure
  * @retval BSP status
  */
int32_t BSP_SAI_IN_Init(uint32_t Instance, BSP_AUDIO_Init_t *AudioInit)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= 1)
  {
    return -1;
  }
  else
  {
    /* Store the audio record context */
    Audio_SaiIn_Ctx.Device          = AudioInit->Device;
    Audio_SaiIn_Ctx.ChannelsNbr     = AudioInit->ChannelsNbr;
    Audio_SaiIn_Ctx.SampleRate      = AudioInit->SampleRate;
    Audio_SaiIn_Ctx.BitsPerSample   = AudioInit->BitsPerSample;
    Audio_SaiIn_Ctx.Volume          = AudioInit->Volume;
    Audio_SaiIn_Ctx.State           = AUDIO_IN_STATE_RESET;

    /* Set SAI instances (SAI1_Block_A needed for MCLK, FS and CLK signals) */
    haudio_in_sai2.Instance  = SAI2_Block_B;

    /* Configure the SAI PLL according to the requested audio frequency if not already done by other instances */
    if (MX_SAI2_ClockConfig(&haudio_in_sai2, AudioInit->SampleRate) != HAL_OK)
    {
      ret = BSP_ERROR_CLOCK_FAILURE;
    }

    if (ret == BSP_ERROR_NONE)
    {
      SAI_IN_MspInit(&haudio_in_sai2);
    }

    if (ret == BSP_ERROR_NONE)
    {
      MX_SAI_Config_t mx_config;

      /* Prepare haudio_in_sai1 handle */
      mx_config.AudioFrequency        = Audio_SaiIn_Ctx.SampleRate;
      mx_config.AudioMode             = SAI_MODESLAVE_RX;
      mx_config.ClockStrobing         = SAI_CLOCKSTROBING_RISINGEDGE;
      mx_config.MonoStereoMode        = SAI_STEREOMODE;
      mx_config.DataSize              = SAI_DATASIZE_16;
      mx_config.FrameLength           = 32;
      mx_config.ActiveFrameLength     = 16;
      mx_config.OutputDrive           = SAI_OUTPUTDRIVE_ENABLE;
      mx_config.Synchro               = SAI_ASYNCHRONOUS;
      mx_config.SynchroExt            = SAI_SYNCEXT_DISABLE; /* provide clock to SAI2 */
      mx_config.SlotActive            = (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1);

      if (MX_SAI2_Init(&haudio_in_sai2, &mx_config) != HAL_OK)
      {
        /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      Audio_SaiIn_Ctx.State = AUDIO_IN_STATE_STOP;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Start audio recording.
  * @param  pBuf     Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes  Size of the record buffer
  * @retval BSP status
  */
int32_t BSP_SAI_IN_Record(uint32_t Instance, uint8_t *pBuf, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check audio in state */
  if (Audio_SaiIn_Ctx.State != AUDIO_IN_STATE_STOP)
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    /* Initiate a DMA transfer of audio samples from the serial audio interface */
    /* Because only 16 bits per sample is supported, DMA transfer is in halfword size */
    if (HAL_SAI_Receive_DMA(&haudio_in_sai2, (uint8_t *) pBuf, (uint16_t) NbrOfBytes / 2U) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }

    if (ret == BSP_ERROR_NONE)
    {
      /* Update audio in state */
      Audio_SaiIn_Ctx.State = AUDIO_IN_STATE_RECORDING;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Stop audio recording.
  * @retval BSP status
  */
int32_t BSP_SAI_IN_Stop(void)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Audio_SaiIn_Ctx.State == AUDIO_IN_STATE_STOP)
  {
    /* Nothing to do */
  }
  else if ((Audio_SaiIn_Ctx.State != AUDIO_IN_STATE_RECORDING) &&
           (Audio_SaiIn_Ctx.State != AUDIO_IN_STATE_PAUSE))
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    if (HAL_SAI_DMAStop(&haudio_in_sai2) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Update BSP AUDIO IN state */
      Audio_SaiIn_Ctx.State = AUDIO_IN_STATE_STOP;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Pause the audio file stream.
  * @retval BSP status
  */
int32_t BSP_SAI_IN_Pause(void)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Audio_SaiIn_Ctx.State != AUDIO_IN_STATE_RECORDING)
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    /* Pause DMA transfer of audio samples from the serial audio interface */
    if (HAL_SAI_DMAPause(&haudio_in_sai2) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Update BSP AUDIO IN state */
      Audio_SaiIn_Ctx.State = AUDIO_IN_STATE_PAUSE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Resume the audio file stream.
  * @retval BSP status
  */
int32_t BSP_SAI_IN_Resume(void)
{
  int32_t ret = BSP_ERROR_NONE;
  if (Audio_SaiIn_Ctx.State != AUDIO_IN_STATE_PAUSE)
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    /* Resume DMA transfer of audio samples from the serial audio interface */
    if (HAL_SAI_DMAResume(&haudio_in_sai2) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Update BSP AUDIO IN state */
      Audio_SaiIn_Ctx.State = AUDIO_IN_STATE_RECORDING;
    }
  }

  /* Return BSP status */
  return ret;
}


/*
****************************** BSP OUT to SPI (CODEC) ******************************
*/
/* @note : mode 1 - allows to chose the SPI to be in sync with an external audio device (AUDIOCLK)
                0 - uses the internal PLL as clock source
*/
int32_t BSP_SPI_OUT_Init(uint32_t mode, BSP_AUDIO_Init_t *AudioInit)
{

  int32_t status = BSP_ERROR_NONE;

  if (mode > 1)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Fill audio out context structure */
    Audio_I2sOut_Ctx.Device         = AudioInit->Device;
    Audio_I2sOut_Ctx.SampleRate     = AudioInit->SampleRate;
    Audio_I2sOut_Ctx.BitsPerSample  = AudioInit->BitsPerSample;
    Audio_I2sOut_Ctx.ChannelsNbr    = AudioInit->ChannelsNbr;
    Audio_I2sOut_Ctx.Volume         = AudioInit->Volume;

    if (status == BSP_ERROR_NONE)
    {
      /* Set SAI instance */
      haudio_out_i2s.Instance = SPI1;

      status = MX_I2S1_ClockConfig(Audio_I2sOut_Ctx.SampleRate, mode);

      if (status == BSP_ERROR_NONE)
      {
        I2S_MspInit(&haudio_out_i2s);
      }

      if (status == BSP_ERROR_NONE)
      {

        /* SAI peripheral initialization */
        if (MX_I2S1_Init(&haudio_out_i2s, AudioInit->SampleRate) != HAL_OK)
        {
          status = BSP_ERROR_PERIPH_FAILURE;
        }

        else
        {
          /* Update audio out context state */
          Audio_I2sOut_Ctx.State = AUDIO_OUT_STATE_STOP;
        }
      }
    }
  }
  return status;
}


int32_t BSP_SPI_OUT_Play(uint32_t Instance, uint8_t *pData, uint32_t NbrOfDmaDatas){
  int32_t  status = BSP_ERROR_NONE;

  if (pData == NULL)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_I2sOut_Ctx.State != AUDIO_OUT_STATE_STOP)
  {
    status = BSP_ERROR_BUSY;
  }
  else
  {
    /* Initiate a DMA transfer of audio samples towards the serial audio interface */
    if (HAL_I2S_Transmit_DMA(&haudio_out_i2s, (uint16_t*)pData, NbrOfDmaDatas/2) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Update audio out state */
      Audio_I2sOut_Ctx.State = AUDIO_OUT_STATE_PLAYING;
    }
  }
  return status;
}


int32_t BSP_SPI_OUT_Pause(uint32_t Instance)
{
  return 0;
}

int32_t BSP_SPI_OUT_Stop(uint32_t Instance)
{
  UNUSED(Instance);
  int32_t ret = BSP_ERROR_NONE;

  if (Audio_I2sOut_Ctx.State == AUDIO_OUT_STATE_STOP)
  {
    /* Nothing to do */
  }
  else if ((Audio_I2sOut_Ctx.State != AUDIO_OUT_STATE_PLAYING) &&
           (Audio_I2sOut_Ctx.State != AUDIO_OUT_STATE_PAUSE))
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    if (HAL_I2S_DMAStop(&haudio_out_i2s) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Update BSP AUDIO IN state */
      Audio_I2sOut_Ctx.State = AUDIO_OUT_STATE_STOP;
    }
  }

  /* Return BSP status */
  return ret;
}


/*
****************************** BSP OUT to SAI ******************************
*/

/**
  * @brief  Initialize SAI OUT based on SAI IN
  * @param  AudioInit Init structure
  * @retval BSP status
  */
int32_t BSP_SAI_OUT_Init(uint32_t Instance, BSP_AUDIO_Init_t *AudioInit)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Audio_SaiIn_Ctx.State == AUDIO_IN_STATE_RESET)
  {
    return BSP_ERROR_NO_INIT; /* BSP IN must be enabled */
  }

  if (Instance >= 1)
  {
    return -1;
  }
  else
  {
    /* Store the audio record context */
    Audio_SaiOut_Ctx.Device          = AudioInit->Device;
    Audio_SaiOut_Ctx.ChannelsNbr     = AudioInit->ChannelsNbr;
    Audio_SaiOut_Ctx.SampleRate      = AudioInit->SampleRate;
    Audio_SaiOut_Ctx.BitsPerSample   = AudioInit->BitsPerSample;
    Audio_SaiOut_Ctx.Volume          = AudioInit->Volume;
    Audio_SaiOut_Ctx.State           = AUDIO_IN_STATE_RESET;

    /* Set SAI instances (SAI1_Block_A needed for MCLK, FS and CLK signals) */
    haudio_out_sai2.Instance  = SAI2_Block_A;

    /* Clock are already configured for BSP in */

    if (ret == BSP_ERROR_NONE)
    {
      SAI_OUT_MspInit(&haudio_out_sai2);
    }

    if (ret == BSP_ERROR_NONE)
    {
      /* no need to reinit SAI */

      Audio_SaiOut_Ctx.State = AUDIO_OUT_STATE_STOP;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Start audio recording.
  * @param  pBuf     Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes  Size of the record buffer
  * @retval BSP status
  */
int32_t BSP_SAI_OUT_Play(uint32_t Instance, uint8_t *pBuf, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;

  /* Check audio in state */
  if (Audio_SaiOut_Ctx.State != AUDIO_OUT_STATE_STOP)
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    /* Initiate a DMA transfer of audio samples from the serial audio interface */
    /* Because only 16 bits per sample is supported, DMA transfer is in halfword size */
    if (HAL_SAI_Transmit_DMA(&haudio_out_sai2, (uint8_t *) pBuf, (uint16_t) NbrOfBytes / 2U) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }

    if (ret == BSP_ERROR_NONE)
    {
      /* Update audio out state */
      Audio_SaiOut_Ctx.State = AUDIO_OUT_STATE_PLAYING;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Stop audio recording.
  * @retval BSP status
  */
int32_t BSP_SAI_OUT_Stop(void)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Audio_SaiOut_Ctx.State == AUDIO_OUT_STATE_STOP)
  {
    /* Nothing to do */
  }
  else if ((Audio_SaiOut_Ctx.State != AUDIO_OUT_STATE_PLAYING) &&
           (Audio_SaiOut_Ctx.State != AUDIO_OUT_STATE_PAUSE))
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    if (HAL_SAI_DMAStop(&haudio_out_sai2) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Update BSP AUDIO OUT state */
      Audio_SaiOut_Ctx.State = AUDIO_OUT_STATE_STOP;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Pause the audio file stream.
  * @retval BSP status
  */
int32_t BSP_SAI_OUT_Pause(void)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Audio_SaiOut_Ctx.State != AUDIO_OUT_STATE_PLAYING)
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    /* Pause DMA transfer of audio samples from the serial audio interface */
    if (HAL_SAI_DMAPause(&haudio_out_sai2) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Update BSP AUDIO OUT state */
      Audio_SaiOut_Ctx.State = AUDIO_OUT_STATE_PAUSE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Resume the audio file stream.
  * @retval BSP status
  */
int32_t BSP_SAI_OUT_Resume(void)
{
  int32_t ret = BSP_ERROR_NONE;
  if (Audio_SaiOut_Ctx.State != AUDIO_OUT_STATE_PAUSE)
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    /* Resume DMA transfer of audio samples from the serial audio interface */
    if (HAL_SAI_DMAResume(&haudio_out_sai2) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Update BSP AUDIO OUT state */
      Audio_SaiOut_Ctx.State = AUDIO_OUT_STATE_PLAYING;
    }
  }

  /* Return BSP status */
  return ret;
}

/*
****************************** Static init ******************************
*/
static HAL_StatusTypeDef MX_SAI2_Init(SAI_HandleTypeDef *hsai, MX_SAI_Config_t *MXInit)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(hsai);

  /* Configure SAI1_Block_X */
  hsai->Init.MonoStereoMode       = MXInit->MonoStereoMode;
  hsai->Init.AudioFrequency       = MXInit->AudioFrequency;
  hsai->Init.AudioMode            = MXInit->AudioMode;
  hsai->Init.NoDivider            = SAI_MASTERDIVIDER_ENABLE;
  hsai->Init.Protocol             = SAI_FREE_PROTOCOL;
  hsai->Init.DataSize             = MXInit->DataSize;
  hsai->Init.FirstBit             = SAI_FIRSTBIT_MSB;
  hsai->Init.ClockStrobing        = MXInit->ClockStrobing;
  hsai->Init.Synchro              = MXInit->Synchro;
  hsai->Init.OutputDrive          = MXInit->OutputDrive;
  hsai->Init.FIFOThreshold        = SAI_FIFOTHRESHOLD_1QF;
  hsai->Init.SynchroExt           = MXInit->SynchroExt;
  hsai->Init.CompandingMode       = SAI_NOCOMPANDING;
  hsai->Init.TriState             = SAI_OUTPUT_NOTRELEASED;
  hsai->Init.Mckdiv               = 0U;
  hsai->Init.MckOutput            = SAI_MCK_OUTPUT_ENABLE;
  hsai->Init.MckOverSampling      = SAI_MCK_OVERSAMPLING_DISABLE;
  hsai->Init.PdmInit.Activation   = DISABLE;

  /* Configure SAI1_Block_X Frame */
  hsai->FrameInit.FrameLength       = MXInit->FrameLength;
  hsai->FrameInit.ActiveFrameLength = MXInit->ActiveFrameLength;
  hsai->FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
  hsai->FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
  hsai->FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;

  /* Configure SAI1_Block_X Slot */
  hsai->SlotInit.FirstBitOffset     = 0;
  if (MXInit->DataSize == AUDIO_RESOLUTION_24B)
  {
    hsai->SlotInit.SlotSize         = SAI_SLOTSIZE_32B;
  }
  else
  {
    hsai->SlotInit.SlotSize         = SAI_SLOTSIZE_16B;
  }
  hsai->SlotInit.SlotNumber         = 2;
  hsai->SlotInit.SlotActive         = MXInit->SlotActive;

  if (HAL_SAI_Init(hsai) != HAL_OK)
  {
    status = HAL_ERROR;
  }

  return status;
}

static HAL_StatusTypeDef MX_SAI1_Init(SAI_HandleTypeDef* hsai, MX_SAI_Config_t *MXConfig)
{
  HAL_StatusTypeDef ret = HAL_OK;

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(hsai);

  /* Configure SAI1_Block_A */
  hsai->Init.AudioFrequency         = SAI_AUDIO_FREQUENCY_MCKDIV;
  hsai->Init.MonoStereoMode         = MXConfig->MonoStereoMode;
  hsai->Init.AudioMode              = MXConfig->AudioMode;
  hsai->Init.NoDivider              = SAI_MASTERDIVIDER_DISABLE;
  hsai->Init.Protocol               = SAI_FREE_PROTOCOL;
  hsai->Init.DataSize               = MXConfig->DataSize;
  hsai->Init.FirstBit               = SAI_FIRSTBIT_LSB;
  hsai->Init.ClockStrobing          = MXConfig->ClockStrobing;
  hsai->Init.Synchro                = MXConfig->Synchro;
  hsai->Init.OutputDrive            = MXConfig->OutputDrive;
  hsai->Init.FIFOThreshold          = SAI_FIFOTHRESHOLD_1QF;
  hsai->Init.SynchroExt             = MXConfig->SynchroExt;
  hsai->Init.CompandingMode         = SAI_NOCOMPANDING;
  hsai->Init.TriState               = SAI_OUTPUT_NOTRELEASED;
  /* Set Mckdiv according sample rate */
  if ((MXConfig->AudioFrequency == AUDIO_FREQUENCY_8K) || (MXConfig->AudioFrequency == AUDIO_FREQUENCY_16K) ||
      (MXConfig->AudioFrequency == AUDIO_FREQUENCY_32K))
  {
    /* Set PDM clock to 2048KHz */
    hsai->Init.Mckdiv = 3U;
  }
  else if ((MXConfig->AudioFrequency == AUDIO_FREQUENCY_48K) || (MXConfig->AudioFrequency == AUDIO_FREQUENCY_96K) ||
           (MXConfig->AudioFrequency == AUDIO_FREQUENCY_192K))
  {
    hsai->Init.Mckdiv = 0U;
    ret = HAL_ERROR;
  }
  else
  {
    ret = HAL_ERROR;
  }

  hsai->Init.PdmInit.Activation     = ENABLE;
  hsai->Init.PdmInit.MicPairsNbr    = 1;
  hsai->Init.PdmInit.ClockEnable    = SAI_PDM_CLOCK1_ENABLE;

  /* Configure SAI_Block_x Frame */
  hsai->FrameInit.FrameLength       = MXConfig->FrameLength;
  hsai->FrameInit.ActiveFrameLength = MXConfig->ActiveFrameLength;
  hsai->FrameInit.FSDefinition      = SAI_FS_STARTFRAME;
  hsai->FrameInit.FSPolarity        = SAI_FS_ACTIVE_HIGH;
  hsai->FrameInit.FSOffset          = SAI_FS_FIRSTBIT;

  /* Configure SAI Block_x Slot */
  hsai->SlotInit.FirstBitOffset     = 0;
  hsai->SlotInit.SlotSize           = SAI_SLOTSIZE_DATASIZE;
  hsai->SlotInit.SlotNumber         = 2;
  hsai->SlotInit.SlotActive         = MXConfig->SlotActive;

  if(HAL_SAI_Init(hsai) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  return ret;
}

static HAL_StatusTypeDef MX_I2S1_Init(I2S_HandleTypeDef *hi2s, uint32_t frequency)
{
  HAL_StatusTypeDef status = HAL_OK;

  hi2s->Instance = SPI1;
  hi2s->Init.Mode = I2S_MODE_MASTER_FULLDUPLEX;
  hi2s->Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s->Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s->Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s->Init.AudioFreq = frequency;
  hi2s->Init.CPOL = I2S_CPOL_LOW;
  hi2s->Init.FirstBit = I2S_FIRSTBIT_MSB;
  hi2s->Init.WSInversion = I2S_WS_INVERSION_DISABLE;
  hi2s->Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
  hi2s->Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_DISABLE;
  if (HAL_I2S_Init(hi2s) != HAL_OK)
  {
    status = HAL_ERROR;
  }

  return status;
}

/**
  * @brief  Initialize BSP_AUDIO_OUT MSP.
  * @param  hsai  SAI handle
  * @retval None
  */
static void SAI_IN_MspInit(SAI_HandleTypeDef *hsai)
{
  GPIO_InitTypeDef  gpio_init_structure;
  static DMA_NodeTypeDef RxNode;
  static DMA_NodeConfTypeDef dmaNodeConfig;

  /* Enable SAI clock */
  __HAL_RCC_SAI2_CLK_ENABLE();

  /* Enable GPIO clock */
  SAI_GPIO_CLK_ENABLE;

  /* CODEC_SAI pins configuration: FS, SCK, MCK and SD pins ------------------*/

  gpio_init_structure.Pin = SAI_SD_IN_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_MEDIUM;
  gpio_init_structure.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(SAI_SD_IN_PORT, &gpio_init_structure);

  gpio_init_structure.Pin = SAI_FS_PIN;
  gpio_init_structure.Alternate = GPIO_AF8_SAI2;
  HAL_GPIO_Init(SAI_FS_PORT, &gpio_init_structure);

  gpio_init_structure.Pin =  SAI_MCK_PIN;
  gpio_init_structure.Alternate = GPIO_AF8_SAI2;
  HAL_GPIO_Init(SAI_MCK_PORT, &gpio_init_structure);


  /* Enable the DMA clock */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  if (SAIRxQueue.Head != NULL)
  {
    /* Reset the DMA Channel configuration*/
    if (HAL_DMAEx_List_DeInit(&hDmaSai2Rx) != HAL_OK)
    {
     BSP_Error_Handler();
    }

    /* Reset RxQueue */
    if (HAL_DMAEx_List_ResetQ(&SAIRxQueue) != HAL_OK)
    {
      BSP_Error_Handler();
    }
  }

  /* DMA for Rx */
  /* Set node type */
  dmaNodeConfig.NodeType                            = DMA_GPDMA_LINEAR_NODE;
  /* Set common node parameters */
  dmaNodeConfig.Init.Request                        = GPDMA1_REQUEST_SAI2_B;
  dmaNodeConfig.Init.BlkHWRequest                   = DMA_BREQ_SINGLE_BURST;
  dmaNodeConfig.Init.Direction                      = DMA_PERIPH_TO_MEMORY;
  dmaNodeConfig.Init.SrcInc                         = DMA_SINC_FIXED;
  dmaNodeConfig.Init.DestInc                        = DMA_DINC_INCREMENTED;
  if (Audio_SaiIn_Ctx.BitsPerSample == AUDIO_RESOLUTION_16B)
  {
    dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_HALFWORD;
    dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_HALFWORD;
  }
  else /* AUDIO_RESOLUTION_24b */
  {
    dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_WORD;
    dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_WORD;
  }
  dmaNodeConfig.Init.SrcBurstLength                 = 1;
  dmaNodeConfig.Init.DestBurstLength                = 1;
  dmaNodeConfig.Init.Priority                       = DMA_HIGH_PRIORITY;
  dmaNodeConfig.Init.TransferEventMode              = DMA_TCEM_BLOCK_TRANSFER;
  dmaNodeConfig.Init.TransferAllocatedPort          = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
  /* Set node data handling parameters */
  dmaNodeConfig.DataHandlingConfig.DataExchange     = DMA_EXCHANGE_NONE;
  dmaNodeConfig.DataHandlingConfig.DataAlignment    = DMA_DATA_RIGHTALIGN_ZEROPADDED;
  /* Set node trigger parameters */
  dmaNodeConfig.TriggerConfig.TriggerPolarity       = DMA_TRIG_POLARITY_MASKED;

  /* Build NodeRx */
  if (HAL_DMAEx_List_BuildNode(&dmaNodeConfig, &RxNode) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Insert NodeTx to SAI queue */
  if (HAL_DMAEx_List_InsertNode_Tail(&SAIRxQueue, &RxNode) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Set queue circular mode for sai queue */
  if (HAL_DMAEx_List_SetCircularMode(&SAIRxQueue) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* DMA for Rx */
  hDmaSai2Rx.Instance                         = AUDIO_IN_SAI2_DMA_CHANNEL;

  hDmaSai2Rx.InitLinkedList.Priority          = DMA_HIGH_PRIORITY;
  hDmaSai2Rx.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
  hDmaSai2Rx.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
  hDmaSai2Rx.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
  hDmaSai2Rx.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;

  /* DMA linked list init */
  if (HAL_DMAEx_List_Init(&hDmaSai2Rx) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Link SAI queue to DMA channel */
  if (HAL_DMAEx_List_LinkQ(&hDmaSai2Rx, &SAIRxQueue) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Associate the DMA handle */
  __HAL_LINKDMA(hsai, hdmarx, hDmaSai2Rx);

  /* SAI DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_IN_SAI2_DMA_IRQ, BSP_AUDIO_IN_IT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(AUDIO_IN_SAI2_DMA_IRQ);
}

/**
  * @brief  Initialize BSP_AUDIO_OUT MSP.
  * @param  hsai  SAI handle
  * @retval None
  */
static void SAI_OUT_MspInit(SAI_HandleTypeDef *hsai)
{
  GPIO_InitTypeDef  gpio_init_structure;
  static DMA_NodeTypeDef TxNode;
  static DMA_NodeConfTypeDef dmaNodeConfig;

  /* Enable SAI clock */
  __HAL_RCC_SAI2_CLK_ENABLE();

  /* Enable GPIO clock */
  SAI_GPIO_CLK_ENABLE;

  /* CODEC_SAI SD only ------------------*/

  gpio_init_structure.Pin = SAI_SD_OUT_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_MEDIUM;
  gpio_init_structure.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(SAI_SD_OUT_PORT, &gpio_init_structure);

  /* Enable the DMA clock */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  if (SAITxQueue.Head != NULL)
  {
    /* Reset the DMA Channel configuration*/
    if (HAL_DMAEx_List_DeInit(&hDmaSai2Tx) != HAL_OK)
    {
     BSP_Error_Handler();
    }

    /* Reset RxQueue */
    if (HAL_DMAEx_List_ResetQ(&SAITxQueue) != HAL_OK)
    {
      BSP_Error_Handler();
    }
  }

  /* DMA for Rx */
  /* Set node type */
  dmaNodeConfig.NodeType                            = DMA_GPDMA_LINEAR_NODE;
  /* Set common node parameters */
  dmaNodeConfig.Init.Request                        = GPDMA1_REQUEST_SAI2_A;
  dmaNodeConfig.Init.BlkHWRequest                   = DMA_BREQ_SINGLE_BURST;
  dmaNodeConfig.Init.Direction                      = DMA_MEMORY_TO_PERIPH;
  dmaNodeConfig.Init.SrcInc                         = DMA_SINC_INCREMENTED;
  dmaNodeConfig.Init.DestInc                        = DMA_DINC_FIXED;
  if (Audio_SaiIn_Ctx.BitsPerSample == AUDIO_RESOLUTION_16B)
  {
    dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_HALFWORD;
    dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_HALFWORD;
  }
  else /* AUDIO_RESOLUTION_24b */
  {
    dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_WORD;
    dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_WORD;
  }
  dmaNodeConfig.Init.SrcBurstLength                 = 1;
  dmaNodeConfig.Init.DestBurstLength                = 1;
  dmaNodeConfig.Init.Priority                       = DMA_HIGH_PRIORITY;
  dmaNodeConfig.Init.TransferEventMode              = DMA_TCEM_BLOCK_TRANSFER;
  dmaNodeConfig.Init.TransferAllocatedPort          = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
  /* Set node data handling parameters */
  dmaNodeConfig.DataHandlingConfig.DataExchange     = DMA_EXCHANGE_NONE;
  dmaNodeConfig.DataHandlingConfig.DataAlignment    = DMA_DATA_RIGHTALIGN_ZEROPADDED;
  /* Set node trigger parameters */
  dmaNodeConfig.TriggerConfig.TriggerPolarity       = DMA_TRIG_POLARITY_MASKED;

  /* Build NodeRx */
  if (HAL_DMAEx_List_BuildNode(&dmaNodeConfig, &TxNode) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Insert NodeTx to SAI queue */
  if (HAL_DMAEx_List_InsertNode_Tail(&SAITxQueue, &TxNode) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Set queue circular mode for sai queue */
  if (HAL_DMAEx_List_SetCircularMode(&SAITxQueue) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* DMA for Rx */
  hDmaSai2Tx.Instance                         = AUDIO_OUT_SAI2_DMA_CHANNEL;

  hDmaSai2Tx.InitLinkedList.Priority          = DMA_HIGH_PRIORITY;
  hDmaSai2Tx.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
  hDmaSai2Tx.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
  hDmaSai2Tx.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
  hDmaSai2Tx.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;

  /* DMA linked list init */
  if (HAL_DMAEx_List_Init(&hDmaSai2Tx) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Link SAI queue to DMA channel */
  if (HAL_DMAEx_List_LinkQ(&hDmaSai2Tx, &SAITxQueue) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Associate the DMA handle */
  __HAL_LINKDMA(hsai, hdmatx, hDmaSai2Tx);

  /* SAI DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_OUT_SAI2_DMA_IRQ, BSP_AUDIO_OUT_IT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(AUDIO_OUT_SAI2_DMA_IRQ);
}


HAL_StatusTypeDef MX_I2S1_ClockConfig(uint32_t SampleRate, uint32_t mode)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI1;

  if (mode == 1)
  {
    PeriphClkInitStruct.Spi1ClockSelection = RCC_SPI1CLKSOURCE_PIN;
  }else{
    PeriphClkInitStruct.Spi1ClockSelection = RCC_SPI1CLKSOURCE_PLL1Q;
  }
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    return HAL_ERROR;
  }

  __HAL_RCC_SPI1_CLK_ENABLE();

  return HAL_OK;
}


HAL_StatusTypeDef MX_SAI2_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
  PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PIN;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    return HAL_ERROR;
  }

  __HAL_RCC_SAI2_CLK_ENABLE();

  return HAL_OK;
}

HAL_StatusTypeDef MX_SAI1_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PIN;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    return HAL_ERROR;
  }

  __HAL_RCC_SAI1_CLK_ENABLE();

  return HAL_OK;
}

/**
  * @brief  Initialize BSP_AUDIO_OUT MSP.
  * @param  hsai  SAI handle
  * @retval None
  */
static void SAIPDM_MspInit(SAI_HandleTypeDef *hsai)
{
  GPIO_InitTypeDef  gpio_init_structure;
  static DMA_NodeTypeDef RxNode;
  static DMA_NodeConfTypeDef dmaNodeConfig;

  /* Enable SAI clock */
  __HAL_RCC_SAI1_CLK_ENABLE();

  /* Enable GPIO clock */
  SAIPDM_GPIO_CLK_ENABLE;

  /*  pins configuration CK and DATA ------------------*/
  gpio_init_structure.Pin = SAIPDM_SD_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF2_SAI1;
  HAL_GPIO_Init(SAIPDM_SD_PORT, &gpio_init_structure);

  gpio_init_structure.Pin = SAIPDM_CK_PIN;
  gpio_init_structure.Alternate = GPIO_AF2_SAI1;
  HAL_GPIO_Init(SAIPDM_CK_PORT, &gpio_init_structure);

  /* Enable the DMA clock */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

    if (SAIPDMRxQueue.Head == NULL)
    {
      /* DMA for Rx */
      /* Set node type */
      dmaNodeConfig.NodeType                            = DMA_GPDMA_LINEAR_NODE;
      /* Set common node parameters */
      dmaNodeConfig.Init.Request                        = GPDMA1_REQUEST_SAI1_A;
      dmaNodeConfig.Init.BlkHWRequest                   = DMA_BREQ_SINGLE_BURST;
      dmaNodeConfig.Init.Direction                      = DMA_PERIPH_TO_MEMORY;
      dmaNodeConfig.Init.SrcInc                         = DMA_SINC_FIXED;
      dmaNodeConfig.Init.DestInc                        = DMA_DINC_INCREMENTED;
      dmaNodeConfig.Init.SrcDataWidth                   = DMA_SRC_DATAWIDTH_BYTE;
      dmaNodeConfig.Init.DestDataWidth                  = DMA_DEST_DATAWIDTH_BYTE;
      dmaNodeConfig.Init.SrcBurstLength                 = 1;
      dmaNodeConfig.Init.DestBurstLength                = 1;
      dmaNodeConfig.Init.Priority                       = DMA_HIGH_PRIORITY;
      dmaNodeConfig.Init.TransferEventMode              = DMA_TCEM_BLOCK_TRANSFER;
      dmaNodeConfig.Init.TransferAllocatedPort          = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
      dmaNodeConfig.Init.Mode                           = DMA_NORMAL;
      /* Set node data handling parameters */
      dmaNodeConfig.DataHandlingConfig.DataExchange     = DMA_EXCHANGE_NONE;
      dmaNodeConfig.DataHandlingConfig.DataAlignment    = DMA_DATA_RIGHTALIGN_ZEROPADDED;
      /* Set node trigger parameters */
      dmaNodeConfig.TriggerConfig.TriggerPolarity       = DMA_TRIG_POLARITY_MASKED;

      /* Build NodeRx */
      if (HAL_DMAEx_List_BuildNode(&dmaNodeConfig, &RxNode) != HAL_OK)
      {
        BSP_Error_Handler();
      }

      /* Insert NodeTx to SAI queue */
      if (HAL_DMAEx_List_InsertNode_Tail(&SAIPDMRxQueue, &RxNode) != HAL_OK)
      {
        BSP_Error_Handler();
      }

      /* Set queue circular mode for sai queue */
      if (HAL_DMAEx_List_SetCircularMode(&SAIPDMRxQueue) != HAL_OK)
      {
        BSP_Error_Handler();
      }
    }

    /* DMA for Rx */
    hDmaSai1Rx.Instance                         = AUDIO_IN_SAI1_DMA_CHANNEL;

    hDmaSai1Rx.InitLinkedList.Priority          = DMA_HIGH_PRIORITY;
    hDmaSai1Rx.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    hDmaSai1Rx.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
    hDmaSai1Rx.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
    hDmaSai1Rx.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;

    /* DMA linked list init */
    if (HAL_DMAEx_List_Init(&hDmaSai1Rx) != HAL_OK)
    {
      BSP_Error_Handler();
    }

    /* Link SAI queue to DMA channel */
    if (HAL_DMAEx_List_LinkQ(&hDmaSai1Rx, &SAIPDMRxQueue) != HAL_OK)
    {
      BSP_Error_Handler();
    }

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmarx, hDmaSai1Rx);

    /* SAI DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_IN_SAI1_DMA_IRQ, BSP_AUDIO_IN_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(AUDIO_IN_SAI1_DMA_IRQ);
}

/**
  * @brief  Initialize BSP_AUDIO_OUT MSP.
  * @param  hsai  SAI handle
  * @retval None
  */
static void I2S_MspInit(I2S_HandleTypeDef *hi2s)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  static DMA_NodeTypeDef TxNode;
  static DMA_NodeConfTypeDef dmaNodeConfig;

  /* Peripheral clock enable */
  __HAL_RCC_SPI1_CLK_ENABLE();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  /**I2S1 GPIO Configuration
  PA4     ------> I2S1_WS
  PA5     ------> I2S1_CK
  PA6     ------> I2S1_SDI
  PA7     ------> I2S1_SDO
  */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Enable the DMA clock */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  if (I2STxQueue.Head != NULL)
  {
    /* Reset the DMA Channel configuration*/
    if (HAL_DMAEx_List_DeInit(&hDmaI2sTx) != HAL_OK)
    {
     BSP_Error_Handler();
    }

    /* Reset RxQueue */
    if (HAL_DMAEx_List_ResetQ(&I2STxQueue) != HAL_OK)
    {
      BSP_Error_Handler();
    }
  }

  /* DMA for Rx */
  /* Set node type */
  dmaNodeConfig.NodeType                            = DMA_GPDMA_LINEAR_NODE;
  /* Set common node parameters */
  dmaNodeConfig.Init.Request                        = GPDMA1_REQUEST_SPI1_TX;
  dmaNodeConfig.Init.BlkHWRequest                   = DMA_BREQ_SINGLE_BURST;
  dmaNodeConfig.Init.Direction                      = DMA_MEMORY_TO_PERIPH;
  dmaNodeConfig.Init.SrcInc                         = DMA_SINC_INCREMENTED;
  dmaNodeConfig.Init.DestInc                        = DMA_DINC_FIXED;
  if (Audio_I2sOut_Ctx.BitsPerSample == AUDIO_RESOLUTION_16B)
  {
    dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_HALFWORD;
    dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_HALFWORD;
  }
  else /* AUDIO_RESOLUTION_24b */
  {
    dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_WORD;
    dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_WORD;
  }
  dmaNodeConfig.Init.SrcBurstLength                 = 1;
  dmaNodeConfig.Init.DestBurstLength                = 1;
  dmaNodeConfig.Init.Priority                       = DMA_HIGH_PRIORITY;
  dmaNodeConfig.Init.TransferEventMode              = DMA_TCEM_BLOCK_TRANSFER;
  dmaNodeConfig.Init.TransferAllocatedPort          = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
  /* Set node data handling parameters */
  dmaNodeConfig.DataHandlingConfig.DataExchange     = DMA_EXCHANGE_NONE;
  dmaNodeConfig.DataHandlingConfig.DataAlignment    = DMA_DATA_RIGHTALIGN_ZEROPADDED;
  /* Set node trigger parameters */
  dmaNodeConfig.TriggerConfig.TriggerPolarity       = DMA_TRIG_POLARITY_MASKED;

  /* Build NodeRx */
  if (HAL_DMAEx_List_BuildNode(&dmaNodeConfig, &TxNode) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Insert NodeTx to SAI queue */
  if (HAL_DMAEx_List_InsertNode_Tail(&I2STxQueue, &TxNode) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Set queue circular mode for sai queue */
  if (HAL_DMAEx_List_SetCircularMode(&I2STxQueue) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* DMA for Rx */
  hDmaI2sTx.Instance                         = AUDIO_OUT_SPI1_DMA_CHANNEL;

  hDmaI2sTx.InitLinkedList.Priority          = DMA_HIGH_PRIORITY;
  hDmaI2sTx.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
  hDmaI2sTx.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
  hDmaI2sTx.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
  hDmaI2sTx.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;

  /* DMA linked list init */
  if (HAL_DMAEx_List_Init(&hDmaI2sTx) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Link SAI queue to DMA channel */
  if (HAL_DMAEx_List_LinkQ(&hDmaI2sTx, &I2STxQueue) != HAL_OK)
  {
    BSP_Error_Handler();
  }

  /* Associate the DMA handle */
  __HAL_LINKDMA(hi2s, hdmatx, hDmaI2sTx);

  /* SAI DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_OUT_SPI1_DMA_IRQ, BSP_AUDIO_OUT_IT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(AUDIO_OUT_SPI1_DMA_IRQ);
}


void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI2_Block_B)
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(ID_IN_SAI2_B);
  }
  else if(hsai->Instance == SAI1_Block_A)
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(ID_IN_SAI1_A);
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI2_Block_B)
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(ID_IN_SAI2_B);
  }
  else if(hsai->Instance == SAI1_Block_A)
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(ID_IN_SAI1_A);
  }
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI2_Block_A)
  {
    BSP_AUDIO_OUT_HalfTransfer_CallBack(ID_OUT_SAI2_A);
  }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI2_Block_A)
  {
    BSP_AUDIO_OUT_TransferComplete_CallBack(ID_OUT_SAI2_A);
  }
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    APP_AUDIO_OUT_Tranfert_Callback(1);

    BSP_AUDIO_OUT_HalfTransfer_CallBack(ID_OUT_SPI);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    APP_AUDIO_OUT_Tranfert_Callback(0);

    BSP_AUDIO_OUT_TransferComplete_CallBack(ID_OUT_SPI);
}

void BSP_Error_Handler(void)
{
  while(1);
}