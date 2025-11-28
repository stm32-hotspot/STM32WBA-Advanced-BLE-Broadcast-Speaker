/**
******************************************************************************
* @file    audio_chain_spectrum.c
* @author  MCD Application Team
* @brief   wrapper of spectrum algo (spectrum estimation from input time samples) to match usage inside audio_chain.c
*          remark: this algo has no output samples
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
#include "spectrum/audio_chain_spectrum.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  arm_rfft_fast_instance_f32  fftHandle;            // nbChIn+1 : nbChIn for input, "+1" for output (same samples on all output channels)
  float                      *pFftWindow;           // fftLength
  float                      *pFftSamplesWin;       // nbChIn*fftLength
  float                      *pFftBands;            // fftLength
  float                      *pSquareMag[2];        // 2 sets of nbChIn*nbBands : 1 set for flip, 1 set for flop
  uint32_t                    splIdx;               // index into pFftSamplesWin
  uint8_t                     flipFlop;             // flip for process, flop for control
  uint32_t                    nbAccSpectrum;

  // input buffer information stored at init in spectrum context to avoid read them through buffer API on each call
  uint8_t                     nbChIn;
  uint32_t                    nbSamplesIn;
  uint32_t                    splOffsetIn;
  uint32_t                    chOffsetIn;
  audio_buffer_type_t         typeIn;
  memPool_t                   memPool;
} spectrumCtx_t;

typedef union
{
  int16_t const *pInt16;
  int32_t const *pInt32;
  float   const *pFloat;
  void    const *pVoid;
} inputSample_ptr_t;

/* Private defines -----------------------------------------------------------*/
#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_spectrum_deinit(audio_algo_t                *const pAlgo);
static int32_t s_spectrum_init(audio_algo_t                  *const pAlgo);
static int32_t s_spectrum_dataInOut(audio_algo_t             *const pAlgo);
static int32_t s_spectrum_process(audio_algo_t               *const pAlgo);

static void s_windowing(spectrumCtx_t *const pSpectrumCtx, inputSample_ptr_t *const pIn, uint32_t const fftLength, uint32_t const nbSamples);
static void s_fftSquareMagAccumulate(spectrumCtx_t *const pSpectrumCtx, float *const pSquareMagIn, float *const pSquareMagOut, uint32_t const fftLength, bool const accumulate);

/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_spectrum_common =
{
  .pName                     = "spectrum",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_NOT_APPLICABLE,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL_AND_CUSTOM,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_NONE,
  .iosOut.nbChan             = (audio_capability_channel_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosOut.fs                 = (audio_capability_fs_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosOut.interleaving       = (audio_capability_interleaving_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosOut.time_freq          = (audio_capability_time_freq_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosOut.type               = (audio_capability_type_t)AUDIO_ALGO_FORMAT_UNDEF,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Compute squared magnitude spectrum of an input signal"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_DISABLE_AUTO_MOUNT
};

audio_algo_cbs_t AudioChainWrp_spectrum_cbs =
{
  .init                       = s_spectrum_init,
  .deinit                     = s_spectrum_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_spectrum_dataInOut,
  .process                    = s_spectrum_process,
  .control                    = NULL,                                     // user defined routine
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_spectrum_init(audio_algo_t *const pAlgo)
{
  int32_t                               error         = AUDIO_ERR_MGNT_NONE;
  spectrum_static_config_t const *const pStaticConfig = (spectrum_static_config_t const *)AudioAlgo_getStaticConfig(pAlgo);
  audio_chunk_t                  *const pChunkIn      = AudioAlgo_getChunkPtrIn(pAlgo, 0U);

  if (pStaticConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static conf !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    if (pChunkIn == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "no input chunk!");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }

  if (AudioError_isOk(error))
  {
    switch (pStaticConfig->fftLength)
    {
      case 64UL:
      case 128UL:
      case 256UL:
      case 512UL:
      case 1024UL:
      case 2048UL:
      case 4096UL:
        break;
      default:
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported FFT length");
        error = AUDIO_ERR_MGNT_INIT;
        break;
    }
  }

  if (AudioError_isOk(error))
  {
    audio_buffer_t *const pBuffIn    = AudioChunk_getBuffInfo(pChunkIn);
    uint8_t         const nbChIn     = AudioBuffer_getNbChannels(pBuffIn);
    uint32_t        const nbBands    = (pStaticConfig->fftLength / 2UL) + 1UL;
    size_t          const allocSize1 = sizeof(spectrumCtx_t);                                             // pSpectrumCtx
    size_t          const allocSize2 = sizeof(spectrumCtrl_t);                                            // pSpectrumCtrl
    size_t          const allocSize3 = (size_t)pStaticConfig->fftLength * sizeof(float);                  // pSpectrumCtx->pFftWindow
    size_t          const allocSize4 = (size_t)nbChIn * (size_t)pStaticConfig->fftLength * sizeof(float); // pSpectrumCtx->pFftSamplesWin
    size_t          const allocSize5 = (size_t)pStaticConfig->fftLength * sizeof(float);                  // pSpectrumCtx->pFftBands
    size_t          const allocSize6 = (size_t)nbChIn * (size_t)nbBands * sizeof(float);                  // pSpectrumCtx->pSquareMag
    size_t          const allocSize  = allocSize1 + allocSize2 + allocSize3 + allocSize4 + allocSize5 + (2UL * allocSize6);
    memPool_t       const memPool    = (memPool_t)pStaticConfig->ramType;
    void           *const pAlloc     = AudioAlgo_malloc(allocSize, memPool);

    if (pAlloc == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "spectrum context malloc failed!");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
    else
    {
      uint8_t        *pByte = (uint8_t *)pAlloc;
      double          dTheta, theta;
      spectrumCtx_t  *pSpectrumCtx;
      spectrumCtrl_t *pSpectrumCtrl;

      memset(pAlloc, 0, allocSize);

      pSpectrumCtx                    = (spectrumCtx_t *)pByte;  pByte += allocSize1;
      pSpectrumCtrl                   = (spectrumCtrl_t *)pByte; pByte += allocSize2;
      pSpectrumCtx->pFftWindow        = (float *)pByte;          pByte += allocSize3;
      pSpectrumCtx->pFftSamplesWin    = (float *)pByte;          pByte += allocSize4;
      pSpectrumCtx->pFftBands         = (float *)pByte;          pByte += allocSize5;
      pSpectrumCtx->pSquareMag[0]     = (float *)pByte;          pByte += allocSize6;
      pSpectrumCtx->pSquareMag[1]     = (float *)pByte;

      pSpectrumCtx->memPool           = memPool;
      pSpectrumCtx->nbSamplesIn       = AudioBuffer_getNbElements(pBuffIn);
      pSpectrumCtx->nbChIn            = nbChIn;
      pSpectrumCtx->splOffsetIn       = AudioBuffer_getSamplesOffset(pBuffIn);
      pSpectrumCtx->chOffsetIn        = AudioBuffer_getChannelsOffset(pBuffIn);
      pSpectrumCtx->typeIn            = AudioBuffer_getType(pBuffIn);

      dTheta = 2.0 * M_PI / (double)pStaticConfig->fftLength;
      theta  = 0.0;
      switch (pSpectrumCtx->typeIn)
      {
        case ABUFF_FORMAT_FIXED16:
          for (uint32_t i = 0UL; i < pStaticConfig->fftLength; i++)
          {
            theta                      += dTheta;
            pSpectrumCtx->pFftWindow[i] = (1.0f - arm_cos_f32((float)theta)) / 32768.0f;  // "/ 32768" for int16_t -> float conversion
          }
          break;

        case ABUFF_FORMAT_FIXED32:
          for (uint32_t i = 0UL; i < pStaticConfig->fftLength; i++)
          {
            theta                      += dTheta;
            pSpectrumCtx->pFftWindow[i] = (1.0f - arm_cos_f32((float)theta)) / 2147483648.0f;  // "/ 2147483648" for int32_t -> float conversion
          }
          break;

        case ABUFF_FORMAT_FLOAT:
          for (uint32_t i = 0U; i < pStaticConfig->fftLength; i++)
          {
            theta                      += dTheta;
            pSpectrumCtx->pFftWindow[i] = 1.0f - arm_cos_f32((float)theta);
          }
          break;

        default:
          break;
      }

      if (arm_rfft_fast_init_f32(&pSpectrumCtx->fftHandle, (uint16_t)pStaticConfig->fftLength) != ARM_MATH_SUCCESS)
      {
        error = AUDIO_ERR_MGNT_CMSIS_FFT_INIT;
      }

      if (AudioError_isOk(error))
      {
        AudioAlgo_setWrapperContext(pAlgo, pSpectrumCtx);
        AudioAlgo_setCtrlData(pAlgo, pSpectrumCtrl);
      }
    }
  }

  if (AudioError_isError(error))
  {
    s_spectrum_deinit(pAlgo);
  }

  return error;
}


static int32_t s_spectrum_deinit(audio_algo_t *const pAlgo)
{
  spectrumCtx_t *const pSpectrumCtx = (spectrumCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pSpectrumCtx != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pSpectrumCtx->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pSpectrumCtx, memPool);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_spectrum_dataInOut(audio_algo_t *const pAlgo)
{
  // no need to disable irq because process task is lower priority
  AudioAlgo_incReadyForProcess(pAlgo);

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_spectrum_process(audio_algo_t *const pAlgo)
{
  spectrum_static_config_t const *const pStaticConfig   = (spectrum_static_config_t const *)AudioAlgo_getStaticConfig(pAlgo);
  spectrumCtx_t                  *const pSpectrumCtx    = (spectrumCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  spectrumCtrl_t                 *const pSpectrumCtrl   = (spectrumCtrl_t *)AudioAlgo_getCtrlData(pAlgo);
  float                                *pSquareMagIn    = pSpectrumCtx->pSquareMag[pSpectrumCtx->flipFlop];      // output of last process
  float                          *const pSquareMagOut   = pSpectrumCtx->pSquareMag[1U - pSpectrumCtx->flipFlop]; // output of current process
  audio_chunk_t                  *const pChunkIn        = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_buffer_t                 *const pBuffIn         = AudioChunk_getBuffInfo(pChunkIn);
  uint32_t                        const fs              = AudioBuffer_getFs(pBuffIn);
  uint32_t                              nbSamplesIn     = pSpectrumCtx->nbSamplesIn;
  bool                                  readyForControl = false;
  inputSample_ptr_t                     inputSamplesPtr;

  inputSamplesPtr.pVoid = AudioChunk_getReadPtr0(pChunkIn);
  while (nbSamplesIn > 0UL)
  {
    uint32_t n;

    if ((pSpectrumCtx->splIdx + nbSamplesIn) <= pStaticConfig->fftLength)
    {
      n = nbSamplesIn;
    }
    else
    {
      n = pStaticConfig->fftLength - pSpectrumCtx->splIdx;
    }

    // first part of buffer windowing (before pFftSamplesWin wrap)
    s_windowing(pSpectrumCtx, &inputSamplesPtr, pStaticConfig->fftLength, n);
    pSpectrumCtx->splIdx += n;
    nbSamplesIn          -= n;

    if (pSpectrumCtx->splIdx == pStaticConfig->fftLength)
    {
      // pFftSamplesWin buffer is full
      pSpectrumCtx->splIdx = 0UL;

      // if pSpectrumCtrl->nbAccSpectrum == 0 (it is first time or control has acknowledged and wants to restart accumulation) => it is the first square magnitude => no accumulation
      if (pSpectrumCtrl->nbAccSpectrum == 0UL)
      {
        pSpectrumCtx->nbAccSpectrum = 0UL;
      }
      s_fftSquareMagAccumulate(pSpectrumCtx, pSquareMagIn, pSquareMagOut, pStaticConfig->fftLength, (pSpectrumCtx->nbAccSpectrum > 0UL));

      // if more than 1 spectrum is computed in current process, from second spectrum, square magnitude input is square magnitude output
      pSquareMagIn = pSquareMagOut;

      pSpectrumCtx->nbAccSpectrum++;
      readyForControl = true;
    }
  }

  if (readyForControl)
  {
    // we change of output buffer at each end of process to give time to control to get last square magnitude accumulation
    audio_chain_utilities_t *const pUtilsHdle = AudioAlgo_getUtilsHdle(pAlgo);

    if ((pUtilsHdle != NULL) && (pUtilsHdle->mutexLock != NULL))
    {
      pUtilsHdle->mutexLock();
    }
    pSpectrumCtrl->pSquareMag        = pSquareMagOut;
    pSpectrumCtrl->nbChannels        = pSpectrumCtx->nbChIn;
    pSpectrumCtrl->fftLength         = pStaticConfig->fftLength;
    pSpectrumCtrl->nbAccSpectrum     = pSpectrumCtx->nbAccSpectrum;
    pSpectrumCtrl->normalizationCoef = 4.0f / ((float)pSpectrumCtrl->nbAccSpectrum * (float)pStaticConfig->fftLength * (float)pStaticConfig->fftLength);
    pSpectrumCtrl->fs                = fs;
    pSpectrumCtx->flipFlop           = 1U - pSpectrumCtx->flipFlop;
    if ((pUtilsHdle != NULL) && (pUtilsHdle->mutexUnlock != NULL))
    {
      pUtilsHdle->mutexUnlock();
    }

    AudioAlgo_incReadyForControl(pAlgo);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static void s_windowing(spectrumCtx_t *const pSpectrumCtx, inputSample_ptr_t *const pIn, uint32_t const fftLength, uint32_t const nbSamples)
{
  inputSample_ptr_t  inputSamplesPtr = *pIn;
  float             *pSplOut         = &pSpectrumCtx->pFftSamplesWin[pSpectrumCtx->splIdx];
  float const *const pWin            = &pSpectrumCtx->pFftWindow[pSpectrumCtx->splIdx];
  uint32_t           spl, ch;

  switch (pSpectrumCtx->typeIn)
  {
    case ABUFF_FORMAT_FIXED16:
      for (spl = 0UL; spl < nbSamples; spl++)
      {
        int16_t const *const pSplInSave  = inputSamplesPtr.pInt16;
        float         *const pSplOutSave = pSplOut;
        float          const win         = pWin[spl];

        // input windowing (nbChIn channels)
        for (ch = 0U; ch < pSpectrumCtx->nbChIn; ch++)
        {
          *pSplOut                = (float)(*inputSamplesPtr.pInt16) * win;
          inputSamplesPtr.pInt16 += pSpectrumCtx->chOffsetIn;
          pSplOut                += fftLength;
        }
        inputSamplesPtr.pInt16 = pSplInSave  + pSpectrumCtx->splOffsetIn;
        pSplOut                = pSplOutSave + 1;
      }
      break;

    case ABUFF_FORMAT_FIXED32:
      for (spl = 0UL; spl < nbSamples; spl++)
      {
        int32_t const *const pSplInSave  = inputSamplesPtr.pInt32;
        float         *const pSplOutSave = pSplOut;
        float          const win         = pWin[spl];

        // input windowing (nbChIn channels)
        for (ch = 0U; ch < pSpectrumCtx->nbChIn; ch++)
        {
          *pSplOut                = (float)(*inputSamplesPtr.pInt32) * win;
          inputSamplesPtr.pInt32 += pSpectrumCtx->chOffsetIn;
          pSplOut                += fftLength;
        }
        inputSamplesPtr.pInt32 = pSplInSave  + pSpectrumCtx->splOffsetIn;
        pSplOut                = pSplOutSave + 1;
      }
      break;

    case ABUFF_FORMAT_FLOAT:
      for (spl = 0UL; spl < nbSamples; spl++)
      {
        float const *const pSplInSave  = inputSamplesPtr.pFloat;
        float       *const pSplOutSave = pSplOut;
        float        const win         = pWin[spl];

        // input windowing (nbChIn channels)
        for (ch = 0U; ch < pSpectrumCtx->nbChIn; ch++)
        {
          *pSplOut                = (float)(*inputSamplesPtr.pFloat) * win;
          inputSamplesPtr.pFloat += pSpectrumCtx->chOffsetIn;
          pSplOut                += fftLength;
        }
        inputSamplesPtr.pFloat = pSplInSave  + pSpectrumCtx->splOffsetIn;
        pSplOut                = pSplOutSave + 1;
      }
      break;

    default:
      break;
  }
  *pIn = inputSamplesPtr;
}


static void s_fftSquareMagAccumulate(spectrumCtx_t *const pSpectrumCtx, float *const pSquareMagIn, float *const pSquareMagOut, uint32_t const fftLength, bool const accumulate)
{
  uint32_t const nbBands            = ((uint32_t)fftLength / 2UL) + 1UL;
  float         *pSquareMagInLocal  = pSquareMagIn;
  float         *pSquareMagOutLocal = pSquareMagOut;

  for (uint8_t ch = 0U; ch < pSpectrumCtx->nbChIn; ch++)
  {
    // compute FFT
    arm_rfft_fast_f32(&pSpectrumCtx->fftHandle,
                      &pSpectrumCtx->pFftSamplesWin[ch * fftLength],
                      pSpectrumCtx->pFftBands,
                      0U);

    if (accumulate)
    {
      // compute and accumulate spectrum
      pSquareMagOutLocal[0]             = pSquareMagInLocal[0]             + (pSpectrumCtx->pFftBands[0] * pSpectrumCtx->pFftBands[0]);
      pSquareMagOutLocal[nbBands - 1UL] = pSquareMagInLocal[nbBands - 1UL] + (pSpectrumCtx->pFftBands[1] * pSpectrumCtx->pFftBands[1]);
      arm_cmplx_mag_squared_f32(&pSpectrumCtx->pFftBands[2], pSpectrumCtx->pFftBands,     nbBands - 2UL); // use pFftBands as input and output temporary buffer
      arm_add_f32(&pSquareMagInLocal[1], pSpectrumCtx->pFftBands, &pSquareMagOutLocal[1], nbBands - 2UL);
    }
    else
    {
      // compute spectrum
      pSquareMagOutLocal[0]             = pSpectrumCtx->pFftBands[0] * pSpectrumCtx->pFftBands[0];
      pSquareMagOutLocal[nbBands - 1UL] = pSpectrumCtx->pFftBands[1] * pSpectrumCtx->pFftBands[1];
      arm_cmplx_mag_squared_f32(&pSpectrumCtx->pFftBands[2], &pSquareMagOutLocal[1], nbBands - 2UL);
    }

    // next channel
    pSquareMagInLocal  += nbBands;
    pSquareMagOutLocal += nbBands;
  }
}
