/**
******************************************************************************
* @file    audio_chain_mono2stereo.c
* @author  MCD Application Team
* @brief   wrapper of mono2stereo algo to match usage inside audio_chain.c
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
#include "mono2stereo/audio_chain_mono2stereo.h"
#include "sfc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  int            nbElements;
  audio_chunk_t *pChunkIn;
  audio_chunk_t *pChunkOut;
  sfcContext_t   sfcContextLeft;
  sfcContext_t   sfcContextRight;
} mono2stereoCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_mono2stereo_deinit(audio_algo_t               *const pAlgo);
static int32_t s_mono2stereo_init(audio_algo_t                 *const pAlgo);
static int32_t s_mono2stereo_configure(audio_algo_t            *const pAlgo);
static int32_t s_mono2stereo_dataInOut(audio_algo_t            *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_mono2stereo_common =
{
  .pName                     = "mono2stereo",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_FS),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_1CH,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("mono\0"),
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("input mono signal\0"),

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_2CH,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,
  .iosOut.pPinNames          = AUDIO_ALGO_OPT_STR("stereo\0"),
  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("output stereo signal\0"),

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("upmix mono to stereo"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("mono2stereo")
};

audio_algo_cbs_t AudioChainWrp_mono2stereo_cbs =
{
  .init                       = s_mono2stereo_init,
  .deinit                     = s_mono2stereo_deinit,
  .configure                  = s_mono2stereo_configure,
  .dataInOut                  = s_mono2stereo_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_mono2stereo_init(audio_algo_t *const pAlgo)
{
  int32_t           error    = AUDIO_ERR_MGNT_NONE;
  mono2stereoCtx_t *pContext = (mono2stereoCtx_t *)AudioAlgo_malloc(sizeof(mono2stereoCtx_t), AUDIO_MEM_RAMINT);

  if (pContext == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  if (AudioError_isOk(error))
  {
    mono2stereo_dynamic_config_t *const pDynamicConfig = (mono2stereo_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
    if (pDynamicConfig == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "dynamic config is missing!");
      error = AUDIO_ERR_MGNT_CONFIG;
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
                          1.0f,
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
                            false,
                            1.0f,
                            pUtilsHdle);
      if (AudioError_isError(error))
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue for right channel !");
      }
    }
  }
  if (AudioError_isOk(error))
  {
    error = s_mono2stereo_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_mono2stereo_deinit(pAlgo);
  }

  return error;
}


static int32_t s_mono2stereo_deinit(audio_algo_t *const pAlgo)
{
  mono2stereoCtx_t *const pContext = (mono2stereoCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, AUDIO_MEM_RAMINT);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_mono2stereo_configure(audio_algo_t *const pAlgo)
{
  int32_t                             error          = AUDIO_ERR_MGNT_NONE;
  mono2stereoCtx_t             *const pContext       = (mono2stereoCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  mono2stereo_dynamic_config_t *const pDynamicConfig = (mono2stereo_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  float                         const gainLeft       = (pDynamicConfig->balance < 0.0f) ? 1.0f : (1.0f - pDynamicConfig->balance);
  float                         const gainRight      = (pDynamicConfig->balance > 0.0f) ? 1.0f : (1.0f + pDynamicConfig->balance);

  error = sfcUpdateContext(&pContext->sfcContextLeft, false, gainLeft);
  if (AudioError_isError(error))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc config issue for left channel !");
  }
  if (AudioError_isOk(error))
  {
    error = sfcUpdateContext(&pContext->sfcContextRight, false, gainRight);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc config issue for left channel !");
    }
  }

  return error;
}


static int32_t s_mono2stereo_dataInOut(audio_algo_t *const pAlgo)
{
  mono2stereoCtx_t *const pContext         = (mono2stereoCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  void             *const pSamplesIn       = AudioChunk_getReadPtr0(pContext->pChunkIn);
  void             *const pSamplesOutLeft  = AudioChunk_getWritePtr(pContext->pChunkOut, 0U, 0UL);
  void             *const pSamplesOutRight = AudioChunk_getWritePtr(pContext->pChunkOut, 1U, 0UL);

  sfcSampleBufferConvert(&pContext->sfcContextLeft,  pSamplesIn, pSamplesOutLeft,  1, pContext->nbElements);
  sfcSampleBufferConvert(&pContext->sfcContextRight, pSamplesIn, pSamplesOutRight, 1, pContext->nbElements);

  return AUDIO_ERR_MGNT_NONE;
}


