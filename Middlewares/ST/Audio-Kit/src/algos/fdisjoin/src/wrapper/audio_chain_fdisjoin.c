/**
******************************************************************************
* @file    audio_chain_fdisjoin.c
* @author  MCD Application Team
* @brief   frequency disjoin allows to select a sub set of contiguous bands
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
/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */
#include "fdisjoin/audio_chain_fdisjoin.h"
//#include "fdisjoin.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t  nbChannels;
  uint16_t begId;
  size_t   szBytesDisjoinedFreqs;
}
FDISJOIN_context_t;

/* Private defines -----------------------------------------------------------*/
#define FDISJOIN_MEM_POOL AUDIO_MEM_RAMINT

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_fdisjoin_common =
{
  .pName                     = "fdisjoin",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_ALL ^ (uint32_t)ABUFF_PARAM_DURATION ^ (uint32_t)ABUFF_PARAM_NB_ELEMENTS),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_NO,
  .iosIn.time_freq           = AUDIO_CAPABILITY_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FLOAT,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_NO,
  .iosOut.time_freq          = AUDIO_CAPABILITY_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FLOAT,
  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("selection of frequency bands"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("fdisjoin")

};


/* Private function prototypes -----------------------------------------------*/
static int32_t s_fdisjoin_deinit(audio_algo_t                *const pAlgo);
static int32_t s_fdisjoin_init(audio_algo_t                  *const pAlgo);
static int32_t s_fdisjoin_dataInOut(audio_algo_t             *const pAlgo);
static int32_t s_fdisjoin_checkConsistency(audio_algo_t      *const pAlgo);

/* Global variables ----------------------------------------------------------*/
audio_algo_cbs_t  AudioChainWrp_fdisjoin_cbs =
{
  .init                       = s_fdisjoin_init,
  .deinit                     = s_fdisjoin_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_fdisjoin_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = s_fdisjoin_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_fdisjoin_init(audio_algo_t *const pAlgo)
{
  int32_t                         error         = AUDIO_ERR_MGNT_NONE;
  FDISJOIN_context_t       *const pContext      = (FDISJOIN_context_t *)AudioAlgo_malloc(sizeof(FDISJOIN_context_t), FDISJOIN_MEM_POOL);
  fdisjoin_static_config_t *const pStaticConfig = (fdisjoin_static_config_t *)AudioAlgo_getStaticConfig(pAlgo);

  if (pContext == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    if (pStaticConfig == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static conf !");
      error = AUDIO_ERR_MGNT_CONFIG;
    }
  }

  if (AudioError_isOk(error))
  {
    audio_chunk_t  *const pChunkIn = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
    audio_buffer_t *const pBuffIn  = AudioChunk_getBuffInfo(pChunkIn);
    uint16_t              nbDisjoinedFreqs;

    pContext->nbChannels = AudioBuffer_getNbChannels(pBuffIn);
    pContext->begId      = pStaticConfig->begId;
    nbDisjoinedFreqs     = pStaticConfig->endId - pContext->begId + 1U;

    pContext->szBytesDisjoinedFreqs = 2UL * sizeof(float32_t) * nbDisjoinedFreqs;

    AudioAlgo_setWrapperContext(pAlgo, pContext);
  }

  if (AudioError_isError(error))
  {
    s_fdisjoin_deinit(pAlgo);
  }

  return error; // Todo implement the  algo error;
}


static int32_t s_fdisjoin_deinit(audio_algo_t *const pAlgo)
{
  FDISJOIN_context_t *const pContext = (FDISJOIN_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, FDISJOIN_MEM_POOL);
  }

  return AUDIO_ERR_MGNT_NONE;
}

static int32_t s_fdisjoin_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                         error         = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t            *const pChunkOut     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t           *const pBuffOut      = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                  const nbBandsOut    = AudioBuffer_getNbElements(pBuffOut);
  audio_chunk_t            *const pChunkIn      = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_buffer_t           *const pBuffIn       = AudioChunk_getBuffInfo(pChunkIn);
  uint32_t                  const nbBandsIn     = AudioBuffer_getNbElements(pBuffIn);
  fdisjoin_static_config_t *const pStaticConfig = (fdisjoin_static_config_t *)AudioAlgo_getStaticConfig4Check(pAlgo);

  if (pStaticConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static conf !");
    error = AUDIO_ERR_MGNT_CONFIG;
  }

  if (AudioError_isOk(error))
  {
    uint32_t nbDisjoinedFreqs = (uint32_t)pStaticConfig->endId - (uint32_t)pStaticConfig->begId + 1UL;

    if (nbDisjoinedFreqs != nbBandsOut)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Output chunk size is not aligned with frequencies selected!");
      error = AUDIO_ERR_MGNT_INIT;
    }

    if (nbDisjoinedFreqs >= nbBandsIn)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "frequencies selected are not consistent with input chunk!");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }
  return error;
}


static int32_t s_fdisjoin_dataInOut(audio_algo_t *const pAlgo)
{
  FDISJOIN_context_t *const pContext   = (FDISJOIN_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  audio_chunk_t      *const pChunkOut  = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_chunk_t      *const pChunkIn   = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  float32_t          *const pSampleOut = (float32_t *)AudioChunk_getWritePtr0(pChunkOut);

  for (uint8_t chId = 0U; chId < pContext->nbChannels; chId++)
  {
    float32_t *pSampleIn  = (float32_t *)AudioChunk_getReadPtr(pChunkIn, chId, (uint32_t)pContext->begId);
    memcpy(pSampleOut, pSampleIn, pContext->szBytesDisjoinedFreqs);
  }

  return AUDIO_ERR_MGNT_NONE;
}


