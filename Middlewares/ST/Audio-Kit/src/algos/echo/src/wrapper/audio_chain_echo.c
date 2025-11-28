/**
******************************************************************************
* @file    audio_chain_echo.c
* @author  MCD Application Team
* @brief   produce an echo effect
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
#include "echo/audio_chain_echo.h"
#include "sfc.h"


/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  const echoDynamicConfig_t *pDynamic;

  uint32_t             fs;
  uint32_t             nbSamples;
  uint8_t              nbChannels;
  const audio_chunk_t *pChunkIn;
  const audio_chunk_t *pChunkOut;
  audio_buffer_t       audioBuffer;
  sfcContext_t         sfcInContext;
  sfcContext_t         sfcOutContext;
  uint32_t             idx;
  uint32_t             delaySamples;
  float                gFeebackDirect;
  float                gFeebackDelay;
  float                gLevelDirect;
  float                gLevelDelay;
  #ifdef ECHO_CONFIG_WITH_STEREO_PARAM
  float                stereo;
  #endif
  struct
  {
    float             *pSample;
    int16_t           *pDelay;
    float              prevDelayVal;
  } chCtx[2];

  memPool_t            memPool;
}  echoCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_echo_common =
{
  .pName                     = "echo",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_ALL,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_1CH_2CH,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_1CH_2CH,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Simple echo effect.")
};


/* Private function prototypes -----------------------------------------------*/
static int32_t s_echo_deinit(audio_algo_t    *const pAlgo);
static int32_t s_echo_init(audio_algo_t      *const pAlgo);
static int32_t s_echo_configure(audio_algo_t *const pAlgo);
static int32_t s_echo_dataInOut(audio_algo_t *const pAlgo);


/* Global variables ----------------------------------------------------------*/
audio_algo_cbs_t AudioChainWrp_echo_cbs =
{
  .init                       = s_echo_init,
  .deinit                     = s_echo_deinit,
  .configure                  = s_echo_configure,
  .dataInOut                  = s_echo_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
/**
* @brief Init the algo
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_echo_init(audio_algo_t *const pAlgo)
{
  int32_t                           error          = AUDIO_ERR_MGNT_NONE;
  const echoStaticConfig_t   *const pStaticConfig  = (echoStaticConfig_t  const *)AudioAlgo_getStaticConfig(pAlgo);
  const echoDynamicConfig_t  *const pDynamicConfig = (echoDynamicConfig_t const *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chunk_t              *const pChunkIn       = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t              *const pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t       const *const pBuffIn        = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t       const *const pBuffOut       = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                    const fsIn           = AudioBuffer_getFs(pBuffIn);
  uint8_t                     const nbChannels     = AudioBuffer_getNbChannels(pBuffIn);
  audio_chain_utilities_t    *const pUtilsHandle   = AudioAlgo_getUtilsHdle(pAlgo);
  size_t                            allocSize      = sizeof(echoCtx_t);
  uint32_t                          delaySamples   = 0UL;
  echoCtx_t                        *pCtx           = NULL;
  memPool_t                         memPool        = AUDIO_MEM_UNKNOWN;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "static or dynamic config is missing !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    float const delaySamplesFloat = pStaticConfig->delay * (float)fsIn;

    delaySamples = (uint32_t)delaySamplesFloat;
    allocSize   += (uint32_t)nbChannels * delaySamples * sizeof(int16_t);
    memPool      = (memPool_t)pStaticConfig->ramType;
    pCtx         = (echoCtx_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pCtx == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "context malloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    memset(pCtx, 0, allocSize);
    pCtx->memPool      = memPool;
    pCtx->pDynamic     = pDynamicConfig;
    pCtx->pChunkIn     = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
    pCtx->pChunkOut    = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
    pCtx->nbSamples    = AudioBuffer_getNbSamples(pBuffIn);
    pCtx->nbChannels   = nbChannels;
    pCtx->fs           = fsIn;
    pCtx->delaySamples = delaySamples;
    AudioAlgo_setWrapperContext(pAlgo, pCtx);

    // create pCtx->audioBuffer (for sfcSetContext purpose and for processing purpose) to be compliant with pSample samples format (float, non-interleaved)
    error = AudioBuffer_create(&pCtx->audioBuffer, pCtx->nbChannels, fsIn, pCtx->nbSamples, ABUFF_FORMAT_TIME, ABUFF_FORMAT_FLOAT, ABUFF_FORMAT_NON_INTERLEAVED, memPool);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "AudioBuffer_create(&pCtx->audioBuffer, ...) error !");
    }
  }

  if (AudioError_isOk(error))
  {
    int16_t *pDelay = (int16_t *)&pCtx[1];

    for (uint8_t ch = 0U; ch < pCtx->nbChannels; ch++)
    {
      pCtx->chCtx[ch].pSample = AudioBuffer_getPdataCh(&pCtx->audioBuffer, ch);
      pCtx->chCtx[ch].pDelay  = pDelay;
      pDelay                 += delaySamples;
    }

    error = sfcSetContext(&pCtx->sfcInContext, pBuffIn, &pCtx->audioBuffer, false, 1.0f, pUtilsHandle);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(&pCtx->SfcInContext, ...) error !");
    }
  }

  if (AudioError_isOk(error))
  {
    error = sfcSetContext(&pCtx->sfcOutContext, &pCtx->audioBuffer, pBuffOut, false, 1.0f, pUtilsHandle);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(&pCtx->SfcOutContext, ...) error !");
    }
  }

  if (AudioError_isOk(error))
  {
    error = s_echo_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_echo_deinit(pAlgo);
  }

  return error;
}


/**
* @brief deInit the algo
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_echo_deinit(audio_algo_t *const pAlgo)
{
  int32_t          error = AUDIO_ERR_MGNT_NONE;
  echoCtx_t *const pCtx  = (echoCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pCtx != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pCtx->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioBuffer_deinit(&pCtx->audioBuffer);
    AudioAlgo_free(pCtx, memPool);
  }

  return error;
}


/**
* @brief Configure the algo
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_echo_configure(audio_algo_t *const pAlgo)
{
  int32_t          error = AUDIO_ERR_MGNT_NONE;
  echoCtx_t *const pCtx  = (echoCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pCtx == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "context not allocated !");
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    pCtx->gFeebackDirect  = 1.0f / (pCtx->pDynamic->feedback + 1.0f);
    pCtx->gFeebackDelay   = pCtx->pDynamic->feedback * pCtx->gFeebackDirect;
    pCtx->gLevelDirect    = 1.0f / (pCtx->pDynamic->level + 1.0f);
    pCtx->gLevelDelay     = pCtx->pDynamic->level * pCtx->gLevelDirect;
    pCtx->gFeebackDirect *= 32768.0f; // *32768 for float to int16_t conversion into delay buffer
    pCtx->gLevelDelay    /= 32768.0f; // /32768 for int16_t to float conversion from delay buffer
    #ifdef ECHO_CONFIG_WITH_STEREO_PARAM
    pCtx->stereo         = pCtx->pDynamic->stereo;
    #endif
  }

  return error;
}


/**
* @brief Manage the data in out
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_echo_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t          error        = AUDIO_ERR_MGNT_NONE;
  echoCtx_t *const pCtx         = (echoCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  float     *const pSampleL     = pCtx->chCtx[0].pSample; // float values with [-1.0, +1.0] range
  uint32_t   const delaySamples = pCtx->delaySamples;

  sfcSampleBufferConvert(&pCtx->sfcInContext, AudioChunk_getReadPtr0(pCtx->pChunkIn), pSampleL, (int)pCtx->nbChannels, (int)pCtx->nbSamples);

  if (delaySamples > 0UL)
  {
    int16_t   *const pDelayL        = pCtx->chCtx[0].pDelay;  // int16_t values
    uint32_t   const lastDelayIdx   = delaySamples - 1UL;
    float      const gFeebackDirect = pCtx->gFeebackDirect;   // gFeebackDirect is already multiplied by 32768 for float to int16_t conversion into delay buffer
    float      const gFeebackDelay  = pCtx->gFeebackDelay;
    float      const gLevelDirect   = pCtx->gLevelDirect;
    float      const gLevelDelay    = pCtx->gLevelDelay;      // gLevelDelay is already divided by 32768 for int16_t to float conversion from delay buffer
    float            delayValL, prevDelayValL, delayValTmp;   // float values with [-32768.0, +32767.0] range
    uint32_t         idx;

    prevDelayValL = pCtx->chCtx[0].prevDelayVal;
    idx           = pCtx->idx;
    if (pCtx->nbChannels == 1U)
    {
      for (uint32_t i = 0UL; i < pCtx->nbSamples; i++)
      {
        delayValL     = (float)pDelayL[idx];
        delayValTmp   = (gFeebackDirect * pSampleL[i]) + (gFeebackDelay * prevDelayValL);
        delayValTmp   = (delayValTmp < -32768.0f) ? -32768.0f : delayValTmp;
        delayValTmp   = (delayValTmp > +32767.0f) ? +32767.0f : delayValTmp;
        pDelayL[idx]  = (int16_t)delayValTmp;
        pSampleL[i]   = (gLevelDirect * pSampleL[i]) + (gLevelDelay * delayValL);
        prevDelayValL = delayValL;
        idx           = (idx == lastDelayIdx) ? 0UL : (idx + 1UL);
      }
    }
    else
    {
      #ifdef ECHO_CONFIG_WITH_STEREO_PARAM
      float    const stereo   = pCtx->stereo;
      float          stereoDiff;
      #endif
      float   *const pSampleR = pCtx->chCtx[1].pSample; // float values with [-1.0, +1.0] range
      int16_t *const pDelayR  = pCtx->chCtx[1].pDelay;  // int16_t values
      float          delayValR, prevDelayValR;          // float values with [-32768.0, +32767.0] range

      prevDelayValR = pCtx->chCtx[1].prevDelayVal;
      #ifdef ECHO_CONFIG_WITH_STEREO_PARAM
      stereoDiff    = stereo * (prevDelayValL - prevDelayValR);
      #endif
      for (uint32_t i = 0UL; i < pCtx->nbSamples; i++)
      {
        delayValL     = (float)pDelayL[idx];
        delayValR     = (float)pDelayR[idx];
        #ifdef ECHO_CONFIG_WITH_STEREO_PARAM
        delayValTmp   = (gFeebackDirect * pSampleL[i]) + (gFeebackDelay * (prevDelayValL - stereoDiff));
        delayValTmp   = (delayValTmp < -32768.0f) ? -32768.0f : delayValTmp;
        delayValTmp   = (delayValTmp > +32767.0f) ? +32767.0f : delayValTmp;
        pDelayL[idx]  = (int16_t)delayValTmp;
        delayValTmp   = (gFeebackDirect * pSampleR[i]) + (gFeebackDelay * (prevDelayValR + stereoDiff));
        delayValTmp   = (delayValTmp < -32768.0f) ? -32768.0f : delayValTmp;
        delayValTmp   = (delayValTmp > +32767.0f) ? +32767.0f : delayValTmp;
        pDelayR[idx]  = (int16_t)delayValTmp;
        stereoDiff    = stereo * (delayValL - delayValR);
        pSampleL[i]   = (gLevelDirect * pSampleL[i]) + (gLevelDelay * (delayValL - stereoDiff));
        pSampleR[i]   = (gLevelDirect * pSampleR[i]) + (gLevelDelay * (delayValR + stereoDiff));
        #else
        delayValTmp   = (gFeebackDirect * pSampleL[i]) + (gFeebackDelay * prevDelayValL);
        delayValTmp   = (delayValTmp < -32768.0f) ? -32768.0f : delayValTmp;
        delayValTmp   = (delayValTmp > +32767.0f) ? +32767.0f : delayValTmp;
        pDelayL[idx]  = (int16_t)delayValTmp;
        delayValTmp   = (gFeebackDirect * pSampleR[i]) + (gFeebackDelay * prevDelayValR);
        delayValTmp   = (delayValTmp < -32768.0f) ? -32768.0f : delayValTmp;
        delayValTmp   = (delayValTmp > +32767.0f) ? +32767.0f : delayValTmp;
        pDelayR[idx]  = (int16_t)delayValTmp;
        pSampleL[i]   = (gLevelDirect * pSampleL[i]) + (gLevelDelay * delayValL);
        pSampleR[i]   = (gLevelDirect * pSampleR[i]) + (gLevelDelay * delayValR);
        #endif
        prevDelayValL = delayValL;
        prevDelayValR = delayValR;
        idx           = (idx == lastDelayIdx) ? 0UL : (idx + 1UL);
      }
      pCtx->chCtx[1].prevDelayVal = prevDelayValR;
    }
    pCtx->chCtx[0].prevDelayVal = prevDelayValL;
    pCtx->idx                   = idx;
  }

  sfcSampleBufferConvert(&pCtx->sfcOutContext, pSampleL, AudioChunk_getWritePtr0(pCtx->pChunkOut), (int)pCtx->nbChannels, (int)pCtx->nbSamples);

  return error;
}
