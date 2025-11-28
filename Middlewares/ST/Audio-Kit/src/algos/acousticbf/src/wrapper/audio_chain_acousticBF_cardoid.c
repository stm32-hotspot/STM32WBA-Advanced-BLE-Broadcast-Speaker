/**
******************************************************************************
* @file    audio_chain_acousticBF_cardoid.c
* @author  MCD Application Team
* @brief   wrapper of acoustic beamforming cardiod algo to match usage inside audio_chain.c
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
#include "acousticBF/audio_chain_acousticBF_cardoid.h"
//#include "../rms/rms.h"
//#include "stm32xxx_voice_bsp.h"


/* Private typedef -----------------------------------------------------------*/
//typedef struct rmsContextStruct
//{
//  float                        alpha;     // for RMS computation
//  float                        r2;        // RMS^2 (not normalized)
//} rmsContext_t; // RMS needed to perform internal monitoring on opposite antenna (16-bit mono signal)

typedef struct
{
  uint8_t                      nbFirstStepCalls;
  uint8_t                      nbChOut;
  uint8_t                      selectOutput;
  //  uint8_t                      direction;
  uint8_t                      idMic1;
  uint8_t                      idMic2;
  uint32_t                     inOffset;
  uint32_t                     outOffset;
  uint32_t                     nbSamplesOut;
  size_t                       szBytes;
  size_t                       szBytesMove;
  //  rmsContext_t                 rmsWrp;
  AcousticBF_cardoid_Handler_t lowLevelHdle;
  AcousticBF_cardoid_Config_t  lowLevelCfg;
  //  int16_t                     *pDelayedBuff;
  audio_chain_utilities_t     *pUtilsHandle;
  audio_chunk_t               *pChunkIn;      /* save pChunkIn  in context to avoid to call AudioAlgo_getChunkPtrIn(pAlgo,  0U) each time in process */
  audio_chunk_t               *pChunkOut;     /* save pChunkOut in context to avoid to call AudioAlgo_getChunkPtrOut(pAlgo, 0U) each time in process */
  memPool_t                    memPool;
}
acousticBF_cardoid_context_t;


/* Private defines -----------------------------------------------------------*/
//#define ACOUSTIC_BF_DELAY_RAW_MIC 256UL
//#define ACOUSTIC_BF_RMS_NORMALIZER 32768.0f

#define ACOUSTIC_BF_OUT_SELECT_ANTENNA    ((uint8_t)0x00000000)
#define ACOUSTIC_BF_OUT_SELECT_MICROPHONE ((uint8_t)0x00000001)

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_acousticBF_cardoid_deinit(audio_algo_t            *const pAlgo);
static int32_t s_acousticBF_cardoid_init(audio_algo_t              *const pAlgo);
static int32_t s_acousticBF_cardiod_checkConsistency(audio_algo_t  *const pAlgo);
static int32_t s_acousticBF_cardoid_configure(audio_algo_t         *const pAlgo);
static int32_t s_acousticBF_cardoid_dataInOut(audio_algo_t         *const pAlgo);
static int32_t s_acousticBF_cardoid_process(audio_algo_t           *const pAlgo);
static int32_t s_acousticBF_cardoid_initLowLevel(audio_algo_t      *const pAlgo, AcousticBF_cardoid_Handler_t *const pAcousticBFHandler, memPool_t const memPool);
static int32_t s_acousticBF_cardoid_configureLowLevel(audio_algo_t *const pAlgo, AcousticBF_cardoid_Handler_t *const pAcousticBFHandler, AcousticBF_cardoid_Config_t *const pAcousticBFConfig);

/* Using RMS for logic detecting if BF should be used or omni mic only */
//static void  s_rms_init(rmsContext_t *const pRms, uint32_t const fs, uint16_t const smoothingTimeInMs);
//static float s_rms_process(rmsContext_t *const pHdle, int16_t const *const pData, uint32_t const nbSamples, uint8_t const nbChannels, uint8_t const chId);
//static void  s_delay_first_ch(acousticBF_cardoid_context_t *const pContext, int16_t const *const pSrc);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_acousticBF_cardoid_common =
{
  .pName                     = "acousticBF-cardoid",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_ALL ^ (uint32_t)ABUFF_PARAM_TYPE ^ (uint32_t)ABUFF_PARAM_FS ^ (uint32_t)ABUFF_PARAM_CH ^ (uint32_t)ABUFF_PARAM_NB_ELEMENTS),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_2CH,
  .iosIn.fs                  = (audio_capability_fs_t)((uint32_t)AUDIO_CAPABILITY_FS_16000 | (uint32_t)AUDIO_CAPABILITY_FS_256000 | (uint32_t)AUDIO_CAPABILITY_FS_512000 | (uint32_t)AUDIO_CAPABILITY_FS_768000 | (uint32_t)AUDIO_CAPABILITY_FS_1024000 | (uint32_t)AUDIO_CAPABILITY_FS_1280000 | (uint32_t)AUDIO_CAPABILITY_FS_2048000),
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_YES,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = (audio_capability_type_t)((uint32_t)AUDIO_CAPABILITY_TYPE_FIXED16 | (uint32_t)AUDIO_CAPABILITY_TYPE_PDM_LSB_FIRST | (uint32_t)AUDIO_CAPABILITY_TYPE_PDM_MSB_FIRST),
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("Mic\0"),
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("Microphones data\0"),

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_1CH_2CH,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_16000,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_YES,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16,
  .iosOut.pPinNames          = AUDIO_ALGO_OPT_STR("Beam\0"),
  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("Beam in first channel, raw input on second channel if output wire is stereo\0"),

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Beam-Forming with 2 output antennas (temporal)"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("acousticbf-cardoid")
};

audio_algo_cbs_t AudioChainWrp_acousticBF_cardoid_cbs =
{
  .init                       = s_acousticBF_cardoid_init,
  .deinit                     = s_acousticBF_cardoid_deinit,
  .configure                  = s_acousticBF_cardoid_configure,
  .dataInOut                  = s_acousticBF_cardoid_dataInOut,
  .process                    = s_acousticBF_cardoid_process,
  .control                    = NULL,                                     // user defined routine
  .checkConsistency           = s_acousticBF_cardiod_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/

static int32_t s_acousticBF_cardoid_init(audio_algo_t *const pAlgo)
{
  int32_t                                          error              = AUDIO_ERR_MGNT_NONE;
  acousticBF_cardoid_context_t                    *pContext           = NULL;
  audio_chain_utilities_t                   *const pUtilsHandle       = AudioAlgo_getUtilsHdle(pAlgo);
  audio_chunk_t                             *const pChunkIn           = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t                             *const pChunkOut          = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t                            *const pBuffIn            = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t                            *const pBuffOut           = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                                   const fsIn               = AudioBuffer_getFs(pBuffIn) / 1000UL;
  uint32_t                                   const fsOut              = AudioBuffer_getFs(pBuffOut) / 1000UL;
  uint8_t                                    const nbChOut            = AudioBuffer_getNbChannels(pBuffOut);
  uint8_t                                    const nbChIn             = AudioBuffer_getNbChannels(pBuffIn);
  acousticBF_cardoid_static_config_t  const *const pStaticConfig      = (acousticBF_cardoid_static_config_t *)AudioAlgo_getStaticConfig(pAlgo);
  acousticBF_cardoid_dynamic_config_t const *const pDynamicConfig     = (acousticBF_cardoid_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  size_t                                     const allocSize          = AUDIO_MEM_SIZEOF_ALIGN(acousticBF_cardoid_context_t);
  AcousticBF_cardoid_Handler_t                    *pAcousticBFHandler = NULL;
  memPool_t                                        memPool            = AUDIO_MEM_UNKNOWN;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static or dynamic config !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool  = (memPool_t)pStaticConfig->ramType;
    pContext = (acousticBF_cardoid_context_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pContext == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }

  if (AudioError_isOk(error))
  {
    memset(pContext, 0, allocSize);
    AudioAlgo_setWrapperContext(pAlgo, pContext);
    /* Saving info in context to have more efficient dataInOut & process routine */
    pContext->memPool        = memPool;
    pContext->pChunkIn       = pChunkIn;
    pContext->pChunkOut      = pChunkOut;
    pContext->pUtilsHandle   = pUtilsHandle;
    pContext->nbSamplesOut   = (uint32_t)AudioBuffer_getNbElements(pBuffIn);
    pContext->nbChOut        = nbChOut;

    uint32_t duration        = 0UL;
    AudioBuffer_getDurationNs(pBuffIn, &duration);
    pContext->nbFirstStepCalls = (uint8_t)(duration / 1000000UL);

    /* Input stream can be PDM or PCM, offset of 1 ms value depends on it */
    pContext->inOffset         = (AudioBuffer_isPdmType(pBuffIn)) ? ((uint32_t)nbChIn * fsIn / 8UL) : ((uint32_t)nbChIn * fsIn * sizeof(int16_t));
    /* Output stream can be PCM only */
    pContext->outOffset        = (uint32_t)nbChOut * fsOut * sizeof(int16_t);

    //    pContext->direction   = pDynamicConfig->direction;

    /* Set config structure for init of lowLevel algo */
    pAcousticBFHandler = &pContext->lowLevelHdle;

    /* Set lowlevel handler from user params before init */
    pAcousticBFHandler->rear_enable        = nbChOut - 1U;
    pAcousticBFHandler->ptr_out_channels   = nbChOut;
    pAcousticBFHandler->delay_enable       = pStaticConfig->delay_enable;
    pAcousticBFHandler->sampling_frequency = fsIn;
    pAcousticBFHandler->ptr_M1_channels    = nbChIn;
    pAcousticBFHandler->ptr_M2_channels    = nbChIn;
    switch (AudioBuffer_getType(pBuffIn))
    {
      case ABUFF_FORMAT_PDM_LSB_FIRST:
        pAcousticBFHandler->data_format = ACOUSTIC_BF_CARDOID_DATA_FORMAT_PDM_LSB;
        break;
      case ABUFF_FORMAT_PDM_MSB_FIRST:
        pAcousticBFHandler->data_format = ACOUSTIC_BF_CARDOID_DATA_FORMAT_PDM_MSB;
        break;
      case ABUFF_FORMAT_FIXED16:
        pAcousticBFHandler->data_format = ACOUSTIC_BF_CARDOID_DATA_FORMAT_PCM;
        break;
      default:
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported input sample format !");
        error = AUDIO_ERR_MGNT_INIT;
        break;
    }
  }

  if (AudioError_isOk(error))
  {
    error = s_acousticBF_cardoid_initLowLevel(pAlgo, pAcousticBFHandler, memPool);
  }

  if (AudioError_isOk(error))
  {
    error = s_acousticBF_cardoid_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_acousticBF_cardoid_deinit(pAlgo);
  }

  return error;
}


static int32_t s_acousticBF_cardoid_deinit(audio_algo_t *const pAlgo)
{
  int32_t                       error    = AUDIO_ERR_MGNT_NONE;
  acousticBF_cardoid_context_t *pContext = (acousticBF_cardoid_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    AcousticBF_cardoid_Handler_t *const pAcousticBFHandler = &pContext->lowLevelHdle;
    memPool_t                     const memPool            = pContext->memPool;

    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);

    if (pAcousticBFHandler != NULL)
    {
      if (pAcousticBFHandler->pInternalMemory != NULL)
      {
        AudioAlgo_free(pAcousticBFHandler->pInternalMemory, memPool);
      }
    }

    AudioAlgo_free(pContext, memPool);
  }
  return error;
}


static int32_t s_acousticBF_cardiod_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                                   error         = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t                      *const pChunkIn      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t                     *const pBuffIn       = AudioChunk_getBuffInfo(pChunkIn);
  //  audio_chunk_t                      *const pChunkOut     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  //  audio_buffer_t                     *const pBuffOut      = AudioChunk_getBuffInfo(pChunkOut);
  //  uint8_t                             const nbChOut       = AudioBuffer_getNbChannels(pBuffOut);
  acousticBF_cardoid_static_config_t *const pStaticConfig = (acousticBF_cardoid_static_config_t *)AudioAlgo_getStaticConfig4Check(pAlgo);

  switch (AudioBuffer_getType(pBuffIn))
  {
    case ABUFF_FORMAT_PDM_LSB_FIRST:
    case ABUFF_FORMAT_PDM_MSB_FIRST:
      break;
    case ABUFF_FORMAT_FIXED16:
      if ((pStaticConfig->mic_distance < 210U) || (pStaticConfig->mic_distance > 212U))
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "In case of PCM input, mic distance must be in the range [210-212]!");
        error = AUDIO_ERR_MGNT_INIT;
      }
      break;
    default:
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported input sample format!");
      error = AUDIO_ERR_MGNT_INIT;
      break;
  }

  return error;
}


static int32_t s_acousticBF_cardoid_configure(audio_algo_t *const pAlgo)
{
  int32_t                                    error              = AUDIO_ERR_MGNT_NONE;
  acousticBF_cardoid_context_t        *const pContext           = (acousticBF_cardoid_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  acousticBF_cardoid_dynamic_config_t *const pDynamicConfig     = (acousticBF_cardoid_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  acousticBF_cardoid_static_config_t  *const pStaticConfig      = (acousticBF_cardoid_static_config_t *)AudioAlgo_getStaticConfig(pAlgo);
  AcousticBF_cardoid_Config_t         *const pAcousticBFConfig  = &pContext->lowLevelCfg;
  AcousticBF_cardoid_Handler_t        *const pAcousticBFHandler = &pContext->lowLevelHdle;

  /* Update index to get good direction */
  if (pDynamicConfig->direction)
  {
    pContext->idMic1 = 1U;
    pContext->idMic2 = 0U;
  }
  else
  {
    pContext->idMic1 = 0U;
    pContext->idMic2 = 1U;
  }

  /* Set lowlevel configuration from user params */
  pAcousticBFConfig->ref_select     = pDynamicConfig->ref_select;
  pAcousticBFConfig->mic_distance   = pStaticConfig->mic_distance;
  pAcousticBFConfig->volume         = pDynamicConfig->volume;
  pAcousticBFConfig->M2_gain        = 0.0f; /* Force automatic mode */

  error = s_acousticBF_cardoid_configureLowLevel(pAlgo, pAcousticBFHandler, pAcousticBFConfig);

  return error;
}


static int32_t s_acousticBF_cardoid_configureLowLevel(audio_algo_t *const pAlgo, AcousticBF_cardoid_Handler_t *const pAcousticBFHandler, AcousticBF_cardoid_Config_t *const pAcousticBFConfig)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  /* Set lowlevel configuration from user params */
  uint32_t errorBf = AcousticBF_cardoid_SetConfig(pAcousticBFHandler, pAcousticBFConfig);
  //  errorBf = AcousticBF_getConfig(pAcousticBFHandler, pAcousticBFConfig);

  if (errorBf != 0UL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Confid failed with error = %d! (check in header acoustic_bf.h for code)", errorBf);
    error = AUDIO_ERR_MGNT_INIT;
  }

  return error;
}


static int32_t s_acousticBF_cardoid_initLowLevel(audio_algo_t *const pAlgo, AcousticBF_cardoid_Handler_t *const pAcousticBFHandler, memPool_t const memPool)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if (AcousticBF_cardoid_GetMemorySize(pAcousticBFHandler) != 0UL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "GetMemorySize failed!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    pAcousticBFHandler->pInternalMemory = (uint32_t *)AudioAlgo_malloc(pAcousticBFHandler->internal_memory_size, memPool);
    if (pAcousticBFHandler->pInternalMemory == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed!");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    uint32_t errorBf = AcousticBF_cardoid_Init(pAcousticBFHandler);

    if (errorBf != 0UL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Init failed with error = %d; (check in header acoustic_bf.h for code)", errorBf);
      error = AUDIO_ERR_MGNT_INIT;
    }
  }
  return error;
}


static int32_t s_acousticBF_cardoid_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                             error    = AUDIO_ERR_MGNT_NONE;
  acousticBF_cardoid_context_t *const pContext = (acousticBF_cardoid_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    audio_chunk_t                *const pChunkIn            = pContext->pChunkIn;
    audio_chunk_t                *const pChunkOut           = pContext->pChunkOut;
    uint8_t                      *const pMic1Base           = (uint8_t *)AudioChunk_getReadPtr(pChunkIn, pContext->idMic1, 0U);
    uint8_t                      *const pMic2Base           = (uint8_t *)AudioChunk_getReadPtr(pChunkIn, pContext->idMic2, 0U);
    uint8_t                      *const pOut1Base           = (uint8_t *)AudioChunk_getWritePtr(pChunkOut, 0U, 0U);
    //uint8_t                      *const pOut2Base           = (uint8_t *)AudioChunk_getWritePtr(pChunkOut, 1U, 0U);
    AcousticBF_cardoid_Handler_t *const pAcousticBFHandler  = &pContext->lowLevelHdle;
    uint32_t                            inOffset            = 0UL;
    uint32_t                            outOffset           = 0UL;

    for (uint8_t i = 0U; i < pContext->nbFirstStepCalls; i++)
    {
      uint8_t *const pMic1 = &pMic1Base[inOffset];
      uint8_t *const pMic2 = &pMic2Base[inOffset];
      uint8_t *const pOut1 = &pOut1Base[outOffset];

      if (AcousticBF_cardoid_FirstStep(pMic1, pMic2, pOut1, pAcousticBFHandler))
      {
        // no need to disable irq because process task is lower priority
        AudioAlgo_incReadyForProcess(pAlgo);
      }

      inOffset  += pContext->inOffset;
      outOffset += pContext->outOffset;
    }
  }
  else
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  return error;
}


static int32_t s_acousticBF_cardoid_process(audio_algo_t *const pAlgo)
{
  int32_t                     error    = AUDIO_ERR_MGNT_NONE;
  acousticBF_cardoid_context_t *const pContext = (acousticBF_cardoid_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    AcousticBF_cardoid_Handler_t *const pAcousticBFHandler = &pContext->lowLevelHdle;
    error = (int32_t)AcousticBF_cardoid_SecondStep(pAcousticBFHandler);
  }
  else
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  return error;
}


//static void s_rms_init(rmsContext_t *const pRms, uint32_t const fs, uint16_t const smoothingTimeInMs)
//{
//  pRms->alpha = 1.0f - expf(-1000.0f / ((float)fs * (float)smoothingTimeInMs)); /*cstat !MISRAC2012-Rule-22.8 errno check is useless*/
//}
//
//static void s_delay_first_ch(acousticBF_cardoid_context_t *const pContext, int16_t const *const pSrc)
//{
//  uint32_t const nbSamples  = pContext->nbSamplesOut;
//  uint32_t const nbChannels = (uint32_t)pContext->nbChOut;
//  int16_t *const pDest      = pContext->pDelayedBuff;
//
//  memmove(pDest, &pDest[nbSamples * nbChannels], pContext->szBytesMove);
//  for (uint32_t spl = 0UL; spl < nbSamples; spl++)
//  {
//    pDest[nbChannels * (spl + ACOUSTIC_BF_DELAY_RAW_MIC)] = pSrc[nbChannels * spl];
//    /* Keep second channel non delayed */
//    pDest[(nbChannels * spl) + 1UL] = pSrc[(nbChannels * spl) + 1UL];
//  }
//}
//
//
//static float s_rms_process(rmsContext_t *const pHdle, int16_t const *const pData, uint32_t const nbSamples, uint8_t const nbChannels, uint8_t const chId)
//{
//  float xn, r2;
//
//  r2 = pHdle->r2;
//  for (uint16_t spl = 0U; spl < nbSamples; spl++)
//  {
//    xn = (float)pData[(nbChannels * spl) + chId];
//    r2 += pHdle->alpha * ((xn * xn) - r2);
//  }
//  pHdle->r2 = r2;
//  r2 = (r2 <= 0.0f) ? 0.0f : r2;
//  return sqrtf(r2) / ACOUSTIC_BF_RMS_NORMALIZER;  /*cstat !MISRAC2012-Rule-22.8 errno check is useless (r2 is > 0)*/
//}

