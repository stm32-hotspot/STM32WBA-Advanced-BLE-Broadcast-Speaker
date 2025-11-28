/**
******************************************************************************
* @file    audio_chain_split.c
* @author  MCD Application Team
* @brief   wrapper of split algo (input chunk split in several output chunks) to match usage inside audio_chain.c
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
#include "split/audio_chain_split.h"
#include "sfc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  int           nbChannels;
  int           nbElements;
  sfcContext_t *pSfcContext;
} splitCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_split_deinit(audio_algo_t    *const pAlgo);
static int32_t s_split_init(audio_algo_t      *const pAlgo);
static int32_t s_split_dataInOut(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_split_common =
{
  .pName                     = "split",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_FS),
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_FS),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_ALL,
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("Source connection\0"),

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_MULTIPLE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_ALL,
  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("Accepts several connections\0"),

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Distribute the dataflow to several chunks"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("split")

};

audio_algo_cbs_t AudioChainWrp_split_cbs =
{
  .init                       = s_split_init,
  .deinit                     = s_split_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_split_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_split_init(audio_algo_t *const pAlgo)
{
  int32_t       error      = AUDIO_ERR_MGNT_NONE;
  uint8_t const nbChunkOut = AudioChunkList_getNbElements(AudioAlgo_getChunksOut(pAlgo));
  splitCtx_t   *pContext   = (splitCtx_t *)AudioAlgo_malloc(sizeof(splitCtx_t) + (nbChunkOut * sizeof(sfcContext_t)), AUDIO_MEM_RAMINT);

  if (pContext == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  if (AudioError_isOk(error))
  {
    audio_buffer_t const *const pBuffIn    = AudioChunk_getBuffInfo(AudioAlgo_getChunkPtrIn(pAlgo, 0U));
    uint8_t                     chunkOutId = 0U;

    pContext->nbChannels  = (int)AudioBuffer_getNbChannels(pBuffIn);
    pContext->nbElements  = (int)AudioBuffer_getNbElements(pBuffIn);
    pContext->pSfcContext = (sfcContext_t *)((uint8_t *)pContext + sizeof(splitCtx_t));

    for (audio_chunk_list_t *pChunkList = AudioAlgo_getChunksOut(pAlgo); pChunkList != NULL; pChunkList = pChunkList->next)
    {
      audio_buffer_t const *const pBuffOut = AudioChunk_getBuffInfo(pChunkList->pChunk);

      sfcResetContext(&pContext->pSfcContext[chunkOutId]);
      error = sfcSetContext(&pContext->pSfcContext[chunkOutId],
                            pBuffIn,
                            pBuffOut,
                            false,
                            1.0f,
                            AudioAlgo_getUtilsHdle(pAlgo));
      if (AudioError_isError(error))
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue !");
        break;
      }
      chunkOutId++;
    }

    AudioAlgo_setWrapperContext(pAlgo, pContext);
  }

  if (AudioError_isError(error))
  {
    s_split_deinit(pAlgo);
  }

  return error;
}


static int32_t s_split_deinit(audio_algo_t *const pAlgo)
{
  splitCtx_t *const pContext = (splitCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, AUDIO_MEM_RAMINT);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_split_dataInOut(audio_algo_t *const pAlgo)
{
  splitCtx_t *const pContext   = (splitCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  void       *const pSamplesIn = AudioChunk_getReadPtr0(AudioAlgo_getChunkPtrIn(pAlgo, 0U));
  uint8_t           chunkOutId = 0U;

  for (audio_chunk_list_t *pChunkList = AudioAlgo_getChunksOut(pAlgo); pChunkList != NULL; pChunkList = pChunkList->next)
  {
    sfcSampleBufferConvert(&pContext->pSfcContext[chunkOutId],
                           pSamplesIn,
                           AudioChunk_getWritePtr0(pChunkList->pChunk),
                           pContext->nbChannels,
                           pContext->nbElements);
    chunkOutId++;
  }

  return AUDIO_ERR_MGNT_NONE;
}
