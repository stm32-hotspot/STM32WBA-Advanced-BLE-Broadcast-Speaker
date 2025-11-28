/**
******************************************************************************
* @file    audio_chain_route.c
* @author  MCD Application Team
* @brief   wrapper of router algo to match usage inside audio_chain.c
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
#include <stdio.h>
#include <stdlib.h>
#include "route/audio_chain_route.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  sfcContext_t   sfcContext;
  audio_chunk_t *pChunk;
  uint8_t        nbCh;
}
ROUTE_inConf;

typedef struct
{
  router_dynamic_config_t *pDynamicConfig;
  uint8_t                  nbChunkIn;
  uint8_t                  nbChOut;
  uint32_t                 nbElements; /* An element is a sample in real domain and a bin in freq domain; a bin is complex so two float 1 for real 1 for Imaginary */
  ROUTE_inConf             inConf[AUDIOCHAINWRP_ROUTE_NB_MAX];
  audio_chunk_t           *pChunkOut;
}
ROUTE_Handler_t;

/* Private defines -----------------------------------------------------------*/
#define ROUTER_MEM_POOL AUDIO_MEM_RAMINT

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_route_deinit(audio_algo_t           *const pAlgo);
static int32_t s_route_init(audio_algo_t             *const pAlgo);
static int32_t s_route_dataInOut(audio_algo_t        *const pAlgo);
static int32_t s_route_checkConsistency(audio_algo_t *const pAlgo);
static int32_t s_route_configure(audio_algo_t        *const pAlgo);


/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_route_common =
{
  .pName                     = "route",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_FS),
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_FS),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_ALL,
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("Accepts several connections\0"),
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("In0\0In1\0In2\0In3\0"),

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_ALL,
  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("Chunk with channels routed\0"),

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Routes a selection of channels of input chunks to output chunk"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("route"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_OPTIONAL_INPUTS
};

audio_algo_cbs_t AudioChainWrp_route_cbs =
{
  .init                       = s_route_init,
  .deinit                     = s_route_deinit,
  .configure                  = s_route_configure,
  .dataInOut                  = s_route_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = s_route_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_route_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                              error          = AUDIO_ERR_MGNT_NONE;
  router_dynamic_config_t const *const pDynamicConfig = (router_dynamic_config_t const *)AudioAlgo_getDynamicConfig4Check(pAlgo);
  audio_chunk_t           const *const pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t          const *const pBuffOut       = AudioChunk_getBuffInfo(pChunkOut);
  uint8_t                              nbChOut        = AudioBuffer_getNbChannels(pBuffOut);

  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing dynamic config !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    if (nbChOut > (uint8_t)AUDIOCHAINWRP_ROUTE_NB_MAX)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "nbChOut > AUDIOCHAINWRP_ROUTE_NB_MAX !");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }

  if (AudioError_isOk(error))
  {
    audio_chunk_t *pChunkInTable[AUDIOCHAINWRP_ROUTE_NB_MAX] = {NULL};
    uint8_t        idChunkIn                                 = 0U;
    bool           noInput                                   = true;
    bool     const isPdmTypeOut                              = AudioBuffer_isPdmType(pBuffOut);

    for (audio_chunk_list_t *pCurrent = AudioAlgo_getChunksIn(pAlgo); pCurrent != NULL; pCurrent = pCurrent->next)
    {
      pChunkInTable[idChunkIn] = pCurrent->pChunk;
      if (pCurrent->pChunk != NULL)
      {
        noInput = false;
      }
      idChunkIn++;
    }
    if (noInput)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "no connected input chunk");
      error = AUDIO_ERR_MGNT_INIT;
    }

    for (uint8_t chOut = 0U; AudioError_isNoError(error) && (chOut < nbChOut); chOut++)
    {
      router_index_t const *const pRouterIndex = &pDynamicConfig->routeChOutConfig.tIndexes[chOut];
      bool                  const muted        = (chOut >= pDynamicConfig->routeChOutConfig.nbChOut) || (pRouterIndex->chId == AC_CHANNEL_MUTED);

      if (muted)
      {
        // it's OK whatever the input chunk and the number of channels of this input chunk
      }
      else
      {
        idChunkIn = pRouterIndex->inputId;
        if (pChunkInTable[idChunkIn] == NULL)
        {
          AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Route configuration for chOut %d: inputId %d not connected", chOut, idChunkIn);
          error = AUDIO_ERR_MGNT_CONFIG;
        }

        if (AudioError_isNoError(error))
        {
          audio_buffer_t *const pBuffIn     = AudioChunk_getBuffInfo(pChunkInTable[idChunkIn]);
          bool            const isPdmTypeIn = AudioBuffer_isPdmType(pBuffIn);
          uint8_t         const nbChIn      = AudioBuffer_getNbChannels(pBuffIn);

          if (isPdmTypeOut)
          {
            if (!isPdmTypeIn)
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "PCM input chunk %d incompatible with PDM output chunk", idChunkIn);
              error = AUDIO_ERR_MGNT_CONFIG;
            }
          }
          else
          {
            if (isPdmTypeIn)
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "PDM input chunk %d incompatible with PCM output chunk", idChunkIn);
              error = AUDIO_ERR_MGNT_CONFIG;
            }
          }

          if (AudioError_isNoError(error))
          {
            if (pRouterIndex->chId >= nbChIn)
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Route configuration for chOut %d: chosen chunk with id %d has only %d channels! User selection channel = %d is a mistake", chOut, idChunkIn, nbChIn, pRouterIndex->chId);
              error = AUDIO_ERR_MGNT_CONFIG;
            }
          }
        }
      }
    }
  }

  return error;
}


static int32_t s_route_init(audio_algo_t *const pAlgo)
{
  int32_t                        error          = AUDIO_ERR_MGNT_NONE;
  ROUTE_Handler_t               *pRouteHandler  = NULL;
  audio_chain_utilities_t *const pUtilsHandle   = AudioAlgo_getUtilsHdle(pAlgo);
  router_dynamic_config_t *const pDynamicConfig = (router_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chunk_list_t      *const pChunkInList   = AudioAlgo_getChunksIn(pAlgo);
  audio_chunk_t           *const pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t          *const pBuffOut       = AudioChunk_getBuffInfo(pChunkOut);
  uint8_t                  const nbChOut        = AudioBuffer_getNbChannels(pBuffOut);
  size_t                   const allocSize      = sizeof(ROUTE_Handler_t);
  uint8_t                        idChunkIn      = 0U;

  pRouteHandler = (ROUTE_Handler_t *)AudioAlgo_malloc(allocSize, ROUTER_MEM_POOL);
  if (pRouteHandler == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing dynamic config !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    AudioAlgo_setWrapperContext(pAlgo, pRouteHandler);
    memset(pRouteHandler, 0, allocSize);
    pRouteHandler->pDynamicConfig = pDynamicConfig;
    pRouteHandler->nbChOut        = nbChOut;
    pRouteHandler->nbElements     = AudioBuffer_getNbElements(pBuffOut);
    pRouteHandler->pChunkOut      = pChunkOut;
    for (audio_chunk_list_t *pCurrent = pChunkInList; pCurrent != NULL; pCurrent = pCurrent->next)
    {
      audio_chunk_t *const pChunkIn = pCurrent->pChunk;

      if (pChunkIn != NULL)
      {
        audio_buffer_t *const pBuffIn = AudioChunk_getBuffInfo(pChunkIn);

        pRouteHandler->inConf[idChunkIn].pChunk = pChunkIn;
        pRouteHandler->inConf[idChunkIn].nbCh   = AudioBuffer_getNbChannels(pBuffIn);
        sfcResetContext(&pRouteHandler->inConf[idChunkIn].sfcContext);
        if (AudioError_isError(sfcSetContext(&pRouteHandler->inConf[idChunkIn].sfcContext, pBuffIn, pBuffOut, false, 1.0f, pUtilsHandle)))
        {
          AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue!");
          error = AUDIO_ERR_MGNT_INIT;
          break;
        }
      }
      idChunkIn++;
    }
  }

  if (AudioError_isOk(error))
  {
    error = s_route_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_route_deinit(pAlgo);
  }

  return error;
}


static int32_t s_route_deinit(audio_algo_t *const pAlgo)
{
  ROUTE_Handler_t *const pRouteHandler = (ROUTE_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pRouteHandler != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pRouteHandler, ROUTER_MEM_POOL);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_route_configure(audio_algo_t *const pAlgo)
{
  int32_t                error         = AUDIO_ERR_MGNT_NONE;
  ROUTE_Handler_t *const pRouteHandler = (ROUTE_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pRouteHandler == NULL)
  {
    error = AUDIO_ERR_MGNT_CONFIG;
  }
  else
  {
    error = s_route_checkConsistency(pAlgo);
  }

  return error;
}


static int32_t s_route_dataInOut(audio_algo_t *const pAlgo)
{
  ROUTE_Handler_t const *const pRouteHandler = (ROUTE_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);

  for (uint8_t chOut = 0U; chOut < pRouteHandler->nbChOut; chOut++)
  {
    router_index_t const *const pRouterIndex = &pRouteHandler->pDynamicConfig->routeChOutConfig.tIndexes[chOut];
    bool                  const muted        = (chOut >= pRouteHandler->pDynamicConfig->routeChOutConfig.nbChOut) || (pRouterIndex->chId == AC_CHANNEL_MUTED);
    sfcContext_t   const *const pSfcContext  = &pRouteHandler->inConf[pRouterIndex->inputId].sfcContext;
    void                 *const pSampleOut   = AudioChunk_getWritePtr(pRouteHandler->pChunkOut, chOut, 0UL);

    if (muted)
    {
      sfcSampleBufferClear(pSfcContext, pSampleOut, 1, (int)pRouteHandler->nbElements);
    }
    else
    {
      audio_chunk_t *const pChunkIn  = pRouteHandler->inConf[pRouterIndex->inputId].pChunk;
      void          *const pSampleIn = AudioChunk_getReadPtr(pChunkIn, pRouterIndex->chId, 0UL);

      sfcSampleBufferConvert(pSfcContext, pSampleIn, pSampleOut, 1, (int)pRouteHandler->nbElements);
    }
  }

  return AUDIO_ERR_MGNT_NONE;
}

