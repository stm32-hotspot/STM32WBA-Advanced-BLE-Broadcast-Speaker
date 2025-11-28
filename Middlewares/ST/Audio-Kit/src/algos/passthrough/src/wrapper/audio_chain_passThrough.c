/**
  ******************************************************************************
  * @file    audio_chain_passThrough.c
  * @author  MCD Application Team
  * @brief   wrapper of pass-through algo to match usage inside audio_chain.c
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
#include "passthrough/audio_chain_passThrough.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_passThrough_deinit(audio_algo_t    *const pAlgo);
static int32_t s_passThrough_init(audio_algo_t      *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_passThrough_common =
{
  .pName                     = "passThrough",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_ALL,

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

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("simple buffer copy (no processing)")
};

audio_algo_cbs_t AudioChainWrp_passThrough_cbs =
{
  .init                       = s_passThrough_init,
  .deinit                     = s_passThrough_deinit,
  .configure                  = NULL,
  .dataInOut                  = AudioChainWrp_passThrough_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = AudioChainWrp_passThrough_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_passThrough_init(audio_algo_t *const pAlgo)
{
  (void)pAlgo;  // unused parameter
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_passThrough_deinit(audio_algo_t *const pAlgo)
{
  (void)pAlgo;  // unused parameter
  return AUDIO_ERR_MGNT_NONE;
}


int32_t AudioChainWrp_passThrough_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                     error     = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t  const *const pChunkIn  = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t  const *const pChunkOut = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t const *const pBuffIn   = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t const *const pBuffOut  = AudioChunk_getBuffInfo(pChunkOut);

  if (AudioError_isOk(error))
  {
    if (AudioError_isError(AudioBuffer_checkAudioBuffersCompatibility(pBuffIn, pBuffOut, ABUFF_PARAM_ALL)))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "input & output buffers MUST have same characteristics!");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }

  return error;
}


int32_t AudioChainWrp_passThrough_dataInOut(audio_algo_t *const pAlgo)
{
  audio_chunk_t  *const pChunkIn   = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t  *const pChunkOut  = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t *const pBuffIn    = AudioChunk_getBuffInfo(pChunkIn);
  uint8_t         const nbChannels = AudioBuffer_getNbChannels(pBuffIn);

  if ((AudioBuffer_getInterleaved(pBuffIn) == ABUFF_FORMAT_INTERLEAVED) || (nbChannels == 1U))
  {
    void *const ptrIn  = AudioChunk_getReadPtr0(pChunkIn);
    void *const ptrOut = AudioChunk_getWritePtr0(pChunkOut);

    memcpy(ptrOut, ptrIn, AudioBuffer_getBufferSize(pBuffIn));
  }
  else
  {
    uint32_t size = AudioBuffer_getNbElements(pBuffIn) * (uint32_t)AudioBuffer_getSampleSize(pBuffIn);

    if (AudioBuffer_isPdmType(pBuffIn))
    {
      size >>= 3U;  // PDM sample size is 1 bit; i.e. 8 samples per byte
    }
    for (uint8_t ch = 0U; ch < nbChannels; ch++)
    {
      void *const ptrIn  = AudioChunk_getReadPtr(pChunkIn,   ch, 0UL);
      void *const ptrOut = AudioChunk_getWritePtr(pChunkOut, ch, 0UL);

      memcpy(ptrOut, ptrIn, (size_t)size);
    }
  }

  return AUDIO_ERR_MGNT_NONE;
}
