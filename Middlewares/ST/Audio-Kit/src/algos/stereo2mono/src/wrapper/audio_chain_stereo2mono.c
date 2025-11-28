/**
******************************************************************************
* @file    audio_chain_stereo2mono.c
* @author  MCD Application Team
* @brief   wrapper of gain algo to match usage inside audio_chain.c
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
#include <math.h>
#include "stereo2mono/audio_chain_stereo2mono.h"
#include "sfc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  int            nbElements;
  audio_chunk_t *pChunkIn;
  audio_chunk_t *pChunkOut;
  sfcContext_t   sfcContextLeft;
  sfcContext_t   sfcContextRight;
} stereo2monoCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_stereo2mono_deinit(audio_algo_t               *const pAlgo);
static int32_t s_stereo2mono_init(audio_algo_t                 *const pAlgo);
static int32_t s_stereo2mono_configure(audio_algo_t            *const pAlgo);
static int32_t s_stereo2mono_dataInOut(audio_algo_t            *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_stereo2mono_common =
{
  .pName                     = "stereo2mono",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_FS),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_2CH,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("stereo\0"),
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("input stereo signal\0"),

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_1CH,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,
  .iosOut.pPinNames          = AUDIO_ALGO_OPT_STR("mono\0"),
  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("output mono signal\0"),

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("downmix stereo to mono"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("stereo2mono")
};

audio_algo_cbs_t AudioChainWrp_stereo2mono_cbs =
{
  .init                       = s_stereo2mono_init,
  .deinit                     = s_stereo2mono_deinit,
  .configure                  = s_stereo2mono_configure,
  .dataInOut                  = s_stereo2mono_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_stereo2mono_init(audio_algo_t *const pAlgo)
{
  int32_t                             error          = AUDIO_ERR_MGNT_NONE;
  stereo2monoCtx_t                   *pContext       = (stereo2monoCtx_t *)AudioAlgo_malloc(sizeof(stereo2monoCtx_t), AUDIO_MEM_RAMINT);
  stereo2mono_dynamic_config_t *const pDynamicConfig = (stereo2mono_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);

  if (pContext == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }

  if (AudioError_isOk(error))
  {
    if (pDynamicConfig == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing dynamic conf !");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }

  if (AudioError_isOk(error))
  {
    audio_chain_utilities_t *const pUtilsHdle = AudioAlgo_getUtilsHdle(pAlgo);
    audio_chunk_t           *const pChunkIn   = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
    audio_chunk_t           *const pChunkOut  = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
    audio_buffer_t          *const pBuffIn    = AudioChunk_getBuffInfo(pChunkIn);
    audio_buffer_t          *const pBuffOut   = AudioChunk_getBuffInfo(pChunkOut);

    AudioAlgo_setWrapperContext(pAlgo, pContext);

    pContext->pChunkIn   = pChunkIn;
    pContext->pChunkOut  = pChunkOut;
    pContext->nbElements = (int)AudioBuffer_getNbElements(pBuffIn);

    sfcResetContext(&pContext->sfcContextLeft);
    error = sfcSetContext(&pContext->sfcContextLeft,
                          pBuffIn,
                          pBuffOut,
                          false,
                          0.5f,
                          pUtilsHdle);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue for left channel !");
    }
    if (AudioError_isOk(error))
    {
      sfcResetContext(&pContext->sfcContextRight);
      error = sfcSetContext(&pContext->sfcContextRight,
                            pBuffIn,
                            pBuffOut,
                            true,
                            0.5f,
                            pUtilsHdle);
      if (AudioError_isError(error))
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue for right channel !");
      }
    }
  }
  if (AudioError_isOk(error))
  {
    error = s_stereo2mono_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_stereo2mono_deinit(pAlgo);
  }

  return error;
}


static int32_t s_stereo2mono_deinit(audio_algo_t *const pAlgo)
{
  stereo2monoCtx_t *const pContext = (stereo2monoCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, AUDIO_MEM_RAMINT);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_stereo2mono_configure(audio_algo_t *const pAlgo)
{
  int32_t                             error          = AUDIO_ERR_MGNT_NONE;
  stereo2monoCtx_t             *const pContext       = (stereo2monoCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  stereo2mono_dynamic_config_t *const pDynamicConfig = (stereo2mono_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  float                         const gainLeft       = 0.5f * (1.0f - pDynamicConfig->balance);
  float                         const gainRight      = 0.5f * (1.0f + pDynamicConfig->balance);

  error = sfcUpdateContext(&pContext->sfcContextLeft, false, gainLeft);
  if (AudioError_isError(error))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc config issue for left channel !");
  }
  if (AudioError_isOk(error))
  {
    error = sfcUpdateContext(&pContext->sfcContextRight, true, gainRight);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc config issue for right channel !");
    }
  }

  return error;
}


static int32_t s_stereo2mono_dataInOut(audio_algo_t *const pAlgo)
{
  stereo2monoCtx_t *const pContext        = (stereo2monoCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  void             *const pSamplesInLeft  = AudioChunk_getReadPtr(pContext->pChunkIn, 0U, 0UL);
  void             *const pSamplesInRight = AudioChunk_getReadPtr(pContext->pChunkIn, 1U, 0UL);
  void             *const pSamplesOut     = AudioChunk_getWritePtr0(pContext->pChunkOut);

  sfcSampleBufferConvert(&pContext->sfcContextLeft,  pSamplesInLeft,  pSamplesOut, 1, pContext->nbElements);
  sfcSampleBufferConvert(&pContext->sfcContextRight, pSamplesInRight, pSamplesOut, 1, pContext->nbElements);

  return AUDIO_ERR_MGNT_NONE;
}



