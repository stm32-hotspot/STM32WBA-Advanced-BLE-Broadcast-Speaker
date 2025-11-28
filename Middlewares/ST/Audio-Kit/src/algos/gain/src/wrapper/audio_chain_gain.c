/**
******************************************************************************
* @file    audio_chain_gain.c
* @author  MCD Application Team
* @brief   wrapper of gain algo to match usage inside audio_chain.c
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
#include "gain/audio_chain_gain.h"
#include "sfc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  int          nbChannels;
  int          nbElements;
  sfcContext_t sfcContext;
} gainCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_gain_deinit(audio_algo_t               *const pAlgo);
static int32_t s_gain_init(audio_algo_t                 *const pAlgo);
static int32_t s_gain_configure(audio_algo_t            *const pAlgo);
static int32_t s_gain_dataInOut(audio_algo_t            *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_gain_common =
{
  .pName                     = "gain",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_NB_ELEMENTS | (uint32_t)ABUFF_PARAM_TIME_FREQ | (uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_FS),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Apply a gain"),
};

audio_algo_cbs_t AudioChainWrp_gain_cbs =
{
  .init                       = s_gain_init,
  .deinit                     = s_gain_deinit,
  .configure                  = s_gain_configure,
  .dataInOut                  = s_gain_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_gain_init(audio_algo_t *const pAlgo)
{
  int32_t                      error          = AUDIO_ERR_MGNT_NONE;
  gainCtx_t                   *pContext       = (gainCtx_t *)AudioAlgo_malloc(sizeof(gainCtx_t), AUDIO_MEM_RAMINT);
  gain_dynamic_config_t *const pDynamicConfig = (gain_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);

  if (pContext == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
    error = AUDIO_ERR_MGNT_ALLOCATION;
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
    audio_buffer_t const *const pBuffIn  = AudioChunk_getBuffInfo(AudioAlgo_getChunkPtrIn(pAlgo,  0U));
    audio_buffer_t const *const pBuffOut = AudioChunk_getBuffInfo(AudioAlgo_getChunkPtrOut(pAlgo, 0U));

    AudioAlgo_setWrapperContext(pAlgo, pContext);

    pContext->nbChannels = (int)AudioBuffer_getNbChannels(pBuffIn);
    pContext->nbElements  = (int)AudioBuffer_getNbElements(pBuffIn);

    sfcResetContext(&pContext->sfcContext);
    error = sfcSetContext(&pContext->sfcContext,
                          pBuffIn,
                          pBuffOut,
                          false,
                          1.0f,
                          AudioAlgo_getUtilsHdle(pAlgo));
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue !");
    }
  }
  if (AudioError_isOk(error))
  {
    error = s_gain_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_gain_deinit(pAlgo);
  }

  return error;
}


static int32_t s_gain_deinit(audio_algo_t *const pAlgo)
{
  gainCtx_t *const pContext = (gainCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, AUDIO_MEM_RAMINT);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_gain_configure(audio_algo_t *const pAlgo)
{
  int32_t                      error          = AUDIO_ERR_MGNT_NONE;
  gainCtx_t             *const pContext       = (gainCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  gain_dynamic_config_t *const pDynamicConfig = (gain_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);

  error = sfcUpdateContext(&pContext->sfcContext, false, powf(10.0f, pDynamicConfig->gain / 20.0f));  /*cstat !MISRAC2012-Rule-22.8 no issue with powf(10, ...) => errno check is useless */
  if (AudioError_isError(error))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc config issue !");
  }

  return error;
}


static int32_t s_gain_dataInOut(audio_algo_t *const pAlgo)
{
  gainCtx_t *const pContext    = (gainCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  void      *const pSamplesIn  = AudioChunk_getReadPtr0(AudioAlgo_getChunkPtrIn(pAlgo,   0U));
  void      *const pSamplesOut = AudioChunk_getWritePtr0(AudioAlgo_getChunkPtrOut(pAlgo, 0U));

  sfcSampleBufferConvert(&pContext->sfcContext,
                         pSamplesIn,
                         pSamplesOut,
                         pContext->nbChannels,
                         pContext->nbElements);

  return AUDIO_ERR_MGNT_NONE;
}


