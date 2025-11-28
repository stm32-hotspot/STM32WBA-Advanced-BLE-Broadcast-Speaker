/**
******************************************************************************
* @file    audio_chain_switch.c
* @author  MCD Application Team
* @brief   wrapper of switch algo (select 1 input chunk) to match usage inside audio_chain.c
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
#include "switch/audio_chain_switch.h"
#include "sfc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t        nbChunkIn;
  audio_chunk_t *pChunkIn;
  audio_chunk_t *pChunkOut;
  int            nbChannels;
  int            nbElements;
  sfcContext_t   sfcContext;
} switchCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_switch_deinit(audio_algo_t           *const pAlgo);
static int32_t s_switch_init(audio_algo_t             *const pAlgo);
static int32_t s_switch_configure(audio_algo_t        *const pAlgo);
static int32_t s_switch_checkConsistency(audio_algo_t *const pAlgo);
static int32_t s_switch_dataInOut(audio_algo_t        *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_switch_common =
{
  .pName                     = "switch",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_FS),
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_FS),

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

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("select a stream out of all input pins"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_OPTIONAL_INPUTS
};

audio_algo_cbs_t AudioChainWrp_switch_cbs =
{
  .init                       = s_switch_init,
  .deinit                     = s_switch_deinit,
  .configure                  = s_switch_configure,
  .dataInOut                  = s_switch_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = s_switch_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_switch_init(audio_algo_t *const pAlgo)
{
  int32_t                        error          = AUDIO_ERR_MGNT_NONE;
  switchCtx_t             *const pContext       = (switchCtx_t *)AudioAlgo_malloc(sizeof(switchCtx_t), AUDIO_MEM_RAMINT);
  switch_dynamic_config_t *const pDynamicConfig = (switch_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);

  if (pContext == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }

  if (AudioError_isOk(error))
  {
    if (pDynamicConfig == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "dynamic config is missing !");
      error = AUDIO_ERR_MGNT_CONFIG;
    }
  }

  if (AudioError_isOk(error))
  {
    memset(pContext, 0, sizeof(switchCtx_t));
    AudioAlgo_setWrapperContext(pAlgo, pContext);

    pContext->nbChunkIn  = AudioChunkList_getNbElements(AudioAlgo_getChunksIn(pAlgo));
    pContext->pChunkOut  = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
    pContext->nbChannels = (int)AudioBuffer_getNbChannels(AudioChunk_getBuffInfo(pContext->pChunkOut));
    pContext->nbElements = (int)AudioBuffer_getNbElements(AudioChunk_getBuffInfo(pContext->pChunkOut));

    error = s_switch_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_switch_deinit(pAlgo);
  }

  return error;
}


static int32_t s_switch_configure(audio_algo_t *const pAlgo)
{
  int32_t                        error          = AUDIO_ERR_MGNT_NONE;
  switchCtx_t             *const pContext       = (switchCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  switch_dynamic_config_t *const pDynamicConfig = (switch_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);

  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "dynamic config is missing !");
    error = AUDIO_ERR_MGNT_CONFIG;
  }

  if (AudioError_isOk(error))
  {
    audio_chunk_t *pFirstChunkIn = NULL;
    uint8_t        chunkId       = 0U;

    /* get pDynamicConfig->inputId input chunk if connected (non-NULL), else get first connected (non-NULL) input chunk */
    pContext->pChunkIn = NULL;
    for (audio_chunk_list_t *pChunkInList = AudioAlgo_getChunksIn(pAlgo); pChunkInList != NULL; pChunkInList = pChunkInList->next)
    {
      if (pChunkInList->pChunk != NULL)
      {
        if (pFirstChunkIn == NULL)
        {
          pFirstChunkIn = pChunkInList->pChunk;
        }
        if (chunkId == pDynamicConfig->inputId)
        {
          pContext->pChunkIn = pChunkInList->pChunk;
          break;
        }
      }
      chunkId++;
    }
    if (pContext->pChunkIn == NULL)
    {
      pContext->pChunkIn = pFirstChunkIn;
    }

    sfcResetContext(&pContext->sfcContext);
    error = sfcSetContext(&pContext->sfcContext,
                          AudioChunk_getBuffInfo(pContext->pChunkIn),
                          AudioChunk_getBuffInfo(pContext->pChunkOut),
                          false,
                          1.0f,
                          AudioAlgo_getUtilsHdle(pAlgo));
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue !");
    }
  }

  return error;
}


static int32_t s_switch_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                        error          = AUDIO_ERR_MGNT_NONE;
  switch_dynamic_config_t *const pDynamicConfig = (switch_dynamic_config_t *)AudioAlgo_getDynamicConfig4Check(pAlgo);

  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "dynamic config is missing !");
    error = AUDIO_ERR_MGNT_CONFIG;
  }

  if (AudioError_isOk(error))
  {
    if (AudioAlgo_getChunkPtrIn(pAlgo, pDynamicConfig->inputId) == NULL)
    {
      //AudioAlgo_trace(pAlgo, TRACE_LVL_WARNING, NULL, 0, "input id not connected");
      AudioChainUtils_sendWarningMsg(AudioAlgo_getUtilsHdle(pAlgo), "WARNING:%s:%s:input id not connected; use first connected input", pAlgo->pDesc, SWITCH_INPUT_ID);
      error = AUDIO_ERR_MGNT_CONFIG_WARNING;
    }
    else
    {
      AudioChainUtils_sendWarningMsg(AudioAlgo_getUtilsHdle(pAlgo), "WARNING:%s:%s:", pAlgo->pDesc, SWITCH_INPUT_ID);
    }
  }

  return error;
}


static int32_t s_switch_deinit(audio_algo_t *const pAlgo)
{
  switchCtx_t *const pContext = (switchCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, AUDIO_MEM_RAMINT);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_switch_dataInOut(audio_algo_t *const pAlgo)
{
  switchCtx_t *const pContext = (switchCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  sfcSampleBufferConvert(&pContext->sfcContext,
                         AudioChunk_getReadPtr0(pContext->pChunkIn),
                         AudioChunk_getWritePtr0(pContext->pChunkOut),
                         pContext->nbChannels,
                         pContext->nbElements);

  return AUDIO_ERR_MGNT_NONE;
}
