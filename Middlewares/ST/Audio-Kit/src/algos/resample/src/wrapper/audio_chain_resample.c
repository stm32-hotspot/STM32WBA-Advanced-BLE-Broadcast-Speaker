/**
******************************************************************************
* @file    audio_chain_resample.c
* @author  MCD Application Team
* @brief   wrapper of resampling (upsampling & downsampling) algo to match usage inside audio_chain.c
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
#include "resample/audio_chain_resample.h"
#include "sfc.h"
#include "common/FIR_resampling_lowpass_coef.h"
#include "common/IIR_high_pass_coef.h"  // for flat biquad
#include "common/IIR_low_pass_coef.h"
#include "common/fir.h"
#include "common/biquad.h"
#include "common/util.h"

/* Private typedef -----------------------------------------------------------*/

typedef enum
{
  RESAMPLE_RATIO_1,
  RESAMPLE_RATIO_2,
  RESAMPLE_RATIO_3,
  RESAMPLE_RATIO_4,
  RESAMPLE_RATIO_6,
  RESAMPLE_RATIO_8,
  RESAMPLE_RATIO_12,
  RESAMPLE_RATIO_NB
} resample_ratio_t;

typedef enum
{
  RESAMPLE_METHOD_NOFILTERING,
  RESAMPLE_METHOD_IIR,
  RESAMPLE_METHOD_FIR
} resample_method_t;

typedef struct
{
  resample_method_t          method;
  resample_ratio_t           eDownSamplingRatio;
  resample_ratio_t           eUpSamplingRatio;
  resample_ratio_t           eRatioLowPassFilter;
  uint8_t                    downSamplingRatio;
  uint8_t                    upSamplingRatio;
  const audio_chunk_t       *pChunkIn;
  const audio_chunk_t       *pChunkOut;
  uint8_t                    nbChannels;
  uint32_t                   nbSamplesIn;
  uint32_t                   nbSamplesOut;
  audio_buffer_interleaved_t interleaved;
  audio_buffer_type_t        sampleType;
  uint8_t                    nbFilterCtx;
  audio_buffer_t            *pSfcBuffIn;
  audio_buffer_t            *pSfcBuffOut;
  sfcContext_t              *pSfcInForFirContext;
  sfcContext_t              *pSfcOutForFirContext;
  sfcContext_t              *pSfcForNoFilterContext;
  union filterContext
  {
    firHandler_t             firContext;
    biquadIntContext_t       biquadIntContext;
    biquadFloatContext_t     biquadFloatContext;
  } *pFilterContext;
  memPool_t                  memPool;
} resampleCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef USE_RESAMPLE_TYPE_FIR
static float const flatTapsFloat[1] = {1.0f};
static firCoeffFloat_t const FIR_flatFloat =
{
  .nbTaps = 1U,
  .pTaps  = flatTapsFloat
};

static int16_t const flatTapsInt16[1] = {(int16_t)37267};
static firCoeffInt16_t const FIR_flatInt16 =
{
  .nbTaps = 1U,
  .pTaps  = flatTapsInt16
};

static int32_t const flatTapsInt32[1] = {2147483647L};
static firCoeffInt32_t const FIR_flatInt32 =
{
  .nbTaps = 1U,
  .pTaps  = flatTapsInt32
};

// WARNING: temporary workaround while ratio8 filter bank doesn't exist !!!
#define FIR_ParksMcClellan_resampling_ratio8_firFloat FIR_ParksMcClellan_resampling_ratio6_firFloat
#define FIR_KaiserWindow_resampling_ratio8_firFloat   FIR_KaiserWindow_resampling_ratio6_firFloat
#define FIR_ParksMcClellan_resampling_ratio8_firInt16 FIR_ParksMcClellan_resampling_ratio6_firInt16
#define FIR_KaiserWindow_resampling_ratio8_firInt16   FIR_KaiserWindow_resampling_ratio6_firInt16
#define FIR_ParksMcClellan_resampling_ratio8_firInt32 FIR_ParksMcClellan_resampling_ratio6_firInt32
#define FIR_KaiserWindow_resampling_ratio8_firInt32   FIR_KaiserWindow_resampling_ratio6_firInt32

static firCoeffFloat_t const *const s_firCoeffsFloat[RESAMPLE_TYPE_LAST_FIR - RESAMPLE_TYPE_FIRST_FIR + 1][RESAMPLE_RATIO_NB] =
{
  [RESAMPLE_TYPE_PARKSMCCLELLAN - RESAMPLE_TYPE_FIRST_FIR] =
  {
    [RESAMPLE_RATIO_1] = &FIR_flatFloat,
    [RESAMPLE_RATIO_2] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO2_FIR_FLOAT,
    [RESAMPLE_RATIO_3] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO3_FIR_FLOAT,
    [RESAMPLE_RATIO_4] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO4_FIR_FLOAT,
    [RESAMPLE_RATIO_6] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO6_FIR_FLOAT,
    [RESAMPLE_RATIO_8] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO8_FIR_FLOAT,
    [RESAMPLE_RATIO_12] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO12_FIR_FLOAT
  },
  [RESAMPLE_TYPE_KAISERWINDOW - RESAMPLE_TYPE_FIRST_FIR] =
  {
    [RESAMPLE_RATIO_1] = &FIR_flatFloat,
    [RESAMPLE_RATIO_2] = FIR_KAISERWINDOW_RESAMPLING_RATIO2_FIR_FLOAT,
    [RESAMPLE_RATIO_3] = FIR_KAISERWINDOW_RESAMPLING_RATIO3_FIR_FLOAT,
    [RESAMPLE_RATIO_4] = FIR_KAISERWINDOW_RESAMPLING_RATIO4_FIR_FLOAT,
    [RESAMPLE_RATIO_6] = FIR_KAISERWINDOW_RESAMPLING_RATIO6_FIR_FLOAT,
    [RESAMPLE_RATIO_8] = FIR_KAISERWINDOW_RESAMPLING_RATIO8_FIR_FLOAT,
    [RESAMPLE_RATIO_12] = FIR_KAISERWINDOW_RESAMPLING_RATIO12_FIR_FLOAT
  }
};

static firCoeffInt16_t const *const s_firCoeffsInt16[RESAMPLE_TYPE_LAST_FIR - RESAMPLE_TYPE_FIRST_FIR + 1][RESAMPLE_RATIO_NB] =
{
  [RESAMPLE_TYPE_PARKSMCCLELLAN - RESAMPLE_TYPE_FIRST_FIR] =
  {
    [RESAMPLE_RATIO_1] = &FIR_flatInt16,
    [RESAMPLE_RATIO_2] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO2_FIR_INT16,
    [RESAMPLE_RATIO_3] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO3_FIR_INT16,
    [RESAMPLE_RATIO_4] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO4_FIR_INT16,
    [RESAMPLE_RATIO_6] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO6_FIR_INT16,
    [RESAMPLE_RATIO_8] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO8_FIR_INT16,
    [RESAMPLE_RATIO_12] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO12_FIR_INT16
  },
  [RESAMPLE_TYPE_KAISERWINDOW - RESAMPLE_TYPE_FIRST_FIR] =
  {
    [RESAMPLE_RATIO_1] = &FIR_flatInt16,
    [RESAMPLE_RATIO_2] = FIR_KAISERWINDOW_RESAMPLING_RATIO2_FIR_INT16,
    [RESAMPLE_RATIO_3] = FIR_KAISERWINDOW_RESAMPLING_RATIO3_FIR_INT16,
    [RESAMPLE_RATIO_4] = FIR_KAISERWINDOW_RESAMPLING_RATIO4_FIR_INT16,
    [RESAMPLE_RATIO_6] = FIR_KAISERWINDOW_RESAMPLING_RATIO6_FIR_INT16,
    [RESAMPLE_RATIO_8] = FIR_KAISERWINDOW_RESAMPLING_RATIO8_FIR_INT16,
    [RESAMPLE_RATIO_12] = FIR_KAISERWINDOW_RESAMPLING_RATIO12_FIR_INT16
  }
};

static firCoeffInt32_t const *const s_firCoeffsInt32[RESAMPLE_TYPE_LAST_FIR - RESAMPLE_TYPE_FIRST_FIR + 1][RESAMPLE_RATIO_NB] =
{
  [RESAMPLE_TYPE_PARKSMCCLELLAN - RESAMPLE_TYPE_FIRST_FIR] =
  {
    [RESAMPLE_RATIO_1] = &FIR_flatInt32,
    [RESAMPLE_RATIO_2] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO2_FIR_INT32,
    [RESAMPLE_RATIO_3] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO3_FIR_INT32,
    [RESAMPLE_RATIO_4] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO4_FIR_INT32,
    [RESAMPLE_RATIO_6] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO6_FIR_INT32,
    [RESAMPLE_RATIO_8] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO8_FIR_INT32,
    [RESAMPLE_RATIO_12] = FIR_PARKSMCCLELLAN_RESAMPLING_RATIO12_FIR_INT32
  },
  [RESAMPLE_TYPE_KAISERWINDOW - RESAMPLE_TYPE_FIRST_FIR] =
  {
    [RESAMPLE_RATIO_1] = &FIR_flatInt32,
    [RESAMPLE_RATIO_2] = FIR_KAISERWINDOW_RESAMPLING_RATIO2_FIR_INT32,
    [RESAMPLE_RATIO_3] = FIR_KAISERWINDOW_RESAMPLING_RATIO3_FIR_INT32,
    [RESAMPLE_RATIO_4] = FIR_KAISERWINDOW_RESAMPLING_RATIO4_FIR_INT32,
    [RESAMPLE_RATIO_6] = FIR_KAISERWINDOW_RESAMPLING_RATIO6_FIR_INT32,
    [RESAMPLE_RATIO_8] = FIR_KAISERWINDOW_RESAMPLING_RATIO8_FIR_INT32,
    [RESAMPLE_RATIO_12] = FIR_KAISERWINDOW_RESAMPLING_RATIO12_FIR_INT32
  }
};
#endif

#ifdef USE_RESAMPLE_TYPE_IIR
static biquadFloat_t const *const s_biquadCoeffsFloat[RESAMPLE_TYPE_LAST_IIR - RESAMPLE_TYPE_FIRST_IIR + 1][RESAMPLE_RATIO_NB] =
{
  [RESAMPLE_TYPE_BUTTERWORTH - RESAMPLE_TYPE_FIRST_IIR] =
  {
    [RESAMPLE_RATIO_1] = &IIR_flat_biquadFloat,
    [RESAMPLE_RATIO_2] = IIR_BUTTERWORTH_RESAMPLING_RATIO2_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_3] = IIR_BUTTERWORTH_RESAMPLING_RATIO3_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_4] = IIR_BUTTERWORTH_RESAMPLING_RATIO4_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_6] = IIR_BUTTERWORTH_RESAMPLING_RATIO6_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_8] = IIR_BUTTERWORTH_RESAMPLING_RATIO8_BIQUAD_FLOAT
  },
  [RESAMPLE_TYPE_CHEBYSHEV1 - RESAMPLE_TYPE_FIRST_IIR] =
  {
    [RESAMPLE_RATIO_1] = &IIR_flat_biquadFloat,
    [RESAMPLE_RATIO_2] = IIR_CHEBYSHEV1_RESAMPLING_RATIO2_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_3] = IIR_CHEBYSHEV1_RESAMPLING_RATIO3_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_4] = IIR_CHEBYSHEV1_RESAMPLING_RATIO4_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_6] = IIR_CHEBYSHEV1_RESAMPLING_RATIO6_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_8] = IIR_CHEBYSHEV1_RESAMPLING_RATIO8_BIQUAD_FLOAT
  },
  [RESAMPLE_TYPE_CHEBYSHEV2 - RESAMPLE_TYPE_FIRST_IIR] =
  {
    [RESAMPLE_RATIO_1] = &IIR_flat_biquadFloat,
    [RESAMPLE_RATIO_2] = IIR_CHEBYSHEV2_RESAMPLING_RATIO2_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_3] = IIR_CHEBYSHEV2_RESAMPLING_RATIO3_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_4] = IIR_CHEBYSHEV2_RESAMPLING_RATIO4_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_6] = IIR_CHEBYSHEV2_RESAMPLING_RATIO6_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_8] = IIR_CHEBYSHEV2_RESAMPLING_RATIO8_BIQUAD_FLOAT
  },
  [RESAMPLE_TYPE_ELLIPTIC - RESAMPLE_TYPE_FIRST_IIR] =
  {
    [RESAMPLE_RATIO_1] = &IIR_flat_biquadFloat,
    [RESAMPLE_RATIO_2] = IIR_ELLIPTIC_RESAMPLING_RATIO2_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_3] = IIR_ELLIPTIC_RESAMPLING_RATIO3_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_4] = IIR_ELLIPTIC_RESAMPLING_RATIO4_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_6] = IIR_ELLIPTIC_RESAMPLING_RATIO6_BIQUAD_FLOAT,
    [RESAMPLE_RATIO_8] = IIR_ELLIPTIC_RESAMPLING_RATIO8_BIQUAD_FLOAT
  }
};

static biquadInt32_t const *const s_biquadCoeffsInt32[RESAMPLE_TYPE_LAST_IIR - RESAMPLE_TYPE_FIRST_IIR + 1][RESAMPLE_RATIO_NB] =
{
  [RESAMPLE_TYPE_BUTTERWORTH - RESAMPLE_TYPE_FIRST_IIR] =
  {
    [RESAMPLE_RATIO_1] = &IIR_flat_biquadInt32,
    [RESAMPLE_RATIO_2] = IIR_BUTTERWORTH_RESAMPLING_RATIO2_BIQUAD_INT32,
    [RESAMPLE_RATIO_3] = IIR_BUTTERWORTH_RESAMPLING_RATIO3_BIQUAD_INT32,
    [RESAMPLE_RATIO_4] = IIR_BUTTERWORTH_RESAMPLING_RATIO4_BIQUAD_INT32,
    [RESAMPLE_RATIO_6] = IIR_BUTTERWORTH_RESAMPLING_RATIO6_BIQUAD_INT32,
    [RESAMPLE_RATIO_8] = IIR_BUTTERWORTH_RESAMPLING_RATIO8_BIQUAD_INT32
  },
  [RESAMPLE_TYPE_CHEBYSHEV1 - RESAMPLE_TYPE_FIRST_IIR] =
  {
    [RESAMPLE_RATIO_1] = &IIR_flat_biquadInt32,
    [RESAMPLE_RATIO_2] = IIR_CHEBYSHEV1_RESAMPLING_RATIO2_BIQUAD_INT32,
    [RESAMPLE_RATIO_3] = IIR_CHEBYSHEV1_RESAMPLING_RATIO3_BIQUAD_INT32,
    [RESAMPLE_RATIO_4] = IIR_CHEBYSHEV1_RESAMPLING_RATIO4_BIQUAD_INT32,
    [RESAMPLE_RATIO_6] = IIR_CHEBYSHEV1_RESAMPLING_RATIO6_BIQUAD_INT32,
    [RESAMPLE_RATIO_8] = IIR_CHEBYSHEV1_RESAMPLING_RATIO8_BIQUAD_INT32
  },
  [RESAMPLE_TYPE_CHEBYSHEV2 - RESAMPLE_TYPE_FIRST_IIR] =
  {
    [RESAMPLE_RATIO_1] = &IIR_flat_biquadInt32,
    [RESAMPLE_RATIO_2] = IIR_CHEBYSHEV2_RESAMPLING_RATIO2_BIQUAD_INT32,
    [RESAMPLE_RATIO_3] = IIR_CHEBYSHEV2_RESAMPLING_RATIO3_BIQUAD_INT32,
    [RESAMPLE_RATIO_4] = IIR_CHEBYSHEV2_RESAMPLING_RATIO4_BIQUAD_INT32,
    [RESAMPLE_RATIO_6] = IIR_CHEBYSHEV2_RESAMPLING_RATIO6_BIQUAD_INT32,
    [RESAMPLE_RATIO_8] = IIR_CHEBYSHEV2_RESAMPLING_RATIO8_BIQUAD_INT32
  },
  [RESAMPLE_TYPE_ELLIPTIC - RESAMPLE_TYPE_FIRST_IIR] =
  {
    [RESAMPLE_RATIO_1] = &IIR_flat_biquadInt32,
    [RESAMPLE_RATIO_2] = IIR_ELLIPTIC_RESAMPLING_RATIO2_BIQUAD_INT32,
    [RESAMPLE_RATIO_3] = IIR_ELLIPTIC_RESAMPLING_RATIO3_BIQUAD_INT32,
    [RESAMPLE_RATIO_4] = IIR_ELLIPTIC_RESAMPLING_RATIO4_BIQUAD_INT32,
    [RESAMPLE_RATIO_6] = IIR_ELLIPTIC_RESAMPLING_RATIO6_BIQUAD_INT32,
    [RESAMPLE_RATIO_8] = IIR_ELLIPTIC_RESAMPLING_RATIO8_BIQUAD_INT32
  }
};
#endif

/* Private function prototypes -----------------------------------------------*/
static int32_t s_resample_deinit(audio_algo_t    *const pAlgo);
static int32_t s_resample_init(audio_algo_t      *const pAlgo);
static int32_t s_resample_dataInOut(audio_algo_t *const pAlgo);
static int32_t s_resample_process(audio_algo_t   *const pAlgo);

static int32_t s_set_ratio(resampleCtx_t *pResampleCtx, uint32_t const fsIn, uint32_t const fsOut);
static int32_t s_set_ratio_enum(uint32_t const ratio_value, resample_ratio_t *const pRatioEnum);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_resample_common =
{
  .pName                     = "resample",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_NOT_NB_ELEMENTS_FS,

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

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("sampling frequency adaptation, available resampling ratios r=n1/n2 with n1 and n2 in {1, 2, 3, 4, 6, 8}"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("resample")
};

audio_algo_cbs_t AudioChainWrp_resample_cbs =
{
  .init                       = s_resample_init,
  .deinit                     = s_resample_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_resample_dataInOut,
  .process                    = s_resample_process,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_resample_init(audio_algo_t *const pAlgo)
{
  int32_t                               error         = AUDIO_ERR_MGNT_NONE;
  resample_static_config_t const *const pStaticConfig = (resample_static_config_t const *)AudioAlgo_getStaticConfig(pAlgo);
  audio_chunk_t                  *const pChunkIn      = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t                  *const pChunkOut     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t           const *const pBuffIn       = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t           const *const pBuffOut      = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                        const fsIn          = AudioBuffer_getFs(pBuffIn);
  uint32_t                        const fsOut         = AudioBuffer_getFs(pBuffOut);
  uint8_t                         const nbChannels    = AudioBuffer_getNbChannels(pBuffIn);
  uint32_t                        const nbSamplesIn   = AudioBuffer_getNbElements(pBuffIn);
  uint32_t                        const nbSamplesOut  = AudioBuffer_getNbElements(pBuffOut);
  audio_buffer_type_t             const sampleType    = AudioBuffer_getType(pBuffIn);
  audio_buffer_interleaved_t      const interleaved   = AudioBuffer_getInterleaved(pBuffIn);
  resample_method_t                     method        = RESAMPLE_METHOD_NOFILTERING;
  size_t                                allocSize     = 0UL;
  memPool_t                             memPool       = AUDIO_MEM_UNKNOWN;
  resampleCtx_t                        *pResampleCtx  = NULL;
  uint8_t                               nbFilterCtx   = 0U;
  #ifdef USE_RESAMPLE_TYPE_FIR
  audio_chain_utilities_t        *const pUtilsHdle    = AudioAlgo_getUtilsHdle(pAlgo);
  bool                                  sfcForFir     = false;
  #endif

  if (pStaticConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static conf !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool   = (memPool_t)pStaticConfig->ramType;
    allocSize = sizeof(resampleCtx_t);
    switch (pStaticConfig->filterType)
    {
      case RESAMPLE_TYPE_NO_FILTERING:
        method     = RESAMPLE_METHOD_NOFILTERING;
        allocSize += sizeof(sfcContext_t);
        break;

        #ifdef USE_RESAMPLE_TYPE_FIR
      case RESAMPLE_TYPE_PARKSMCCLELLAN:
      case RESAMPLE_TYPE_KAISERWINDOW:
        method      = RESAMPLE_METHOD_FIR;
        nbFilterCtx = 1U;
        sfcForFir   = (nbChannels > 1U) && (interleaved == ABUFF_FORMAT_INTERLEAVED);
        if (sfcForFir)
        {
          allocSize += 2UL * (sizeof(sfcContext_t) + sizeof(audio_buffer_t));
        }
        break;
        #endif

        #ifdef USE_RESAMPLE_TYPE_IIR
      case RESAMPLE_TYPE_BUTTERWORTH:
      case RESAMPLE_TYPE_CHEBYSHEV1:
      case RESAMPLE_TYPE_CHEBYSHEV2:
      case RESAMPLE_TYPE_ELLIPTIC:
        method      = RESAMPLE_METHOD_IIR;
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
    pResampleCtx = (resampleCtx_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pResampleCtx == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "resample context malloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    AudioAlgo_setWrapperContext(pAlgo, pResampleCtx);
    memset(pResampleCtx, 0, allocSize);

    pResampleCtx->memPool      = memPool;
    pResampleCtx->method       = method;
    pResampleCtx->nbFilterCtx  = nbFilterCtx;
    pResampleCtx->pChunkIn     = pChunkIn;
    pResampleCtx->pChunkOut    = pChunkOut;
    pResampleCtx->nbChannels   = nbChannels;
    pResampleCtx->nbSamplesIn  = nbSamplesIn;
    pResampleCtx->nbSamplesOut = nbSamplesOut;
    pResampleCtx->interleaved  = interleaved;
    pResampleCtx->sampleType   = sampleType;

    error = s_set_ratio(pResampleCtx, fsIn, fsOut);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported resampling ratio (from %d Hz to %d Hz)", fsIn, fsOut);
    }
  }

  if (AudioError_isOk(error))
  {
    switch (method)
    {
      case RESAMPLE_METHOD_NOFILTERING:
        if (pResampleCtx->upSamplingRatio != 1U)
        {
          AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "resample without filtering forbidden in case of upsampling (fs %d->%d implies up-sampling x%d, down-sampling /%d) !", fsIn, fsOut, pResampleCtx->upSamplingRatio, pResampleCtx->downSamplingRatio);
          error = AUDIO_ERR_MGNT_INIT;
        }
        else
        {
          pResampleCtx->pSfcForNoFilterContext = (sfcContext_t *)&pResampleCtx[1];
          sfcSetContext(pResampleCtx->pSfcForNoFilterContext, pBuffIn, pBuffOut, false, 1.0f, AudioAlgo_getUtilsHdle(pAlgo));
          // apply down sampling ratio: multiply inSamplesOffsetParam by downSamplingRatio so that sfcSampleBufferConvert takes 1 sample every downSamplingRatio samples
          pResampleCtx->pSfcForNoFilterContext->inSamplesOffsetParam *= (int)pResampleCtx->downSamplingRatio;
        }
        break;

        #ifdef USE_RESAMPLE_TYPE_FIR
      case RESAMPLE_METHOD_FIR:
        pResampleCtx->pFilterContext = (union filterContext *)&pResampleCtx[1];
        if (sfcForFir)
        {
          pResampleCtx->pSfcInForFirContext  = (sfcContext_t *)&pResampleCtx->pFilterContext[nbFilterCtx];
          pResampleCtx->pSfcOutForFirContext = &pResampleCtx->pSfcInForFirContext[1];
          pResampleCtx->pSfcBuffIn           = (audio_buffer_t *)&pResampleCtx->pSfcOutForFirContext[1];
          error                              = AudioBuffer_create(pResampleCtx->pSfcBuffIn, nbChannels, fsIn, nbSamplesIn, ABUFF_FORMAT_TIME, sampleType, ABUFF_FORMAT_NON_INTERLEAVED, memPool);
          if (AudioError_isError(error))
          {
            AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "AudioBuffer_create(pResampleCtx->pSfcBuffIn, ...) error !");
          }
          if (AudioError_isOk(error))
          {
            pResampleCtx->pSfcBuffOut = &pResampleCtx->pSfcBuffIn[1];
            error                     = AudioBuffer_create(pResampleCtx->pSfcBuffOut, nbChannels, fsOut, nbSamplesOut, ABUFF_FORMAT_TIME, sampleType, ABUFF_FORMAT_NON_INTERLEAVED, memPool);
            if (AudioError_isError(error))
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "AudioBuffer_create(pResampleCtx->pSfcBuffOut, ...) error !");
            }
          }
          if (AudioError_isOk(error))
          {
            error = sfcSetContext(pResampleCtx->pSfcInForFirContext, pBuffIn, pResampleCtx->pSfcBuffIn, false, 1.0f, pUtilsHdle);
            if (AudioError_isError(error))
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(pResampleCtx->pSfcInForFirContext, ...) error");
            }
          }
          if (AudioError_isOk(error))
          {
            error = sfcSetContext(pResampleCtx->pSfcOutForFirContext, pResampleCtx->pSfcBuffOut, pBuffOut, false, 1.0f, pUtilsHdle);
            if (AudioError_isError(error))
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(pResampleCtx->pSfcOutForFirContext, ...) error");
            }
          }
        }

        if (AudioError_isOk(error))
        {
          void    const *pFirVoid = NULL;
          uint8_t const  firType  = (uint8_t)pStaticConfig->filterType - (uint8_t)RESAMPLE_TYPE_FIRST_FIR;

          switch (sampleType)
          {
            case ABUFF_FORMAT_FIXED16:
              pFirVoid = s_firCoeffsInt16[firType][pResampleCtx->eRatioLowPassFilter];
              break;

            case ABUFF_FORMAT_FIXED32:
              pFirVoid = s_firCoeffsInt32[firType][pResampleCtx->eRatioLowPassFilter];
              break;

            case ABUFF_FORMAT_FLOAT:
              pFirVoid = s_firCoeffsFloat[firType][pResampleCtx->eRatioLowPassFilter];
              break;

            default:
              error = AUDIO_ERR_MGNT_INIT;
              break;
          }
          error = firInit(&pResampleCtx->pFilterContext[0].firContext,
                          pFirVoid,
                          sampleType,
                          nbChannels,
                          nbSamplesIn,
                          pResampleCtx->downSamplingRatio,
                          pResampleCtx->upSamplingRatio,
                          memPool);
          if (AudioError_isError(error))
          {
            AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "firInit error");
          }
        }
        break;
        #endif

        #ifdef USE_RESAMPLE_TYPE_IIR
      case RESAMPLE_METHOD_IIR:
      {
        int     const nbBiquadCh = (interleaved == ABUFF_FORMAT_INTERLEAVED) ? (int)nbChannels : 1;
        uint8_t const iirType    = (uint8_t)pStaticConfig->filterType - (uint8_t)RESAMPLE_TYPE_FIRST_IIR;

        pResampleCtx->pFilterContext = (union filterContext *)&pResampleCtx[1];
        switch (sampleType)
        {
          case ABUFF_FORMAT_FIXED16:
            for (uint8_t i = 0U; AudioError_isOk(error) && (i < nbFilterCtx); i++)
            {
              error = biquadInt16Init(&pResampleCtx->pFilterContext[i].biquadIntContext,
                                      s_biquadCoeffsInt32[iirType][pResampleCtx->eRatioLowPassFilter],
                                      nbBiquadCh,
                                      (int)pResampleCtx->downSamplingRatio,
                                      (int)pResampleCtx->upSamplingRatio,
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
              error = biquadInt32Init(&pResampleCtx->pFilterContext[i].biquadIntContext,
                                      s_biquadCoeffsInt32[iirType][pResampleCtx->eRatioLowPassFilter],
                                      nbBiquadCh,
                                      (int)pResampleCtx->downSamplingRatio,
                                      (int)pResampleCtx->upSamplingRatio,
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
              error = biquadFloatInit(&pResampleCtx->pFilterContext[i].biquadFloatContext,
                                      s_biquadCoeffsFloat[iirType][pResampleCtx->eRatioLowPassFilter],
                                      nbBiquadCh,
                                      (int)pResampleCtx->downSamplingRatio,
                                      (int)pResampleCtx->upSamplingRatio,
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
        /* MISRAC: do nothing */
        break;
    }
  }

  if (AudioError_isError(error))
  {
    s_resample_deinit(pAlgo);
  }

  return error;
}


static int32_t s_resample_deinit(audio_algo_t *const pAlgo)
{
  int32_t              error        = AUDIO_ERR_MGNT_NONE;
  resampleCtx_t *const pResampleCtx = (resampleCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pResampleCtx != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pResampleCtx->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);

    switch (pResampleCtx->method)
    {
      case RESAMPLE_METHOD_NOFILTERING:
        break;

        #ifdef USE_RESAMPLE_TYPE_FIR
      case RESAMPLE_METHOD_FIR:
        if (pResampleCtx->pSfcBuffIn != NULL)
        {
          AudioBuffer_deinit(pResampleCtx->pSfcBuffIn);
        }
        if (pResampleCtx->pSfcBuffOut != NULL)
        {
          AudioBuffer_deinit(pResampleCtx->pSfcBuffOut);
        }
        firDeInit(&pResampleCtx->pFilterContext[0].firContext);
        break;
        #endif

        #ifdef USE_RESAMPLE_TYPE_IIR
      case RESAMPLE_METHOD_IIR:
        switch (pResampleCtx->sampleType)
        {
          case ABUFF_FORMAT_FIXED16:
            for (uint8_t i = 0U; i < pResampleCtx->nbFilterCtx; i++)
            {
              biquadInt16DeInit(&pResampleCtx->pFilterContext[i].biquadIntContext);
            }
            break;

          case ABUFF_FORMAT_FIXED32:
            for (uint8_t i = 0U; i < pResampleCtx->nbFilterCtx; i++)
            {
              biquadInt32DeInit(&pResampleCtx->pFilterContext[i].biquadIntContext);
            }
            break;

          case ABUFF_FORMAT_FLOAT:
            for (uint8_t i = 0U; i < pResampleCtx->nbFilterCtx; i++)
            {
              biquadFloatDeInit(&pResampleCtx->pFilterContext[i].biquadFloatContext);
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

    AudioAlgo_free(pResampleCtx, memPool);
  }

  return error;
}


static int32_t s_resample_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  #ifdef USE_RESAMPLE_TYPE_FIR
  resampleCtx_t *const pResampleCtx = (resampleCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pResampleCtx->pSfcInForFirContext != NULL)
  {
    // copy input samples into pSfcBuffIn with interleaving conversion
    sfcSampleBufferConvert(pResampleCtx->pSfcInForFirContext, AudioChunk_getReadPtr0(pResampleCtx->pChunkIn), AudioBuffer_getPdata(pResampleCtx->pSfcBuffIn), (int)pResampleCtx->nbChannels, (int)pResampleCtx->nbSamplesIn);
  }

  if (pResampleCtx->pSfcOutForFirContext != NULL)
  {
    // copy output samples from pSfcBuffOut with interleaving conversion
    sfcSampleBufferConvert(pResampleCtx->pSfcOutForFirContext, AudioBuffer_getPdata(pResampleCtx->pSfcBuffOut), AudioChunk_getWritePtr0(pResampleCtx->pChunkOut), (int)pResampleCtx->nbChannels, (int)pResampleCtx->nbSamplesOut);
  }
  #endif

  // no need to disable irq because process task is lower priority
  AudioAlgo_incReadyForProcess(pAlgo);

  return error;
}


static int32_t s_resample_process(audio_algo_t *const pAlgo)
{
  resampleCtx_t *const pResampleCtx = (resampleCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  switch (pResampleCtx->method)
  {
    case RESAMPLE_METHOD_NOFILTERING:
    {
      // downsampling without filtering (upsampling without filtering is rejected)
      void *const pIn  = AudioChunk_getReadPtr0(pResampleCtx->pChunkIn);
      void *const pOut = AudioChunk_getWritePtr0(pResampleCtx->pChunkOut);

      sfcSampleBufferConvert(pResampleCtx->pSfcForNoFilterContext, pIn, pOut, (int)pResampleCtx->nbChannels, (int)pResampleCtx->nbSamplesOut);
      break;
    }

    #ifdef USE_RESAMPLE_TYPE_FIR
    case RESAMPLE_METHOD_FIR:
      // low-pass filter of this signal and resample it
      for (uint8_t ch = 0U; ch < pResampleCtx->nbChannels; ch++)
      {
        void *const pIn  = (pResampleCtx->pSfcBuffIn  != NULL) ? AudioBuffer_getPdataCh(pResampleCtx->pSfcBuffIn, ch)  : AudioChunk_getReadPtr(pResampleCtx->pChunkIn,   ch, 0UL);
        void *const pOut = (pResampleCtx->pSfcBuffOut != NULL) ? AudioBuffer_getPdataCh(pResampleCtx->pSfcBuffOut, ch) : AudioChunk_getWritePtr(pResampleCtx->pChunkOut, ch, 0UL);

        firProcess(&pResampleCtx->pFilterContext[0].firContext, pIn, pOut, (int)ch, (int)pResampleCtx->nbSamplesIn);
      }
      break;
      #endif

      #ifdef USE_RESAMPLE_TYPE_IIR
    case RESAMPLE_METHOD_IIR:
      // low-pass filter of this signal and resample it
      switch (pResampleCtx->sampleType)
      {
        case ABUFF_FORMAT_FIXED16:
          for (uint8_t i = 0U; i < pResampleCtx->nbFilterCtx; i++)
          {
            biquadInt16Process(&pResampleCtx->pFilterContext[i].biquadIntContext,
                               (int16_t *)AudioChunk_getReadPtr(pResampleCtx->pChunkIn,   i, 0UL),
                               (int16_t *)AudioChunk_getWritePtr(pResampleCtx->pChunkOut, i, 0UL),
                               (int)pResampleCtx->nbSamplesIn);
          }
          break;
        case ABUFF_FORMAT_FIXED32:
          for (uint8_t i = 0U; i < pResampleCtx->nbFilterCtx; i++)
          {
            biquadInt32Process(&pResampleCtx->pFilterContext[i].biquadIntContext,
                               (int32_t *)AudioChunk_getReadPtr(pResampleCtx->pChunkIn,   i, 0UL),
                               (int32_t *)AudioChunk_getWritePtr(pResampleCtx->pChunkOut, i, 0UL),
                               (int)pResampleCtx->nbSamplesIn);
          }
          break;
        case ABUFF_FORMAT_FLOAT:
          for (uint8_t i = 0U; i < pResampleCtx->nbFilterCtx; i++)
          {
            biquadFloatProcess(&pResampleCtx->pFilterContext[i].biquadFloatContext,
                               (float *)AudioChunk_getReadPtr(pResampleCtx->pChunkIn,   i, 0UL),
                               (float *)AudioChunk_getWritePtr(pResampleCtx->pChunkOut, i, 0UL),
                               (int)pResampleCtx->nbSamplesIn);
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


static int32_t s_set_ratio(resampleCtx_t *pResampleCtx, uint32_t const fsIn, uint32_t const fsOut)
{
  /***********************************************************
  * up/down ratios for input fs (line) vs output fs (column)
  *         ------------------------------------------------
  *         | 8000 | 12000 | 16000 | 24000 | 32000 | 48000 |
  * --------------------------------------------------------
  * |  8000 |  1/1 |  3/2  |  2/1  |  3/1  |  4/1  |  6/1  |
  * | 12000 |  2/3 |  1/1  |  4/3  |  2/1  |  8/3  |  4/1  |
  * | 16000 |  1/2 |  3/4  |  1/1  |  3/2  |  2/1  |  3/1  |
  * | 24000 |  1/3 |  1/2  |  2/3  |  1/1  |  4/3  |  2/1  |
  * | 32000 |  1/4 |  3/8  |  1/2  |  3/4  |  1/1  |  3/2  |
  * | 48000 |  1/6 |  1/4  |  1/3  |  1/2  |  2/3  |  1/1  |
  * --------------------------------------------------------
  * thus ratios 2, 3, 4, 6, 8 are necessary
  ***********************************************************/

  int32_t          error              = AUDIO_ERR_MGNT_NONE;
  uint32_t const   fsPpcm             = util_ppcm(fsIn, fsOut);
  uint32_t const   upSamplingRatio    = fsPpcm / fsIn;
  uint32_t const   downSamplingRatio  = fsPpcm / fsOut;
  resample_ratio_t eUpSamplingRatio   = RESAMPLE_RATIO_1;
  resample_ratio_t eDownSamplingRatio = RESAMPLE_RATIO_1;

  if (AudioError_isOk(error))
  {
    error = s_set_ratio_enum(upSamplingRatio, &eUpSamplingRatio);
  }
  if (AudioError_isOk(error))
  {
    error = s_set_ratio_enum(downSamplingRatio, &eDownSamplingRatio);
  }
  if (AudioError_isOk(error))
  {
    pResampleCtx->upSamplingRatio     = (uint8_t)upSamplingRatio;
    pResampleCtx->downSamplingRatio   = (uint8_t)downSamplingRatio;
    pResampleCtx->eUpSamplingRatio    = eUpSamplingRatio;
    pResampleCtx->eDownSamplingRatio  = eDownSamplingRatio;
    pResampleCtx->eRatioLowPassFilter = (upSamplingRatio > downSamplingRatio) ? eUpSamplingRatio : eDownSamplingRatio;
  }

  return error;
}


static int32_t s_set_ratio_enum(uint32_t const ratio_value, resample_ratio_t *const pRatioEnum)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  switch (ratio_value)
  {
    case 1U:
      pRatioEnum[0] = RESAMPLE_RATIO_1;
      break;
    case 2U:
      pRatioEnum[0] = RESAMPLE_RATIO_2;
      break;
    case 3U:
      pRatioEnum[0] = RESAMPLE_RATIO_3;
      break;
    case 4U:
      pRatioEnum[0] = RESAMPLE_RATIO_4;
      break;
    case 6U:
      pRatioEnum[0] = RESAMPLE_RATIO_6;
      break;
    case 8U:
      pRatioEnum[0] = RESAMPLE_RATIO_8;
      break;
    case 12U:
      pRatioEnum[0] = RESAMPLE_RATIO_12;
      break;
    default:
      error = AUDIO_ERR_MGNT_INIT;
      break;
  }
  return error;
}
