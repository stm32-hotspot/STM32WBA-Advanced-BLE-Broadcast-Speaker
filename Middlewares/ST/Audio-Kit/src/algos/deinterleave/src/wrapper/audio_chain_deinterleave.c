/**
******************************************************************************
* @file    audio_chain_deinterleave.c
* @author  MCD Application Team
* @brief   wrapper of deinterleave algo to match usage inside audio_chain.c
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
#include "deinterleave/audio_chain_deinterleave.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t              nbChunkIn;
  uint8_t              nbChOut;
  uint8_t              nbChunkOut;
  uint32_t             nbElements; /*An element is a sample in real domain and a bin in freq domain; a bin is complex so two float 1 for real 1 for Imaginary */
  audio_chunk_t       *pChunkIn;
  audio_chunk_list_t  *pChunksOut;
  sfcContext_t        *pSfcContext;
}
DEINTERLEAVE_Handler_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_deinterleave_deinit(audio_algo_t           *const pAlgo);
static int32_t s_deinterleave_init(audio_algo_t             *const pAlgo);
static int32_t s_deinterleave_dataInOut(audio_algo_t        *const pAlgo);
static int32_t s_deinterleave_checkConsistency(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_deinterleave_common =
{
  .pName                     = "deinterleave",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_FS | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_DURATION),
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_FS | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_DURATION),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_ALL,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_ALL,
  .iosOut.pPinNames          = AUDIO_ALGO_OPT_STR("Out0\0Out1\0Out2\0Out3\0"),

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Convert multi-channel input to multiple outputs (number of channels of input chunk must be equal to sum of number of output channels of output chunks)"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("deinterleave")
};

audio_algo_cbs_t AudioChainWrp_deinterleave_cbs =
{
  .init                       = s_deinterleave_init,
  .deinit                     = s_deinterleave_deinit,
  .dataInOut                  = s_deinterleave_dataInOut,
  .configure                  = NULL,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = s_deinterleave_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/

static int32_t s_deinterleave_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t       error   = AUDIO_ERR_MGNT_NONE;
  uint8_t const nbChIn  = AudioBuffer_getNbChannels(AudioChunk_getBuffInfo(AudioAlgo_getChunkPtrIn(pAlgo, 0U)));
  uint8_t const nbChOut = AudioChunkList_getTotalNbChan(AudioAlgo_getChunksOut(pAlgo));

  if (nbChIn != nbChOut)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0,
                    "Wrong channels number ! number of channels of input chunk must be equal to sum of number of channels of output chunks");
    error = AUDIO_ERR_MGNT_INIT;
  }

  return error;
}


static int32_t s_deinterleave_init(audio_algo_t *const pAlgo)
{
  int32_t                        error                = AUDIO_ERR_MGNT_NONE;
  uint8_t                  const nbChunkOut           = AudioChunkList_getNbElements(AudioAlgo_getChunksOut(pAlgo));
  size_t                   const allocSize            = sizeof(DEINTERLEAVE_Handler_t) + (nbChunkOut * sizeof(sfcContext_t));
  DEINTERLEAVE_Handler_t  *const pDeinterleaveHandler = (DEINTERLEAVE_Handler_t *)AudioAlgo_malloc(allocSize, AUDIO_MEM_RAMINT);
  audio_chunk_t           *const pChunkIn             = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_buffer_t          *const pBuffIn              = AudioChunk_getBuffInfo(pChunkIn);
  uint32_t                 const nbElements           = AudioBuffer_getNbElements(pBuffIn);
  audio_chain_utilities_t *const pUtilsHandle         = AudioAlgo_getUtilsHdle(pAlgo);

  AudioAlgo_setWrapperContext(pAlgo, pDeinterleaveHandler);
  if (pDeinterleaveHandler == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    uint8_t idChunkOut = 0U;

    pDeinterleaveHandler->nbChunkOut  = nbChunkOut;
    pDeinterleaveHandler->pChunkIn    = pChunkIn;
    pDeinterleaveHandler->nbElements  = nbElements;
    pDeinterleaveHandler->pChunksOut  = AudioAlgo_getChunksOut(pAlgo);
    pDeinterleaveHandler->pSfcContext = (sfcContext_t *)&pDeinterleaveHandler[1];

    for (audio_chunk_list_t *pCurrent = AudioAlgo_getChunksOut(pAlgo); pCurrent != NULL; pCurrent = pCurrent->next)
    {
      if (pCurrent->pChunk != NULL)
      {
        sfcContext_t *const pSfcContext = &pDeinterleaveHandler->pSfcContext[idChunkOut];

        sfcResetContext(pSfcContext);
        if (AudioError_isError(sfcSetContext(pSfcContext,
                                             pBuffIn,
                                             AudioChunk_getBuffInfo(pCurrent->pChunk),
                                             false,
                                             1.0f,
                                             pUtilsHandle)))
        {
          AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue !");
          error = AUDIO_ERR_MGNT_INIT;
          break;
        }
        idChunkOut++;
      }
    }
  }

  if (AudioError_isError(error))
  {
    s_deinterleave_deinit(pAlgo);
  }

  return error;
}


static int32_t s_deinterleave_deinit(audio_algo_t *const pAlgo)
{
  int32_t                       error                = AUDIO_ERR_MGNT_NONE;
  DEINTERLEAVE_Handler_t *const pDeinterleaveHandler = (DEINTERLEAVE_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pDeinterleaveHandler != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pDeinterleaveHandler, AUDIO_MEM_RAMINT);
  }

  return error;
}


static int32_t s_deinterleave_dataInOut(audio_algo_t *const pAlgo)
{
  DEINTERLEAVE_Handler_t const *const pDeinterleaveHandler = (DEINTERLEAVE_Handler_t const *)AudioAlgo_getWrapperContext(pAlgo);
  audio_chunk_t                *const pChunkIn             = pDeinterleaveHandler->pChunkIn;
  uint32_t                      const nbElements           = pDeinterleaveHandler->nbElements;
  uint8_t                             idChunkOut           = 0U;
  uint8_t                             idCh                 = 0U;

  for (audio_chunk_list_t *pCurrent = pDeinterleaveHandler->pChunksOut; pCurrent != NULL; pCurrent = pCurrent->next)
  {
    audio_chunk_t const *const pChunkOut = pCurrent->pChunk;

    if (pChunkOut != NULL)
    {
      audio_buffer_t const *const pBuffOut   = AudioChunk_getBuffInfo(pChunkOut);
      uint8_t               const nbChOut    = AudioBuffer_getNbChannels(pBuffOut);
      void                 *const pSampleOut = AudioChunk_getWritePtr0(pChunkOut);
      void                 *const pSampleIn  = AudioChunk_getReadPtr(pChunkIn, idCh, 0UL);

      sfcSampleBufferConvert(&pDeinterleaveHandler->pSfcContext[idChunkOut], pSampleIn, pSampleOut, (int)nbChOut, (int)nbElements);
      idChunkOut++;
      idCh += nbChOut;
    }
  }

  return AUDIO_ERR_MGNT_NONE;
}
