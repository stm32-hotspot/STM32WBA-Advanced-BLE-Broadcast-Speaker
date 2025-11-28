/**
******************************************************************************
* @file    audio_chain_signal_generator.c
* @author  MCD Application Team
* @brief   wrapper of signal_generator algo to match usage inside audio_chain.c
*          remark: this algo has no input samples
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
#include "signal_generator/audio_chain_signal_generator.h"
#include "common/util.h"
#include "audio_assert.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  int32_t              noiseSeed;           // for WHITE_NOISE or PINK_NOISE signal
  biquadFloatContext_t pinkNoiseFilter;     // for PINK_NOISE signal
  uint32_t             fs;                  // for SINE, SQUARE or TRIANGLE signal
  uint32_t             cptSpl;              // for SINE, SQUARE or TRIANGLE signal
  uint32_t             f1;                  // for SINE, SQUARE or TRIANGLE signal
  uint32_t             f2;                  // for SINE, SQUARE or TRIANGLE signal
  double               theta0;              // for SINE, SQUARE or TRIANGLE signal
  double               lastTheta;           // for SINE, SQUARE or TRIANGLE signal
  double               dTheta;              // for SINE, SQUARE or TRIANGLE signal
  float                gain;                // for all signals
  float               *pSplBuf;             // for all signals

  bool                 firstTime;           // for correct sample rate management
  uint64_t             lastNbMsFromStart;   // for correct sample rate management

  // output buffer information stored at init in signal generator context to avoid read them through buffer API on each call
  uint32_t             nbSamplesOut;
  uint32_t             nbChOut;
  uint32_t             splOffsetOut;
  uint32_t             chOffsetOut;

  memPool_t            memPool;
} signal_generator_ctx_t;

/* Private defines -----------------------------------------------------------*/
#ifndef M_PI
  #define M_PI 3.14159265358979323846f
#endif

/* Private macros ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_signal_generator_deinit(audio_algo_t           *const pAlgo);
static int32_t s_signal_generator_init(audio_algo_t             *const pAlgo);
static int32_t s_signal_generator_configure(audio_algo_t        *const pAlgo);
static int32_t s_signal_generator_dataInOut(audio_algo_t        *const pAlgo);
static int32_t s_signal_generator_process(audio_algo_t          *const pAlgo);
static int32_t s_signal_generator_checkConsistency(audio_algo_t *const pAlgo);

static inline int16_t s_saturate_int16(float const sampleFloat) { return (sampleFloat <= -32768.0f)      ? (int16_t) -32768       : ((sampleFloat >= 32767.0f)      ? 32767       : (int16_t)sampleFloat); }
static inline int32_t s_saturate_int32(float const sampleFloat) { return (sampleFloat <= -2147483648.0f) ? (int32_t) -2147483648L : ((sampleFloat >= 2147483647.0f) ? 2147483647L : (int32_t)sampleFloat); }
static inline float   s_saturate_float(float const sampleFloat) { return (sampleFloat <= -1.0f)          ? -1.0f                  : ((sampleFloat >= 1.0f)          ? 1.0f        :          sampleFloat); }

/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_signal_generator_common =
{
  .pName                     = "signal-generator",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_NOT_APPLICABLE,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_NONE,
  .iosIn.nbChan              = (audio_capability_channel_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosIn.fs                  = (audio_capability_fs_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosIn.interleaving        = (audio_capability_interleaving_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosIn.time_freq           = (audio_capability_time_freq_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosIn.type                = (audio_capability_type_t)AUDIO_ALGO_FORMAT_UNDEF,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL_AND_CUSTOM,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,

  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("Chunk Out\0"),
  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Generate a parametric signal"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_INIT_DEFAULT /* Init as default chunk parameters rather to init the NO_CHANGE */
};

audio_algo_cbs_t AudioChainWrp_signal_generator_cbs =
{
  .init                       = s_signal_generator_init,
  .deinit                     = s_signal_generator_deinit,
  .configure                  = s_signal_generator_configure,
  .dataInOut                  = s_signal_generator_dataInOut,
  .process                    = s_signal_generator_process,
  .control                    = NULL,
  .checkConsistency           = s_signal_generator_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_signal_generator_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                                        error          = AUDIO_ERR_MGNT_NONE;
  signal_generator_dynamic_config_t const *const pDynamicConfig = (signal_generator_dynamic_config_t const *)AudioAlgo_getDynamicConfig4Check(pAlgo);
  audio_chunk_t                           *const pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t                          *const pBuffOut       = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                                       fs             = AudioBuffer_getFs(pBuffOut);

  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "dynamic config is missing!");
    error = AUDIO_ERR_MGNT_INIT;
  }
  if (AudioError_isOk(error))
  {
    switch (pDynamicConfig->signalType)
    {
      case WHITE_NOISE:
      case PINK_NOISE:
        break;

      case SINE:
      case SQUARE:
      case TRIANGLE:
        if ((pDynamicConfig->frequency < SIGNAL_GENERATOR_FREQ_MARGIN) || (pDynamicConfig->frequency > ((fs >> 1) - SIGNAL_GENERATOR_FREQ_MARGIN)))
        {
          AudioChainUtils_sendWarningMsg(AudioAlgo_getUtilsHdle(pAlgo), "WARNING:%s:%s:out of range frequency => will be changed by aliasing", pAlgo->pDesc, SIGNAL_GENERATOR_FREQUENCY);
          error = AUDIO_ERR_MGNT_CONFIG_WARNING;
        }
        else
        {
          /* clear config warning */
          AudioChainUtils_sendWarningMsg(AudioAlgo_getUtilsHdle(pAlgo), "WARNING:%s:%s:", pAlgo->pDesc, SIGNAL_GENERATOR_FREQUENCY);
        }
        break;

      default:
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unknown signal type!");
        error = AUDIO_ERR_MGNT_INIT;
        break;
    }
  }

  return error;
}


static int32_t s_signal_generator_init(audio_algo_t *const pAlgo)
{
  int32_t                                       error         = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t                          *const pChunkOut     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  signal_generator_static_config_t const *const pStaticConfig = (signal_generator_static_config_t const *)AudioAlgo_getStaticConfig(pAlgo);

  if (pStaticConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "static config is missing!");
    error = AUDIO_ERR_MGNT_CONFIG;
  }

  if (AudioError_isOk(error))
  {
    if (pChunkOut == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "no output chunk!");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }

  if (AudioError_isOk(error))
  {
    audio_buffer_t *const pBuffOut     = AudioChunk_getBuffInfo(pChunkOut);
    uint32_t        const nbSamplesOut = AudioBuffer_getNbElements(pBuffOut);
    size_t          const allocSize    = sizeof(signal_generator_ctx_t) + ((size_t)nbSamplesOut * sizeof(float));
    memPool_t       const memPool      = (memPool_t)pStaticConfig->ramType;
    void           *const pAlloc       = AudioAlgo_malloc(allocSize, memPool);

    if (pAlloc == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "signal generator context malloc failed!");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
    else
    {
      signal_generator_ctx_t *pSignalGeneratorCtx;

      memset(pAlloc, 0, allocSize);

      pSignalGeneratorCtx               = (signal_generator_ctx_t *)pAlloc;
      pSignalGeneratorCtx->memPool      = memPool;
      pSignalGeneratorCtx->nbSamplesOut = nbSamplesOut;
      pSignalGeneratorCtx->nbChOut      = (uint32_t)AudioBuffer_getNbChannels(pBuffOut);
      pSignalGeneratorCtx->splOffsetOut = AudioBuffer_getSamplesOffset(pBuffOut);
      pSignalGeneratorCtx->chOffsetOut  = AudioBuffer_getChannelsOffset(pBuffOut);
      pSignalGeneratorCtx->pSplBuf      = (float *)((uint8_t *)pAlloc + sizeof(signal_generator_ctx_t));
      pSignalGeneratorCtx->fs           = AudioBuffer_getFs(pBuffOut);
      pSignalGeneratorCtx->lastTheta    = 0.0;
      pSignalGeneratorCtx->firstTime    = true;
      AudioAlgo_setWrapperContext(pAlgo, pSignalGeneratorCtx);

      /* signal generator chunk out ptr update must be managed by signal generator itself: it can't be automatic */
      AudioAlgo_setManualPtrUpdate(pAlgo);

      error = s_signal_generator_configure(pAlgo);
    }
  }

  if (AudioError_isError(error))
  {
    s_signal_generator_deinit(pAlgo);
  }

  return error;
}


static int32_t s_signal_generator_deinit(audio_algo_t *const pAlgo)
{
  int32_t                                        error               = AUDIO_ERR_MGNT_NONE;
  signal_generator_ctx_t                  *const pSignalGeneratorCtx = (signal_generator_ctx_t *)AudioAlgo_getWrapperContext(pAlgo);
  signal_generator_dynamic_config_t const *const pDynamicConfig      = (signal_generator_dynamic_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);

  if (pSignalGeneratorCtx != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pSignalGeneratorCtx->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    if (pDynamicConfig != NULL)
    {
      if ((signalType_t)pDynamicConfig->signalType != PINK_NOISE)
      {
        biquadFloatDeInit(&pSignalGeneratorCtx->pinkNoiseFilter);
      }
    }
    AudioAlgo_free(pSignalGeneratorCtx, memPool);
  }

  return error;
}


static int32_t s_signal_generator_configure(audio_algo_t *const pAlgo)
{
  int32_t                                  error               = AUDIO_ERR_MGNT_NONE;
  signal_generator_dynamic_config_t *const pDynamicConfig      = (signal_generator_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  signal_generator_ctx_t            *const pSignalGeneratorCtx = (signal_generator_ctx_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint32_t                                 freqPgcd;

  if (pSignalGeneratorCtx == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "context alloc failed!");
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "dynamic config is missing!");
    error = AUDIO_ERR_MGNT_CONFIG;
  }

  if (AudioError_isOk(error))
  {
    signalType_t const signalType = (signalType_t)pDynamicConfig->signalType;

    pSignalGeneratorCtx->gain = powf(10.0f, pDynamicConfig->gain / 20.0f);  /*cstat !MISRAC2012-Rule-22.8 no issue with powf(10, ...) => errno check is useless*/

    if ((signalType != PINK_NOISE) && (pSignalGeneratorCtx->pinkNoiseFilter.pBiquadMem != NULL))
    {
      biquadFloatDeInit(&pSignalGeneratorCtx->pinkNoiseFilter);
    }
    switch (signalType)
    {
      case WHITE_NOISE:
        pSignalGeneratorCtx->lastTheta = 0.0;
        break;

      case PINK_NOISE:
      {
        // see https://qastack.fr/signals/322/pink-1-f-pseudo-random-noise-generation
        // then with matlab:
        //   B = [0.049922035, -0.095993537, 0.050612699, -0.004408786];
        //   A = [1, -2.494956002, 2.017265875, -0.522189400];
        //   [sos, g] = tf2sos(B, A, 'up');
        //   sos(1, 1:3) = sos(1, 1:3) * g;
        //   fprintf(1, '%.10f, %.10f, %.10f, %.10f, %.10f\n', sos(1,1), sos(1,2), sos(1,3), sos(1,5), sos(1,6));
        //   fprintf(1, '%.10f, %.10f, %.10f, %.10f, %.10f\n', sos(2,1), sos(2,2), sos(2,3), sos(2,5), sos(2,6));
        static const biquadCellFloat_t pinkNoiseCoef[] =
        {
          {0.0499220350, -0.0053906260, 0.0000000000, -0.5559452594, 0.0000000000},
          {1.0000000000, -1.8148881760, 0.8178616053, -1.9390107426, 0.9392820448}
        };
        static const biquadFloat_t pinkNoiseBiquad =
        {
          sizeof(pinkNoiseCoef) / sizeof(pinkNoiseCoef[0]),
          pinkNoiseCoef
        };

        if (pSignalGeneratorCtx->pinkNoiseFilter.pBiquadMem == NULL)
        {
          // do not initialize a new biquad filter if it was already initialized
          error = biquadFloatInit(&pSignalGeneratorCtx->pinkNoiseFilter, &pinkNoiseBiquad, 1, 1, 1, pSignalGeneratorCtx->memPool);
        }
        pSignalGeneratorCtx->lastTheta = 0.0;
        break;
      }

      case SINE:
      case SQUARE:
      case TRIANGLE:
      {
        uint32_t configFreq = pDynamicConfig->frequency;

        configFreq                  = configFreq % pSignalGeneratorCtx->fs;
        configFreq                  = (configFreq < (pSignalGeneratorCtx->fs >> 1))                                  ? configFreq : (pSignalGeneratorCtx->fs - configFreq);
        configFreq                  = (configFreq > SIGNAL_GENERATOR_FREQ_MARGIN)                                    ? configFreq : SIGNAL_GENERATOR_FREQ_MARGIN;
        configFreq                  = (configFreq < ((pSignalGeneratorCtx->fs >> 1) - SIGNAL_GENERATOR_FREQ_MARGIN)) ? configFreq : ((pSignalGeneratorCtx->fs >> 1) - SIGNAL_GENERATOR_FREQ_MARGIN);
        freqPgcd                    = util_pgcd(configFreq, pSignalGeneratorCtx->fs);
        freqPgcd                    = (freqPgcd == 0UL) ? 1UL : freqPgcd;   // MISRAC: avoid division by 0 (but it can't happen)
        pSignalGeneratorCtx->f1     = configFreq / freqPgcd;
        pSignalGeneratorCtx->f2     = pSignalGeneratorCtx->fs / freqPgcd;
        pSignalGeneratorCtx->dTheta = 2.0 * M_PI * (double)pSignalGeneratorCtx->f1 / (double)pSignalGeneratorCtx->f2;
        pSignalGeneratorCtx->cptSpl = 0UL;
        pSignalGeneratorCtx->theta0 = pSignalGeneratorCtx->lastTheta + pSignalGeneratorCtx->dTheta;   /* preserves continuity when user changes frequency */
        if (configFreq != pDynamicConfig->frequency)
        {
          AudioAlgo_trace(pAlgo, TRACE_LVL_WARNING, NULL, 0, "out of range frequency changed from %d Hz to %d Hz by aliasing", pDynamicConfig->frequency, configFreq);
          error = AUDIO_ERR_MGNT_CONFIG_WARNING;
        }
        break;
      }

      default:
        break;
    }
  }

  return error;
}


static int32_t s_signal_generator_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                        error               = AUDIO_ERR_MGNT_NONE;
  signal_generator_ctx_t  *const pSignalGeneratorCtx = (signal_generator_ctx_t *)AudioAlgo_getWrapperContext(pAlgo);
  audio_chunk_t           *const pChunkOut           = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_chain_utilities_t *const pUtils              = AudioAlgo_getUtilsHdle(pAlgo);
  uint64_t                 const nbMsFromStart       = pUtils->getNbMsFromStart();
  bool                           readyForProcess     = false;

  /*
  signal generator produces samples without input
  thus sample duration is not managed by audioChain
  thus it must be managed internally
  */
  if (pSignalGeneratorCtx->firstTime)
  {
    pSignalGeneratorCtx->firstTime = false;
    readyForProcess                = true;
  }
  else
  {
    uint32_t const nElements = AudioBuffer_getNbElements(AudioChunk_getBuffInfo(pChunkOut));
    uint64_t const nbMs64    = nbMsFromStart - pSignalGeneratorCtx->lastNbMsFromStart;
    uint32_t const nbMs32    = (uint32_t)nbMs64;  // we are sure nbMs64 doesn't exceed 32 bits

    if ((nbMs32 * pSignalGeneratorCtx->fs) >= (1000UL * nElements))
    {
      /* number of elapsed second: nbMs / 1000 >= nElements / pSignalGeneratorCtx->fs */
      readyForProcess = true;
    }
  }

  if (readyForProcess)
  {
    pSignalGeneratorCtx->lastNbMsFromStart = nbMsFromStart;

    /* no need to disable irq because process task is lower priority */
    AudioAlgo_incReadyForProcess(pAlgo);

    /* chunk write ptr update can't be managed by audioChain itself => do it here */
    error = AudioChunk_defaultUpdateWritePtr(pChunkOut);
  }

  return error;
}


static int32_t s_signal_generator_process(audio_algo_t *const pAlgo)
{
  signal_generator_dynamic_config_t const *const pDynamicConfig      = (signal_generator_dynamic_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chunk_t                     const *const pChunkOut           = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  void                                    *const pSplOutVoid         = AudioChunk_getWritePtr0(pChunkOut);
  signal_generator_ctx_t                  *const pSignalGeneratorCtx = (signal_generator_ctx_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint8_t                                  const signalType          = pDynamicConfig->signalType;
  float                                          gainFloat           = pSignalGeneratorCtx->gain; /* initial gain value: will be merged with other gains depending on signal type and sample type */
  uint32_t                                       n;
  double                                         theta;
  uint32_t                                       spl, ch;

  switch (signalType)
  {
    case WHITE_NOISE:
      for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
      {
        // generate white noise
        pSignalGeneratorCtx->noiseSeed    = (pSignalGeneratorCtx->noiseSeed * 1103515245L) + 12345L;
        pSignalGeneratorCtx->pSplBuf[spl] = (float)pSignalGeneratorCtx->noiseSeed;
      }
      /* additional gain to normalize noise between -1.0 and 1.0 */
      gainFloat /= 2147483648.0f;
      break;

    case PINK_NOISE:
      for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
      {
        // generate white noise
        pSignalGeneratorCtx->noiseSeed    = (pSignalGeneratorCtx->noiseSeed * 1103515245L) + 12345L;
        pSignalGeneratorCtx->pSplBuf[spl] = (float)pSignalGeneratorCtx->noiseSeed;
      }
      /* additional gain to normalize noise between -1.0 and 1.0 */
      gainFloat /= 2147483648.0f;
      /* filter white noise to obtain pink noise */
      biquadFloatProcess(&pSignalGeneratorCtx->pinkNoiseFilter, pSignalGeneratorCtx->pSplBuf, pSignalGeneratorCtx->pSplBuf, (int)pSignalGeneratorCtx->nbSamplesOut);
      break;

    case SINE:
    case SQUARE:
    case TRIANGLE:
      /* common part for SINE, SQUARE & TRIANGLE of theta angle computation depending on cptSpl */
      //n     = (uint32_t)(((uint64_t)pSignalGeneratorCtx->cptSpl * (uint64_t)pSignalGeneratorCtx->f1) % pSignalGeneratorCtx->f2);
      n     = (pSignalGeneratorCtx->cptSpl * pSignalGeneratorCtx->f1) % pSignalGeneratorCtx->f2;
      theta = pSignalGeneratorCtx->theta0 + (2.0 * M_PI * (double)n / (double)pSignalGeneratorCtx->f2);
      if (theta >= (2.0 * M_PI))
      {
        theta -= 2.0 * M_PI;
      }
      /* theta is now between 0 and 2 * M_PI */
      switch (signalType)
      {
        case SINE:
          for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
          {
            if (theta >= M_PI)
            {
              theta -= 2.0 * M_PI;
            }
            /* theta is now between -M_PI and M_PI */
            pSignalGeneratorCtx->pSplBuf[spl] = arm_sin_f32((float)theta);
            theta += pSignalGeneratorCtx->dTheta;
          }
          /* no additional gain needed: sinus is already between -1.0 and 1.0 */
          break;

        case SQUARE:
          for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
          {
            if (theta >= M_PI)
            {
              theta -= 2.0 * M_PI;
            }
            /* theta is now between -M_PI and M_PI */
            pSignalGeneratorCtx->pSplBuf[spl] = (theta >= 0.0) ? 1.0f : -1.0f;
            theta += pSignalGeneratorCtx->dTheta;
          }
          /* no additional gain needed: square is already between -1.0 and 1.0 */
          break;

        case TRIANGLE:
          for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
          {
            if (theta >= M_PI)
            {
              theta -= 2.0 * M_PI;
            }
            /* theta is now between -M_PI and M_PI */
            if ((theta < (M_PI / 2.0)) && (theta > (-M_PI / 2.0)))
            {
              // -PI/2 to PI/2: theta / (PI/2)
              pSignalGeneratorCtx->pSplBuf[spl] = (float)theta;
            }
            else if (theta > 0.0)
            {
              // PI/2 to PI   : (PI - Theta) / (PI/2)
              pSignalGeneratorCtx->pSplBuf[spl] = (float)(M_PI - theta);
            }
            else
            {
              // -PI to -PI/2 : -(PI + Theta) / (PI/2)
              pSignalGeneratorCtx->pSplBuf[spl] = (float)(-M_PI - theta);
            }
            theta += pSignalGeneratorCtx->dTheta;
          }
          /* additional gain to normalize triangle between -1.0 and 1.0 */
          gainFloat *= 2.0f / M_PI;
          break;

        default:
          /* MISRAC: do nothing */
          break;
      }
      /* cptSpl wrapping management: common for SINE, SQUARE & TRIANGLE */
      pSignalGeneratorCtx->cptSpl    = (pSignalGeneratorCtx->cptSpl + pSignalGeneratorCtx->nbSamplesOut) % pSignalGeneratorCtx->f2;
      pSignalGeneratorCtx->lastTheta = theta - pSignalGeneratorCtx->dTheta;
      break;

    default:
      /* MISRAC: do nothing */
      break;
  }

  switch (AudioBuffer_getType(AudioChunk_getBuffInfo(pChunkOut)))
  {
    case ABUFF_FORMAT_FIXED16:
    {
      int16_t *pSplOutFixed16 = (int16_t *)pSplOutVoid;
      int16_t  sampleFixed16;

      gainFloat *= 32768.0f;  /* additional gain for float to int16_t sample conversion */
      if (pSignalGeneratorCtx->nbChOut == 1UL)
      {
        for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
        {
          pSplOutFixed16[spl * pSignalGeneratorCtx->splOffsetOut] = s_saturate_int16(gainFloat * pSignalGeneratorCtx->pSplBuf[spl]);
        }
      }
      else
      {
        for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
        {
          // same signal for all channels
          sampleFixed16 = s_saturate_int16(gainFloat * pSignalGeneratorCtx->pSplBuf[spl]);
          for (ch = 0UL; ch < pSignalGeneratorCtx->nbChOut; ch++)
          {
            pSplOutFixed16[ch * pSignalGeneratorCtx->chOffsetOut] = sampleFixed16;
          }
          pSplOutFixed16 += pSignalGeneratorCtx->splOffsetOut;
        }
      }
      break;
    }

    case ABUFF_FORMAT_FIXED32:
    {
      int32_t *pSplOutFixed32 = (int32_t *)pSplOutVoid;
      int32_t  sampleFixed32;

      gainFloat *= 2147483648.0f; /* additional gain for float to int32_t samples conversion */
      if (pSignalGeneratorCtx->nbChOut == 1UL)
      {
        for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
        {
          pSplOutFixed32[spl * pSignalGeneratorCtx->splOffsetOut] = s_saturate_int32(gainFloat * pSignalGeneratorCtx->pSplBuf[spl]);
        }
      }
      else
      {
        for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
        {
          // same signal for all channels
          sampleFixed32 = s_saturate_int32(gainFloat * pSignalGeneratorCtx->pSplBuf[spl]);
          for (ch = 0UL; ch < pSignalGeneratorCtx->nbChOut; ch++)
          {
            pSplOutFixed32[ch * pSignalGeneratorCtx->chOffsetOut] = sampleFixed32;
          }
          pSplOutFixed32 += pSignalGeneratorCtx->splOffsetOut;
        }
      }
      break;
    }

    case ABUFF_FORMAT_FLOAT:
    {
      float *pSplOutFloat = (float *)pSplOutVoid;
      float  sampleFloat;

      /* no additional gain needed: no sample conversion */
      if (pSignalGeneratorCtx->nbChOut == 1UL)
      {
        for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
        {
          pSplOutFloat[spl * pSignalGeneratorCtx->splOffsetOut] = s_saturate_float(gainFloat * pSignalGeneratorCtx->pSplBuf[spl]);
        }
      }
      else
      {
        for (spl = 0UL; spl < pSignalGeneratorCtx->nbSamplesOut; spl++)
        {
          // same signal for all channels
          sampleFloat = s_saturate_float(gainFloat * pSignalGeneratorCtx->pSplBuf[spl]);
          for (ch = 0UL; ch < pSignalGeneratorCtx->nbChOut; ch++)
          {
            pSplOutFloat[ch * pSignalGeneratorCtx->chOffsetOut] = sampleFloat;
          }
          pSplOutFloat += pSignalGeneratorCtx->splOffsetOut;
        }
      }
      break;
    }

    default:
      /* MISRAC: do nothing */
      break;
  }

  return AUDIO_ERR_MGNT_NONE;
}


