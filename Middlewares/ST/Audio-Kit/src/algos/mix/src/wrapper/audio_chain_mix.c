/**
******************************************************************************
* @file    audio_chain_mix.c
* @author  MCD Application Team
* @brief   wrapper of mix algo to match usage inside audio_chain.c
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
#include "mix/audio_chain_mix.h"
#include "audio_assert.h"
#include "sfc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  int           nbChannels;
  int           nbElements;
  sfcContext_t *pSfcContext;
} mixCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_mix_deinit(audio_algo_t    *const pAlgo);
static int32_t s_mix_init(audio_algo_t      *const pAlgo);
static int32_t s_mix_configure(audio_algo_t *const pAlgo);
static int32_t s_mix_dataInOut(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_mix_common =
{
  .pName                     = "mix",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_FS),
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_FS),

  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,
  #if MIX_NB_INPUTS == 1U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0"),
  #elif MIX_NB_INPUTS == 2U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0"),
  #elif MIX_NB_INPUTS == 3U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0"),
  #elif MIX_NB_INPUTS == 4U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0In3\0"),
  #elif MIX_NB_INPUTS == 5U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0In3\0In4\0"),
  #elif MIX_NB_INPUTS == 6U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0In3\0In4\0In5\0"),
  #elif MIX_NB_INPUTS == 7U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0In3\0In4\0In5\0In6\0"),
  #elif MIX_NB_INPUTS == 8U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN_EIGHT,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0In3\0In4\0In5\0In6\0In7\0"),
  #elif MIX_NB_INPUTS == 9U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN_EIGHT_NINE,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0In3\0In4\0In5\0In6\0In7\0In8\0"),
  #elif MIX_NB_INPUTS == 10U
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN_EIGHT_NINE_TEN,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0In3\0In4\0In5\0In6\0In7\0In8\0In9\0"),
  #endif

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Mix entries"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("mix"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_OPTIONAL_INPUTS
};

audio_algo_cbs_t AudioChainWrp_mix_cbs =
{
  .init                       = s_mix_init,
  .deinit                     = s_mix_deinit,
  .configure                  = s_mix_configure,
  .dataInOut                  = s_mix_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_mix_init(audio_algo_t *const pAlgo)
{
  int32_t                     error          = AUDIO_ERR_MGNT_NONE;
  uint8_t               const nbChunkIn      = AudioChunkList_getNbElements(AudioAlgo_getChunksIn(pAlgo));
  mixCtx_t             *const pContext       = (mixCtx_t *)AudioAlgo_malloc(sizeof(mixCtx_t) + (nbChunkIn * sizeof(sfcContext_t)), AUDIO_MEM_RAMINT);
  mix_dynamic_config_t *const pDynamicConfig = (mix_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);

  if (AudioError_isOk(error))
  {
    if (pContext == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    if (pDynamicConfig == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "dynamic config is missing!");
      error = AUDIO_ERR_MGNT_CONFIG;
    }
  }

  if (AudioError_isOk(error))
  {
    audio_chain_utilities_t *const pUtilsHdle = AudioAlgo_getUtilsHdle(pAlgo);
    audio_buffer_t    const *const pBuffOut   = AudioChunk_getBuffInfo(AudioAlgo_getChunkPtrOut(pAlgo, 0U));
    int                            chunkId    = 0;

    AudioAlgo_setWrapperContext(pAlgo, pContext);

    pContext->pSfcContext = (sfcContext_t *)&pContext[1];
    pContext->nbChannels  = (int)AudioBuffer_getNbChannels(pBuffOut);
    pContext->nbElements  = (int)AudioBuffer_getNbElements(pBuffOut);
    for (audio_chunk_list_t *pChunkInList = AudioAlgo_getChunksIn(pAlgo); AudioError_isOk(error) && (pChunkInList != NULL); pChunkInList = pChunkInList->next)
    {
      if (pChunkInList->pChunk != NULL)
      {
        sfcResetContext(&pContext->pSfcContext[chunkId]);
        error = sfcSetContext(&pContext->pSfcContext[chunkId],
                              AudioChunk_getBuffInfo(pChunkInList->pChunk),
                              pBuffOut,
                              false,
                              1.0f,
                              pUtilsHdle);
        if (AudioError_isError(error))
        {
          AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc input %d issue !", chunkId);
        }
        chunkId++;
      }
    }
  }

  if (AudioError_isOk(error))
  {
    error = s_mix_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_mix_deinit(pAlgo);
  }

  return error;
}


static int32_t s_mix_deinit(audio_algo_t *const pAlgo)
{
  mixCtx_t *const pContext = (mixCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, AUDIO_MEM_RAMINT);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_mix_configure(audio_algo_t *const pAlgo)
{
  int32_t                     error          = AUDIO_ERR_MGNT_NONE;
  mixCtx_t             *const pContext       = (mixCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  mix_dynamic_config_t *const pDynamicConfig = (mix_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  float                *const pGain          = &pDynamicConfig->gain0;
  bool                        mix            = false; // no mix for first input chunk
  int                         confId         = 0;
  int                         chunkId        = 0;

  for (audio_chunk_list_t *pChunkInList = AudioAlgo_getChunksIn(pAlgo); AudioError_isOk(error) && (pChunkInList != NULL); pChunkInList = pChunkInList->next)
  {
    if (pChunkInList->pChunk != NULL)
    {
      error = sfcUpdateContext(&pContext->pSfcContext[chunkId], mix, powf(10.0f, pGain[confId] / 20.0f)); /*cstat !MISRAC2012-Rule-22.8 no issue with powf(10, ...) => errno check is useless*/
      mix   = true;  // mix for next input chunks
      if (AudioError_isError(error))
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "config input %d issue!", chunkId);
      }
      chunkId++;
    }
    confId++;
  }

  return error;
}


static int32_t s_mix_dataInOut(audio_algo_t *const pAlgo)
{
  mixCtx_t *const pContext    = (mixCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  void     *const pSamplesOut = AudioChunk_getWritePtr0(AudioAlgo_getChunkPtrOut(pAlgo, 0U));
  int             chunkId     = 0;

  for (audio_chunk_list_t *pChunkInList = AudioAlgo_getChunksIn(pAlgo); pChunkInList != NULL; pChunkInList = pChunkInList->next)
  {
    if (pChunkInList->pChunk != NULL)
    {
      sfcSampleBufferConvert(&pContext->pSfcContext[chunkId],
                             AudioChunk_getReadPtr0(pChunkInList->pChunk),
                             pSamplesOut,
                             pContext->nbChannels,
                             pContext->nbElements);
      chunkId++;
    }
  }

  return AUDIO_ERR_MGNT_NONE;
}


