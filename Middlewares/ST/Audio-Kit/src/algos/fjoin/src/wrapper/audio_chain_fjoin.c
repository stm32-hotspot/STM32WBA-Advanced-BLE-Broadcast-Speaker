/**
******************************************************************************
* @file    audio_chain_fjoin.c
* @author  MCD Application Team
* @brief   frequency join allows to merge different set of bands into a single stream
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
#include "fjoin/audio_chain_fjoin.h"
//#include "fjoin.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint32_t begId;
  uint32_t endId;
  uint32_t nbBands;
  size_t   szBytesBands;
}
fjoin_inchunk_config_t;


typedef struct
{
  uint8_t                  nbChannels;
  audio_chunk_list_t     *pChunksInList;
  fjoin_inchunk_config_t *pInChunksConf;
}
FJOIN_context_t;

/* Private defines -----------------------------------------------------------*/
#define FJOIN_MEM_POOL AUDIO_MEM_RAMINT

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_fjoin_deinit(audio_algo_t                *const pAlgo);
static int32_t s_fjoin_init(audio_algo_t                  *const pAlgo);
static int32_t s_fjoin_dataInOut(audio_algo_t             *const pAlgo);
static int32_t s_fjoin_checkConsistency(audio_algo_t      *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_fjoin_common =
{
  .pName                     = "fjoin",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_ALL ^ (uint32_t)ABUFF_PARAM_DURATION ^ (uint32_t)ABUFF_PARAM_NB_ELEMENTS),
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_ALL ^ (uint32_t)ABUFF_PARAM_DURATION ^ (uint32_t)ABUFF_PARAM_NB_ELEMENTS),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR,
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
  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("merge frequency bands of all inputs into a single output so that output_nb_bands = sum of all inputs nb_bands. Take good care to the order of the input connections (low frequencies first)"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_OPTIONAL_INPUTS,
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("fjoin")
};

audio_algo_cbs_t AudioChainWrp_fjoin_cbs =
{
  .init                       = s_fjoin_init,
  .deinit                     = s_fjoin_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_fjoin_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = s_fjoin_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_fjoin_init(audio_algo_t *const pAlgo)
{
  int32_t                   error         = AUDIO_ERR_MGNT_NONE;
  audio_chunk_list_t *const pChunksInList = AudioAlgo_getChunksIn(pAlgo);
  uint8_t             const nbChunkIn     = AudioChunkList_getNbElements(pChunksInList);
  size_t              const allocSize     = sizeof(FJOIN_context_t) + ((size_t)nbChunkIn * sizeof(fjoin_inchunk_config_t));
  FJOIN_context_t    *const pContext      = (FJOIN_context_t *)AudioAlgo_malloc(allocSize, FJOIN_MEM_POOL);
  fjoin_inchunk_config_t   *pInChunksConf = NULL;

  if (pContext != NULL)
  {
    AudioAlgo_setWrapperContext(pAlgo, pContext);
    memset(pContext, 0, allocSize);
    pContext->pChunksInList = pChunksInList;
    pInChunksConf = pContext->pInChunksConf = (fjoin_inchunk_config_t *)&pContext[1];
  }
  else
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    uint8_t               idChunkIn = 0U;
    uint32_t              idBand    = 0UL;
    audio_chunk_t  *const pChunkOut = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
    audio_buffer_t *const pBuffOut  = AudioChunk_getBuffInfo(pChunkOut);

    pContext->nbChannels = AudioBuffer_getNbChannels(pBuffOut);

    for (audio_chunk_list_t *pCurrent = pChunksInList; pCurrent != NULL; pCurrent = pCurrent->next)
    {
      audio_chunk_t *const pChunkIn = pCurrent->pChunk;

      if (pChunkIn != NULL)
      {
        audio_buffer_t *const pBuffIn = AudioChunk_getBuffInfo(pChunkIn);
        uint32_t        const nbBands = AudioBuffer_getNbElements(pBuffIn);

        pInChunksConf[idChunkIn].begId        = idBand;
        idBand                               += (nbBands - 1UL);
        pInChunksConf[idChunkIn].endId        = idBand;
        pInChunksConf[idChunkIn].nbBands      = nbBands;
        pInChunksConf[idChunkIn].szBytesBands = 2UL * sizeof(float32_t) * nbBands;

        idBand++;
        idChunkIn++;
      }
    }
  }

  if (AudioError_isError(error))
  {
    s_fjoin_deinit(pAlgo);
  }

  return error; // Todo implement the  algo error;
}


static int32_t s_fjoin_deinit(audio_algo_t *const pAlgo)
{
  FJOIN_context_t *const pContext = (FJOIN_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, FJOIN_MEM_POOL);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_fjoin_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                   error         = AUDIO_ERR_MGNT_NONE;
  audio_chunk_list_t *const pChunksInList = AudioAlgo_getChunksIn(pAlgo);
  audio_chunk_t      *const pChunkOut     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t     *const pBuffOut      = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t            const nbBandsOut    = AudioBuffer_getNbElements(pBuffOut);
  uint32_t                  nbBands       = 0UL;

  for (audio_chunk_list_t *pCurrent = pChunksInList; pCurrent != NULL; pCurrent = pCurrent->next)
  {
    audio_chunk_t *const pChunkIn = pCurrent->pChunk;

    if (pChunkIn != NULL)
    {
      audio_buffer_t *const pBuffIn = AudioChunk_getBuffInfo(pChunkIn);
      nbBands += AudioBuffer_getNbElements(pBuffIn);
    }
  }
  if (nbBands != nbBandsOut)
  {
    /* Do not return error allows to filter out some band brutally (might be helpful)*/
    AudioAlgo_trace(pAlgo, TRACE_LVL_WARNING, NULL, 0, "number of bands of output chunk is not aligned with input chunks bands!");
  }
  if (nbBandsOut < nbBands)
  {
    error = AUDIO_ERR_MGNT_INIT;
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "number of bands of output chunk is smaller than sum of input chunks bands!");
  }

  return error;
}


static int32_t s_fjoin_dataInOut(audio_algo_t *const pAlgo)
{
  FJOIN_context_t        *const pContext      = (FJOIN_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  audio_chunk_t          *const pChunkOut     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_chunk_list_t     *const pChunksInList = pContext->pChunksInList;
  fjoin_inchunk_config_t *const pInChunksConf = pContext->pInChunksConf;
  uint8_t                       idChunkIn     = 0U;

  for (audio_chunk_list_t *pCurrent = pChunksInList; pCurrent != NULL; pCurrent = pCurrent->next)
  {
    audio_chunk_t *const pChunkIn = pCurrent->pChunk;

    if (pChunkIn != NULL)
    {
      float32_t *const pSampleIn = (float32_t *)AudioChunk_getReadPtr0(pChunkIn);
      uint32_t         begId     = pInChunksConf[idChunkIn].begId;
      size_t           szBytes   = pInChunksConf[idChunkIn].szBytesBands;

      for (uint8_t chId = 0U; chId < pContext->nbChannels; chId++)
      {
        float32_t *pSampleOut = (float32_t *)AudioChunk_getWritePtr(pChunkOut, chId, begId);
        memcpy(pSampleOut, pSampleIn, szBytes);
      }

      idChunkIn++;
    }
  }

  return AUDIO_ERR_MGNT_NONE;
}


