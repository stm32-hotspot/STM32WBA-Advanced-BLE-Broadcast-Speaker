/**
******************************************************************************
* @file    audio_chain_sound_chorus.c
* @author  MCD Application Team
* @brief   produce a chorus effect
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
#include "faust/audio_chain_sound_chorus.h"
#include <assert.h>
#include "common/commonMath.h"
#include "soundLibs/WaveGenerator.h"


/* Private typedef -----------------------------------------------------------*/

#define kFullBufferLength  4096U
#define kBufferLength  (kFullBufferLength - 2U)
#define kMaxPreDelay  2048U
#define kMaxFrequency  5U
#define kWaveTableLength  (512U - 1U)

typedef struct
{
  uint32_t                             szBuffer;
  uint8_t                              nbChannels;
  const audio_chunk_t                 *pChunkIn;
  const audio_chunk_t                 *pChunkOut;
  audio_buffer_t                       soundChorusAudioBuffer;
  sfcContext_t                         sfcInContext;
  sfcContext_t                         sfcOutContext;
  float                               *pScratchSample[2];

  const sound_chorus_dynamic_config_t *pDynamic;
  float                                mLeftBuffer[kFullBufferLength];
  float                                mRightBuffer[kFullBufferLength];
  float                                mSampleRate;
  int32_t                              mCurrTime;
  int32_t                              mTimeOut;
  float_t                              mLFOWave[kWaveTableLength + 1U];
  int32_t                              mWaveLength;
  float                                mBaseFreq;
  float                                mLeftLFOPos;
  float                                mRightLFOPos;
  float                                mPrevLeftDelay;
  float                                mPrevRightDelay;
  memPool_t                            memPool;
} soundChorusCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static int32_t s_soundChorus_deinit(audio_algo_t    *const pAlgo);
static int32_t s_soundChorus_init(audio_algo_t      *const pAlgo);
static int32_t s_soundChorus_dataInOut(audio_algo_t *const pAlgo);


/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_sound_chorus_common =
{
  .pName                     = "faust-chorus",
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

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Simple Chorus Effect."),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("faust")

};

audio_algo_cbs_t AudioChainWrp_sound_chorus_cbs =
{
  .init                       = s_soundChorus_init,
  .deinit                     = s_soundChorus_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_soundChorus_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/

/**
* @brief Load the scratch buffer and convert it
*
* @param pCtx the class context instance
*/
static void s_load_scratch(const soundChorusCtx_t *pCtx)
{
  sfcSampleBufferConvert(&pCtx->sfcInContext, AudioChunk_getReadPtr0(pCtx->pChunkIn), pCtx->pScratchSample[0], (int)pCtx->nbChannels, (int)pCtx->szBuffer);
  if (pCtx->nbChannels == 1U)
  {
    memcpy(pCtx->pScratchSample[1], pCtx->pScratchSample[0], pCtx->szBuffer * sizeof(float));
  }
}


/**
* @brief Write  the scratch buffer and convert it
*
* @param pEchoCnx the class context instance
*/
static void s_save_scratch(const soundChorusCtx_t *pCtx)
{
  sfcSampleBufferConvert(&pCtx->sfcOutContext, pCtx->pScratchSample[0], AudioChunk_getWritePtr0(pCtx->pChunkOut), (int)pCtx->nbChannels, (int)pCtx->szBuffer);
}


/**
* @brief purge the list
*
* @param pCls the class context
*/
static void s_soundChorus_purge(soundChorusCtx_t *pCls)
{
  pCls->mTimeOut = 0;
  float *leftBuffer = pCls->mLeftBuffer;
  float *rightBuffer = pCls->mRightBuffer;
  for (uint32_t i = 0; i < kFullBufferLength; i += 4U)
  {
    *leftBuffer ++ = 0.0f;
    *leftBuffer ++ = 0.0f;
    *leftBuffer ++ = 0.0f;
    *leftBuffer ++ = 0.0f;

    *rightBuffer ++ = 0.0f;
    *rightBuffer ++ = 0.0f;
    *rightBuffer ++ = 0.0f;
    *rightBuffer ++ = 0.0f;
  }

  // - LFOs
  pCls->mLeftLFOPos = 0.0f;
  pCls->mRightLFOPos = (float)pCls->mWaveLength * 0.25F;  // Dephasage  LFO (0.25 = 1/4 de period)
  float frq = ((float)pCls->szBuffer  * (float)pCls->mWaveLength * (float)kMaxFrequency);
  pCls->mBaseFreq       = (frq != 0.0f) ? (pCls->mSampleRate / frq) : 0.0f;
  pCls->mPrevLeftDelay  = pCls->pDynamic->preDelay * (float_t)kMaxPreDelay;
  pCls->mPrevRightDelay = pCls->pDynamic->preDelay * (float_t)kMaxPreDelay;
}


/**
* @brief Transform the buffer
*
* @param pCls the algo instance
*/

static void  plugin_processing_transform(soundChorusCtx_t *pCls)
{
  uint32_t szBuffer = pCls->szBuffer;

  if ((float)pCls->mTimeOut <= (2.0F * pCls->mSampleRate))
  {

    float *samplePtrLeft  = pCls->pScratchSample[0];
    float *samplePtrRight = pCls->pScratchSample[1];

    // - Mirroring
    float *leftBuffer  =  pCls->mLeftBuffer;
    float *rightBuffer =  pCls->mRightBuffer;
    int32_t currTime      =  pCls->mCurrTime;
    const float_t feedback = pCls->pDynamic->feedback;

    float_t leftDelay     = pCls->mPrevLeftDelay;
    float_t rightDelay    = pCls->mPrevRightDelay;

    float_t s0;
    uint32_t t0;
    float_t preDelay = (float)pCls->pDynamic->preDelay * (float)kMaxPreDelay;
    assert((preDelay >= 0.0f) && (preDelay <= (float)kMaxPreDelay));
    /* compute the pre-delay factor f or left and right */

    float_t new_leftDelay = pCls->pDynamic->preDelay * (float_t)kMaxPreDelay; // conversion en samples
    float_t new_rightDelay = pCls->pDynamic->preDelay * (float_t)kMaxPreDelay;

    // - LFOs ; index in  the buffer **/
    float_t chorusAmp = (float)pCls->pDynamic->chorusAmp * (float)((uint32_t)kBufferLength - (uint32_t)kMaxPreDelay - 1U) * 0.5f; // compense LFO amplitude=2

    /* speed of the LFO max ..01  to 5 HZ = kMaxFrequency */
    float_t lfoSpeed = pCls->pDynamic->frequency / pCls->mBaseFreq;


    /* compute the position offset in the LFO, the new_Delay delay is factor of the LFO  */
    t0 = (uint32_t)pCls->mLeftLFOPos;
    s0 = pCls->mLFOWave[ t0 ];
    new_leftDelay += (float)((1.0f + ((pCls->mLeftLFOPos - (float)t0) * (pCls->mLFOWave[ t0 + 1U ] - s0)) + s0) * chorusAmp);

    t0 = (uint32_t)pCls->mRightLFOPos;
    s0 = pCls->mLFOWave[ t0 ];
    new_rightDelay += (float_t)(1.0f + ((pCls->mRightLFOPos - (float)t0) * (pCls->mLFOWave[ t0 + 1U ] - s0)) + s0) * chorusAmp;

    /* Move the LFO of 1 pos */

    pCls->mLeftLFOPos += lfoSpeed;
    if (pCls->mLeftLFOPos >= (float)pCls->mWaveLength)
    {
      pCls->mLeftLFOPos -= (float)pCls->mWaveLength;
    }

    pCls->mRightLFOPos += lfoSpeed;
    if (pCls->mRightLFOPos >= (float)pCls->mWaveLength)
    {
      pCls->mRightLFOPos -= (float)pCls->mWaveLength;
    }


    // - Preprocess
    const float_t oneOverLen = 1.0f / (float_t)szBuffer;
    /* linear interpolation from start sample to last sample */
    const float_t add_LeftDelay = (new_leftDelay - leftDelay) * oneOverLen;
    const float_t add_RightDelay = (new_rightDelay - rightDelay) * oneOverLen;

    float_t outLeftPoint;
    float_t outRightPoint;

    float_t sampleLeft;
    float_t sampleRight;
    float_t leftEcho;
    float_t rightEcho;

    // - Transform
    for (uint32_t tick = 0UL; tick < szBuffer; tick ++)
    {
      // - Calcul des points de delai
      leftDelay += add_LeftDelay;
      outLeftPoint = ((float)currTime - leftDelay);
      if (outLeftPoint < -(float)kBufferLength)
      {
        assert(1 == 0);
      }

      if (outLeftPoint < 0.0f)
      {
        outLeftPoint += (float)kBufferLength;
      }

      rightDelay += add_RightDelay;
      outRightPoint = (float)currTime - rightDelay;
      if (outRightPoint < -(float)kBufferLength)
      {
        assert(1 == 0);
      }

      if (outRightPoint < 0.0f)
      {
        outRightPoint += (float)kBufferLength;
      }


      sampleLeft = *samplePtrLeft;
      sampleRight = *samplePtrRight;

      /* - ring Buffer  */
      if (currTime >= (int32_t)kBufferLength)
      {
        leftBuffer[ kBufferLength ] = sampleLeft; // recopie le sample pour l'interpolation
        rightBuffer[ kBufferLength ] = sampleRight; // recopie le sample pour l'interpolation
        currTime -= (int32_t)kBufferLength;
        assert(currTime < (int32_t)kBufferLength);
      }
      leftBuffer[ currTime ] = sampleLeft;
      rightBuffer[ currTime ] = sampleRight;
      currTime ++;

      /* - Interpolation lineaire*/
      t0 = (uint32_t)outLeftPoint;
      s0 = leftBuffer[ t0 ];
      leftEcho = ((outLeftPoint - (float)t0) * (leftBuffer[ t0 + 1U ] - s0)) + s0;

      t0 = (uint32_t)outRightPoint;
      s0 = rightBuffer[ t0 ];
      rightEcho = ((outRightPoint - (float)t0) * (rightBuffer[ t0 + 1U ] - s0)) + s0;

      // - Ecriture
      *samplePtrLeft ++  = (sampleLeft + (leftEcho * feedback));
      *samplePtrRight ++ = (sampleRight + (rightEcho * feedback));
    }

    // - Finishing
    pCls->mCurrTime = currTime;
    pCls->mPrevLeftDelay = leftDelay;
    pCls->mPrevRightDelay = rightDelay;
  }

}


/**
* @brief Init the algo
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_soundChorus_init(audio_algo_t *const pAlgo)
{
  int32_t                                    error          = AUDIO_ERR_MGNT_NONE;
  sound_chorus_static_config_t  const *const pStaticConfig  = (sound_chorus_static_config_t  const *)AudioAlgo_getStaticConfig(pAlgo);
  sound_chorus_dynamic_config_t const *const pDynamicConfig = (sound_chorus_dynamic_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chunk_t                       *const pChunkIn       = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t                       *const pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t                const *const pBuffIn        = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t                const *const pBuffOut       = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                             const fsIn           = AudioBuffer_getFs(pBuffIn);
  audio_chain_utilities_t             *const pUtilsHandle   = AudioAlgo_getUtilsHdle(pAlgo);
  soundChorusCtx_t                          *pChorusCnx     = NULL;
  memPool_t                                  memPool        = AUDIO_MEM_UNKNOWN;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "static or dynamic config is missing!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool    = (memPool_t)pStaticConfig->ramType;
    pChorusCnx = (soundChorusCtx_t *)AudioAlgo_malloc(sizeof(soundChorusCtx_t), memPool);
    if (pChorusCnx == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "soundChorus context malloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    AudioAlgo_setWrapperContext(pAlgo, pChorusCnx);
    memset(pChorusCnx, 0, sizeof(soundChorusCtx_t));
    pChorusCnx->memPool     = memPool;
    pChorusCnx->pDynamic    = pDynamicConfig;
    pChorusCnx->szBuffer    = AudioBuffer_getNbSamples(pBuffIn);
    pChorusCnx->nbChannels  = AudioBuffer_getNbChannels(pBuffIn);
    pChorusCnx->pChunkIn    = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
    pChorusCnx->pChunkOut   = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
    pChorusCnx->mSampleRate = (float)fsIn;
    pChorusCnx->mWaveLength = (int32_t)kWaveTableLength;
    generateWave(pChorusCnx ->mLFOWave, (int32_t)(pChorusCnx ->mWaveLength + 1), (int16_t)kSinus, (int16_t)1, 0.8F);

    s_soundChorus_purge(pChorusCnx);

    // create pCtx->soundChorusAudioBuffer (for sfcSetContext purpose and for soundChorus processing purpose) to be compliant with pScratchSample samples format (float, non-interleaved, stereo)
    error = AudioBuffer_create(&pChorusCnx->soundChorusAudioBuffer, 2U, fsIn, pChorusCnx->szBuffer, ABUFF_FORMAT_TIME, ABUFF_FORMAT_FLOAT, ABUFF_FORMAT_NON_INTERLEAVED, memPool);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "AudioBuffer_create(&pChorusCnx->soundChorusAudioBuffer, ...) error !");
    }
  }

  if (AudioError_isOk(error))
  {
    pChorusCnx->pScratchSample[0] = AudioBuffer_getPdataCh(&pChorusCnx->soundChorusAudioBuffer, 0);
    pChorusCnx->pScratchSample[1] = AudioBuffer_getPdataCh(&pChorusCnx->soundChorusAudioBuffer, 1);

    error = sfcSetContext(&pChorusCnx->sfcInContext, pBuffIn, &pChorusCnx->soundChorusAudioBuffer, false, 1.0f, pUtilsHandle);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(&pChorusCnx->SfcInContext, ...) error !");
    }
  }

  if (AudioError_isOk(error))
  {
    error = sfcSetContext(&pChorusCnx->sfcOutContext, &pChorusCnx->soundChorusAudioBuffer, pBuffOut, false, 1.0f, pUtilsHandle);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(&pChorusCnx->SfcOutContext, ...) error !");
    }
  }

  if (AudioError_isError(error))
  {
    s_soundChorus_deinit(pAlgo);
  }

  return error;
}


/**
* @brief deInit the algo
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_soundChorus_deinit(audio_algo_t *const pAlgo)
{
  int32_t                 error      = AUDIO_ERR_MGNT_NONE;
  soundChorusCtx_t *const pChorusCnx = (soundChorusCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pChorusCnx != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pChorusCnx->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioBuffer_deinit(&pChorusCnx->soundChorusAudioBuffer);
    AudioAlgo_free(pChorusCnx, memPool);
  }

  return error;
}


/**
* @brief Manage the data in out
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_soundChorus_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                 error      = AUDIO_ERR_MGNT_NONE;
  soundChorusCtx_t *const pChorusCnx = (soundChorusCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  s_load_scratch(pChorusCnx);
  if (pChorusCnx->pDynamic->bypass == 0U)
  {
    plugin_processing_transform(pChorusCnx);
  }
  s_save_scratch(pChorusCnx);
  return error;
}
