/**
******************************************************************************
* @file    audio_chain_interleave.c
* @author  MCD Application Team
* @brief   wrapper of interleaving (mix of input channels) algo to match usage inside audio_chain.c
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
#include "interleave/audio_chain_interleave.h"
#include "sfc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  sfcContext_t *pSfcContext;
}
INTERLEAVE_Handler_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_interleave_deinit(audio_algo_t           *const pAlgo);
static int32_t s_interleave_init(audio_algo_t             *const pAlgo);
static int32_t s_interleave_dataInOut(audio_algo_t        *const pAlgo);
static int32_t s_interleave_checkConsistency(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_interleave_common =
{
  .pName                     = "interleave",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_FS | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_DURATION),
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_FS | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_DURATION),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_ALL,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0In3\0"),

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_ALL,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Convert multiple inputs to multi-channel output (sum of number of channels of input chunks must be equal to number of channels output chunk)"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("interleave"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_OPTIONAL_INPUTS
};

audio_algo_cbs_t AudioChainWrp_interleave_cbs =
{
  .init                       = s_interleave_init,
  .deinit                     = s_interleave_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_interleave_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = s_interleave_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_interleave_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t       error   = AUDIO_ERR_MGNT_NONE;
  uint8_t const nbChIn  = AudioChunkList_getTotalNbChan(AudioAlgo_getChunksIn(pAlgo));
  uint8_t const nbChOut = AudioBuffer_getNbChannels(AudioChunk_getBuffInfo(AudioAlgo_getChunkPtrOut(pAlgo, 0U)));

  if (nbChIn != nbChOut)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0,
                    "Wrong channels number ! sum of number of channels of input chunks must be equal to number of channels output chunk");
    error = AUDIO_ERR_MGNT_INIT;
  }

  return error;
}


static int32_t s_interleave_init(audio_algo_t *const pAlgo)
{
  int32_t                     error              = AUDIO_ERR_MGNT_NONE;
  uint8_t               const nbChunkIn          = AudioChunkList_getNbElements(AudioAlgo_getChunksIn(pAlgo));
  size_t                const allocSize          = sizeof(INTERLEAVE_Handler_t) + (nbChunkIn * sizeof(sfcContext_t));
  INTERLEAVE_Handler_t *const pInterleaveHandler = (INTERLEAVE_Handler_t *)AudioAlgo_malloc(allocSize, AUDIO_MEM_RAMINT);

  AudioAlgo_setWrapperContext(pAlgo, pInterleaveHandler);
  if (pInterleaveHandler == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    audio_chain_utilities_t *const pUtilsHandle = AudioAlgo_getUtilsHdle(pAlgo);
    audio_buffer_t          *const pBuffOut     = AudioChunk_getBuffInfo(AudioAlgo_getChunkPtrOut(pAlgo, 0U));
    uint8_t                        idChunkIn    = 0U;

    pInterleaveHandler->pSfcContext = (sfcContext_t *)&pInterleaveHandler[1];
    for (audio_chunk_list_t *pCurrent = AudioAlgo_getChunksIn(pAlgo); pCurrent != NULL; pCurrent = pCurrent->next)
    {
      if (pCurrent->pChunk != NULL)
      {
        sfcContext_t *const pSfcContext = &pInterleaveHandler->pSfcContext[idChunkIn];

        sfcResetContext(pSfcContext);
        if (AudioError_isError(sfcSetContext(pSfcContext,
                                             AudioChunk_getBuffInfo(pCurrent->pChunk),
                                             pBuffOut,
                                             false,
                                             1.0f,
                                             pUtilsHandle)))
        {
          AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue !");
          error = AUDIO_ERR_MGNT_INIT;
          break;
        }
        idChunkIn++;
      }
    }
  }

  if (AudioError_isError(error))
  {
    s_interleave_deinit(pAlgo);
  }

  return error;
}


static int32_t s_interleave_deinit(audio_algo_t *const pAlgo)
{
  INTERLEAVE_Handler_t *const pInterleaveHandler = (INTERLEAVE_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pInterleaveHandler != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pInterleaveHandler, AUDIO_MEM_RAMINT);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_interleave_dataInOut(audio_algo_t *const pAlgo)
{
  INTERLEAVE_Handler_t const *const pInterleaveHandler = (INTERLEAVE_Handler_t const *)AudioAlgo_getWrapperContext(pAlgo);
  audio_chunk_t              *const pChunkOut          = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t             *const pBuffOut           = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                    const nbSamples          = AudioBuffer_getNbElements(pBuffOut);
  uint8_t                           idChunkIn          = 0U;
  uint8_t                           idCh               = 0U;

  for (audio_chunk_list_t *pCurrent = AudioAlgo_getChunksIn(pAlgo); pCurrent != NULL; pCurrent = pCurrent->next)
  {
    audio_chunk_t const *const pChunkIn = pCurrent->pChunk;

    if (pChunkIn != NULL)
    {
      audio_buffer_t const *const pBuffIn    = AudioChunk_getBuffInfo(pChunkIn);
      uint8_t               const nbChIn     = AudioBuffer_getNbChannels(pBuffIn);
      void                 *const pSampleIn  = AudioChunk_getReadPtr0(pChunkIn);
      void                 *const pSampleOut = AudioChunk_getWritePtr(pChunkOut, idCh, 0UL);

      sfcSampleBufferConvert(&pInterleaveHandler->pSfcContext[idChunkIn], pSampleIn, pSampleOut, (int)nbChIn, (int)nbSamples);
      idChunkIn++;
      idCh += nbChIn;
    }
  }

  return AUDIO_ERR_MGNT_NONE;
}
