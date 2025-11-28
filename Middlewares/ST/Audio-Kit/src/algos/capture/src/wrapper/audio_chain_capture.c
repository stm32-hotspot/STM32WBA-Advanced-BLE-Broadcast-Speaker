/**
******************************************************************************
* @file    audio_chain_capture.c
* @author  MCD Application Team
* @brief   wrapper of sample capture algo
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
#include "capture/audio_chain_capture.h"
#include "sfc.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  audio_chunk_t *pChunkIn;
  audio_buffer_t buffOut;
  sfcContext_t   sfcContext;
} captureCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_capture_deinit(audio_algo_t    *const pAlgo);
static int32_t s_capture_init(audio_algo_t      *const pAlgo);
static int32_t s_capture_dataInOut(audio_algo_t *const pAlgo);
static int32_t s_capture_process(audio_algo_t   *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_capture_common =
{
  .pName                     = "capture",
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

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("This algo allows to capture time samples using the control callback"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_DISABLE_AUTO_MOUNT
};

audio_algo_cbs_t AudioChainWrp_capture_cbs =
{
  .init                       = s_capture_init,
  .deinit                     = s_capture_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_capture_dataInOut,
  .process                    = s_capture_process,
  .control                    = NULL,                                     // user defined routine
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_capture_init(audio_algo_t *const pAlgo)
{
  int32_t               error    = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t  *const pChunkIn = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_buffer_t *const pBuffIn  = AudioChunk_getBuffInfo(pChunkIn);

  if ((pChunkIn == NULL) || (pBuffIn == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "no input chunk or no associated audio buffer !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    uint8_t  const nbChannels = AudioBuffer_getNbChannels(pBuffIn);
    uint32_t const nbSamples  = AudioBuffer_getNbSamples(pBuffIn);
    uint32_t const fs         = AudioBuffer_getFs(pBuffIn);
    size_t   const szContext  = sizeof(captureCtx_t) + sizeof(captureCtrl_t);
    captureCtx_t  *pContext   = NULL;
    captureCtrl_t *pCtrl      = NULL;

    pContext = (captureCtx_t *)AudioAlgo_malloc(szContext, AUDIO_MEM_RAMINT);
    if (pContext == NULL)
    {
      error = AUDIO_ERR_MGNT_INIT;
    }
    else
    {
      memset(pContext, 0, szContext);
      pCtrl = (captureCtrl_t *)&pContext[1]; /* right after the context */
      AudioAlgo_setWrapperContext(pAlgo, pContext);
      AudioAlgo_setCtrlData(pAlgo, pCtrl);

      pContext->pChunkIn = pChunkIn;
      error = AudioBuffer_create(&pContext->buffOut,
                                 nbChannels,
                                 fs,
                                 nbSamples,
                                 ABUFF_FORMAT_TIME,
                                 ABUFF_FORMAT_FIXED16,
                                 ABUFF_FORMAT_NON_INTERLEAVED,
                                 AUDIO_MEM_RAMINT);
    }
    if (AudioError_isOk(error))
    {
      pCtrl->nbChannels          = nbChannels;
      pCtrl->nbSamplesByChannels = nbSamples;
      pCtrl->fs                  = fs;
      pCtrl->pSamples            = AudioBuffer_getPdata(&pContext->buffOut);
      sfcResetContext(&pContext->sfcContext);
      error = sfcSetContext(&pContext->sfcContext,
                            pBuffIn,
                            &pContext->buffOut,
                            false,
                            1.0f,
                            AudioAlgo_getUtilsHdle(pAlgo));
    }
  }

  return error;
}


static int32_t s_capture_deinit(audio_algo_t *const pAlgo)
{
  captureCtx_t *const pSamplesViewCtx = (captureCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  AudioAlgo_setCtrlData(pAlgo, NULL);
  if (pSamplesViewCtx != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioBuffer_deinit(&pSamplesViewCtx->buffOut);
    AudioAlgo_free(pSamplesViewCtx, AUDIO_MEM_RAMINT);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_capture_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t              error           = AUDIO_ERR_MGNT_NONE;
  captureCtx_t  *const pSamplesViewCtx = (captureCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  captureCtrl_t *const pCtrl           = (captureCtrl_t *)AudioAlgo_getCtrlData(pAlgo);

  if ((pSamplesViewCtx != NULL) && (pCtrl != NULL))
  {
    for (uint16_t indexCh = 0U; indexCh < pCtrl->nbChannels; indexCh++)
    {
      sfcSampleBufferConvert(&pSamplesViewCtx->sfcContext,
                             AudioChunk_getReadPtr(pSamplesViewCtx->pChunkIn, (uint8_t)indexCh, 0UL),
                             AudioBuffer_getPdataCh(&pSamplesViewCtx->buffOut, (uint8_t)indexCh),
                             1,
                             (int)pCtrl->nbSamplesByChannels);
    }
    AudioAlgo_incReadyForProcess(pAlgo);
  }
  else
  {
    error = AUDIO_ERR_MGNT_ERROR;
  }

  return error;
}


static int32_t s_capture_process(audio_algo_t *const pAlgo)
{
  AudioAlgo_incReadyForControlProtected(pAlgo);
  return AUDIO_ERR_MGNT_NONE;
}
