/**
  ******************************************************************************
  * @file   BLE_Speaker_H5_bsp_audio.h
  * @author  MCD Application Team
  * @brief
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

#ifndef BSP_BOARD_AUDIO_H
#define BSP_BOARD_AUDIO_H

#include "stm32h5xx_hal.h"
#include <stdio.h>

#define BSP_AUDIO_OUT_IT_PRIORITY            7U
#define BSP_AUDIO_IN_IT_PRIORITY             7U

/* Common Error codes */
#define BSP_ERROR_NONE                        0
#define BSP_ERROR_NO_INIT                    -1
#define BSP_ERROR_WRONG_PARAM                -2
#define BSP_ERROR_BUSY                       -3
#define BSP_ERROR_PERIPH_FAILURE             -4
#define BSP_ERROR_COMPONENT_FAILURE          -5
#define BSP_ERROR_UNKNOWN_FAILURE            -6
#define BSP_ERROR_UNKNOWN_COMPONENT          -7
#define BSP_ERROR_BUS_FAILURE                -8
#define BSP_ERROR_CLOCK_FAILURE              -9
#define BSP_ERROR_MSP_FAILURE                -10
#define BSP_ERROR_FEATURE_NOT_SUPPORTED      -11


/** @defgroup STM32H573I_DK_AUDIO_Exported_Types AUDIO Exported Types
  * @{
  */
typedef struct
{
  uint32_t Device;        /* Output or input device */
  uint32_t SampleRate;    /* From 8kHz to 192 kHz */
  uint32_t BitsPerSample; /* From 8 bits per sample to 32 bits per sample */
  uint32_t ChannelsNbr;   /* 1 for mono and 2 for stereo */
  uint32_t Volume;        /* In percentage from 0 to 100 */
} BSP_AUDIO_Init_t;

typedef struct
{
  uint32_t                    Instance;            /* Audio OUT instance              */
  uint32_t                    Device;              /* Audio OUT device to be used     */
  uint32_t                    SampleRate;          /* Audio OUT Sample rate           */
  uint32_t                    BitsPerSample;       /* Audio OUT Sample Bit Per Sample */
  uint32_t                    Volume;              /* Audio OUT volume                */
  uint32_t                    ChannelsNbr;         /* Audio OUT number of channel     */
  uint32_t                    IsMute;              /* Mute state                      */
  uint32_t                    State;               /* Audio OUT State                 */
  uint32_t                    IsMspCallbacksValid; /* Is Msp Callbacks registered     */
}AUDIO_OUT_Ctx_t;

/* Audio in context */
typedef struct
{
  uint32_t  Device;              /* Audio IN device to be used     */
  uint32_t  SampleRate;          /* Audio IN Sample rate           */
  uint32_t  BitsPerSample;       /* Audio IN Sample resolution     */
  uint32_t  ChannelsNbr;         /* Audio IN number of channel     */
  uint8_t   *pBuff;              /* Audio IN record buffer         */
  uint32_t  Size;                /* Audio IN record buffer size    */
  uint32_t  Volume;              /* Audio IN volume                */
  uint32_t  State;               /* Audio IN State                 */
  uint32_t  IsMspCallbacksValid; /* Is Msp Callbacks registered    */
} AUDIO_IN_Ctx_t;

typedef struct
{
  uint32_t AudioFrequency;
  uint32_t AudioMode;
  uint32_t DataSize;
  uint32_t MonoStereoMode;
  uint32_t ClockStrobing;
  uint32_t Synchro;
  uint32_t OutputDrive;
  uint32_t SynchroExt;
  uint32_t FrameLength;
  uint32_t ActiveFrameLength;
  uint32_t SlotActive;
} MX_SAI_Config_t;

void AUDIO_IN_SAI2_DMA_IRQHandler(void);
void AUDIO_OUT_SPI1_DMA_IRQHandler(void);
void AUDIO_IN_SAI2_DMA_IRQHandler(void);
void AUDIO_OUT_SAI2_DMA_IRQHandler(void);

#define AUDIO_OUT_SPI1_DMA_CHANNEL      GPDMA1_Channel2
#define AUDIO_OUT_SPI1_DMA_IRQ          GPDMA1_Channel2_IRQn
#define AUDIO_OUT_SPI1_DMA_IRQHandler   GPDMA1_Channel2_IRQHandler

#define AUDIO_IN_SAI2_DMA_CHANNEL       GPDMA1_Channel4
#define AUDIO_IN_SAI2_DMA_IRQ           GPDMA1_Channel4_IRQn
#define AUDIO_IN_SAI2_DMA_IRQHandler    GPDMA1_Channel4_IRQHandler

#define AUDIO_OUT_SAI2_DMA_CHANNEL      GPDMA1_Channel5
#define AUDIO_OUT_SAI2_DMA_IRQ          GPDMA1_Channel5_IRQn
#define AUDIO_OUT_SAI2_DMA_IRQHandler   GPDMA1_Channel5_IRQHandler

#define AUDIO_IN_SAI1_DMA_CHANNEL       GPDMA1_Channel3
#define AUDIO_IN_SAI1_DMA_IRQ           GPDMA1_Channel3_IRQn
#define AUDIO_IN_SAI1_DMA_IRQHandler    GPDMA1_Channel3_IRQHandler

/** @defgroup STM32H573I_DK_AUDIO_Exported_Constants AUDIO Exported Constants
  * @{
  */

/* Audio sample rate */
#define AUDIO_FREQUENCY_192K           192000U
#define AUDIO_FREQUENCY_176K           176400U
#define AUDIO_FREQUENCY_96K             96000U
#define AUDIO_FREQUENCY_88K             88200U
#define AUDIO_FREQUENCY_48K             48000U
#define AUDIO_FREQUENCY_44K             44100U
#define AUDIO_FREQUENCY_32K             32000U
#define AUDIO_FREQUENCY_22K             22050U
#define AUDIO_FREQUENCY_16K             16000U
#define AUDIO_FREQUENCY_11K             11025U
#define AUDIO_FREQUENCY_8K               8000U

/* Audio bits per sample */
#define AUDIO_RESOLUTION_8B                 8U
#define AUDIO_RESOLUTION_16B               16U
#define AUDIO_RESOLUTION_24B               24U
#define AUDIO_RESOLUTION_32B               32U

/* Audio mute state */
#define AUDIO_MUTE_DISABLED                 0U
#define AUDIO_MUTE_ENABLED                  1U

/* Instance ID */
#define ID_IN_SAI2_B                        0U
#define ID_IN_SAI1_A                        1U /* PDM microphone */
#define ID_OUT_SAI2_A                       1U
#define ID_OUT_SPI                          0U

/*------------------------------------------------------------------------------
                        AUDIO OUT CONFIGURATION
------------------------------------------------------------------------------*/
/* Audio Out states */
#define AUDIO_OUT_STATE_RESET               0U
#define AUDIO_OUT_STATE_PLAYING             1U
#define AUDIO_OUT_STATE_STOP                2U
#define AUDIO_OUT_STATE_PAUSE               3U

/* Audio Out states */
/* Volume Input Output selection */
#define AUDIO_VOLUME_INPUT                  0U
#define AUDIO_VOLUME_OUTPUT                 1U

/*------------------------------------------------------------------------------
                        AUDIO IN CONFIGURATION
------------------------------------------------------------------------------*/

/* Audio in states */
#define AUDIO_IN_STATE_RESET     0U
#define AUDIO_IN_STATE_RECORDING 1U
#define AUDIO_IN_STATE_STOP      2U
#define AUDIO_IN_STATE_PAUSE     3U

extern SAI_HandleTypeDef      haudio_in_sai1;
extern SAI_HandleTypeDef      haudio_in_sai2;
extern I2S_HandleTypeDef      haudio_out_i2s;
extern SAI_HandleTypeDef      haudio_out_sai2;

/** @defgroup AUDIO OUT to SPI
  * @{
  */
int32_t BSP_SPI_OUT_Init(uint32_t Instance, BSP_AUDIO_Init_t *AudioInit);
int32_t BSP_SPI_OUT_Play(uint32_t Instance, uint8_t *pBuf, uint32_t NbrOfsamples);
int32_t BSP_SPI_OUT_Stop(uint32_t Instance);
int32_t BSP_SPI_OUT_Pause(uint32_t Instance);
int32_t BSP_SPI_OUT_Resume(void);

HAL_StatusTypeDef MX_I2S1_ClockConfig(uint32_t SampleRate, uint32_t mode);

void BSP_AUDIO_OUT_TransferComplete_CallBack(uint32_t Instance);
void BSP_AUDIO_OUT_HalfTransfer_CallBack(uint32_t Instance);

/** @defgroup AUDIO OUT to SAI
  * @{
  */
int32_t BSP_SAI_OUT_Init(uint32_t Instance, BSP_AUDIO_Init_t *AudioInit);
int32_t BSP_SAI_OUT_Play(uint32_t Instance, uint8_t *pBuf, uint32_t NbrOfBytes);
int32_t BSP_SAI_OUT_Stop(void);
int32_t BSP_SAI_OUT_Pause(void);
int32_t BSP_SAI_OUT_Resume(void);

/** @defgroup AUDIO IN from SAI
  * @{
  */
int32_t BSP_SAI_IN_Init(uint32_t Instance, BSP_AUDIO_Init_t *AudioInit);
int32_t BSP_SAI_IN_Record(uint32_t Instance, uint8_t *pBuf, uint32_t NbrOfBytes);
int32_t BSP_SAI_IN_Stop(void);
int32_t BSP_SAI_IN_Pause(void);
int32_t BSP_SAI_IN_Resume(void);

HAL_StatusTypeDef MX_SAI2_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate);

void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance);
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance);

/** @defgroup AUDIO IN from SAI PDM
  * @{
  */
int32_t BSP_AUDIO_IN_InitPDM(uint32_t Instance, BSP_AUDIO_Init_t* AudioInit);
int32_t BSP_AUDIO_IN_RecordPDM(uint32_t Instance, uint8_t* pBuf, uint32_t NbrOfBytes);

HAL_StatusTypeDef MX_SAI1_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate);

#endif /* BSP_BOARD_AUDIO_H */