/**
******************************************************************************
* @file    audio_chain_hpf.c
* @author  MCD Application Team
* @brief   wrapper of dc removing
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
#include <stdbool.h>
#include "hpf/audio_chain_hpf.h"
#include "sfc.h"
#include "common/FIR_DC_remove_highpass_coef.h"
#include "common/IIR_high_pass_coef.h"
#include "common/fir.h"
#include "common/biquad.h"

/* Private typedef -----------------------------------------------------------*/

typedef enum
{
  HPF_FS_8000,
  HPF_FS_12000,
  HPF_FS_16000,
  HPF_FS_24000,
  HPF_FS_32000,
  HPF_FS_48000,
  HPF_FS_96000,
  HPF_FS_NB,
} hpf_fs_t;

typedef enum
{
  HPF_METHOD_IIR,
  HPF_METHOD_FIR
} hpf_method_t;

typedef struct
{
  hpf_fs_t                   eFs;
  hpf_method_t               method;
  const audio_chunk_t       *pChunkIn;
  const audio_chunk_t       *pChunkOut;
  uint32_t                   nbSamples;
  uint8_t                    nbChannels;
  audio_buffer_interleaved_t interleaved;
  audio_buffer_type_t        sampleType;
  uint8_t                    nbFilterCtx;
  sfcContext_t              *pSfcInContext;
  sfcContext_t              *pSfcOutContext;
  audio_buffer_t            *pSfcBuffIn;
  audio_buffer_t            *pSfcBuffOut;
  union filterContext
  {
    firHandler_t             firContext;
    biquadIntContext_t       biquadIntContext;
    biquadFloatContext_t     biquadFloatContext;
  } *pFilterContext;
  memPool_t                  memPool;
} hpfCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef USE_HPF_TYPE_FIR
static firCoeffFloat_t const *const s_firCoeffsFloat[HPF_TYPE_LAST_FIR - HPF_TYPE_FIRST_FIR + 1][HPF_FS_NB] =
{
  [HPF_TYPE_PARKSMCCLELLAN - HPF_TYPE_FIRST_FIR] =
  {
    [HPF_FS_8000]  = FIR_PARKSMCCLELLAN_DC_REMOVE_FS8000_FIR_FLOAT,
    [HPF_FS_12000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS12000_FIR_FLOAT,
    [HPF_FS_16000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS16000_FIR_FLOAT,
    [HPF_FS_24000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS24000_FIR_FLOAT,
    [HPF_FS_32000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS32000_FIR_FLOAT,
    [HPF_FS_48000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS48000_FIR_FLOAT,
    [HPF_FS_96000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS96000_FIR_FLOAT,
  },
  [HPF_TYPE_KAISERWINDOW - HPF_TYPE_FIRST_FIR] =
  {
    [HPF_FS_8000]  = FIR_KAISERWINDOW_DC_REMOVE_FS8000_FIR_FLOAT,
    [HPF_FS_12000] = FIR_KAISERWINDOW_DC_REMOVE_FS12000_FIR_FLOAT,
    [HPF_FS_16000] = FIR_KAISERWINDOW_DC_REMOVE_FS16000_FIR_FLOAT,
    [HPF_FS_24000] = FIR_KAISERWINDOW_DC_REMOVE_FS24000_FIR_FLOAT,
    [HPF_FS_32000] = FIR_KAISERWINDOW_DC_REMOVE_FS32000_FIR_FLOAT,
    [HPF_FS_48000] = FIR_KAISERWINDOW_DC_REMOVE_FS48000_FIR_FLOAT,
    [HPF_FS_96000] = FIR_KAISERWINDOW_DC_REMOVE_FS96000_FIR_FLOAT
  }
};

static firCoeffInt16_t const *const s_firCoeffsInt16[HPF_TYPE_LAST_FIR - HPF_TYPE_FIRST_FIR + 1][HPF_FS_NB] =
{
  [HPF_TYPE_PARKSMCCLELLAN - HPF_TYPE_FIRST_FIR] =
  {
    [HPF_FS_8000]  = FIR_PARKSMCCLELLAN_DC_REMOVE_FS8000_FIR_INT16,
    [HPF_FS_12000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS12000_FIR_INT16,
    [HPF_FS_16000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS16000_FIR_INT16,
    [HPF_FS_24000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS24000_FIR_INT16,
    [HPF_FS_32000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS32000_FIR_INT16,
    [HPF_FS_48000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS48000_FIR_INT16,
    [HPF_FS_96000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS96000_FIR_INT16
  },
  [HPF_TYPE_KAISERWINDOW - HPF_TYPE_FIRST_FIR] =
  {
    [HPF_FS_8000]  = FIR_KAISERWINDOW_DC_REMOVE_FS8000_FIR_INT16,
    [HPF_FS_12000] = FIR_KAISERWINDOW_DC_REMOVE_FS12000_FIR_INT16,
    [HPF_FS_16000] = FIR_KAISERWINDOW_DC_REMOVE_FS16000_FIR_INT16,
    [HPF_FS_24000] = FIR_KAISERWINDOW_DC_REMOVE_FS24000_FIR_INT16,
    [HPF_FS_32000] = FIR_KAISERWINDOW_DC_REMOVE_FS32000_FIR_INT16,
    [HPF_FS_48000] = FIR_KAISERWINDOW_DC_REMOVE_FS48000_FIR_INT16,
    [HPF_FS_96000] = FIR_KAISERWINDOW_DC_REMOVE_FS96000_FIR_INT16
  }
};

static firCoeffInt32_t const *const s_firCoeffsInt32[HPF_TYPE_LAST_FIR - HPF_TYPE_FIRST_FIR + 1][HPF_FS_NB] =
{
  [HPF_TYPE_PARKSMCCLELLAN - HPF_TYPE_FIRST_FIR] =
  {
    [HPF_FS_8000]  = FIR_PARKSMCCLELLAN_DC_REMOVE_FS8000_FIR_INT32,
    [HPF_FS_12000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS12000_FIR_INT32,
    [HPF_FS_16000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS16000_FIR_INT32,
    [HPF_FS_24000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS24000_FIR_INT32,
    [HPF_FS_32000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS32000_FIR_INT32,
    [HPF_FS_48000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS48000_FIR_INT32,
    [HPF_FS_96000] = FIR_PARKSMCCLELLAN_DC_REMOVE_FS96000_FIR_INT32
  },
  [HPF_TYPE_KAISERWINDOW - HPF_TYPE_FIRST_FIR] =
  {
    [HPF_FS_8000]  = FIR_KAISERWINDOW_DC_REMOVE_FS8000_FIR_INT32,
    [HPF_FS_12000] = FIR_KAISERWINDOW_DC_REMOVE_FS12000_FIR_INT32,
    [HPF_FS_16000] = FIR_KAISERWINDOW_DC_REMOVE_FS16000_FIR_INT32,
    [HPF_FS_24000] = FIR_KAISERWINDOW_DC_REMOVE_FS24000_FIR_INT32,
    [HPF_FS_32000] = FIR_KAISERWINDOW_DC_REMOVE_FS32000_FIR_INT32,
    [HPF_FS_48000] = FIR_KAISERWINDOW_DC_REMOVE_FS48000_FIR_INT32,
    [HPF_FS_96000] = FIR_KAISERWINDOW_DC_REMOVE_FS96000_FIR_INT32
  }
};
#endif

#ifdef USE_HPF_TYPE_IIR
static biquadFloat_t const *const s_biquadCoeffsFloat[HPF_TYPE_LAST_IIR - HPF_TYPE_FIRST_IIR + 1][HPF_FS_NB] =
{
  [HPF_TYPE_BUTTERWORTH - HPF_TYPE_FIRST_IIR] =
  {
    [HPF_FS_8000]  = IIR_BUTTERWORTH_DC_REMOVE_FS8000_BIQUAD_FLOAT,
    [HPF_FS_12000] = IIR_BUTTERWORTH_DC_REMOVE_FS12000_BIQUAD_FLOAT,
    [HPF_FS_16000] = IIR_BUTTERWORTH_DC_REMOVE_FS16000_BIQUAD_FLOAT,
    [HPF_FS_24000] = IIR_BUTTERWORTH_DC_REMOVE_FS24000_BIQUAD_FLOAT,
    [HPF_FS_32000] = IIR_BUTTERWORTH_DC_REMOVE_FS32000_BIQUAD_FLOAT,
    [HPF_FS_48000] = IIR_BUTTERWORTH_DC_REMOVE_FS48000_BIQUAD_FLOAT,
    [HPF_FS_96000] = IIR_BUTTERWORTH_DC_REMOVE_FS96000_BIQUAD_FLOAT
  },
  [HPF_TYPE_CHEBYSHEV1 - HPF_TYPE_FIRST_IIR] =
  {
    [HPF_FS_8000]  = IIR_CHEBYSHEV1_DC_REMOVE_FS8000_BIQUAD_FLOAT,
    [HPF_FS_12000] = IIR_CHEBYSHEV1_DC_REMOVE_FS12000_BIQUAD_FLOAT,
    [HPF_FS_16000] = IIR_CHEBYSHEV1_DC_REMOVE_FS16000_BIQUAD_FLOAT,
    [HPF_FS_24000] = IIR_CHEBYSHEV1_DC_REMOVE_FS24000_BIQUAD_FLOAT,
    [HPF_FS_32000] = IIR_CHEBYSHEV1_DC_REMOVE_FS32000_BIQUAD_FLOAT,
    [HPF_FS_48000] = IIR_CHEBYSHEV1_DC_REMOVE_FS48000_BIQUAD_FLOAT,
    [HPF_FS_96000] = IIR_CHEBYSHEV1_DC_REMOVE_FS96000_BIQUAD_FLOAT
  },
  [HPF_TYPE_CHEBYSHEV2 - HPF_TYPE_FIRST_IIR] =
  {
    [HPF_FS_8000]  = IIR_CHEBYSHEV2_DC_REMOVE_FS8000_BIQUAD_FLOAT,
    [HPF_FS_12000] = IIR_CHEBYSHEV2_DC_REMOVE_FS12000_BIQUAD_FLOAT,
    [HPF_FS_16000] = IIR_CHEBYSHEV2_DC_REMOVE_FS16000_BIQUAD_FLOAT,
    [HPF_FS_24000] = IIR_CHEBYSHEV2_DC_REMOVE_FS24000_BIQUAD_FLOAT,
    [HPF_FS_32000] = IIR_CHEBYSHEV2_DC_REMOVE_FS32000_BIQUAD_FLOAT,
    [HPF_FS_48000] = IIR_CHEBYSHEV2_DC_REMOVE_FS48000_BIQUAD_FLOAT,
    [HPF_FS_96000] = IIR_CHEBYSHEV2_DC_REMOVE_FS96000_BIQUAD_FLOAT
  },
  [HPF_TYPE_ELLIPTIC - HPF_TYPE_FIRST_IIR] =
  {
    [HPF_FS_8000]  = IIR_ELLIPTIC_DC_REMOVE_FS8000_BIQUAD_FLOAT,
    [HPF_FS_12000] = IIR_ELLIPTIC_DC_REMOVE_FS12000_BIQUAD_FLOAT,
    [HPF_FS_16000] = IIR_ELLIPTIC_DC_REMOVE_FS16000_BIQUAD_FLOAT,
    [HPF_FS_24000] = IIR_ELLIPTIC_DC_REMOVE_FS24000_BIQUAD_FLOAT,
    [HPF_FS_32000] = IIR_ELLIPTIC_DC_REMOVE_FS32000_BIQUAD_FLOAT,
    [HPF_FS_48000] = IIR_ELLIPTIC_DC_REMOVE_FS48000_BIQUAD_FLOAT,
    [HPF_FS_96000] = IIR_ELLIPTIC_DC_REMOVE_FS96000_BIQUAD_FLOAT
  }
};

static biquadInt32_t const *const s_biquadCoeffsInt32[HPF_TYPE_LAST_IIR - HPF_TYPE_BUTTERWORTH + 1][HPF_FS_NB] =
{
  [HPF_TYPE_BUTTERWORTH - HPF_TYPE_FIRST_IIR] =
  {
    [HPF_FS_8000]  = IIR_BUTTERWORTH_DC_REMOVE_FS8000_BIQUAD_INT32,
    [HPF_FS_12000] = IIR_BUTTERWORTH_DC_REMOVE_FS12000_BIQUAD_INT32,
    [HPF_FS_16000] = IIR_BUTTERWORTH_DC_REMOVE_FS16000_BIQUAD_INT32,
    [HPF_FS_24000] = IIR_BUTTERWORTH_DC_REMOVE_FS24000_BIQUAD_INT32,
    [HPF_FS_32000] = IIR_BUTTERWORTH_DC_REMOVE_FS32000_BIQUAD_INT32,
    [HPF_FS_48000] = IIR_BUTTERWORTH_DC_REMOVE_FS48000_BIQUAD_INT32,
    [HPF_FS_96000] = IIR_BUTTERWORTH_DC_REMOVE_FS96000_BIQUAD_INT32
  },
  [HPF_TYPE_CHEBYSHEV1 - HPF_TYPE_FIRST_IIR] =
  {
    [HPF_FS_8000]  = IIR_CHEBYSHEV1_DC_REMOVE_FS8000_BIQUAD_INT32,
    [HPF_FS_12000] = IIR_CHEBYSHEV1_DC_REMOVE_FS12000_BIQUAD_INT32,
    [HPF_FS_16000] = IIR_CHEBYSHEV1_DC_REMOVE_FS16000_BIQUAD_INT32,
    [HPF_FS_24000] = IIR_CHEBYSHEV1_DC_REMOVE_FS24000_BIQUAD_INT32,
    [HPF_FS_32000] = IIR_CHEBYSHEV1_DC_REMOVE_FS32000_BIQUAD_INT32,
    [HPF_FS_48000] = IIR_CHEBYSHEV1_DC_REMOVE_FS48000_BIQUAD_INT32,
    [HPF_FS_96000] = IIR_CHEBYSHEV1_DC_REMOVE_FS96000_BIQUAD_INT32
  },
  [HPF_TYPE_CHEBYSHEV2 - HPF_TYPE_FIRST_IIR] =
  {
    [HPF_FS_8000]  = IIR_CHEBYSHEV2_DC_REMOVE_FS8000_BIQUAD_INT32,
    [HPF_FS_12000] = IIR_CHEBYSHEV2_DC_REMOVE_FS12000_BIQUAD_INT32,
    [HPF_FS_16000] = IIR_CHEBYSHEV2_DC_REMOVE_FS16000_BIQUAD_INT32,
    [HPF_FS_24000] = IIR_CHEBYSHEV2_DC_REMOVE_FS24000_BIQUAD_INT32,
    [HPF_FS_32000] = IIR_CHEBYSHEV2_DC_REMOVE_FS32000_BIQUAD_INT32,
    [HPF_FS_48000] = IIR_CHEBYSHEV2_DC_REMOVE_FS48000_BIQUAD_INT32,
    [HPF_FS_96000] = IIR_CHEBYSHEV2_DC_REMOVE_FS96000_BIQUAD_INT32
  },
  [HPF_TYPE_ELLIPTIC - HPF_TYPE_FIRST_IIR] =
  {
    [HPF_FS_8000]  = IIR_ELLIPTIC_DC_REMOVE_FS8000_BIQUAD_INT32,
    [HPF_FS_12000] = IIR_ELLIPTIC_DC_REMOVE_FS12000_BIQUAD_INT32,
    [HPF_FS_16000] = IIR_ELLIPTIC_DC_REMOVE_FS16000_BIQUAD_INT32,
    [HPF_FS_24000] = IIR_ELLIPTIC_DC_REMOVE_FS24000_BIQUAD_INT32,
    [HPF_FS_32000] = IIR_ELLIPTIC_DC_REMOVE_FS32000_BIQUAD_INT32,
    [HPF_FS_48000] = IIR_ELLIPTIC_DC_REMOVE_FS48000_BIQUAD_INT32,
    [HPF_FS_96000] = IIR_ELLIPTIC_DC_REMOVE_FS96000_BIQUAD_INT32
  }
};
#endif

/* Private function prototypes -----------------------------------------------*/
static int32_t s_hpf_deinit(audio_algo_t    *const pAlgo);
static int32_t s_hpf_init(audio_algo_t      *const pAlgo);
static int32_t s_hpf_dataInOut(audio_algo_t *const pAlgo);
static int32_t s_hpf_process(audio_algo_t   *const pAlgo);

static int32_t s_set_fs_enum(uint32_t const fs_value, hpf_fs_t *const pFsEnum);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_hpf_common =
{
  .pName                     = "hpf",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_ALL,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Direct current remover (cut frequency 50 Hz, pass frequency 200 Hz)"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("hpf")
};

audio_algo_cbs_t AudioChainWrp_hpf_cbs =
{
  .init                       = s_hpf_init,
  .deinit                     = s_hpf_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_hpf_dataInOut,
  .process                    = s_hpf_process,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_hpf_init(audio_algo_t *const pAlgo)
{
  int32_t                          error         = AUDIO_ERR_MGNT_NONE;
  hpf_static_config_t const *const pStaticConfig = (hpf_static_config_t const *)AudioAlgo_getStaticConfig(pAlgo);
  audio_chunk_t       const *const pChunkIn      = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t       const *const pChunkOut     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t      const *const pBuffIn       = AudioChunk_getBuffInfo(pChunkIn);
  uint8_t                    const nbChannels    = AudioBuffer_getNbChannels(pBuffIn);
  uint32_t                   const nbSamples     = AudioBuffer_getNbElements(pBuffIn);
  uint32_t                   const fs            = AudioBuffer_getFs(pBuffIn);
  audio_buffer_type_t        const sampleType    = AudioBuffer_getType(pBuffIn);
  audio_buffer_interleaved_t const interleaved   = AudioBuffer_getInterleaved(pBuffIn);
  hpfCtx_t                        *pHpfCtx       = NULL;
  uint8_t                          nbFilterCtx   = 0U;
  size_t                           allocSize     = 0UL;
  memPool_t                        memPool       = AUDIO_MEM_UNKNOWN;
  hpf_method_t                     method        = HPF_METHOD_IIR;
  #ifdef USE_HPF_TYPE_FIR
  audio_chain_utilities_t   *const pUtilsHdle    = AudioAlgo_getUtilsHdle(pAlgo);
  bool                             sfcForFir     = false;
  #endif

  if (pStaticConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static conf !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool   = (memPool_t)pStaticConfig->ramType;
    allocSize = sizeof(hpfCtx_t);
    switch (pStaticConfig->filterType)
    {
        #ifdef USE_HPF_TYPE_FIR
      case HPF_TYPE_PARKSMCCLELLAN:
      case HPF_TYPE_KAISERWINDOW:
        method      = HPF_METHOD_FIR;
        nbFilterCtx = 1U;
        sfcForFir   = (nbChannels > 1U) && (interleaved == ABUFF_FORMAT_INTERLEAVED);
        if (sfcForFir)
        {
          allocSize += 2UL * (sizeof(sfcContext_t) + sizeof(audio_buffer_t));
        }
        break;
        #endif

        #ifdef USE_HPF_TYPE_IIR
      case HPF_TYPE_BUTTERWORTH:
      case HPF_TYPE_CHEBYSHEV1:
      case HPF_TYPE_CHEBYSHEV2:
      case HPF_TYPE_ELLIPTIC:
        method      = HPF_METHOD_IIR;
        nbFilterCtx = (interleaved == ABUFF_FORMAT_NON_INTERLEAVED) ? nbChannels : 1U;
        break;
        #endif

      default:
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported filter type %d", pStaticConfig->filterType);
        error = AUDIO_ERR_MGNT_INIT;
        break;
    }
    allocSize += (size_t)nbFilterCtx * sizeof(union filterContext);
  }

  if (AudioError_isOk(error))
  {
    pHpfCtx = (hpfCtx_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pHpfCtx == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "context malloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    AudioAlgo_setWrapperContext(pAlgo, pHpfCtx);
    memset(pHpfCtx, 0, allocSize);

    pHpfCtx->memPool     = memPool;
    pHpfCtx->method      = method;
    pHpfCtx->pChunkIn    = pChunkIn;
    pHpfCtx->pChunkOut   = pChunkOut;
    pHpfCtx->nbChannels  = nbChannels;
    pHpfCtx->nbSamples   = nbSamples;
    pHpfCtx->interleaved = interleaved;
    pHpfCtx->sampleType  = sampleType;
    pHpfCtx->nbFilterCtx = nbFilterCtx;

    error = s_set_fs_enum(fs, &pHpfCtx->eFs);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported sampling frequency %d", fs);
    }
  }

  if (AudioError_isOk(error))
  {
    switch (method)
    {
        #ifdef USE_HPF_TYPE_FIR
      case HPF_METHOD_FIR:
        pHpfCtx->pFilterContext = (union filterContext *)&pHpfCtx[1];
        if (sfcForFir)
        {
          pHpfCtx->pSfcInContext  = (sfcContext_t *)&pHpfCtx->pFilterContext[nbFilterCtx];
          pHpfCtx->pSfcOutContext = &pHpfCtx->pSfcInContext[1];
          pHpfCtx->pSfcBuffIn     = (audio_buffer_t *)&pHpfCtx->pSfcOutContext[1];
          error                   = AudioBuffer_create(pHpfCtx->pSfcBuffIn, nbChannels, fs, nbSamples, ABUFF_FORMAT_TIME, sampleType, ABUFF_FORMAT_NON_INTERLEAVED, memPool);
          if (AudioError_isError(error))
          {
            AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "AudioBuffer_create(pHpfCtx->pSfcBuffIn, ...) error !");
          }
          if (AudioError_isOk(error))
          {
            pHpfCtx->pSfcBuffOut = &pHpfCtx->pSfcBuffIn[1];
            error                = AudioBuffer_create(pHpfCtx->pSfcBuffOut, nbChannels, fs, nbSamples, ABUFF_FORMAT_TIME, sampleType, ABUFF_FORMAT_NON_INTERLEAVED, memPool);
            if (AudioError_isError(error))
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "AudioBuffer_create(pHpfCtx->pSfcBuffOut, ...) error !");
            }
          }
          if (AudioError_isOk(error))
          {
            error = sfcSetContext(pHpfCtx->pSfcInContext, pBuffIn, pHpfCtx->pSfcBuffIn, false, 1.0f, pUtilsHdle);
            if (AudioError_isError(error))
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(pHpfCtx->pSfcInContext, ...) error");
            }
          }
          if (AudioError_isOk(error))
          {
            error = sfcSetContext(pHpfCtx->pSfcOutContext, pHpfCtx->pSfcBuffOut, AudioChunk_getBuffInfo(pChunkOut), false, 1.0f, pUtilsHdle);
            if (AudioError_isError(error))
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(pHpfCtx->pSfcOutContext, ...) error");
            }
          }
        }

        if (AudioError_isOk(error))
        {
          void    const *pFirVoid = NULL;
          uint8_t const  firType  = (uint8_t)pStaticConfig->filterType - (uint8_t)HPF_TYPE_FIRST_FIR;

          switch (sampleType)
          {
            case ABUFF_FORMAT_FIXED16:
              pFirVoid = s_firCoeffsInt16[firType][pHpfCtx->eFs];
              break;

            case ABUFF_FORMAT_FIXED32:
              pFirVoid = s_firCoeffsInt32[firType][pHpfCtx->eFs];
              break;

            case ABUFF_FORMAT_FLOAT:
              pFirVoid = s_firCoeffsFloat[firType][pHpfCtx->eFs];
              break;

            default:
              error = AUDIO_ERR_MGNT_INIT;
              break;
          }
          error = firInit(&pHpfCtx->pFilterContext[0].firContext, pFirVoid, sampleType, nbChannels, nbSamples, 1U, 1U, memPool);
          if (AudioError_isError(error))
          {
            AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "firInit error");
          }
        }
        break;
        #endif

        #ifdef USE_HPF_TYPE_IIR
      case HPF_METHOD_IIR:
      {
        int     const nbBiquadCh = (interleaved == ABUFF_FORMAT_INTERLEAVED) ? (int)nbChannels : 1;
        uint8_t const iirType    = (uint8_t)pStaticConfig->filterType - (uint8_t)HPF_TYPE_FIRST_IIR;

        pHpfCtx->pFilterContext = (union filterContext *)&pHpfCtx[1];
        switch (sampleType)
        {
          case ABUFF_FORMAT_FIXED16:
            for (uint8_t i = 0U; AudioError_isOk(error) && (i < nbFilterCtx); i++)
            {
              error = biquadInt16Init(&pHpfCtx->pFilterContext[i].biquadIntContext,
                                      s_biquadCoeffsInt32[iirType][pHpfCtx->eFs],
                                      nbBiquadCh,
                                      1,    // decimFactor
                                      1,    // upSamplingFactor
                                      memPool);
              if (AudioError_isError(error))
              {
                AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "biquadInt16Init error");
              }
            }
            break;

          case ABUFF_FORMAT_FIXED32:
            for (uint8_t i = 0U; AudioError_isOk(error) && (i < nbFilterCtx); i++)
            {
              error = biquadInt32Init(&pHpfCtx->pFilterContext[i].biquadIntContext,
                                      s_biquadCoeffsInt32[iirType][pHpfCtx->eFs],
                                      nbBiquadCh,
                                      1,    // decimFactor
                                      1,    // upSamplingFactor
                                      memPool);
              if (AudioError_isError(error))
              {
                AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "biquadInt32Init error");
              }
            }
            break;

          case ABUFF_FORMAT_FLOAT:
            for (uint8_t i = 0U; AudioError_isOk(error) && (i < nbFilterCtx); i++)
            {
              error = biquadFloatInit(&pHpfCtx->pFilterContext[i].biquadFloatContext,
                                      s_biquadCoeffsFloat[iirType][pHpfCtx->eFs],
                                      nbBiquadCh,
                                      1,    // decimFactor
                                      1,    // upSamplingFactor
                                      memPool);
              if (AudioError_isError(error))
              {
                AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "biquadFloatInit error");
              }
            }
            break;

          default:
            error = AUDIO_ERR_MGNT_INIT;
            break;
        }
        break;
      }
      #endif

      default:
        break;
    }
  }

  if (AudioError_isError(error))
  {
    s_hpf_deinit(pAlgo);
  }

  return error;
}


static int32_t s_set_fs_enum(uint32_t const fs_value, hpf_fs_t *const pFsEnum)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  switch (fs_value)
  {
    case 8000UL:
      pFsEnum[0] = HPF_FS_8000;
      break;
    case 12000UL:
      pFsEnum[0] = HPF_FS_12000;
      break;
    case 16000UL:
      pFsEnum[0] = HPF_FS_16000;
      break;
    case 24000UL:
      pFsEnum[0] = HPF_FS_24000;
      break;
    case 32000UL:
      pFsEnum[0] = HPF_FS_32000;
      break;
    case 48000UL:
      pFsEnum[0] = HPF_FS_48000;
      break;
    case 96000UL:
      pFsEnum[0] = HPF_FS_96000;
      break;
    default:
      error = AUDIO_ERR_MGNT_INIT;
      break;
  }
  return error;
}


static int32_t s_hpf_deinit(audio_algo_t *const pAlgo)
{
  int32_t         error   = AUDIO_ERR_MGNT_NONE;
  hpfCtx_t *const pHpfCtx = (hpfCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pHpfCtx != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pHpfCtx->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);

    switch (pHpfCtx->method)
    {
        #ifdef USE_HPF_TYPE_FIR
      case HPF_METHOD_FIR:
        if (pHpfCtx->pSfcBuffIn != NULL)
        {
          AudioBuffer_deinit(pHpfCtx->pSfcBuffIn);
        }
        if (pHpfCtx->pSfcBuffOut != NULL)
        {
          AudioBuffer_deinit(pHpfCtx->pSfcBuffOut);
        }
        firDeInit(&pHpfCtx->pFilterContext[0].firContext);
        break;
        #endif

        #ifdef USE_HPF_TYPE_IIR
      case HPF_METHOD_IIR:
        switch (pHpfCtx->sampleType)
        {
          case ABUFF_FORMAT_FIXED16:
            for (uint8_t i = 0U; i < pHpfCtx->nbFilterCtx; i++)
            {
              biquadInt16DeInit(&pHpfCtx->pFilterContext[i].biquadIntContext);
            }
            break;
          case ABUFF_FORMAT_FIXED32:
            for (uint8_t i = 0U; i < pHpfCtx->nbFilterCtx; i++)
            {
              biquadInt32DeInit(&pHpfCtx->pFilterContext[i].biquadIntContext);
            }
            break;
          case ABUFF_FORMAT_FLOAT:
            for (uint8_t i = 0U; i < pHpfCtx->nbFilterCtx; i++)
            {
              biquadFloatDeInit(&pHpfCtx->pFilterContext[i].biquadFloatContext);
            }
            break;
          default:
            error = AUDIO_ERR_MGNT_INIT;
            break;
        }
        break;
        #endif

      default:
        break;
    }

    AudioAlgo_free(pHpfCtx, memPool);
  }

  return error;
}


static int32_t s_hpf_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  #ifdef USE_HPF_TYPE_FIR
  hpfCtx_t *const pHpfCtx = (hpfCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pHpfCtx->pSfcInContext != NULL)
  {
    // copy input samples into pSfcBuffIn with interleaving conversion
    sfcSampleBufferConvert(pHpfCtx->pSfcInContext, AudioChunk_getReadPtr0(pHpfCtx->pChunkIn), AudioBuffer_getPdata(pHpfCtx->pSfcBuffIn), (int)pHpfCtx->nbChannels, (int)pHpfCtx->nbSamples);
  }

  if (pHpfCtx->pSfcOutContext != NULL)
  {
    // copy output samples from pSfcBuffOut with interleaving conversion
    sfcSampleBufferConvert(pHpfCtx->pSfcOutContext, AudioBuffer_getPdata(pHpfCtx->pSfcBuffOut), AudioChunk_getWritePtr0(pHpfCtx->pChunkOut), (int)pHpfCtx->nbChannels, (int)pHpfCtx->nbSamples);
  }
  #endif

  // no need to disable irq because process task is lower priority
  AudioAlgo_incReadyForProcess(pAlgo);

  return error;
}


static int32_t s_hpf_process(audio_algo_t *const pAlgo)
{
  hpfCtx_t *const pHpfCtx = (hpfCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  switch (pHpfCtx->method)
  {
      #ifdef USE_HPF_TYPE_FIR
    case HPF_METHOD_FIR:
    {
      for (uint8_t ch = 0U; ch < pHpfCtx->nbChannels; ch++)
      {
        void *const pIn  = (pHpfCtx->pSfcBuffIn  != NULL) ? AudioBuffer_getPdataCh(pHpfCtx->pSfcBuffIn, ch)  : AudioChunk_getReadPtr(pHpfCtx->pChunkIn,   ch, 0UL);
        void *const pOut = (pHpfCtx->pSfcBuffOut != NULL) ? AudioBuffer_getPdataCh(pHpfCtx->pSfcBuffOut, ch) : AudioChunk_getWritePtr(pHpfCtx->pChunkOut, ch, 0UL);

        firProcess(&pHpfCtx->pFilterContext[0].firContext, pIn, pOut, (int)ch, (int)pHpfCtx->nbSamples);
      }
      break;
    }
    #endif

    #ifdef USE_HPF_TYPE_IIR
    case HPF_METHOD_IIR:
      // low-pass filter of this signal and hpf it
      switch (pHpfCtx->sampleType)
      {
        case ABUFF_FORMAT_FIXED16:
          for (uint8_t i = 0U; i < pHpfCtx->nbFilterCtx; i++)
          {
            biquadInt16Process(&pHpfCtx->pFilterContext[i].biquadIntContext,
                               (int16_t *)AudioChunk_getReadPtr(pHpfCtx->pChunkIn,   i, 0UL),
                               (int16_t *)AudioChunk_getWritePtr(pHpfCtx->pChunkOut, i, 0UL),
                               (int)pHpfCtx->nbSamples);
          }
          break;

        case ABUFF_FORMAT_FIXED32:
          for (uint8_t i = 0U; i < pHpfCtx->nbFilterCtx; i++)
          {
            biquadInt32Process(&pHpfCtx->pFilterContext[i].biquadIntContext,
                               (int32_t *)AudioChunk_getReadPtr(pHpfCtx->pChunkIn,   i, 0UL),
                               (int32_t *)AudioChunk_getWritePtr(pHpfCtx->pChunkOut, i, 0UL),
                               (int)pHpfCtx->nbSamples);
          }
          break;

        case ABUFF_FORMAT_FLOAT:
          for (uint8_t i = 0U; i < pHpfCtx->nbFilterCtx; i++)
          {
            biquadFloatProcess(&pHpfCtx->pFilterContext[i].biquadFloatContext,
                               (float *)AudioChunk_getReadPtr(pHpfCtx->pChunkIn,   i, 0UL),
                               (float *)AudioChunk_getWritePtr(pHpfCtx->pChunkOut, i, 0UL),
                               (int)pHpfCtx->nbSamples);
          }
          break;

        default:
          break;
      }
      break;
      #endif

    default:
      break;
  }

  return AUDIO_ERR_MGNT_NONE;
}
