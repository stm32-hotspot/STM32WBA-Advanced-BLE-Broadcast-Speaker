/**
  ******************************************************************************
  * @file    rms.c
  * @author  MCD Application Team
  * @brief   rms algo
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
#include <assert.h>
#include <string.h>
/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */
#include "audio_chunk.h"
#include "rms.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct rmsDoubleStruct
{
  double  normalizer;
  double  alpha;       // for RMS computation
  double  lambda;      // for RMS computation
  double *pR2Table;    // RMS^2 (not normalized)
} rmsDouble_t;

typedef struct rmsFloatStruct
{
  float  normalizer;
  float  alpha;     // for RMS computation
  float  lambda;    // for RMS computation
  float *pR2Table;  // RMS^2 (not normalized)
} rmsFloat_t;


typedef struct rmsContextStruct
{
  union
  {
    rmsFloat_t  f32;
    rmsDouble_t f64;
  } data;
  float *pRmsOut;
} rmsContext_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_process_splesQ15_rmsF32(rmsFloat_t  *const pCtxtF32, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, int16_t const *pData);
static int32_t s_process_splesQ15_rmsF64(rmsDouble_t *const pCtxtF64, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, int16_t const *pData);
static int32_t s_process_splesQ31_rmsF32(rmsFloat_t  *const pCtxtF32, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, int32_t const *pData);
static int32_t s_process_splesQ31_rmsF64(rmsDouble_t *const pCtxtF64, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, int32_t const *pData);
static int32_t s_process_splesF32_rmsF32(rmsFloat_t  *const pCtxtF32, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, float   const *pData);
static int32_t s_process_splesF32_rmsF64(rmsDouble_t *const pCtxtF64, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, float   const *pData);


/* Functions Definition ------------------------------------------------------*/


/**
* @brief  initialize rms (floating-point version)
* @param  pHdle           rms pHdle pointer
* @retval Error; AUDIO_ERR_MGNT_NONE if no issue
*/
int32_t rmsInit(rmsHdler_t *const pHdle)
{
  int32_t        error        = AUDIO_ERR_MGNT_NONE;
  rmsContext_t *pContext      = (rmsContext_t *)pHdle->pInternalMem;
  size_t        sampleSzBytes = pHdle->isDoublePrecision ? sizeof(float64_t) : sizeof(float32_t);
  float         normalizer    = 1.0f;

  if (pContext == NULL)
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }

  if (AudioError_isOk(error))
  {
    switch (pHdle->audioType)
    {
      case ABUFF_FORMAT_FIXED16:
        normalizer = 32768.0f;
        break;

      case ABUFF_FORMAT_FIXED32:
        normalizer = 2147483648.0f;
        break;

      case ABUFF_FORMAT_FLOAT:
        normalizer = 1.0f;
        break;

      default:
        error = AUDIO_ERR_MGNT_ERROR;
        break;
    }
  }


  if (AudioError_isOk(error))
  {
    uint8_t *pContext_u8 = (uint8_t *)pContext;
    /* Set context pointer per channel */
    size_t allocOffset   = sizeof(rmsContext_t);
    if (pHdle->isDoublePrecision)
    {
      pContext->data.f64.pR2Table   = (double *)(pContext_u8 + allocOffset);
      pContext->data.f64.normalizer = (double) normalizer;
      allocOffset                  += pHdle->nbChannels * sampleSzBytes;
      pContext->pRmsOut             = (float *)(pContext_u8 + allocOffset);

    }
    else
    {
      pContext->data.f32.pR2Table   = (float *)(pContext_u8 + allocOffset);
      pContext->data.f32.normalizer = normalizer;
      allocOffset                  += pHdle->nbChannels * sampleSzBytes;
      pContext->pRmsOut             = (float *)(pContext_u8 + allocOffset);
    }
    allocOffset += pHdle->nbChannels *  sizeof(float);
    if (allocOffset != pHdle->internalMemSize)
    {
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }

    pHdle->pRmsValues = pContext->pRmsOut ;
  }
  return error;
}

/**
  * @brief  compute required memory size depending on static config
  * @param  pHdle      handler pointer (its field internal_memory_size will be set to the required memory size)
  * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
  */
int32_t rmsGetMemorySize(rmsHdler_t *const pHdle)
{
  int32_t   error = AUDIO_ERR_MGNT_NONE;

  size_t allocSize = 0;
  size_t sampleSzBytes   = pHdle->isDoublePrecision ? sizeof(float64_t) : sizeof(float32_t);

  allocSize  = sizeof(rmsContext_t);
  allocSize += pHdle->nbChannels * sampleSzBytes;
  allocSize += pHdle->nbChannels * sizeof(float); /* pRmsValues is always float for simplicity of API */
  pHdle->internalMemSize = allocSize;
  return error;
}

int32_t rmsConfigure(rmsHdler_t *const pHdle, uint16_t const smoothingTimeInMs)
{
  /***
  see: https://en.wikipedia.org/wiki/Exponential_smoothing
  S(t)   = alpha*x(t) +(1-alpha)*S(t-1)
       = S(t-1) + alpha*(x(t)-S(t-1))
          or
       = alpha*(x(t)) + lambda*S(t-1)
  in our case x(t) = signal(t)*signal(t). RMS = sqrt(S(t))
  alpha  = smoothing factor
  lambda = 1-alpha = forget factor
  the time constant to
  alpha = 1 - exp(-1.0*samplingTimeInterval/TimeConstant)
       = 1 - exp(-1.0/(fs*SmoothingTimeInSec)
       = 1 - exp(-1000.0/(fs*SmoothingTimeInMilliSec)

  ***/
  int32_t             error    = AUDIO_ERR_MGNT_NONE;
  rmsContext_t *const pContext = (rmsContext_t *) pHdle->pInternalMem;

  if ((pHdle->fs == 0UL) || (smoothingTimeInMs == 0U))
  {
    error = AUDIO_ERR_MGNT_CONFIG;
  }
  else
  {
    if (pHdle->isDoublePrecision)
    {
      pContext->data.f64.lambda = exp(-1000.0 / ((double)pHdle->fs * (double)smoothingTimeInMs));   /*cstat !MISRAC2012-Rule-22.8 no issue with exp => errno check is useless*/
      pContext->data.f64.alpha  = 1.0 - pContext->data.f64.lambda;
    }
    else
    {
      pContext->data.f32.lambda = expf(-1000.0f / ((float)pHdle->fs * (float)smoothingTimeInMs));   /*cstat !MISRAC2012-Rule-22.8 no issue with expf => errno check is useless*/
      pContext->data.f32.alpha  = 1.0f - pContext->data.f32.lambda;
    }
  }

  return error;
}


int32_t rmsProcess(rmsHdler_t *const pHdle, audio_chunk_t const *const pChunkIn)
{
  int32_t                     error         = AUDIO_ERR_MGNT_NONE;
  rmsContext_t               *pContext      = (rmsContext_t *) pHdle->pInternalMem;

  void           const *const pData         = AudioChunk_getReadPtr0(pChunkIn);
  audio_buffer_t const *const pBuffIn       = AudioChunk_getBuffInfo(pChunkIn);
  uint32_t              const nbSamples     = AudioBuffer_getNbElements(pBuffIn);
  uint8_t               const nbChannels    = pHdle->nbChannels; // using nbChannels from pHdle allows to compute RMS for only first channel even if chunk is multi channels; previous code = AudioBuffer_getNbChannels(pBuffIn);
  uint32_t              const sampleOffset  = AudioBuffer_getSamplesOffset(pBuffIn);
  uint32_t              const channelOffset = AudioBuffer_getChannelsOffset(pBuffIn);

  switch (pHdle->audioType)
  {
    case ABUFF_FORMAT_FIXED16:
    {
      int16_t const *pDataFixed16 = (int16_t const *)pData;
      if (pHdle->isDoublePrecision)
      {
        error = s_process_splesQ15_rmsF64(&pContext->data.f64, nbSamples, sampleOffset, nbChannels, channelOffset, pDataFixed16);
      }
      else
      {
        error = s_process_splesQ15_rmsF32(&pContext->data.f32, nbSamples, sampleOffset, nbChannels, channelOffset, pDataFixed16);
      }
      break;
    }

    case ABUFF_FORMAT_FIXED32:
    {
      int32_t const *pDataFixed32 = (int32_t const *)pData;
      if (pHdle->isDoublePrecision)
      {
        error = s_process_splesQ31_rmsF64(&pContext->data.f64, nbSamples, sampleOffset, nbChannels, channelOffset, pDataFixed32);
      }
      else
      {
        error = s_process_splesQ31_rmsF32(&pContext->data.f32, nbSamples, sampleOffset, nbChannels, channelOffset, pDataFixed32);
      }
      break;
    }

    case ABUFF_FORMAT_FLOAT:
    {
      float const *pDataFloat = (float const *)pData;
      if (pHdle->isDoublePrecision)
      {
        error = s_process_splesF32_rmsF64(&pContext->data.f64, nbSamples, sampleOffset, nbChannels, channelOffset, pDataFloat);
      }
      else
      {
        error = s_process_splesF32_rmsF32(&pContext->data.f32, nbSamples, sampleOffset, nbChannels, channelOffset, pDataFloat);
      }
      break;
    }

    default:
      error = AUDIO_ERR_MGNT_ERROR;
      break;
  }

  if (AudioError_isOk(error))
  {
    if (pHdle->isDoublePrecision)
    {
      for (uint8_t ch = 0U; ch < nbChannels; ch++)
      {
        double val = pContext->data.f64.pR2Table[ch];
        val = (val <= 0.0) ? 0.0 : val;
        pContext->pRmsOut[ch] = (float)(sqrt(val) / pContext->data.f64.normalizer); /*cstat !MISRAC2012-Rule-22.8 no issue with sqrt of a positive value => errno check is useless*/
      }
    }
    else
    {
      for (uint8_t ch = 0U; ch < nbChannels; ch++)
      {
        float val = pContext->data.f32.pR2Table[ch];
        val = (val <= 0.0f) ? 0.0f : val;
        pContext->pRmsOut[ch] = sqrtf(val) / pContext->data.f32.normalizer; /*cstat !MISRAC2012-Rule-22.8 no issue with sqrt of a positive value => errno check is useless*/
      }
    }
  }

  return error;
}

/* Static functions ***********************************************************/


static int32_t s_process_splesQ15_rmsF32(rmsFloat_t *const pCtxtF32, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, int16_t const *pData)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  float   xn, r2;

  for (uint8_t ch = 0U; ch < nbChannels; ch++)
  {
    r2 = pCtxtF32->pR2Table[ch];
    for (uint32_t spl = 0UL; spl < nbSamples; spl++)
    {
      xn = (float)pData[spl * sampleOffset];
      r2 += pCtxtF32->alpha * ((xn * xn) - r2);
    }
    pCtxtF32->pR2Table[ch] = r2;
    pData += channelOffset;
  }
  return error;
}


static int32_t s_process_splesQ15_rmsF64(rmsDouble_t *const pCtxtF64, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, int16_t const *pData)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  double  xn, r2;

  for (uint8_t ch = 0U; ch < nbChannels; ch++)
  {
    r2 = pCtxtF64->pR2Table[ch];
    for (uint32_t spl = 0UL; spl < nbSamples; spl++)
    {
      xn = (double)pData[spl * sampleOffset];
      r2 += pCtxtF64->alpha * ((xn * xn) - r2);
    }
    pCtxtF64->pR2Table[ch] = r2;
    pData += channelOffset;
  }
  return error;
}


static int32_t s_process_splesQ31_rmsF32(rmsFloat_t *const pCtxtF32, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, int32_t const *pData)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  float   xn, r2;

  for (uint8_t ch = 0U; ch < nbChannels; ch++)
  {
    r2 = pCtxtF32->pR2Table[ch];
    for (uint32_t spl = 0UL; spl < nbSamples; spl++)
    {
      xn = (float)pData[spl * sampleOffset];
      r2 += pCtxtF32->alpha * ((xn * xn) - r2);
    }
    pCtxtF32->pR2Table[ch] = r2;
    pData += channelOffset;
  }
  return error;
}


static int32_t s_process_splesQ31_rmsF64(rmsDouble_t *const pCtxtF64, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, int32_t const *pData)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  double  xn, r2;

  for (uint8_t ch = 0U; ch < nbChannels; ch++)
  {
    r2 = pCtxtF64->pR2Table[ch];
    for (uint32_t spl = 0UL; spl < nbSamples; spl++)
    {
      xn = (double)pData[spl * sampleOffset];
      r2 += pCtxtF64->alpha * ((xn * xn) - r2);
    }
    pCtxtF64->pR2Table[ch] = r2;
    pData += channelOffset;
  }
  return error;
}


static int32_t s_process_splesF32_rmsF32(rmsFloat_t *const pCtxtF32, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, float const *pData)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  float   xn, r2;

  for (uint8_t ch = 0U; ch < nbChannels; ch++)
  {
    r2 = pCtxtF32->pR2Table[ch];
    for (uint32_t spl = 0UL; spl < nbSamples; spl++)
    {
      xn = (float)pData[spl * sampleOffset];
      r2 += pCtxtF32->alpha * ((xn * xn) - r2);
    }
    pCtxtF32->pR2Table[ch] = r2;
    pData += channelOffset;
  }

  return error;
}


static int32_t s_process_splesF32_rmsF64(rmsDouble_t *const pCtxtF64, uint32_t const nbSamples, uint32_t const sampleOffset, uint8_t const nbChannels, uint32_t const channelOffset, float const *pData)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  double  xn, r2;

  for (uint8_t ch = 0U; ch < nbChannels; ch++)
  {
    r2 = pCtxtF64->pR2Table[ch];
    for (uint32_t spl = 0UL; spl < nbSamples; spl++)
    {
      xn = (double)pData[spl * sampleOffset];
      r2 += pCtxtF64->alpha * ((xn * xn) - r2);
    }
    pCtxtF64->pR2Table[ch] = r2;
    pData += channelOffset;
  }

  return error;
}

