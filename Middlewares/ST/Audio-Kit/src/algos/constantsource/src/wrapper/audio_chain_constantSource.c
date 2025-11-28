/**
  ******************************************************************************
  * @file    audio_chain_constantSource.c
  * @author  MCD Application Team
  * @brief   wrapper of constantSource algo to match usage inside audio_chain.c
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
#include "constantsource/audio_chain_constantSource.h"
/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t                    nbChannels;
  uint32_t                   nbElements;
  audio_buffer_time_freq_t   timeFreq;
  audio_buffer_type_t        type;
  audio_buffer_interleaved_t interleaved;
  union
  {
    //      double  f64;
    float32_t   f32;
    int32_t     s32;
    int16_t     s16;
    int8_t      s8;
  } re, im;
}
constantSource_context_t;

/* Private defines -----------------------------------------------------------*/
#define CONSTANTSOURCE_MEM_POOL AUDIO_MEM_RAMINT

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static inline int8_t  s_clampToInt8(audio_algo_t  *const pAlgo, float32_t const x);
static inline int16_t s_clampToInt16(audio_algo_t *const pAlgo, float32_t const x);
static inline int32_t s_clampToInt32(audio_algo_t *const pAlgo, float32_t const x);

static int32_t s_constantSource_deinit(audio_algo_t    *const pAlgo);
static int32_t s_constantSource_init(audio_algo_t      *const pAlgo);
static int32_t s_constantSource_configure(audio_algo_t *const pAlgo);
static int32_t s_constantSource_dataInOut(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_constantSource_common =
{
  .pName                     = "constantSource",
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
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("generate a buffer field with a constant value")
};

audio_algo_cbs_t AudioChainWrp_constantSource_cbs =
{
  .init                       = s_constantSource_init,
  .deinit                     = s_constantSource_deinit,
  .configure                  = s_constantSource_configure,
  .dataInOut                  = s_constantSource_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_constantSource_init(audio_algo_t *const pAlgo)
{
  int32_t                                error          = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t                   *const pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t                  *const pBuffOut       = AudioChunk_getBuffInfo(pChunkOut);
  constantSource_dynamic_config_t *const pDynamicConfig = (constantSource_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  constantSource_context_t              *pContext       = NULL;
  size_t                                 allocSize      = 0UL;

  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "dynamic config is missing !");
    error = AUDIO_ERR_MGNT_CONFIG;
  }

  if (AudioError_isOk(error))
  {
    allocSize = sizeof(constantSource_context_t);
    pContext  = (constantSource_context_t *)AudioAlgo_malloc(allocSize, CONSTANTSOURCE_MEM_POOL);
    if (pContext == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }

  if (AudioError_isOk(error))
  {
    memset(pContext, 0, allocSize);
    pContext->nbChannels  = AudioBuffer_getNbChannels(pBuffOut);
    pContext->nbElements  = AudioBuffer_getNbElements(pBuffOut);
    pContext->timeFreq    = AudioBuffer_getTimeFreq(pBuffOut);
    pContext->type        = AudioBuffer_getType(pBuffOut);
    pContext->interleaved = AudioBuffer_getInterleaved(pBuffOut);
    AudioAlgo_setWrapperContext(pAlgo, pContext);
    error = s_constantSource_configure(pAlgo);
  }

  return error;
}


static int32_t s_constantSource_deinit(audio_algo_t *const pAlgo)
{
  int32_t                         error    = AUDIO_ERR_MGNT_NONE;
  constantSource_context_t *const pContext = (constantSource_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, CONSTANTSOURCE_MEM_POOL);
  }

  return error;
}


static int32_t s_constantSource_configure(audio_algo_t *const pAlgo)
{
  int32_t                                error          = AUDIO_ERR_MGNT_NONE;
  constantSource_dynamic_config_t *const pDynamicConfig = (constantSource_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  constantSource_context_t        *const pContext       = (constantSource_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext->interleaved == ABUFF_FORMAT_INTERLEAVED)
  {
    // when samples are interleaved, audio buffer may be viewed as a single buffer
    pContext->nbElements *= (uint32_t)pContext->nbChannels;
    pContext->nbChannels  = 1U;
  }

  switch (pContext->type)
  {
    case ABUFF_FORMAT_G711_ALAW:
    case ABUFF_FORMAT_G711_MULAW:
      pContext->re.s8 = s_clampToInt8(pAlgo, pDynamicConfig->re);
      pContext->im.s8 = (pContext->timeFreq == ABUFF_FORMAT_TIME) ? 0 : s_clampToInt8(pAlgo, pDynamicConfig->im);
      break;
    case ABUFF_FORMAT_FIXED16:
      pContext->re.s16 = s_clampToInt16(pAlgo, pDynamicConfig->re);
      pContext->im.s16 = (pContext->timeFreq == ABUFF_FORMAT_TIME) ? 0 : s_clampToInt16(pAlgo, pDynamicConfig->im);
      break;
    case ABUFF_FORMAT_FIXED32:
      pContext->re.s32 = s_clampToInt32(pAlgo, pDynamicConfig->re);
      pContext->im.s32 = (pContext->timeFreq == ABUFF_FORMAT_TIME) ? 0 : s_clampToInt32(pAlgo, pDynamicConfig->im);
      break;
    case ABUFF_FORMAT_FLOAT:
      pContext->re.f32 = pDynamicConfig->re;
      pContext->im.f32 = (pContext->timeFreq == ABUFF_FORMAT_TIME) ? 0.0f : pDynamicConfig->im;
      break;
    default:
      // unsupported sample format type
      error = AUDIO_ERR_MGNT_ERROR;
      break;
  }

  return error;
}


int32_t s_constantSource_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                         error     = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t            *const pChunkOut = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t           *const pBuffOut  = AudioChunk_getBuffInfo(pChunkOut);
  constantSource_context_t *const pContext  = (constantSource_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
  {
    void      *ptrOut   = AudioChunk_getWritePtr(pChunkOut, ch, 0UL);
    int8_t    *pOut_s8  = (int8_t *) ptrOut;
    int16_t   *pOut_s16 = (int16_t *) ptrOut;
    int32_t   *pOut_s32 = (int32_t *) ptrOut;
    float32_t *pOut_f32 = (float32_t *) ptrOut;

    switch (pBuffOut->type)
    {
      case ABUFF_FORMAT_G711_ALAW:
      case ABUFF_FORMAT_G711_MULAW:
        if (pContext->timeFreq == ABUFF_FORMAT_TIME)
        {
          memset(pOut_s8, pContext->re.s8, pContext->nbElements);
        }
        else
        {
          for (uint32_t i = 0UL; i < pContext->nbElements; i++)
          {
            *pOut_s8++ = pContext->re.s8;
            *pOut_s8++ = pContext->im.s8;
          }
        }
        break;

      case ABUFF_FORMAT_FIXED16:
        if (pContext->timeFreq == ABUFF_FORMAT_TIME)
        {
          for (uint32_t i = 0UL; i < pContext->nbElements; i++)
          {
            *pOut_s16++ = pContext->re.s16;
          }
        }
        else
        {
          for (uint32_t i = 0UL; i < pContext->nbElements; i++)
          {
            *pOut_s16++ = pContext->re.s16;
            *pOut_s16++ = pContext->im.s16;
          }
        }
        break;

      case ABUFF_FORMAT_FIXED32:
        if (pContext->timeFreq == ABUFF_FORMAT_TIME)
        {
          for (uint32_t i = 0UL; i < pContext->nbElements; i++)
          {
            *pOut_s32++ = pContext->re.s32;
          }
        }
        else
        {
          for (uint32_t i = 0UL; i < pContext->nbElements; i++)
          {
            *pOut_s32++ = pContext->re.s32;
            *pOut_s32++ = pContext->im.s32;
          }
        }
        break;

      case ABUFF_FORMAT_FLOAT:
        if (pContext->timeFreq == ABUFF_FORMAT_TIME)
        {
          for (uint32_t i = 0UL; i < pContext->nbElements; i++)
          {
            *pOut_f32++ = pContext->re.f32;
          }
        }
        else
        {
          for (uint32_t i = 0UL; i < pContext->nbElements; i++)
          {
            *pOut_f32++ = pContext->re.f32;
            *pOut_f32++ = pContext->im.f32;
          }
        }
        break;

      default:
        // unsupported sample format type
        error = AUDIO_ERR_MGNT_ERROR;
        break;
    }
  }

  return error;
}


/**
  * @brief  clamping floating-point to 32 bits fixed-point sample conversion
  * @param  x: floating-point input sample
  * @retval 32 bits fixed-point output sample
  */
static inline int8_t s_clampToInt8(audio_algo_t *const pAlgo, float32_t const x)
{
  float const y       = 128.0f * x;
  bool        clamped = true;
  int8_t      dataClampedInt8;

  if (y < -128.0f)
  {
    dataClampedInt8 = (int8_t)(-128);
  }
  else if (y > 127.0f)
  {
    dataClampedInt8 = 127;
  }
  else
  {
    dataClampedInt8 = (int8_t)y;
    clamped         = false;
  }
  if (clamped)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_WARNING, NULL, 0, "Value is being clamped from %f to %d, you might want to check this is OK!", (double)y, dataClampedInt8);
  }

  return dataClampedInt8;
}


/**
  * @brief  clamping floating-point to 16 bits fixed-point sample conversion
  * @param  x: floating-point input sample
  * @retval 16 bits fixed-point output sample
  */
static inline int16_t s_clampToInt16(audio_algo_t *const pAlgo, float32_t const x)
{
  float const y       = 32768.0f * x;
  bool        clamped = true;
  int16_t     dataClampedInt16;

  if (y < -32768.0f)
  {
    dataClampedInt16 = (int16_t)(-32768);
  }
  else if (y > 32767.0f)
  {
    dataClampedInt16 = 32767;
  }
  else
  {
    dataClampedInt16 = (int16_t)y;
    clamped          = false;
  }
  if (clamped)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_WARNING, NULL, 0, "Value is being clamped from %f to %d, you might want to check this is OK!", (double)y, dataClampedInt16);
  }

  return dataClampedInt16;
}


/**
  * @brief  clamping floating-point to 32 bits fixed-point sample conversion
  * @param  x: floating-point input sample
  * @retval 32 bits fixed-point output sample
  */
static inline int32_t s_clampToInt32(audio_algo_t *const pAlgo, float32_t const x)
{
  float const y       = 2147483648.0f * x;
  bool        clamped = true;
  int32_t     dataClampedInt32;

  if (y < -2147483648.0f)
  {
    dataClampedInt32 = (int32_t)(-2147483648);
  }
  else if (y > 2147483647.0f)
  {
    dataClampedInt32 = 2147483647;
  }
  else
  {
    dataClampedInt32 = (int32_t)y;
    clamped          = false;
  }
  if (clamped)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_WARNING, NULL, 0, "Value is being clamped from %f to %d, you might want to check this is OK!", (double)y, dataClampedInt32);
  }

  return dataClampedInt32;
}
