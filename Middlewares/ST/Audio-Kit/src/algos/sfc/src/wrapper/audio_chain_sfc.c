/**
******************************************************************************
* @file    audio_chain_sfc.c
* @author  MCD Application Team
* @brief   wrapper of SFC (Sample Format Converter) algo to match usage inside audio_chain.c
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
#include "sfc/audio_chain_sfc.h"
#include "sfc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  audio_chunk_t *pChunkIn;
  audio_chunk_t *pChunkOut;
  int            nbChannels;
  int            nbElements;
  sfcContext_t   sfcContext;
}
SFC_Handler_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_sfc_deinit(audio_algo_t    *const pAlgo);
static int32_t s_sfc_init(audio_algo_t      *const pAlgo);
static int32_t s_sfc_configure(audio_algo_t *const pAlgo);
static int32_t s_sfc_dataInOut(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_sfc_common =
{
  .pName                     = "sfc",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_FS),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_ALL,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_ALL,
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("sfc"),

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Sample format conversion (sample type & interleaving)")
};

audio_algo_cbs_t AudioChainWrp_sfc_cbs =
{
  .init                       = s_sfc_init,
  .deinit                     = s_sfc_deinit,
  .configure                  = s_sfc_configure,
  .dataInOut                  = s_sfc_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_sfc_init(audio_algo_t *const pAlgo)
{
  int32_t              error       = AUDIO_ERR_MGNT_NONE;
  SFC_Handler_t *const pSfcHandler = (SFC_Handler_t *)AudioAlgo_malloc(sizeof(SFC_Handler_t), AUDIO_MEM_RAMINT);

  AudioAlgo_setWrapperContext(pAlgo, pSfcHandler);

  if (pSfcHandler == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    audio_buffer_t *pBuffIn, *pBuffOut;

    pSfcHandler->pChunkIn   = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
    pSfcHandler->pChunkOut  = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
    pBuffIn                 = AudioChunk_getBuffInfo(pSfcHandler->pChunkIn);
    pBuffOut                = AudioChunk_getBuffInfo(pSfcHandler->pChunkOut);
    pSfcHandler->nbChannels = (int)AudioBuffer_getNbChannels(pBuffIn);
    pSfcHandler->nbElements = (int)AudioBuffer_getNbElements(pBuffIn);

    sfcResetContext(&pSfcHandler->sfcContext);
    error = sfcSetContext(&pSfcHandler->sfcContext,
                          pBuffIn,
                          pBuffOut,
                          false,
                          1.0f,
                          AudioAlgo_getUtilsHdle(pAlgo));
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext issue !");
    }
  }

  if (AudioError_isOk(error))
  {
    error = s_sfc_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_sfc_deinit(pAlgo);
  }

  return error;
}


static int32_t s_sfc_deinit(audio_algo_t *const pAlgo)
{
  SFC_Handler_t *const pSfcHandler = (SFC_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pSfcHandler != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pSfcHandler, AUDIO_MEM_RAMINT);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_sfc_configure(audio_algo_t *const pAlgo)
{
  int32_t                     error          = AUDIO_ERR_MGNT_NONE;
  SFC_Handler_t        *const pSfcHandler    = (SFC_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);
  sfc_dynamic_config_t *const pDynamicConfig = (sfc_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);

  if (pSfcHandler == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "context alloc failed!");
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "dynamic config is missing!");
    error = AUDIO_ERR_MGNT_CONFIG;
  }
  if (AudioError_isOk(error))
  {
    error = sfcUpdateContext(&pSfcHandler->sfcContext, false, powf(10.0f, pDynamicConfig->gain / 20.0f)); /*cstat !MISRAC2012-Rule-22.8 no issue with powf(10, ...) => errno check is useless*/
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcUpdateContext issue!");
    }
  }

  return error;
}


static int32_t s_sfc_dataInOut(audio_algo_t *const pAlgo)
{
  SFC_Handler_t *const pSfcHandler = (SFC_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);
  void          *const pSampleIn   = AudioChunk_getReadPtr0(pSfcHandler->pChunkIn);
  void          *const pSampleOut  = AudioChunk_getWritePtr0(pSfcHandler->pChunkOut);

  sfcSampleBufferConvert(&pSfcHandler->sfcContext, pSampleIn, pSampleOut, pSfcHandler->nbChannels, pSfcHandler->nbElements);

  return AUDIO_ERR_MGNT_NONE;
}
