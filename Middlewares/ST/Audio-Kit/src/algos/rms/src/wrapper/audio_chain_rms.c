/**
******************************************************************************
* @file    audio_chain_rms.c
* @author  MCD Application Team
* @brief   RMS computation of input stream as an audio chain algo
*          RMS value for each channel goes from 0.0 to 1.0
*          a new value is available each <rmswindow> samples
*          the time smoothing is computing with Lambda (should be close to 1.0)
*          remark: this algo has no output samples
*          known limitations: 8 channels maximum
*                             16 bit samples only
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
#include "rms/audio_chain_rms.h"
#include "../rms.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  rmsHdler_t          algoWrp;
  rmsCtrl_t           algoCtrl;
  uint32_t            fs;
  uint8_t             nbChIn;           /* save in context to avoid to call AudioBuffer_getNbChannels each time in process */
  uint32_t            nbElements;       /* save in context to avoid to call AudioBuffer_getNbElements each time in process */
  uint32_t            missingSamples;   /* missing to compute rms from r2 */
  uint32_t            rmsWindowSamples; /* for output                     */
  audio_chunk_t      *pChunkIn;         /* save in context to avoid to call AudioAlgo_getChunkPtrIn(pAlgo, 0U)  each time in process */
}
rms_context_t;

/* Private defines -----------------------------------------------------------*/
#define RMS_MEM_POOL AUDIO_MEM_RAMINT

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_rms_deinit(audio_algo_t                *const pAlgo);
static int32_t s_rms_init(audio_algo_t                  *const pAlgo);
static int32_t s_rms_checkConsistency(audio_algo_t      *const pAlgo);
static int32_t s_rms_configure(audio_algo_t             *const pAlgo);
static int32_t s_rms_dataInOut(audio_algo_t             *const pAlgo);
static int32_t s_rms_process(audio_algo_t               *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_rms_common =
{
  .pName                     = "rms",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_NOT_APPLICABLE,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_NONE,
  .iosOut.nbChan             = (audio_capability_channel_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosOut.fs                 = (audio_capability_fs_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosOut.interleaving       = (audio_capability_interleaving_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosOut.time_freq          = (audio_capability_time_freq_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosOut.type               = (audio_capability_type_t)AUDIO_ALGO_FORMAT_UNDEF,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Calculation of the root mean square (signal level)"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_DISABLE_AUTO_MOUNT
};

audio_algo_cbs_t AudioChainWrp_rms_cbs =
{
  .init                       = s_rms_init,
  .deinit                     = s_rms_deinit,
  .configure                  = s_rms_configure,
  .dataInOut                  = s_rms_dataInOut,
  .process                    = s_rms_process,
  .control                    = NULL,
  .checkConsistency           = s_rms_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_rms_init(audio_algo_t *const pAlgo)
{
  int32_t                        error          = AUDIO_ERR_MGNT_NONE;
  rms_context_t                 *pContext       = NULL;
  audio_chunk_t           *const pChunkIn       = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_buffer_t    const *const pBuffIn        = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_type_t      const audioType      = AudioBuffer_getType(pBuffIn);
  rms_stat_config_t const *const pStaticConfig  = (rms_stat_config_t const *)AudioAlgo_getStaticConfig(pAlgo);
  rms_dyn_config_t  const *const pDynamicConfig = (rms_dyn_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);
  rmsHdler_t                    *pRmsHdle       = NULL;
  rmsCtrl_t                     *pRmsCtrl       = NULL;
  size_t                   const allocSize      = sizeof(rms_context_t);

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "static or dynamic config is missing!");
    error = AUDIO_ERR_MGNT_CONFIG;
  }

  if (AudioError_isOk(error))
  {
    pContext = (rms_context_t *)AudioAlgo_malloc(allocSize, RMS_MEM_POOL);
    if (pContext == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    memset(pContext, 0, allocSize);
    AudioAlgo_setWrapperContext(pAlgo, pContext);
    /* Set general info in context for faster process (avoid get from AC) */
    pContext->pChunkIn   = pChunkIn;
    pContext->nbChIn     = AudioBuffer_getNbChannels(pBuffIn);
    pContext->nbElements = AudioBuffer_getNbElements(pBuffIn);
    pContext->fs         = AudioBuffer_getFs(pBuffIn);

    /* Set control handler  */
    pRmsCtrl                    = &pContext->algoCtrl;
    pRmsCtrl->nbChannels        = (uint32_t)pContext->nbChIn;
    AudioAlgo_setCtrlData(pAlgo, pRmsCtrl);

    /* Set algo handler  */
    pRmsHdle                    = &pContext->algoWrp;
    pRmsHdle->isDoublePrecision = (pStaticConfig->isDoublePrecision != 0U);
    pRmsHdle->nbChannels        = pContext->nbChIn;
    pRmsHdle->audioType         = audioType;
    pRmsHdle->fs                = pContext->fs;

    /* Get mem for algo from given config */
    error = rmsGetMemorySize(pRmsHdle);
  }

  if (AudioError_isOk(error) && (pRmsHdle->internalMemSize != 0UL))
  {
    /* Allocate requested size for algo */
    pRmsHdle->pInternalMem = AudioAlgo_malloc(pRmsHdle->internalMemSize, RMS_MEM_POOL);
    if (pRmsHdle->pInternalMem == NULL)
    {
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
    else
    {
      memset(pRmsHdle->pInternalMem, 0, pRmsHdle->internalMemSize);
      error = rmsInit(pRmsHdle);
    }
  }

  if (AudioError_isOk(error))
  {
    // compute other context members from dynamic info
    error = s_rms_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_rms_deinit(pAlgo);
  }

  return error;
}


static int32_t s_rms_deinit(audio_algo_t *const pAlgo)
{
  rms_context_t *pContext = (rms_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext->algoWrp.pInternalMem, RMS_MEM_POOL);
    AudioAlgo_free(pContext, RMS_MEM_POOL);
    AudioAlgo_setCtrlData(pAlgo, NULL);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_rms_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                     error    = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t        *const pChunkIn = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_buffer_t const *const pBuffIn  = AudioChunk_getBuffInfo(pChunkIn);
  uint8_t               const nbChIn   = AudioBuffer_getNbChannels(pBuffIn);

  if (nbChIn > RMS_MAX_NB_CHANNELS)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0,
                    "Please increase RMS_MAX_NB_CHANNELS before generating AudioChainModule library!");
    error = AUDIO_ERR_MGNT_INIT;
  }
  return error;
}


static int32_t s_rms_configure(audio_algo_t *const pAlgo)
{
  int32_t                       error          = AUDIO_ERR_MGNT_NONE;
  rms_dyn_config_t const *const pDynamicConfig = (rms_dyn_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);
  rms_context_t                *pContext       = (rms_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pDynamicConfig == NULL)
  {
    error = AUDIO_ERR_MGNT_CONFIG;
  }
  else
  {
    error = rmsConfigure(&pContext->algoWrp, pDynamicConfig->smoothingTime);

    if (AudioError_isOk(error))
    {
      pContext->rmsWindowSamples = ((uint32_t)pDynamicConfig->rmsWindow * pContext->fs) / 1000UL;
      pContext->missingSamples   = pContext->rmsWindowSamples;
    }
    else
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "lambda computation from smoothingTime failed !");
    }
  }

  return error;
}


static int32_t s_rms_dataInOut(audio_algo_t *const pAlgo)
{
  AudioAlgo_incReadyForProcess(pAlgo);

  return AUDIO_ERR_MGNT_NONE;
}

static int32_t s_rms_process(audio_algo_t *const pAlgo)
{
  int32_t           error    = AUDIO_ERR_MGNT_NONE;
  rms_context_t    *pContext = (rms_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  rmsHdler_t *const pRmsHdle = &pContext->algoWrp;
  rmsCtrl_t  *const pRmsCtrl = &pContext->algoCtrl;

  error = rmsProcess(pRmsHdle, pContext->pChunkIn);

  if (AudioError_isOk(error))
  {
    for (uint32_t ch = 0UL; ch < pContext->nbChIn; ch++)
    {
      pRmsCtrl->rms[ch] = pRmsHdle->pRmsValues[ch];
    }
    if (pContext->missingSamples <= pContext->nbElements)
    {
      pContext->missingSamples = pContext->rmsWindowSamples;
      AudioAlgo_incReadyForControl(pAlgo);
    }
    else
    {
      pContext->missingSamples -= pContext->nbElements;
    }
  }

  return error;
}


