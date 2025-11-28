/**
  ******************************************************************************
  * @file    audio_chain_speex_nr.c
  * @author  MCD Application Team
  * @brief   wrapper of speex_nr algo to match usage inside audio_chain.c
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "common/speexAlloc.h"
#include "speex_nr/audio_chain_speex_nr.h"
#include "speex/speexdsp_types.h"
#include "speex/speex_preprocess.h"
#include "smallft.h"
#include "filterbank.h"


/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  SpeexPreprocessState     *pHdle;
}
speex_nr_user_context_t;

typedef struct
{
  audio_buffer_t            buff;
  audio_chunk_t            *pChunk;
}
speex_nr_buffer_context_t;


typedef struct
{
  size_t                      buffSize;
  speex_nr_buffer_context_t   pin;
  speex_nr_buffer_context_t   pout;
  speex_nr_user_context_t     user;
  memPool_t                   memPool;
}
speex_nr_context_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/*
 * This code is generated to provide several ways of integrating an algorithm. Therefore, there will be some useless
 * variables that can be removed when wrapper is completed.
 * Input & output data are provided using audio_chunk_t which is used by AudioChain to maintain all data pointers
 * automatically. As a matter of fact it is a slightly complex structure one may not want to use for simple algorithm.
 * AudioChain also uses a simpler structure called audio_buffer_t which hosts fields for data description plus a data
 * pointer.
 *
 * In this generated code, we instantiate a private context structure with several fields that may come in handy to
 * process audio.
 * Chunks are saved at init to avoid getting them every calls of dataInOut or Process callbacks.
 * The context also includes audio_buffer_t fields for those who may use this structure for data manipulation.
 * They can be removed, if not wanted.
 *
 * Method 1: Working with data pointers
 *  The generated code shows how to get these pointers from the audio_chunk_t through both routines:
 *  + void *AudioChunk_getWritePtr(audio_chunk_t const *const pChunk,   uint8_t  const chId, uint32_t const spleId);
 *  + void *AudioChunk_getReadPtr(audio_chunk_t  const *const pChunk,   uint8_t  const chId, uint32_t const spleId);
 *  Note: Necessary info such as number of channels, samples, etc... are saved in the context structure. Of course,
 *       if not needed they can be removed.
 *
 * Method 2: Working with audio_buffer_t
 *  The generated code shows how to set audio_buffer_t from audio_chunk_t through both routines:
 *  + for input:  int32_t AudioChunk_setAudioBufPtrFromRdPtr(audio_chunk_t const *const pChunk,   audio_buffer_t *const pBuff);
 *  + for output: int32_t AudioChunk_setAudioBufPtrFromWrPtr(audio_chunk_t const *const pChunk,   audio_buffer_t *const pBuff);
*/
static int32_t s_speex_nr_init(audio_algo_t      *const pAlgo);
static int32_t s_speex_nr_deinit(audio_algo_t    *const pAlgo);
static int32_t s_speex_nr_configure(audio_algo_t *const pAlgo);
static int32_t s_speex_nr_dataInOut(audio_algo_t *const pAlgo);
static int32_t s_speex_nr_process(audio_algo_t   *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_speex_nr_common =
{
  .pName                     = "speex-nr",

  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_ALL,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_1CH,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_8000_16000,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_1CH,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_8000_16000,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16,
  .misc.pAlgoDesc            = "Noise reduction from https://www.speex.org/",

};

audio_algo_cbs_t AudioChainWrp_speex_nr_cbs =
{
  .init                       = s_speex_nr_init,
  .deinit                     = s_speex_nr_deinit,
  .configure                  = s_speex_nr_configure,
  .dataInOut                  = s_speex_nr_dataInOut,
  .process                    = s_speex_nr_process,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/

static int32_t s_speex_nr_init(audio_algo_t *const pAlgo)
{
  int32_t                                error          = AUDIO_ERR_MGNT_NONE;
  speex_nr_static_config_t  const *const pStaticConfig  = (speex_nr_static_config_t *)AudioAlgo_getStaticConfig(pAlgo);
  speex_nr_dynamic_config_t const *const pDynamicConfig = (speex_nr_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chunk_t                   *const pChunkIn       = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t                   *const pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t            const *const pBuffIn        = AudioChunk_getBuffInfo(pChunkIn);
  uint32_t                         const nbSamples      = AudioBuffer_getNbSamples(pBuffIn);
  uint32_t                         const fs             = AudioBuffer_getFs(pBuffIn);
  uint32_t                         const buffSize       = AudioBuffer_getBufferSize(pBuffIn);
  speex_nr_context_t                    *pContext       = NULL;
  size_t                                 allocSize      = sizeof(speex_nr_context_t);
  memPool_t                              memPool        = AUDIO_MEM_UNKNOWN;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0,  "missing static or dynamic config !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool  = (memPool_t)pStaticConfig->ramType;
    pContext = (speex_nr_context_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pContext == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    memset(pContext, 0, allocSize);
    pContext->buffSize  = buffSize;
    AudioAlgo_setWrapperContext(pAlgo, pContext);

    speex_setMemPool(memPool);  // set speex mem pool for speex allocation
    pContext->memPool     = memPool;
    pContext->pin.pChunk  = pChunkIn;
    pContext->pout.pChunk = pChunkOut;
    pContext->user.pHdle  = speex_preprocess_state_init((int)nbSamples, (int)fs);
    if (pContext->user.pHdle == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "speex_preprocess_state_init alloc error !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    error = s_speex_nr_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    error = s_speex_nr_deinit(pAlgo);
  }

  return error;
}


static int32_t s_speex_nr_deinit(audio_algo_t *const pAlgo)
{
  int32_t                   error    = AUDIO_ERR_MGNT_NONE;
  speex_nr_context_t *const pContext = (speex_nr_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pContext->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    speex_setMemPool(memPool);  // set speex mem pool for speex deallocation
    if (pContext->user.pHdle != NULL)
    {
      speex_preprocess_state_destroy(pContext->user.pHdle);
    }

    AudioAlgo_free(pContext, memPool);
  }
  return error;
}


static int32_t s_speex_nr_configure(audio_algo_t *const pAlgo)
{
  int32_t                                error          = AUDIO_ERR_MGNT_NONE;
  speex_nr_dynamic_config_t const *const pDynamicConfig = (speex_nr_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  speex_nr_context_t              *const pContext       = (speex_nr_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing dynamic conf !");
    error = AUDIO_ERR_MGNT_INIT;
  }
  if (AudioError_isOk(error))
  {
    if (pDynamicConfig->agc == 0.0f)
    {
      pContext->user.pHdle->agc_enabled = 0;
      pContext->user.pHdle->agc_level   = 0.0f;
    }
    else if ((pDynamicConfig->agc > 0.0f) && (pDynamicConfig->agc <= 1.0f))
    {
      pContext->user.pHdle->agc_enabled = 1;
      pContext->user.pHdle->agc_level   = pDynamicConfig->agc * 32768.0f;
    }
    else
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "s_speex_nr_configure failed !");
      pContext->user.pHdle->agc_enabled = 0;
      error                             = AUDIO_ERR_MGNT_CONFIG;
    }
  }
  if (AudioError_isOk(error))
  {
    pContext->user.pHdle->noise_suppress       = (int)pDynamicConfig->noise_suppress;
    pContext->user.pHdle->echo_suppress        = 0;
    pContext->user.pHdle->echo_suppress_active = 0;
    if (speex_preprocess_ctl(pContext->user.pHdle, SPEEX_PREPROCESS_SET_ECHO_STATE, NULL) != 0)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "speex_preprocess_ctl failed !");
      error = AUDIO_ERR_MGNT_CONFIG;
    }
  }

  return error;
}


static int32_t s_speex_nr_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  AudioAlgo_incReadyForProcess(pAlgo);
  return error;
}


static int32_t s_speex_nr_process(audio_algo_t *const pAlgo)
{
  int32_t                   error    = AUDIO_ERR_MGNT_NONE;
  speex_nr_context_t *const pContext = (speex_nr_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  size_t                    buffSize = pContext->buffSize;
  spx_int16_t  const *const pIn      = (spx_int16_t const *)AudioChunk_getReadPtr0(pContext->pin.pChunk);
  spx_int16_t        *const pOut     = (spx_int16_t *)AudioChunk_getWritePtr0(pContext->pout.pChunk);

  memcpy(pOut, pIn, buffSize);
  speex_preprocess_run(pContext->user.pHdle, pOut);

  return error;
}
