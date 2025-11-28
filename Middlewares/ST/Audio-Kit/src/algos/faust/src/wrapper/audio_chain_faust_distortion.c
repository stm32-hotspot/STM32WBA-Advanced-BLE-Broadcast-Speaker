/**
******************************************************************************
* @file    audio_chain_faust_distortion.c
* @author  MCD Application Team
* @brief   produce a faust-distortion effect
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
#include "faust/audio_chain_faust_distortion.h"
#include <assert.h>
/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */
#include "sfc.h"
#include "common/commonMath.h"

/* ARM CMSIS DSP optimization */
#define sinf  arm_sin_f32
#define cosf  arm_cos_f32
#define tanf  tan_f32
#define sqrtf sqrt_f32

/*cstat -MISRAC2012-* the FAUST code generated is not misra compliant */
#include "faust/faust_distortion.c"
/*cstat +MISRAC2012-* */


/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  uint32_t             szBuffer;
  uint8_t              nbChannels;
  const audio_chunk_t *pChunkIn;
  const audio_chunk_t *pChunkOut;
  audio_buffer_t       faustAudioBuffer;
  sfcContext_t         sfcInContext;
  sfcContext_t         sfcOutContext;
  FAUSTFLOAT          *pScratchSample[2];
  memPool_t            memPool;

  mydsp               dsp;
  const faust_distortion_dynamic_config_t *pDynamic;
}  faust_distortionCtx_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const audio_algo_common_t  AudioChainWrp_faust_distortion_common =
{
  .pName                     = "faust-distortion",
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

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Simple faust-distortion effect."),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("faust")
};


/**
* @brief Update effect params from config param
*
* @param pDsp the dsp instance
* @param pDsp the pParam instance
*/
static void s_dsp_update(mydsp *pDsp, const faust_distortion_dynamic_config_t *pParams)
{
  pDsp->fCheckbox0 = pParams->bypass;
  pDsp->fHslider0 = pParams->drive;
  pDsp->fHslider1 = pParams->offset;

}


/* Private function prototypes -----------------------------------------------*/
static int32_t s_faust_distortion_deinit(audio_algo_t    *const pAlgo);
static int32_t s_faust_distortion_init(audio_algo_t      *const pAlgo);
static int32_t s_faust_distortion_dataInOut(audio_algo_t *const pAlgo);


/* Global variables ----------------------------------------------------------*/
audio_algo_cbs_t AudioChainWrp_faust_distortion_cbs  =
{
  .init                       = s_faust_distortion_init,
  .deinit                     = s_faust_distortion_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_faust_distortion_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/

/**
* @brief deInit the algo extension
*
* @param pAlgo the algo instance
* @return int32_t error code
*/

__weak int32_t AudioChainWrp_faust_distortion_deinit_ext(audio_algo_t *const pAlgo)
{
  return 1;
}


/**
* @brief Init the algo extension
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
__weak int32_t AudioChainWrp_faust_distortion_init_ext(audio_algo_t *const pAlgo)
{
  return 1;
}


/**
* @brief Load the scratch buffer and convert it
*
* @param pCtx the class context instance
*/
static void s_load_scratch(const faust_distortionCtx_t *pCtx)
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
* @param pCtx the class context instance
*/
static void s_save_scratch(const faust_distortionCtx_t *pCtx)
{
  sfcSampleBufferConvert(&pCtx->sfcOutContext, pCtx->pScratchSample[0], AudioChunk_getWritePtr0(pCtx->pChunkOut), (int)pCtx->nbChannels, (int)pCtx->szBuffer);
}


/**
* @brief Transform the buffer
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static void  plugin_processing_transform(faust_distortionCtx_t *pCtx)
{
  s_dsp_update(&pCtx->dsp, pCtx->pDynamic);
  computemydsp(&pCtx->dsp, (int)pCtx->szBuffer, pCtx->pScratchSample, pCtx->pScratchSample);
}


/**
* @brief Init the algo
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_faust_distortion_init(audio_algo_t *const pAlgo)
{
  int32_t                                              error          = AUDIO_ERR_MGNT_NONE;
  const faust_distortion_static_config_t  *const pStaticConfig  = (faust_distortion_static_config_t  const *)AudioAlgo_getStaticConfig(pAlgo);
  const faust_distortion_dynamic_config_t *const pDynamicConfig = (faust_distortion_dynamic_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chunk_t                                 *const pChunkIn       = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t                                 *const pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t                          const *const pBuffIn        = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t                          const *const pBuffOut       = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                                       const fsIn           = AudioBuffer_getFs(pBuffIn);
  audio_chain_utilities_t                       *const pUtilsHandle   = AudioAlgo_getUtilsHdle(pAlgo);
  faust_distortionCtx_t                         *pCtx           = NULL;
  memPool_t                                            memPool        = AUDIO_MEM_UNKNOWN;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "static or dynamic config is missing !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool = (memPool_t)pStaticConfig->ramType;
    pCtx    = (faust_distortionCtx_t *)AudioAlgo_malloc(sizeof(faust_distortionCtx_t), memPool);
    if (pCtx == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "context malloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    AudioAlgo_setWrapperContext(pAlgo, pCtx);
    memset(pCtx, 0, sizeof(faust_distortionCtx_t));
    pCtx->memPool    = memPool;
    pCtx->pDynamic   = pDynamicConfig;
    pCtx->szBuffer   = AudioBuffer_getNbSamples(pBuffIn);
    pCtx->nbChannels = AudioBuffer_getNbChannels(pBuffIn);
    pCtx->pChunkIn   = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
    pCtx->pChunkOut  = AudioAlgo_getChunkPtrOut(pAlgo, 0U);

    // create pCtx->faustAudioBuffer (for sfcSetContext purpose and for FAUST processing purpose) to be compliant with pScratchSample samples format (float, non-interleaved, stereo)
    error = AudioBuffer_create(&pCtx->faustAudioBuffer, 2U, fsIn, pCtx->szBuffer, ABUFF_FORMAT_TIME, ABUFF_FORMAT_FLOAT, ABUFF_FORMAT_NON_INTERLEAVED, memPool);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "AudioBuffer_create(&pCtx->faustAudioBuffer, ...) error !");
    }
  }

  if (AudioError_isOk(error))
  {
    pCtx->pScratchSample[0] = AudioBuffer_getPdataCh(&pCtx->faustAudioBuffer, 0);
    pCtx->pScratchSample[1] = AudioBuffer_getPdataCh(&pCtx->faustAudioBuffer, 1);

    error = sfcSetContext(&pCtx->sfcInContext, pBuffIn, &pCtx->faustAudioBuffer, false, 1.0f, pUtilsHandle);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(&pCtx->SfcInContext, ...) error !");
    }
  }

  if (AudioError_isOk(error))
  {
    error = sfcSetContext(&pCtx->sfcOutContext, &pCtx->faustAudioBuffer, pBuffOut, false, 1.0f, pUtilsHandle);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(&pCtx->SfcOutContext, ...) error !");
    }
  }

  if (AudioError_isOk(error))
  {
    initmydsp(&pCtx->dsp, (int32_t)fsIn);
  }


  if (AudioError_isOk(error))
  {
    AudioChainWrp_faust_distortion_init_ext(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_faust_distortion_deinit(pAlgo);
  }

  return error;
}


/**
* @brief deInit the algo
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_faust_distortion_deinit(audio_algo_t *const pAlgo)
{
  int32_t                            error = AUDIO_ERR_MGNT_NONE;
  faust_distortionCtx_t *const pCtx  = (faust_distortionCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pCtx != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pCtx->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioChainWrp_faust_distortion_deinit_ext(pAlgo);
    AudioBuffer_deinit(&pCtx->faustAudioBuffer);
    AudioAlgo_free(pCtx, memPool);
  }

  return error;
}


/**
* @brief Manage the data in out
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_faust_distortion_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  faust_distortionCtx_t *const pCtx = (faust_distortionCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  s_load_scratch(pCtx);
  plugin_processing_transform(pCtx);
  s_save_scratch(pCtx);
  return error;
}
