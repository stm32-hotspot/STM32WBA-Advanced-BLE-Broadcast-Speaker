/**
******************************************************************************
* @file    audio_chain_speex_aec.c
* @author  MCD Application Team
* @brief   wrapper of SPEEX acoustic echo canceler algo to match usage inside audio_chain.c
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
#include "speex/speexdsp_types.h"
#include "common/speexAlloc.h"
#include "audio_chain.h"
#include "speex_aec/audio_chain_speex_aec.h"
#include "passthrough/audio_chain_passThrough.h"
#include "speex_aec/os_support_custom.h"
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"
#include "smallft.h"
#include "filterbank.h"


/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  SpeexPreprocessState             *den;
  SpeexEchoState                   *st;
  aec_speex_static_config_t  const *pStaticConfig;
  aec_speex_dynamic_config_t const *pDynamicConfig;
  uint32_t                          fs;
  uint32_t                          nbSamples;
  uint32_t                          delaySize;
  uint32_t                          bufferSize;
  int16_t                          *pDelayBuff;
  uint32_t                          debugDelayCpt;
  uint32_t                          debugDelayMicNrj;
  uint32_t                          debugDelayMicNrjMax;
  uint32_t                          debugDelayMicNrjMaxIdx;
  uint16_t                         *pDebugDelayAbsMic;
  audio_chunk_t              const *pChunkEcho;
  audio_chunk_t              const *pChunkRef;
  audio_chunk_t                    *pChunkOut;
  #ifdef CONF_DEBUG_ACTIVE
  audio_chunk_t                    *pChunkOutDebug;
  #endif
  memPool_t                         memPool;
} aec_speex_context_t;

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_speexEc_deinit(audio_algo_t                *const pAlgo);
static int32_t s_speexEc_init(audio_algo_t                  *const pAlgo);
static int32_t s_speexEc_dataInOut(audio_algo_t             *const pAlgo);
static int32_t s_speexEc_configure(audio_algo_t             *const pAlgo);
static int32_t s_speexEc_process(audio_algo_t               *const pAlgo);
static int32_t s_speexEc_checkConsistency(audio_algo_t      *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_speex_aec_common =
{
  .pName                     = "speex-aec",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_ALL,
  .chunks_consistency.out    = ABUFF_PARAM_ALL,
  .chunks_consistency.in_out = ABUFF_PARAM_ALL,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_TWO,
  .iosIn.nbChan              = AUDIO_CAPABILITY_1CH,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_8000_16000,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_YES,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("Mic\0Spk\0"),
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("Input signal with echo (to be de-echoed), typically signal coming from microphone\0Echo source, typically signal sent to loudspeaker\0"),

  #ifdef CONF_DEBUG_ACTIVE
  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE_TWO, /* most of the time one, but a second chunk can be used for debug and probing */
  #else
  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  #endif
  .iosOut.nbChan             = AUDIO_CAPABILITY_1CH,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_8000_16000,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_YES,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16,
  #ifdef CONF_DEBUG_ACTIVE
  .iosOut.pPinNames          = "Out\0Dbg\0",
  .iosOut.pDescs             = "Output de-echoed signal\0Debug output (depend on debug_output config)\0",
  #else
  .iosOut.pPinNames          = "Out\0",
  .iosOut.pDescs             = "output de-echoed signal\0",
  #endif

  .misc.pAlgoDesc            = "Acoustic echo cancellation from https://www.speex.org/",
};

audio_algo_cbs_t AudioChainWrp_speex_aec_cbs =
{
  .init                       = s_speexEc_init,
  .deinit                     = s_speexEc_deinit,
  .configure                  = s_speexEc_configure,
  .dataInOut                  = s_speexEc_dataInOut,
  .process                    = s_speexEc_process,
  .control                    = NULL,
  .checkConsistency           = s_speexEc_checkConsistency,
  .isDisabled                 = AudioChainWrp_passThrough_dataInOut,
  .isDisabledCheckConsistency = AudioChainWrp_passThrough_checkConsistency
};


/* Functions Definition ------------------------------------------------------*/
/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_speexEc_init(audio_algo_t *const pAlgo)
{
  int32_t                                 error           = AUDIO_ERR_MGNT_NONE;
  aec_speex_static_config_t  const *const pStaticConfig   = (aec_speex_static_config_t const *)AudioAlgo_getStaticConfig(pAlgo);
  aec_speex_dynamic_config_t const *const pDynamicConfig  = (aec_speex_dynamic_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chunk_t              const *const pChunkEcho      = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_buffer_t             const *const pBuffEcho       = AudioChunk_getBuffInfo(pChunkEcho);
  uint32_t                          const fs              = AudioBuffer_getFs(pBuffEcho);
  uint32_t                          const nbSamples       = AudioBuffer_getNbElements(pBuffEcho);
  uint8_t                           const spleSize        = AudioBuffer_getSampleSize(pBuffEcho);
  uint8_t                           const nbChannels      = AudioBuffer_getNbChannels(pBuffEcho);
  int                                     delay_samples   = 0;
  int                                     tail_length     = 0;
  int                                     delayBufferSize = 0;
  aec_speex_context_t                    *pContext        = NULL;
  memPool_t                               memPool         = AUDIO_MEM_UNKNOWN;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0,  "missing static or dynamic config !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    size_t      allocSize           = sizeof(aec_speex_context_t);
    float const delay_samples_float = pStaticConfig->delay_length * (float)fs;
    float const tail_length_float   = pStaticConfig->echo_length  * (float)fs;

    memPool       = (memPool_t)pStaticConfig->ramType;
    delay_samples = (int)delay_samples_float;
    tail_length   = (int)tail_length_float;
    if (delay_samples > 0)
    {
      // add delay buffer size
      delayBufferSize = (((int)nbSamples + delay_samples) * (int)nbChannels * (int)spleSize);
      allocSize      += (size_t)delayBufferSize;
    }

    #ifdef CONF_DEBUG_DELAY
    if (pStaticConfig->debug_delay != 0U)
    {
      allocSize += (size_t)((fs >> 4) * (size_t)spleSize);
    }
    #endif

    pContext = (aec_speex_context_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pContext == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "aec_speex_context_t Alloc (size %d) failed !", allocSize);
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    uint8_t *ptr = (uint8_t *)pContext + sizeof(aec_speex_context_t);

    memset(pContext, 0, sizeof(aec_speex_context_t));
    pContext->memPool        = memPool;
    pContext->pDynamicConfig = pDynamicConfig;
    pContext->pStaticConfig  = pStaticConfig;
    pContext->pChunkEcho     = pChunkEcho;
    pContext->pChunkRef      = AudioAlgo_getChunkPtrIn(pAlgo,  1U);
    pContext->pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
    #ifdef CONF_DEBUG_ACTIVE
    pContext->pChunkOutDebug = AudioAlgo_getChunkPtrOut(pAlgo, 1U);
    #endif
    pContext->fs             = fs;
    pContext->nbSamples      = nbSamples;
    pContext->delaySize      = (uint32_t)delay_samples * (uint32_t)nbChannels * (uint32_t)spleSize;
    pContext->bufferSize     = AudioBuffer_getBufferSize(AudioChunk_getBuffInfo(pContext->pChunkEcho));

    if (delay_samples > 0)
    {
      pContext->pDelayBuff = (int16_t *)ptr;
      ptr                 += delayBufferSize;
    }

    #ifdef CONF_DEBUG_DELAY
    if (pStaticConfig->debug_delay != 0U)
    {
      pContext->pDebugDelayAbsMic = (uint16_t *)ptr;
    }
    #endif

    AudioAlgo_setWrapperContext(pAlgo, pContext);
  }

  if (AudioError_isOk(error))
  {
    speex_setMemPool(memPool);  // set speex mem pool for speex allocation
    pContext->st = speex_echo_state_init((int)nbSamples, tail_length);
    if (pContext->st == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "speex_echo_state_init alloc error !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error) && (pStaticConfig->preprocess_init != 0U))
  {
    pContext->den = speex_preprocess_state_init((int)nbSamples, (int)pContext->fs);
    if (pContext->den == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "speex_preprocess_state_init alloc error !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    if (speex_echo_ctl(pContext->st, SPEEX_ECHO_SET_SAMPLING_RATE, &pContext->fs) != 0)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "speex_echo_ctl error !");
      error = AUDIO_ERR_MGNT_ERROR;
    }
  }

  if (AudioError_isOk(error))
  {
    error = s_speexEc_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_speexEc_deinit(pAlgo);
  }

  return error;
}


static int32_t s_speexEc_deinit(audio_algo_t *const pAlgo)
{
  aec_speex_context_t *const pContext = (aec_speex_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pContext->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    speex_setMemPool(memPool);  // set speex mem pool for speex deallocation
    if (pContext->den != NULL)
    {
      speex_preprocess_state_destroy(pContext->den);
    }
    if (pContext->st != NULL)
    {
      speex_echo_state_destroy(pContext->st);
    }
    AudioAlgo_free(pContext, memPool);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_speexEc_configure(audio_algo_t *const pAlgo)
{
  int32_t                    error    = AUDIO_ERR_MGNT_NONE;
  aec_speex_context_t *const pContext = (aec_speex_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (AudioError_isOk(error))
  {
    if (pContext->pStaticConfig->preprocess_init)
    {
      if (pContext->pDynamicConfig->AGC_value == 0.0f)
      {
        pContext->den->agc_enabled = 0;
        pContext->den->agc_level   = 0.0f;
      }
      else if ((pContext->pDynamicConfig->AGC_value > 0.0f) && (pContext->pDynamicConfig->AGC_value <= 1.0f))
      {
        pContext->den->agc_enabled = 1;
        pContext->den->agc_level   = pContext->pDynamicConfig->AGC_value * 32768.0f;
      }
      else
      {
        pContext->den->agc_enabled = 0;
        error                      = AUDIO_ERR_MGNT_CONFIG;
      }
      if (AudioError_isOk(error))
      {
        pContext->den->noise_suppress       = (int)pContext->pDynamicConfig->noise_suppress;
        pContext->den->echo_suppress        = (int)pContext->pDynamicConfig->echo_suppress;
        pContext->den->echo_suppress_active = (int)pContext->pDynamicConfig->echo_suppress_active;
        if (pContext->pDynamicConfig->residual_echo_remove)
        {
          error = speex_preprocess_ctl(pContext->den, SPEEX_PREPROCESS_SET_ECHO_STATE, pContext->st);
        }
        else
        {
          error = speex_preprocess_ctl(pContext->den, SPEEX_PREPROCESS_SET_ECHO_STATE, NULL);
        }
      }
      if (AudioError_isError(error))
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "s_speex_ec_configure failed !");
      }
    }
  }
  return error;
}


static int32_t s_speexEc_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                    error    = AUDIO_ERR_MGNT_NONE;
  aec_speex_context_t *const pContext = (aec_speex_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  #ifdef CONF_DEBUG_DELAY
  if (AudioError_isOk(error) && (pContext->pStaticConfig->debug_delay != 0U))
  {
    uint32_t const fs                = pContext->fs;
    uint32_t const toneSize          = (fs >> 4);
    /* ref and echo samples are ABUFF_FORMAT_FIXED16 (i.e. int16_t): see consistency check */
    int16_t       *pEchoSpl          = (int16_t *)AudioChunk_getReadPtr0(pContext->pChunkEcho);
    int16_t       *pRefSpl           = (int16_t *)AudioChunk_getReadPtr0(pContext->pChunkRef);
    uint32_t       echoSamplesOffset = AudioBuffer_getSamplesOffset(AudioChunk_getBuffInfo(pContext->pChunkEcho));
    uint32_t       refSamplesOffset  = AudioBuffer_getSamplesOffset(AudioChunk_getBuffInfo(pContext->pChunkRef));

    for (uint32_t spl = 0UL; spl < pContext->nbSamples; spl++)
    {
      uint32_t absMicIdx     = pContext->debugDelayCpt % toneSize;
      int16_t  echoSample    = *pEchoSpl;
      uint16_t absEchoSample = (uint16_t)((echoSample < 0) ? -echoSample : echoSample);
      int16_t  refSample     = 0;

      if (pContext->debugDelayCpt < toneSize)
      {
        static int16_t const sinTable[16] = {0, 3827, 7071, 9239, 10000, 9239, 7071, 3827, 0, -3827, -7071, -9239, -10000, -9239, -7071, -3827};

        refSample = sinTable[pContext->debugDelayCpt % 16];
      }
      *pRefSpl = refSample;

      if (pContext->debugDelayCpt == 0UL)
      {
        pContext->debugDelayMicNrj       = 0UL;
        pContext->debugDelayMicNrjMax    = 0UL;
        pContext->debugDelayMicNrjMaxIdx = 0UL;
        memset(pContext->pDebugDelayAbsMic, 0, toneSize * sizeof(uint16_t));
      }
      pContext->debugDelayMicNrj            -= (uint32_t)pContext->pDebugDelayAbsMic[absMicIdx];
      pContext->debugDelayMicNrj            += (uint32_t)absEchoSample;
      pContext->pDebugDelayAbsMic[absMicIdx] = absEchoSample;
      if ((pContext->debugDelayMicNrj > pContext->debugDelayMicNrjMax) && (pContext->debugDelayCpt > toneSize))
      {
        pContext->debugDelayMicNrjMax    = pContext->debugDelayMicNrj;
        pContext->debugDelayMicNrjMaxIdx = pContext->debugDelayCpt - toneSize;  // remove toneSize because pContext->debugDelayCpt is at the end of tone window
      }
      pEchoSpl += echoSamplesOffset;
      pRefSpl  += refSamplesOffset;

      pContext->debugDelayCpt++;
      if (pContext->debugDelayCpt == fs)
      {
        pContext->debugDelayCpt = 0UL;
        AudioAlgo_trace(pAlgo, TRACE_LVL_INFO, NULL, 0, "%secho delay estimation = %1.6f s", TRACE_UART_CHARACTER_GREEN, (float)pContext->debugDelayMicNrjMaxIdx / (float)pContext->fs, TRACE_UART_RESET);
      }
    }
  }
  #endif

  if (AudioError_isOk(error))
  {
    /* write input ref signal into delay buffer & update delay buff write ptr */
    if (pContext->pDelayBuff != NULL)
    {
      memmove(pContext->pDelayBuff, (uint8_t *)pContext->pDelayBuff + pContext->bufferSize, (size_t)pContext->delaySize);
      memcpy((uint8_t *)pContext->pDelayBuff + pContext->delaySize, AudioChunk_getReadPtr0(pContext->pChunkRef), pContext->bufferSize);
    }
  }

  #ifdef CONF_DEBUG_ACTIVE
  if (AudioError_isOk(error))
  {
    if (pContext->pDynamicConfig->debug_output != (uint8_t)AC_SPEEX_AEC_DEBUG_OUTPUT_NONE)
    {
      void *pIn = NULL;

      if (pContext->pDynamicConfig->debug_output == (uint8_t)AC_SPEEX_AEC_DEBUG_OUTPUT_DELAYREF)
      {
        if (pContext->pDelayBuff != NULL)
        {
          pIn = pContext->pDelayBuff;
        }
        else
        {
          pIn = AudioChunk_getReadPtr0(pContext->pChunkRef);
        }
      }
      else
      {
        // AC_SPEEX_AEC_DEBUG_OUTPUT_PROCESSED
        pIn = AudioChunk_getWritePtr0(pContext->pChunkOut);
      }
      memcpy(AudioChunk_getWritePtr0(pContext->pChunkOutDebug), pIn, pContext->bufferSize);
    }
  }
  #endif

  if (AudioError_isOk(error))
  {
    AudioAlgo_incReadyForProcess(pAlgo);
  }

  return error;
}


static int32_t s_speexEc_process(audio_algo_t *const pAlgo)
{
  int32_t                    error     = AUDIO_ERR_MGNT_NONE;
  aec_speex_context_t *const pContext  = (aec_speex_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  int16_t       const *const pReadEcho = (int16_t *)AudioChunk_getReadPtr0(pContext->pChunkEcho);
  int16_t       const *const pReadRef  = (pContext->pDelayBuff == NULL) ? (int16_t *)AudioChunk_getReadPtr0(pContext->pChunkRef) : pContext->pDelayBuff;
  int16_t             *const pReadOut  = (int16_t *)AudioChunk_getWritePtr0(pContext->pChunkOut);

  speex_echo_cancellation(pContext->st, pReadEcho, pReadRef, pReadOut);
  if ((pContext->pStaticConfig->preprocess_init != 0U) && (pContext->pDynamicConfig->preprocess_state != 0U))
  {
    speex_preprocess_run(pContext->den, pReadOut);
  }

  return error;
}


static int32_t s_speexEc_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  #ifdef CONF_DEBUG_ACTIVE
  aec_speex_dynamic_config_t const *const pDynamicConfig = (aec_speex_dynamic_config_t const *)AudioAlgo_getDynamicConfig4Check(pAlgo);

  if ((AudioAlgo_getChunkPtrOut(pAlgo, 1U) == NULL) && (pDynamicConfig->debug_output != (uint8_t)AC_SPEEX_AEC_DEBUG_OUTPUT_NONE))
  {
    error = AUDIO_ERR_MGNT_INIT;
  }
  #else
  (void)pAlgo;
  #endif

  return error;
}


