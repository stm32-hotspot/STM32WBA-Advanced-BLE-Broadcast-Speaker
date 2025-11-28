/**
******************************************************************************
* @file    stm32_audio_setup_*.c
* @author  MCD Application Team
* @brief   Implement stm32_audio.c specificities for the current board
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
#include <stdbool.h>
#include "stm32_audio.h"
#include "stm32_audio_pdm2pcm.h"
#include "stm32_audio_sai_pdm.h"
#include "stm32_audio_tasks.h"
#include "traces.h"
#include "BoardSetup.h"   // for AudioRegisterCbs() prototype
#include "stm32xxx_voice_bsp.h"
#include "BLE_Speaker_H5_bsp_audio.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/



static void s_error(void);

//static int32_t s_in_pdm2pcm(uint32_t Instance, uint8_t *PDMBuf, uint16_t *PCMBuf);
static int32_t s_in_init(uint32_t path_id, UTIL_AUDIO_params_t *AudioInit);
//static int32_t s_in_start(uint32_t Instance, uint8_t *pBuf, uint32_t NbrOfBytes);
//static int32_t s_in_stop(uint32_t Instance);
static int32_t s_out_init(uint32_t Instance, UTIL_AUDIO_params_t *AudioInit);


/* Functions Definition ------------------------------------------------------*/
void UTIL_AUDIO_setCallbacks(Path_id_t path_id)
{
  UTIL_AUDIO_cbs_t util_audio_cbs;
  UTIL_AUDIO_resetCallbacks(&util_audio_cbs);

  if (path_id == MAIN_PATH)
  {
    util_audio_cbs.initIn          = s_in_init;
    util_audio_cbs.preprocIn       = NULL;
    util_audio_cbs.recordIn        = BSP_SAI_IN_Record;
    util_audio_cbs.stopIn          = NULL;
    util_audio_cbs.initOut         = s_out_init;
    util_audio_cbs.getStateIn      = NULL;
    util_audio_cbs.playOut         = BSP_SPI_OUT_Play;
    util_audio_cbs.stopOut         = BSP_SPI_OUT_Stop;
    util_audio_cbs.setVolumeOut    = NULL;
    util_audio_cbs.error           = s_error;
    UTIL_AUDIO_registerCallbacks(&util_audio_cbs, MAIN_PATH);
    UTIL_AUDIO_CAPTURE_setDevice(MAIN_PATH);
  }
  else /* ALTERNATE_PATH */
  {
    util_audio_cbs.initIn          = s_in_init;
    util_audio_cbs.preprocIn       = NULL; //s_in_pdm2pcm; if needed, to the conversion inside the graph;
    util_audio_cbs.recordIn        = BSP_AUDIO_IN_RecordPDM;
    util_audio_cbs.stopIn          = NULL;
    util_audio_cbs.initOut         = NULL; //BSP_SAI_OUT_Init;
    util_audio_cbs.getStateIn      = NULL;
    util_audio_cbs.playOut         = NULL; //BSP_SAI_OUT_Play
    util_audio_cbs.stopOut         = NULL; //BSP_AUDIO_OUT_Stop;
    util_audio_cbs.setVolumeOut    = NULL;
    util_audio_cbs.error           = s_error;
    UTIL_AUDIO_registerCallbacks(&util_audio_cbs, ALTERNATE_PATH);
    UTIL_AUDIO_CAPTURE_setDevice(ALTERNATE_PATH);
  }
}



/*int32_t UTIL_AUDIO_SAI_PDM_ClockConfig(uint32_t SampleRate)
{
  MX_SAI1_ClockConfig(NULL, SampleRate);
  return UTIL_AUDIO_ERROR_NONE;
}*/

static int32_t s_in_init(uint32_t path_id, UTIL_AUDIO_params_t *AudioInit)
{
  if (path_id == MAIN_PATH)
  {
    return BSP_SAI_IN_Init(0, (BSP_AUDIO_Init_t *)AudioInit);
  }
  else
  {
    return BSP_AUDIO_IN_InitPDM(0, (BSP_AUDIO_Init_t *)AudioInit);
  }
}


/*int32_t s_in_pdm2pcm(uint32_t Instance, uint8_t *PDMBuf, uint16_t *PCMBuf)
{
  #ifdef UTIL_AUDIO_IN_PDM2PCM_USED
  return UTIL_AUDIO_PDM2PCM_Decimate(PDMBuf, PCMBuf);
  #else
  return UTIL_AUDIO_ERROR_NONE;
  #endif
}*/


static void s_error(void)
{
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_ERROR, "UTIL_AUDIO_Error: can not be started!\n");
  while (1);
}



/**
  * @brief  BSP Audio out Init
  * @retval None
  */
int32_t s_out_init(uint32_t path_id, UTIL_AUDIO_params_t *AudioInit)
{
  if (path_id == MAIN_PATH)
  {
    return BSP_SPI_OUT_Init(0, (BSP_AUDIO_Init_t *) AudioInit);
  }
  else
  {
    return -1;//BSP_SAI_OUT_Init(0, (BSP_AUDIO_Init_t *) AudioInit);
  }
}



void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  if (Instance == ID_IN_SAI2_B)
  {
    UTIL_AUDIO_CAPTURE_TxComplete_cb(0UL, MAIN_PATH);
  }
  else
  {
    UTIL_AUDIO_CAPTURE_TxComplete_cb(0UL, ALTERNATE_PATH); /* same for both instances */
  }
}

void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  if (Instance == ID_IN_SAI2_B)
  {
    UTIL_AUDIO_CAPTURE_TxComplete_cb(UTIL_AUDIO_CAPTURE_getHalfBuffOffsetBytes(MAIN_PATH), MAIN_PATH);
  }
  else
  {
    UTIL_AUDIO_CAPTURE_TxComplete_cb(UTIL_AUDIO_CAPTURE_getHalfBuffOffsetBytes(ALTERNATE_PATH), ALTERNATE_PATH);
  }
}

/**
  * @brief  Manages the DMA full Transfer complete event.
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(uint32_t Instance)
{
  if (Instance == ID_OUT_SPI)
  {
    UTIL_AUDIO_RENDER_TxComplete_cb(UTIL_AUDIO_RENDER_getHalfAddr(MAIN_PATH), MAIN_PATH);
  }
  else /* ID_OUT_SAI1_A */
  {
    UTIL_AUDIO_RENDER_TxComplete_cb(UTIL_AUDIO_RENDER_getHalfAddr(ALTERNATE_PATH), ALTERNATE_PATH);
  }
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(uint32_t Instance)
{
  if (Instance == ID_OUT_SPI)
  {
    UTIL_AUDIO_RENDER_TxComplete_cb(UTIL_AUDIO_RENDER_getBaseAddr(MAIN_PATH), MAIN_PATH);
  }
  else /* ID_OUT_SAI1_A */
  {
    UTIL_AUDIO_RENDER_TxComplete_cb(UTIL_AUDIO_RENDER_getBaseAddr(ALTERNATE_PATH), ALTERNATE_PATH);
  }
}



/* BSP Audio IRQ handlers */
void AUDIO_OUT_SAI2_DMA_IRQHandler(void);
void AUDIO_OUT_SAI2_DMA_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(haudio_out_sai2.hdmatx);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}

void AUDIO_OUT_SPI1_DMA_IRQHandler(void);
void AUDIO_OUT_SPI1_DMA_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(haudio_out_i2s.hdmatx);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}

void AUDIO_IN_SAI2_DMA_IRQHandler(void);
void AUDIO_IN_SAI2_DMA_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(haudio_in_sai2.hdmarx);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}

void AUDIO_IN_SAI1_DMA_IRQHandler(void);
void AUDIO_IN_SAI1_DMA_IRQHandler(void)
{
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(haudio_in_sai1.hdmarx);
  UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop();
}