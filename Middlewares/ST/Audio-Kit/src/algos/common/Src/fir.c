/**
******************************************************************************
* @file    fir.c (able to manage decimation and upsampling with zeros insertion)
* @author  MCD Application Team
* @brief   fir firing algo
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
#include "audio_chain.h"
#include "common/fir.h"
#include "common/util.h"

/* Global variables ----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
//#ifndef VALIDATION_X86
typedef struct
{
  int              downSamplingFactor;
  int              upSamplingFactor;
  int              blockSize;
  int              nbTaps;
  int              tapOffset;
  int16_t const   *pTaps;
  int16_t         *pState;
} fir_interpolate_decimate_instance_q15_t;

typedef struct
{
  int              downSamplingFactor;
  int              upSamplingFactor;
  int              blockSize;
  int              nbTaps;
  int              tapOffset;
  int32_t const   *pTaps;
  int32_t         *pState;
} fir_interpolate_decimate_instance_q31_t;

typedef struct
{
  int              downSamplingFactor;
  int              upSamplingFactor;
  int              blockSize;
  int              nbTaps;
  int              tapOffset;
  float32_t const *pTaps;
  float32_t       *pState;
} fir_interpolate_decimate_instance_f32_t;

typedef struct firContextStruct
{
  //  int32_t gainMant;
  //  int8_t  gainExp;
  void (*pProcess)(struct firContextStruct *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
  union
  {
    arm_fir_instance_q15                    *pQ15;
    arm_fir_instance_q31                    *pQ31;
    arm_fir_instance_f32                    *pF32;
    arm_fir_interpolate_instance_q15        *pUp_q15;
    arm_fir_interpolate_instance_q31        *pUp_q31;
    arm_fir_interpolate_instance_f32        *pUp_f32;
    arm_fir_decimate_instance_q15           *pDown_q15;
    arm_fir_decimate_instance_q31           *pDown_q31;
    arm_fir_decimate_instance_f32           *pDown_f32;
    fir_interpolate_decimate_instance_q15_t *pUpDown_q15;
    fir_interpolate_decimate_instance_q31_t *pUpDown_q31;
    fir_interpolate_decimate_instance_f32_t *pUpDown_f32;
    uint8_t                                 *pFirHdle;
  };
} firContext_t;

/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static int32_t s_fir_interpolate_decimate_init_q15(fir_interpolate_decimate_instance_q15_t *const pCtx, uint8_t const upSamplingFactor, uint8_t const downSamplingFactor, uint16_t const nbTaps, int16_t   const *const pTaps, int16_t   *const pState, uint32_t const blockSize);
static int32_t s_fir_interpolate_decimate_init_q31(fir_interpolate_decimate_instance_q31_t *const pCtx, uint8_t const upSamplingFactor, uint8_t const downSamplingFactor, uint16_t const nbTaps, int32_t   const *const pTaps, int32_t   *const pState, uint32_t const blockSize);
static int32_t s_fir_interpolate_decimate_init_f32(fir_interpolate_decimate_instance_f32_t *const pCtx, uint8_t const upSamplingFactor, uint8_t const downSamplingFactor, uint16_t const nbTaps, float32_t const *const pTaps, float32_t *const pState, uint32_t const blockSize);

static void s_firProcessFloat(firContext_t             *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessInt16(firContext_t             *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessInt32(firContext_t             *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessUpSampleFloat(firContext_t     *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessUpSampleInt16(firContext_t     *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessUpSampleInt32(firContext_t     *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessDownSampleFloat(firContext_t   *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessDownSampleInt16(firContext_t   *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessDownSampleInt32(firContext_t   *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessUpDownSampleFloat(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessUpDownSampleInt16(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
static void s_firProcessUpDownSampleInt32(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn);
//static void s_updateGainExp            (firHandler_t *const pHdle);

/* Functions Definition ------------------------------------------------------*/


#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
  #define HISTORY_MEM_FACTOR 2U
#else
  #define HISTORY_MEM_FACTOR 1U
#endif


/**
* @brief  initialize fir (floating-point version)
* @param  pHdle              fir pHdle pointer
* @param  pFirVoid           fir coefs
* @param  sampleType         ABUFF_FORMAT_FIXED16, ABUFF_FORMAT_FIXED32 or ABUFF_FORMAT_FLOAT
* @param  nbChannels         number of channels
* @param  nbSamples          number of samples
* @param  downSamplingFactor downsampling factor
* @param  upSamplingFactor   upsampling factor
* @param  memPool            memory pool used for allocation
* @retval Error; AUDIO_ERR_MGNT_NONE if no issue
*/
int32_t firInit(firHandler_t *const pHdle, void const *const pFirVoid, audio_buffer_type_t const sampleType, uint8_t const nbChannels, uint32_t const nbSamples, uint8_t const downSamplingFactor, uint8_t const upSamplingFactor, memPool_t const memPool)
{
  int32_t       error         = AUDIO_ERR_MGNT_NONE;
  firContext_t *pContext      = NULL;
  size_t        sampleSize    = 0UL;
  size_t        cmsisHdleSize = 0UL;
  size_t        allocSize     = 0UL;
  size_t        firStateSize  = 0UL;
  uint16_t      nbTaps        = 0U;

  if (pFirVoid == NULL)
  {
    error = AUDIO_ERR_MGNT_INIT;
  }
  if (AudioError_isOk(error))
  {
    pHdle->pFirVoid = pFirVoid;
    pHdle->memPool  = memPool;
    switch (sampleType)
    {
      case ABUFF_FORMAT_FIXED16:
        sampleSize = sizeof(int16_t);
        if (upSamplingFactor != 1U)
        {
          if (downSamplingFactor != 1U)
          {
            cmsisHdleSize = sizeof(fir_interpolate_decimate_instance_q15_t);
          }
          else
          {
            cmsisHdleSize = sizeof(arm_fir_interpolate_instance_q15);
          }
        }
        else
        {
          if (downSamplingFactor != 1U)
          {
            cmsisHdleSize = sizeof(arm_fir_decimate_instance_q15);
          }
          else
          {
            cmsisHdleSize = sizeof(arm_fir_instance_q15);
          }
        }
        nbTaps = pHdle->pFirQ15->nbTaps;
        break;

      case ABUFF_FORMAT_FIXED32:
        sampleSize = sizeof(int32_t);
        if (upSamplingFactor != 1U)
        {
          if (downSamplingFactor != 1U)
          {
            cmsisHdleSize = sizeof(fir_interpolate_decimate_instance_q31_t);
          }
          else
          {
            cmsisHdleSize = sizeof(arm_fir_interpolate_instance_q31);
          }
        }
        else
        {
          if (downSamplingFactor != 1U)
          {
            cmsisHdleSize = sizeof(arm_fir_decimate_instance_q31);
          }
          else
          {
            cmsisHdleSize = sizeof(arm_fir_instance_q31);
          }
        }
        nbTaps = pHdle->pFirQ31->nbTaps;
        break;

      case ABUFF_FORMAT_FLOAT:
        sampleSize = sizeof(float32_t);
        if (upSamplingFactor != 1U)
        {
          if (downSamplingFactor != 1U)
          {
            cmsisHdleSize = sizeof(fir_interpolate_decimate_instance_f32_t);
          }
          else
          {
            cmsisHdleSize = sizeof(arm_fir_interpolate_instance_f32);
          }
        }
        else
        {
          if (downSamplingFactor != 1U)
          {
            cmsisHdleSize = sizeof(arm_fir_decimate_instance_f32);
          }
          else
          {
            cmsisHdleSize = sizeof(arm_fir_instance_f32);
          }
        }
        nbTaps = pHdle->pFirF32->nbTaps;
        break;

      default:
        error = AUDIO_ERR_MGNT_INIT;
        break;
    }
    cmsisHdleSize *= (size_t)nbChannels;
  }

  if (AudioError_isOk(error))
  {
    if ((upSamplingFactor == 1U) || (downSamplingFactor == 1U))
    {
      // arm CMSIS filtering
      firStateSize = ((HISTORY_MEM_FACTOR * (size_t)nbSamples) + (size_t)nbTaps - 1UL);
    }
    else
    {
      // local filtering
      firStateSize = (size_t)nbSamples + (size_t)nbTaps;
    }
    allocSize = sizeof(firContext_t) + cmsisHdleSize + ((size_t)nbChannels * firStateSize * sampleSize);
    pContext  = (firContext_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pContext == NULL)
    {
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    /* Set context pointer per channel */
    uint8_t   *pState;
    q15_t     *pStateQ15;
    q31_t     *pStateQ31;
    float32_t *pStateF32;

    memset(pContext, 0, allocSize);

    pHdle->pInternalMem = pContext;
    pContext->pFirHdle  = (uint8_t *)&pContext[1];
    pState              = pContext->pFirHdle + cmsisHdleSize;
    pStateQ15           = (q15_t *)pState;
    pStateQ31           = (q31_t *)pState;
    pStateF32           = (float32_t *)pState;

    /* Call cmsis init routine per channel */
    for (uint8_t ch = 0U; AudioError_isOk(error) && (ch < nbChannels); ch++)
    {
      arm_status armErr = ARM_MATH_SUCCESS;

      switch (sampleType)
      {
        case ABUFF_FORMAT_FIXED16:
          if (upSamplingFactor != 1U)
          {
            if (downSamplingFactor != 1U)
            {
              pContext->pProcess = s_firProcessUpDownSampleInt16;
              error = s_fir_interpolate_decimate_init_q15(&pContext->pUpDown_q15[ch],
                                                          upSamplingFactor,
                                                          downSamplingFactor,
                                                          nbTaps,
                                                          pHdle->pFirQ15->pTaps,
                                                          pStateQ15,
                                                          nbSamples);
            }
            else
            {
              pContext->pProcess = s_firProcessUpSampleInt16;
              armErr = arm_fir_interpolate_init_q15(&pContext->pUp_q15[ch],
                                                    upSamplingFactor,
                                                    nbTaps,
                                                    pHdle->pFirQ15->pTaps,
                                                    pStateQ15,
                                                    nbSamples);
            }
          }
          else
          {
            if (downSamplingFactor != 1U)
            {
              pContext->pProcess = s_firProcessDownSampleInt16;
              armErr = arm_fir_decimate_init_q15(&pContext->pDown_q15[ch],
                                                 nbTaps,
                                                 downSamplingFactor,
                                                 pHdle->pFirQ15->pTaps,
                                                 pStateQ15,
                                                 nbSamples);
            }
            else
            {
              pContext->pProcess = s_firProcessInt16;
              armErr = arm_fir_init_q15(&pContext->pQ15[ch],
                                        nbTaps,
                                        pHdle->pFirQ15->pTaps,
                                        pStateQ15,
                                        nbSamples);
            }
          }
          pStateQ15 += firStateSize;
          break;

        case ABUFF_FORMAT_FIXED32:
          if (upSamplingFactor != 1U)
          {
            if (downSamplingFactor != 1U)
            {
              pContext->pProcess = s_firProcessUpDownSampleInt32;
              error = s_fir_interpolate_decimate_init_q31(&pContext->pUpDown_q31[ch],
                                                          upSamplingFactor,
                                                          downSamplingFactor,
                                                          nbTaps,
                                                          pHdle->pFirQ31->pTaps,
                                                          pStateQ31,
                                                          nbSamples);
            }
            else
            {
              pContext->pProcess = s_firProcessUpSampleInt32;
              armErr = arm_fir_interpolate_init_q31(&pContext->pUp_q31[ch],
                                                    upSamplingFactor,
                                                    nbTaps,
                                                    pHdle->pFirQ31->pTaps,
                                                    pStateQ31,
                                                    nbSamples);
            }
          }
          else
          {
            if (downSamplingFactor != 1U)
            {
              pContext->pProcess = s_firProcessDownSampleInt32;
              armErr = arm_fir_decimate_init_q31(&pContext->pDown_q31[ch],
                                                 nbTaps,
                                                 downSamplingFactor,
                                                 pHdle->pFirQ31->pTaps,
                                                 pStateQ31,
                                                 nbSamples);
            }
            else
            {
              pContext->pProcess = s_firProcessInt32;
              arm_fir_init_q31(&pContext->pQ31[ch],
                               nbTaps,
                               pHdle->pFirQ31->pTaps,
                               pStateQ31,
                               nbSamples);
            }
          }
          pStateQ31 += firStateSize;
          break;

        case ABUFF_FORMAT_FLOAT:
          if (upSamplingFactor != 1U)
          {
            if (downSamplingFactor != 1U)
            {
              pContext->pProcess = s_firProcessUpDownSampleFloat;
              error = s_fir_interpolate_decimate_init_f32(&pContext->pUpDown_f32[ch],
                                                          upSamplingFactor,
                                                          downSamplingFactor,
                                                          nbTaps,
                                                          pHdle->pFirF32->pTaps,
                                                          pStateF32,
                                                          nbSamples);
            }
            else
            {
              pContext->pProcess = s_firProcessUpSampleFloat;
              armErr = arm_fir_interpolate_init_f32(&pContext->pUp_f32[ch],
                                                    upSamplingFactor,
                                                    nbTaps,
                                                    pHdle->pFirF32->pTaps,
                                                    pStateF32,
                                                    nbSamples);
            }
          }
          else
          {
            if (downSamplingFactor != 1U)
            {
              pContext->pProcess = s_firProcessDownSampleFloat;
              armErr = arm_fir_decimate_init_f32(&pContext->pDown_f32[ch],
                                                 nbTaps,
                                                 downSamplingFactor,
                                                 pHdle->pFirF32->pTaps,
                                                 pStateF32,
                                                 nbSamples);
            }
            else
            {
              pContext->pProcess = s_firProcessFloat;
              arm_fir_init_f32(&pContext->pF32[ch],
                               nbTaps,
                               pHdle->pFirF32->pTaps,
                               pStateF32,
                               nbSamples);
            }
          }
          pStateF32 += firStateSize;
          break;

        default:
          error = AUDIO_ERR_MGNT_INIT;
          break;
      }

      if (armErr != ARM_MATH_SUCCESS)
      {
        error = AUDIO_ERR_MGNT_ERROR;
      }
    }
  }

  return error;
}


/**
* @brief  initialize fir (floating-point version)
* @param  pHdle           fir pHdle pointer
* @retval Error; AUDIO_ERR_MGNT_NONE if no issue
*/
int32_t firDeInit(firHandler_t *const pHdle)
{
  memPool_t const memPool = pHdle->memPool;

  AudioAlgo_free(pHdle->pInternalMem, memPool);
  pHdle->pInternalMem = NULL;

  return AUDIO_ERR_MGNT_NONE;
}

/**
* @brief  initialize fir (floating-point version)
* @param  pHdle       fir pHdle pointer
* @param  in          input samples pointer
* @param  out         output samples pointer
* @param  ch          channel number
* @param  nbSamplesIn number of samples
* @retval Error; AUDIO_ERR_MGNT_NONE if no issue
*/
int32_t firProcess(firHandler_t *const pHdle, void *const in, void *const out, int const ch, int const nbSamplesIn)
{
  firContext_t *pContext = (firContext_t *)pHdle->pInternalMem;
  /* Call process for all channels */
  //  for (int ch = 0; ch < pHdle->nbChannels ; ch++)
  //  {
  (*pContext->pProcess)(pContext, in, out, ch, nbSamplesIn);
  //  }
  return AUDIO_ERR_MGNT_NONE; /* todo manage error */
}

/* Private Functions Definition ------------------------------------------------------*/

static int32_t s_fir_interpolate_decimate_init_q15(fir_interpolate_decimate_instance_q15_t *const pCtx, uint8_t const upSamplingFactor, uint8_t const downSamplingFactor, uint16_t const nbTaps, int16_t const *const pTaps, int16_t *const pState, uint32_t const blockSize)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  pCtx->upSamplingFactor   = (int)upSamplingFactor;
  pCtx->downSamplingFactor = (int)downSamplingFactor;
  pCtx->blockSize          = (int)blockSize;
  pCtx->nbTaps             = (int)nbTaps;
  pCtx->tapOffset          = 0;
  pCtx->pTaps              = pTaps;
  pCtx->pState             = pState;
  memset(pState, 0, ((size_t)nbTaps + (size_t)blockSize) * sizeof(int16_t));

  return error;
}


static int32_t s_fir_interpolate_decimate_init_q31(fir_interpolate_decimate_instance_q31_t *const pCtx, uint8_t const upSamplingFactor, uint8_t const downSamplingFactor, uint16_t const nbTaps, int32_t const *const pTaps, int32_t *const pState, uint32_t const blockSize)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  pCtx->upSamplingFactor   = (int)upSamplingFactor;
  pCtx->downSamplingFactor = (int)downSamplingFactor;
  pCtx->blockSize          = (int)blockSize;
  pCtx->nbTaps             = (int)nbTaps;
  pCtx->tapOffset          = 0;
  pCtx->pTaps              = pTaps;
  pCtx->pState             = pState;
  memset(pState, 0, ((size_t)nbTaps + (size_t)blockSize) * sizeof(int32_t));

  return error;
}


static int32_t s_fir_interpolate_decimate_init_f32(fir_interpolate_decimate_instance_f32_t *const pCtx, uint8_t const upSamplingFactor, uint8_t const downSamplingFactor, uint16_t const nbTaps, float32_t const *const pTaps, float32_t *const pState, uint32_t const blockSize)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  pCtx->upSamplingFactor   = (int)upSamplingFactor;
  pCtx->downSamplingFactor = (int)downSamplingFactor;
  pCtx->blockSize          = (int)blockSize;
  pCtx->nbTaps             = (int)nbTaps;
  pCtx->tapOffset          = 0;
  pCtx->pTaps              = pTaps;
  pCtx->pState             = pState;
  memset(pState, 0, ((size_t)nbTaps + (size_t)blockSize) * sizeof(float32_t));

  return error;
}


static void s_firProcessFloat(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  arm_fir_f32(&pContext->pF32[chId],
              (float32_t *)in,
              (float32_t *)out,
              (uint32_t)nbSamplesIn);
}


static void s_firProcessInt16(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  arm_fir_q15(&pContext->pQ15[chId],
              (q15_t *)in,
              (q15_t *)out,
              (uint32_t)nbSamplesIn);
}


static void s_firProcessInt32(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  arm_fir_q31(&pContext->pQ31[chId],
              (q31_t *)in,
              (q31_t *)out,
              (uint32_t)nbSamplesIn);
}


static void s_firProcessDownSampleFloat(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  arm_fir_decimate_f32(&pContext->pDown_f32[chId],
                       (float32_t *)in,
                       (float32_t *)out,
                       (uint32_t)nbSamplesIn);
}


static void s_firProcessDownSampleInt16(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  arm_fir_decimate_q15(&pContext->pDown_q15[chId],
                       (q15_t *)in,
                       (q15_t *)out,
                       (uint32_t)nbSamplesIn);
}


static void s_firProcessDownSampleInt32(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  arm_fir_decimate_q31(&pContext->pDown_q31[chId],
                       (q31_t *)in,
                       (q31_t *)out,
                       (uint32_t)nbSamplesIn);
}


static void s_firProcessUpSampleFloat(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  arm_fir_interpolate_f32(&pContext->pUp_f32[chId],
                          (float32_t *)in,
                          (float32_t *)out,
                          (uint32_t)nbSamplesIn);
}


static void s_firProcessUpSampleInt16(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  arm_fir_interpolate_q15(&pContext->pUp_q15[chId],
                          (q15_t *)in,
                          (q15_t *)out,
                          (uint32_t)nbSamplesIn);
}


static void s_firProcessUpSampleInt32(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  arm_fir_interpolate_q31(&pContext->pUp_q31[chId],
                          (q31_t *)in,
                          (q31_t *)out,
                          (uint32_t)nbSamplesIn);
}


static void s_firProcessUpDownSampleFloat(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  fir_interpolate_decimate_instance_f32_t *pCtx               = &pContext->pUpDown_f32[chId];
  float32_t                               *pIn                = (float32_t *)in;
  float32_t                               *pOut               = (float32_t *)out;
  int                                      downSamplingFactor = pCtx->downSamplingFactor;
  int                                      upSamplingFactor   = pCtx->upSamplingFactor;
  int                                      nbSamplesOut, splOut, inOffset, n;
  float32_t                                y;

  downSamplingFactor = (downSamplingFactor == 0) ? 1 : downSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor   = (upSamplingFactor   == 0) ? 1 : upSamplingFactor;    // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut       = (nbSamplesIn / downSamplingFactor) * upSamplingFactor;

  assert(nbSamplesIn <= pCtx->blockSize);
  assert(((nbSamplesOut / upSamplingFactor) * downSamplingFactor) == nbSamplesIn);

  /* copy input samples in filter state */
  memcpy(pCtx->pState + pCtx->nbTaps, pIn, (size_t)nbSamplesIn * sizeof(float32_t));

  inOffset = 0;
  for (splOut = 0; splOut < nbSamplesOut; splOut++)
  {
    pIn = pCtx->pState + pCtx->nbTaps + inOffset;
    y   = 0.0f;
    for (n = pCtx->tapOffset; n < pCtx->nbTaps; n += upSamplingFactor)  /* skip filtering for inserted zeros */
    {
      y += pCtx->pTaps[n] * (*pIn);
      pIn--;
    }
    y    *= (float32_t)upSamplingFactor;  /* upSamplingFactor gain for zeros insertion compensation */
    *pOut = y;
    pOut++;

    /* skip filtering of dropped output samples */
    pCtx->tapOffset += downSamplingFactor;
    while (pCtx->tapOffset >= upSamplingFactor)
    {
      pCtx->tapOffset -= upSamplingFactor;
      inOffset++;
    }
  }

  /* move filter state for next buffer */
  memmove(pCtx->pState, pCtx->pState + nbSamplesIn, (size_t)pCtx->nbTaps * sizeof(float32_t));
}


static void s_firProcessUpDownSampleInt16(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  fir_interpolate_decimate_instance_q15_t *pCtx = &pContext->pUpDown_q15[chId];
  int16_t                                 *pIn  = (int16_t *)in;
  int16_t                                 *pOut = (int16_t *)out;
  int                                      downSamplingFactor = pCtx->downSamplingFactor;
  int                                      upSamplingFactor   = pCtx->upSamplingFactor;
  int                                      nbSamplesOut, splOut, inOffset, n;
  int32_t                                  y;

  downSamplingFactor = (downSamplingFactor == 0) ? 1 : downSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor   = (upSamplingFactor   == 0) ? 1 : upSamplingFactor;    // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut       = (nbSamplesIn / downSamplingFactor) * upSamplingFactor;

  assert(nbSamplesIn <= pCtx->blockSize);
  assert(((nbSamplesOut / upSamplingFactor) * downSamplingFactor) == nbSamplesIn);

  /* copy input samples in filter state */
  memcpy(pCtx->pState + pCtx->nbTaps, pIn, (size_t)nbSamplesIn * sizeof(int16_t));

  inOffset = 0;
  for (splOut = 0; splOut < nbSamplesOut; splOut++)
  {
    pIn = pCtx->pState + pCtx->nbTaps + inOffset;
    y   = 0L;
    for (n = pCtx->tapOffset; n < pCtx->nbTaps; n += upSamplingFactor)  /* skip filtering for inserted zeros */
    {
      /* WARNING: if filter response gain > upSamplingFactor (shouldn't happen) for some frequencies, risk of register wrap without saturation */
      y += ((int32_t)pCtx->pTaps[n] * (int32_t)(*pIn)) >> 15U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
      pIn--;
    }
    y    *= (int32_t)upSamplingFactor;  /* upSamplingFactor gain for zeros insertion compensation */
    *pOut = (int16_t)util_clamp_s32(y, -32768L, 32767L);
    pOut++;

    /* skip filtering of dropped output samples */
    pCtx->tapOffset += downSamplingFactor;
    while (pCtx->tapOffset >= upSamplingFactor)
    {
      pCtx->tapOffset -= upSamplingFactor;
      inOffset++;
    }
  }

  /* move filter state for next buffer */
  memmove(pCtx->pState, pCtx->pState + nbSamplesIn, (size_t)pCtx->nbTaps * sizeof(int16_t));
}


static void s_firProcessUpDownSampleInt32(firContext_t *const pContext, void *const in, void *const out, int const chId, int const nbSamplesIn)
{
  fir_interpolate_decimate_instance_q31_t *pCtx = &pContext->pUpDown_q31[chId];
  int32_t                                 *pIn  = (int32_t *)in;
  int32_t                                 *pOut = (int32_t *)out;
  int                                      downSamplingFactor = pCtx->downSamplingFactor;
  int                                      upSamplingFactor   = pCtx->upSamplingFactor;
  int                                      nbSamplesOut, splOut, inOffset, n;
  int64_t                                  y;

  downSamplingFactor = (downSamplingFactor == 0) ? 1 : downSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor   = (upSamplingFactor   == 0) ? 1 : upSamplingFactor;    // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut       = (nbSamplesIn / downSamplingFactor) * upSamplingFactor;

  assert(nbSamplesIn <= pCtx->blockSize);
  assert(((nbSamplesOut / upSamplingFactor) * downSamplingFactor) == nbSamplesIn);

  /* copy input samples in filter state */
  memcpy(pCtx->pState + pCtx->nbTaps, pIn, (size_t)nbSamplesIn * sizeof(int32_t));

  inOffset = 0;
  for (splOut = 0; splOut < nbSamplesOut; splOut++)
  {
    pIn = pCtx->pState + pCtx->nbTaps + inOffset;
    y   = 0LL;
    for (n = pCtx->tapOffset; n < pCtx->nbTaps; n += upSamplingFactor)  /* skip filtering for inserted zeros */
    {
      /* WARNING: if filter response gain > upSamplingFactor (shouldn't happen) for some frequencies, risk of register wrap without saturation */
      y += ((int64_t)pCtx->pTaps[n] * (int64_t)(*pIn)) >> 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
      pIn--;
    }
    y    *= (int64_t)upSamplingFactor;  /* upSamplingFactor gain for zeros insertion compensation */
    *pOut = (int32_t)util_clamp_s64(y, -2147483648LL, 2147483647LL);
    pOut++;

    /* skip filtering of dropped output samples */
    pCtx->tapOffset += downSamplingFactor;
    while (pCtx->tapOffset >= upSamplingFactor)
    {
      pCtx->tapOffset -= upSamplingFactor;
      inOffset++;
    }
  }

  /* move filter state for next buffer */
  memmove(pCtx->pState, pCtx->pState + nbSamplesIn, (size_t)pCtx->nbTaps * sizeof(int32_t));
}


//static void s_updateGainExp(firHandler_t *const pHdle)
//{
//  int32_t gainMant = 0;
//  int8_t gainExp = 0;
//  int8_t bIsInt = 0;
//  firContext_t *pContext = (firContext_t *)pHdle->pInternalMem;
//  switch (pHdle->type)
//  {
//  case ABUFF_FORMAT_FIXED16:
//    gainMant = pHdle->pFir->q15->gainMant;
//    gainExp  = pHdle->pFir->q15->gainExp;
//    bIsInt = 1;
//    break;
//  case ABUFF_FORMAT_FIXED32:
//    gainMant = pHdle->pFir->q31->gainMant;
//    gainExp  = pHdle->pFir->q31->gainExp;
//    bIsInt = 1;
//    break;
//  case ABUFF_FORMAT_FLOAT:
//    break;
//  default:
//    break;
//  }
//  if (bIsInt != 0)
//  {
//    if (pHdle->upSamplingFactor == 1)
//    {
//      pContext->gainMant = gainMant;
//      pContext->gainExp  = gainExp;
//    }
//    else
//    {
//      // apply a gain=upSamplingFactor inside gainMant*2^gainExp to compensate attenuation introduced by inserted zeros
//      int64_t gainMant_double = (int64_t)gainMant * (int64_t)pHdle->upSamplingFactor;
//      for (pContext->gainExp = gainExp; (gainMant_double > 0x7FFFFFFFLL); pContext->gainExp++)
//      {
//        gainMant_double >>= 1;
//      }
//      pContext->gainMant = (int32_t)gainMant_double;
//    }
//  }
//}


//#else
//
//int32_t firInit(firHandler_t      *const pHdle)
//{
//}
//int32_t firDeInit(firHandler_t    *const pHdle)
//{
//}
//int32_t firProcess(firHandler_t   *const pHdle, void *const in, void *const out, int const ch, int const nbSamplesIn)
//{
//}
//
//
//
//#endif /*VALIDATION_X86*/
