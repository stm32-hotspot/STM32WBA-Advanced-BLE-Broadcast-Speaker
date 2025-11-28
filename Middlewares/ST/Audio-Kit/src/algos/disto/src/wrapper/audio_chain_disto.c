/**
  ******************************************************************************
  * @file    audio_chain_disto.c
  * @author  MCD Application Team
  * @brief   wrapper of disto algo to match usage inside audio_chain.c
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "disto/audio_chain_disto.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

//typedef struct
//{
//  void *pReserved; /* dummy field, if specific field needed please store them here !*/
//}
//disto_user_context_t;

typedef struct
{
  float alpha;
  float xn;
  float yn_1; /* yn-1*/
  float xn_1; /* xn-1*/
} dcrem_t;

typedef float (process_t)(float input);

typedef float (correction_t)(dcrem_t *pHdle, float input);


typedef struct
{
  audio_chunk_t            *pChunk;
}
disto_buffer_context_t;


typedef struct
{
  process_t                  *process_cb;
  correction_t               *correction_cb;
  float                       dry_gain_lin ;
  float                       gain_lin;
  dcrem_t                     dcrem;
  uint8_t                     nbChannels;
  uint32_t                    nbSamples;
  disto_buffer_context_t      pin;
  disto_buffer_context_t      pout;
  //  disto_user_context_t        user;
  memPool_t                   memPool;
}
disto_context_t;
/* Private defines -----------------------------------------------------------*/
#ifndef M_PI
  #define M_PI 3.14159265358979323846f
#endif

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static int32_t s_disto_init(audio_algo_t *const pAlgo);
static int32_t s_disto_deinit(audio_algo_t *const pAlgo);
static int32_t s_disto_dataInOut(audio_algo_t *const pAlgo);

static float s_dcrem(dcrem_t *pHdle, float input);
static float s_softClipping(float input);
static float s_softClippingCubic(float input);


/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_disto_common =
{
  .pName                     = "disto",
  .misc.pAlgoDesc            = "Guitar distortion",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in_out = ABUFF_PARAM_ALL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_1CH,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_48000,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_YES,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = (audio_capability_type_t)((uint32_t)AUDIO_CAPABILITY_TYPE_FLOAT),
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("pIn\0"),
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("input pin\0"),
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_1CH,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_48000,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_YES,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = (audio_capability_type_t)((uint32_t)AUDIO_CAPABILITY_TYPE_FLOAT),
  .iosOut.pPinNames          = AUDIO_ALGO_OPT_STR("pOut\0"),
  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("output pin\0"),
};

audio_algo_cbs_t AudioChainWrp_disto_cbs =
{
  .init                       = s_disto_init,
  .deinit                     = s_disto_deinit,
  .dataInOut                  = s_disto_dataInOut,
  .process                    = NULL,
  .checkConsistency           = NULL,
  .configure                  = NULL,
  .control                    = NULL, /* Do not register any function in here. It is done through user API. */
  .isDisabled                 = NULL, /* Allows to define a behavior when algorithm is bypassed */
  .isDisabledCheckConsistency = NULL
};



/* Private Functions Definition ------------------------------------------------------*/


static int32_t s_disto_init(audio_algo_t *const pAlgo)
{
  int32_t                            error         = AUDIO_ERR_MGNT_NONE;
  disto_static_config_t const *const pStaticConfig = (disto_static_config_t *)AudioAlgo_getStaticConfig(pAlgo);
  audio_chunk_t         const *const pChunkIn      = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_buffer_t        const *const pBuffIn       = AudioChunk_getBuffInfo(pChunkIn);
  uint8_t                      const nbChannels    = AudioBuffer_getNbChannels(pBuffIn);
  uint32_t                     const nbSamples     = AudioBuffer_getNbSamples(pBuffIn);
  disto_context_t                   *pContext      = NULL;
  size_t                             allocSize     = sizeof(disto_context_t);
  memPool_t                          memPool       = AUDIO_MEM_UNKNOWN;

  if (pStaticConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static conf !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool  = (memPool_t)pStaticConfig->ramType;
    pContext = (disto_context_t *)AudioAlgo_malloc(allocSize, memPool);
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
    pContext->memPool         = memPool;
    pContext->nbChannels      = nbChannels;
    pContext->nbSamples       = nbSamples;
    pContext->gain_lin        = powf(10.0f, pStaticConfig->post_gain / 20.0f);  /*cstat !MISRAC2012-Rule-22.8 errno check is useless*/
    pContext->dry_gain_lin    = powf(10.0f, pStaticConfig->dry_gain / 20.0f);   /*cstat !MISRAC2012-Rule-22.8 errno check is useless*/
    pContext->pin.pChunk      = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
    pContext->pout.pChunk     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);

    switch (pStaticConfig->type)
    {
      case DISTO_CLIPPING_SOFT:
        pContext->process_cb = s_softClipping;
        break;
      case DISTO_CLIPPING_CUBIC:
        pContext->process_cb = s_softClippingCubic;
        break;
      default:
        error = AUDIO_ERR_MGNT_NOT_DONE;
        break;
    }

    if (pStaticConfig->off_axis != 0.0f)
    {
      pContext->dcrem.alpha = 0.99f;
      pContext->correction_cb = s_dcrem;
    }
    else
    {
      pContext->correction_cb = NULL;
    }
  }
  //  if (AudioError_isOk(error))
  //  {
  //    error = disto_init(pAlgo, /* ADD YOUR PARAM IF ANY */);
  //  }
  if (AudioError_isError(error))
  {
    error = s_disto_deinit(pAlgo);
  }
  return error;
}


static int32_t s_disto_deinit(audio_algo_t *const pAlgo)
{
  int32_t                error    = AUDIO_ERR_MGNT_NONE;
  disto_context_t *const pContext = (disto_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pContext->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    //    if (pContext->user.pReserved != NULL)
    //    {
    //      /* Free anything that needs to be */
    //    }

    AudioAlgo_free(pContext, memPool);
  }

  return error;
}


static int32_t s_disto_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                                error           = AUDIO_ERR_MGNT_NONE;
  disto_context_t                 *const pContext        = (disto_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint8_t                                nbChannels      = pContext->nbChannels;
  uint32_t                               nbSamples       = pContext->nbSamples;
  disto_static_config_t     const *const pStaticConfig   = AudioAlgo_getStaticConfig(pAlgo);

  /* Following line only needed if untyped data pointers are used to manage samples */
  float        const *const pIn                = (float const *)AudioChunk_getReadPtr(pContext->pin.pChunk, 0U, 0UL);

  /* Following line only needed if untyped data pointers are used to manage samples */
  float              *const pOut               = (float *)AudioChunk_getWritePtr(pContext->pout.pChunk, 0U, 0UL);

  //float post_attenuation = (1.0f / pContext->process_cb(pStaticConfig->drive)); // * pStaticConfig->range));

  if (AudioError_isOk(error))
  {
    for (uint8_t ch = 0U; ch < nbChannels; ch++)
    {
      for (uint32_t spl = 0UL; spl < nbSamples; spl++)
      {
        uint32_t index = ((uint32_t)nbChannels * spl) + (uint32_t)ch;
        float dry = pIn[index];

        // Apply drive to the input signal after offset to enhance even harmonics
        float wet = dry * pStaticConfig->drive;

        // Introduce an offset to create asymmetry
        wet += pStaticConfig->off_axis;

        // Full-wave rectification to enhance even harmonics
        //        if (pStaticConfig->rectification > 0)
        //        {
        //          wet = pStaticConfig->rectification * fabs(wet) * wet;
        //        }
        // Wave shaping function that enhances even harmonics
        // Example: Apply a symmetrical function that squares the signal
        // while maintaining its sign, which enhances even harmonics
        //        wet = (wet >= 0) ? pow(wet, 2) : -pow(wet, 2);

        // Soft clipping function
        float clipped = pContext->process_cb(wet);

        if (pContext->correction_cb != NULL)
        {
          clipped = pContext->correction_cb(&pContext->dcrem, clipped);
        }

        // Mix the processed signal (with even harmonics) with the original signal
        pOut[index] = ((clipped * pStaticConfig->dry_wet) + (dry * pContext->dry_gain_lin * (1.0f - pStaticConfig->dry_wet))) * pContext->gain_lin;
      }
    }

  }
  return error;
}

static float s_dcrem(dcrem_t *pHdle, float input)
{

  pHdle->xn            = input;
  /* Formula is yn = Alpha * (yn-1 + xn - xn-1) ;
  it comes from yn = xn - dcn where dcn = alpha* dcn-1 +(1-alpha) * xn ; going to Z transform
  we get H(z) = alpha * (1-z^(-1))/(1-alpha*z^(-1) )

  H(z) =   Y(z) / X(z)
  (1-alpha* z-1 ) Y(z) =  alpha * (1 - z-1 ) X(z)
  Y(z) = alpha * Y(z) * z-1 + alpha * (X(z) - X(z) * z-1))
  ==> yn = Alpha * (yn-1 + xn - xn-1)
  */
  pHdle->yn_1       = pHdle->alpha * (pHdle->yn_1 + pHdle->xn - pHdle->xn_1);
  pHdle->xn_1       = pHdle->xn;
  return pHdle->yn_1;
}
static float s_softClipping(float input)
{
  return (2.0f / M_PI) * atanf(input);   /*cstat !MISRAC2012-Rule-22.8 errno check is useless*/
}


static float s_softClippingCubic(float input)
{
  float ret = input;
  if (input > 1.0f)
  {
    ret = +0.6666666666666667f; /* +2/3 */
  }
  else if (input < -1.0f)
  {
    ret = -0.6666666666666667f; /* -2/3 */
  }
  else
  {
    ret = input - ((1.0f / 3.0f) * powf(input, 3.0f)); /*cstat !MISRAC2012-Rule-22.8 errno check is useless*/
  }
  return ret;
}




//static int32_t s_disto_process(audio_algo_t *const pAlgo)
//{
//  int32_t                                error           = AUDIO_ERR_MGNT_NONE;
//  if (AudioError_isOk(error))
//  {
//    error = disto_process(pAlgo, /* ADD YOUR PARAM IF ANY */);
//  }
//  return error;
//}
