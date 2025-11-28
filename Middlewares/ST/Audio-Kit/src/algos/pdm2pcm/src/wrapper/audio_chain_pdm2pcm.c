/**
******************************************************************************
* @file    audio_chain_pdm2pcm.c
* @author  MCD Application Team
* @brief   wrapper of pdm2pcm algo to match usage inside audio_chain.c
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
#include "audio_assert.h"
#include "pdm2pcm/audio_chain_pdm2pcm.h"
#include "sfc.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint32_t           nbSamplesIn;
  uint32_t           nbSamplesOut;
  uint8_t            nbChannels;
  uint16_t           libRatio;
  audio_chunk_t     *pChunkIn;      /* save pChunkIn  in context to avoid to call AudioAlgo_getChunkPtrIn(pAlgo,  0U) each time in process */
  audio_chunk_t     *pChunkOut;     /* save pChunkOut in context to avoid to call AudioAlgo_getChunkPtrOut(pAlgo, 0U) each time in process */
  PDM2PCM_Handler_t *pPdmHandler;
  struct pdm2pcmSfc
  {
    sfcContext_t     sfcContext;
    audio_buffer_t   audioBuffer;
  }                 *pPdm2pcmSfcIn, *pPdm2pcmSfcOut;
  memPool_t          memPool;
}
pdm2pcm_context_t;


/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t  s_pdm2pcm_deinit(audio_algo_t           *const pAlgo);
static int32_t  s_pdm2pcm_init(audio_algo_t             *const pAlgo);
static int32_t  s_pdm2pcm_checkConsistency(audio_algo_t *const pAlgo);
static int32_t  s_pdm2pcm_configure(audio_algo_t        *const pAlgo);
static int32_t  s_pdm2pcm_dataInOut(audio_algo_t        *const pAlgo);
static uint16_t s_pdm2pcm_getRatio(uint32_t const fsPdm, uint32_t const fsPcm);


/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_pdm2pcm_common =
{
  .pName                     = "pdm2pcm",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_TIME_FREQ),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PDM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = (audio_capability_type_t)((uint32_t)AUDIO_CAPABILITY_TYPE_PDM_LSB_FIRST | (uint32_t)AUDIO_CAPABILITY_TYPE_PDM_MSB_FIRST),
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("PDM data\0"),

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711,
  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("PCM data\0"),

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("PDM samples to PCM samples decimation module"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("pdm2pcm")
};

audio_algo_cbs_t AudioChainWrp_pdm2pcm_cbs =
{
  .init                       = s_pdm2pcm_init,
  .deinit                     = s_pdm2pcm_deinit,
  .configure                  = s_pdm2pcm_configure,
  .dataInOut                  = s_pdm2pcm_dataInOut,
  .process                    = NULL,
  .control                    = NULL,                                     // user defined routine
  .checkConsistency           = s_pdm2pcm_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/

static int32_t s_pdm2pcm_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                error     = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t   *const pChunkIn  = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t   *const pChunkOut = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t  *const pBuffIn   = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t  *const pBuffOut  = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t         const fsPdm     = AudioBuffer_getFs(pBuffIn);
  uint32_t         const fsPcm     = AudioBuffer_getFs(pBuffOut);
  uint16_t         const libRatio  = s_pdm2pcm_getRatio(fsPdm, fsPcm);

  if (libRatio == 0U)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Unsupported decimation ratio, please review input vs output sampling frequencies!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  return error;
}


static int32_t s_pdm2pcm_init(audio_algo_t *const pAlgo)
{
  int32_t                         error          = AUDIO_ERR_MGNT_NONE;
  uint32_t                        errorLib       = 0UL;
  pdm2pcm_context_t              *pContext       = NULL;
  audio_chunk_t            *const pChunkIn       = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t            *const pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t           *const pBuffIn        = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t           *const pBuffOut       = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                  const fsPdm          = AudioBuffer_getFs(pBuffIn);
  uint32_t                  const fsPcm          = AudioBuffer_getFs(pBuffOut);
  uint32_t                  const nbSamplesIn    = AudioBuffer_getNbSamples(pBuffIn);
  uint32_t                  const nbSamplesOut   = AudioBuffer_getNbSamples(pBuffOut);
  uint8_t                   const nbChannels     = AudioBuffer_getNbChannels(pBuffIn);
  audio_buffer_type_t       const typeIn         = AudioBuffer_getType(pBuffIn);
  audio_buffer_type_t       const typeOut        = AudioBuffer_getType(pBuffOut);
  bool                      const sfcInNeeded    = (AudioBuffer_getInterleaved(pBuffIn) == ABUFF_FORMAT_NON_INTERLEAVED) && (nbChannels > 1U);
  bool                      const sfcOutNeeded   = ((AudioBuffer_getInterleaved(pBuffOut) == ABUFF_FORMAT_NON_INTERLEAVED) && (nbChannels > 1U)) || (typeOut != ABUFF_FORMAT_FIXED16);
  pdm2pcm_static_config_t  *const pStaticConfig  = (pdm2pcm_static_config_t *)AudioAlgo_getStaticConfig(pAlgo);
  pdm2pcm_dynamic_config_t *const pDynamicConfig = (pdm2pcm_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chain_utilities_t  *const pUtilsHandle   = AudioAlgo_getUtilsHdle(pAlgo);
  size_t                    const allocSize      = sizeof(pdm2pcm_context_t) + ((size_t)nbChannels * sizeof(PDM2PCM_Handler_t)) + (sfcInNeeded ? sizeof(struct pdm2pcmSfc) : 0UL) + (sfcOutNeeded ? sizeof(struct pdm2pcmSfc) : 0UL);
  memPool_t                       memPool        = AUDIO_MEM_UNKNOWN;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static or dynamic config !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool  = (memPool_t)pStaticConfig->ramType;
    pContext = (pdm2pcm_context_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pContext == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }

  if (AudioError_isOk(error))
  {
    uint8_t *pU8 = (uint8_t *)pContext + sizeof(pdm2pcm_context_t);

    memset(pContext, 0, allocSize);
    AudioAlgo_setWrapperContext(pAlgo, pContext);
    /* Saving info in context to have more efficient dataInOut & process routine */
    pContext->memPool          = memPool;
    pContext->pChunkIn         = pChunkIn;
    pContext->pChunkOut        = pChunkOut;
    pContext->nbSamplesIn      = nbSamplesIn;
    pContext->nbSamplesOut     = nbSamplesOut;
    pContext->nbChannels       = nbChannels;
    pContext->pPdmHandler      = (PDM2PCM_Handler_t *)pU8;
    pU8                       += (size_t)nbChannels * sizeof(PDM2PCM_Handler_t);
    if (sfcInNeeded)
    {
      pContext->pPdm2pcmSfcIn  = (struct pdm2pcmSfc *)pU8;
      pU8                     += sizeof(struct pdm2pcmSfc);
    }
    if (sfcOutNeeded)
    {
      pContext->pPdm2pcmSfcOut = (struct pdm2pcmSfc *)pU8;
      pU8                     += sizeof(struct pdm2pcmSfc);
    }

    pContext->libRatio = s_pdm2pcm_getRatio(fsPdm, fsPcm);
    if (pContext->libRatio == 0U)
    {
      error = AUDIO_ERR_MGNT_CONFIG;
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported PDM fsIn(%d Hz) -> PCM fsOut(%d Hz) !", fsPdm, fsPcm);
    }
  }

  if (AudioError_isOk(error) && (pContext->pPdm2pcmSfcIn != NULL))
  {
    // pdm2pcm library accepts only ABUFF_FORMAT_INTERLEAVED input format
    error = AudioBuffer_copyConfig(pBuffIn, &pContext->pPdm2pcmSfcIn->audioBuffer);
    error = AudioError_update(error, AudioBuffer_setInterleaved(&pContext->pPdm2pcmSfcIn->audioBuffer, ABUFF_FORMAT_INTERLEAVED));
    error = AudioError_update(error, AudioBuffer_allocate(&pContext->pPdm2pcmSfcIn->audioBuffer));
    if (AudioError_isOk(error))
    {
      sfcResetContext(&pContext->pPdm2pcmSfcIn->sfcContext);
      error = sfcSetContext(&pContext->pPdm2pcmSfcIn->sfcContext, pBuffIn, &pContext->pPdm2pcmSfcIn->audioBuffer, false, 1.0f, pUtilsHandle);
      if (AudioError_isNok(error))
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported conversion for internal input SFC !");
      }
    }
    else
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "audio buffer allocation for internal input SFC failed !");
    }
  }

  if (AudioError_isOk(error) && (pContext->pPdm2pcmSfcOut != NULL))
  {
    // pdm2pcm library accepts only ABUFF_FORMAT_INTERLEAVED/ABUFF_FORMAT_FIXED16 output format
    error = AudioBuffer_copyConfig(pBuffOut, &pContext->pPdm2pcmSfcOut->audioBuffer);
    error = AudioError_update(error, AudioBuffer_setInterleaved(&pContext->pPdm2pcmSfcOut->audioBuffer, ABUFF_FORMAT_INTERLEAVED));
    error = AudioError_update(error, AudioBuffer_setType(&pContext->pPdm2pcmSfcOut->audioBuffer, ABUFF_FORMAT_FIXED16));
    error = AudioError_update(error, AudioBuffer_allocate(&pContext->pPdm2pcmSfcOut->audioBuffer));
    if (AudioError_isOk(error))
    {
      sfcResetContext(&pContext->pPdm2pcmSfcOut->sfcContext);
      error = sfcSetContext(&pContext->pPdm2pcmSfcOut->sfcContext, &pContext->pPdm2pcmSfcOut->audioBuffer, pBuffOut, false, 1.0f, pUtilsHandle);
      if (AudioError_isNok(error))
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported conversion for internal output SFC !");
      }
    }
    else
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "audio buffer allocation for internal output SFC failed !");
    }
  }

  if (AudioError_isOk(error))
  {
    uint16_t const bit_order         = (typeIn == ABUFF_FORMAT_PDM_LSB_FIRST) ? PDM2PCM_BIT_ORDER_LSB : PDM2PCM_BIT_ORDER_MSB;
    uint16_t const endianness        = ((pStaticConfig->from_i2s != 0U) && (bit_order == PDM2PCM_BIT_ORDER_MSB) && (nbChannels == 1U)) ? PDM2PCM_ENDIANNESS_BE : PDM2PCM_ENDIANNESS_LE;
    float    const high_pass_tap     = pStaticConfig->high_pass_tap * 2147483648.0f;
    uint32_t const high_pass_tap_u32 = (high_pass_tap < 0.0f) ? 0UL : (high_pass_tap > 2147483647.0f) ? 0x7FFFFFFFUL : (uint32_t)high_pass_tap;

    for (uint8_t ch = 0U; AudioError_isOk(error) && (ch < nbChannels); ch++)
    {
      pContext->pPdmHandler[ch].bit_order        = bit_order;
      pContext->pPdmHandler[ch].endianness       = endianness;
      pContext->pPdmHandler[ch].high_pass_tap    = high_pass_tap_u32;
      pContext->pPdmHandler[ch].in_ptr_channels  = nbChannels;
      pContext->pPdmHandler[ch].out_ptr_channels = nbChannels;

      errorLib = PDM2PCM_init(&pContext->pPdmHandler[ch]);
      if (errorLib != 0UL)
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Library init failed with error %d !", errorLib);
        error = AUDIO_ERR_MGNT_INIT;
      }
    }
  }

  if (AudioError_isOk(error))
  {
    error = s_pdm2pcm_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_pdm2pcm_deinit(pAlgo);
  }

  return error;
}


static int32_t s_pdm2pcm_deinit(audio_algo_t *const pAlgo)
{
  pdm2pcm_context_t *const pContext = (pdm2pcm_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pContext->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    if (pContext->pPdm2pcmSfcIn != NULL)
    {
      AudioBuffer_deinit(&pContext->pPdm2pcmSfcIn->audioBuffer);
    }
    if (pContext->pPdm2pcmSfcOut != NULL)
    {
      AudioBuffer_deinit(&pContext->pPdm2pcmSfcOut->audioBuffer);
    }
    AudioAlgo_free(pContext, memPool);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_pdm2pcm_configure(audio_algo_t *const pAlgo)
{
  int32_t                         error          = AUDIO_ERR_MGNT_NONE;
  uint32_t                        errorLib       = 0UL;
  pdm2pcm_context_t        *const pContext       = (pdm2pcm_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  pdm2pcm_dynamic_config_t *const pDynamicConfig = (pdm2pcm_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_buffer_t           *const pBuffOut       = AudioChunk_getBuffInfo(pContext->pChunkOut);
  uint32_t                  const nbSamplesOut   = AudioBuffer_getNbSamples(pBuffOut);
  PDM2PCM_Config_t                pdmConfig;

  pdmConfig.output_samples_number = (uint16_t)nbSamplesOut;
  pdmConfig.mic_gain              = pDynamicConfig->gain;
  pdmConfig.decimation_factor     = pContext->libRatio;
  for (uint8_t ch = 0U; AudioError_isOk(error) && (ch < pContext->nbChannels); ch++)
  {
    errorLib = PDM2PCM_setConfig(&pContext->pPdmHandler[ch], &pdmConfig);
    if (errorLib != 0UL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Library configuration failed with error %d !", errorLib);
      error = AUDIO_ERR_MGNT_CONFIG;
    }
  }

  return error;
}


static uint16_t s_pdm2pcm_getRatio(uint32_t const fsPdm, uint32_t const fsPcm)
{
  uint32_t const ratio    = fsPdm / fsPcm;
  uint16_t       libRatio = 0U;

  if (fsPdm == (ratio * fsPcm))   // ratio must be exact
  {
    switch (ratio)  /* Todo: enhance to support all frequencies listed */
    {
      case 16UL:
        libRatio = PDM2PCM_DEC_FACTOR_16;
        break;
      case 24UL:
        libRatio = PDM2PCM_DEC_FACTOR_24;
        break;
      case 32UL:
        libRatio = PDM2PCM_DEC_FACTOR_32;
        break;
      case 48UL:
        libRatio = PDM2PCM_DEC_FACTOR_48;
        break;
      case 64UL:
        libRatio = PDM2PCM_DEC_FACTOR_64;
        break;
      case 80UL:
        libRatio = PDM2PCM_DEC_FACTOR_80;
        break;
      case 128UL:
        libRatio = PDM2PCM_DEC_FACTOR_128;
        break;
      default:
        /* Not supported */
        break;
    }
  }

  return libRatio;
}


static int32_t s_pdm2pcm_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                  error    = AUDIO_ERR_MGNT_NONE;
  pdm2pcm_context_t *const pContext = (pdm2pcm_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    if (pContext->pPdm2pcmSfcIn != NULL)
    {
      // SFC in if necessary (if input PDM format is not supported by PDM2PCM library)
      sfcSampleBufferConvert(&pContext->pPdm2pcmSfcIn->sfcContext,
                             AudioChunk_getReadPtr0(pContext->pChunkIn),
                             AudioBuffer_getPdata(&pContext->pPdm2pcmSfcIn->audioBuffer),
                             (int)pContext->nbChannels,
                             (int)pContext->nbSamplesIn);
    }

    for (uint8_t ch = 0U; AudioError_isOk(error) && (ch < pContext->nbChannels); ch++)
    {
      // PDM2PCM library processing
      void *const PDMBuf   = (pContext->pPdm2pcmSfcIn  == NULL) ? AudioChunk_getReadPtr(pContext->pChunkIn,   ch, 0UL) : AudioBuffer_getPdataCh(&pContext->pPdm2pcmSfcIn->audioBuffer,  ch);
      void *const PCMBuf   = (pContext->pPdm2pcmSfcOut == NULL) ? AudioChunk_getWritePtr(pContext->pChunkOut, ch, 0UL) : AudioBuffer_getPdataCh(&pContext->pPdm2pcmSfcOut->audioBuffer, ch);
      uint32_t    errorLib = PDM2PCM_process(&pContext->pPdmHandler[ch], PDMBuf, PCMBuf);

      if (errorLib != 0UL)
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "processing library failed with error %d !", errorLib);
        error = AUDIO_ERR_MGNT_PDM;
      }
    }

    if (AudioError_isOk(error) && (pContext->pPdm2pcmSfcOut != NULL))
    {
      // SFC output if necessary (if output PCM format is not supported by PDM2PCM library)
      sfcSampleBufferConvert(&pContext->pPdm2pcmSfcOut->sfcContext,
                             AudioBuffer_getPdata(&pContext->pPdm2pcmSfcOut->audioBuffer),
                             AudioChunk_getWritePtr0(pContext->pChunkOut),
                             (int)pContext->nbChannels,
                             (int)pContext->nbSamplesOut);
    }
  }
  else
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }

  return error;
}
