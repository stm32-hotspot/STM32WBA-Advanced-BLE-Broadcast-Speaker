/**
******************************************************************************
* @file    audio_chain_acousticBF.c
* @author  MCD Application Team
* @brief   wrapper of acoustic beamforming algo to match usage inside audio_chain.c
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
#include "acousticBF/audio_chain_acousticBF.h"
//#include "../rms/rms.h"
//#include "stm32xxx_voice_bsp.h"


/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  uint8_t                    nbFirstStepCalls;
  uint8_t                    nbChOut;
  uint8_t                    idMic1;
  uint8_t                    idMic2;
  uint16_t                   controlCnt;
  uint32_t                   inOffset;
  uint32_t                   outOffset;
  uint32_t                   nbSamplesOut;
  uint32_t                   ref_select;
  AcousticBF_Handler_t       lowLevelHdle;
  AcousticBF_Config_t        lowLevelCfg;
  audio_chain_utilities_t   *pUtilsHandle;
  audio_chunk_t             *pChunkIn;      /* save pChunkIn  in context to avoid to call AudioAlgo_getChunkPtrIn(pAlgo,  0U) each time in process */
  audio_chunk_t             *pChunkOut;     /* save pChunkOut in context to avoid to call AudioAlgo_getChunkPtrOut(pAlgo, 0U) each time in process */
  memPool_t                  memPool;
}
acousticBF_context_t;


/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_acousticBF_deinit(audio_algo_t            *const pAlgo);
static int32_t s_acousticBF_init(audio_algo_t              *const pAlgo);
static int32_t s_acousticBF_checkConsistency(audio_algo_t  *const pAlgo);
static int32_t s_acousticBF_configure(audio_algo_t         *const pAlgo);
static int32_t s_acousticBF_dataInOut(audio_algo_t         *const pAlgo);
static int32_t s_acousticBF_process(audio_algo_t           *const pAlgo);
static int32_t s_acousticBF_initLowLevel(audio_algo_t      *const pAlgo, AcousticBF_Handler_t *const pAcousticBFHandler, memPool_t const memPool);
static int32_t s_acousticBF_configureLowLevel(audio_algo_t *const pAlgo, AcousticBF_Handler_t *const pAcousticBFHandler, AcousticBF_Config_t *const pAcousticBFConfig);

/* Using RMS for logic detecting if BF should be used or omni mic only */

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_acousticBF_common =
{
  .pName                     = "acousticBF",
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
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("acousticbf"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_DISABLE_AUTO_MOUNT

};

audio_algo_cbs_t AudioChainWrp_acousticBF_cbs =
{
  .init                       = s_acousticBF_init,
  .deinit                     = s_acousticBF_deinit,
  .configure                  = s_acousticBF_configure,
  .dataInOut                  = s_acousticBF_dataInOut,
  .process                    = s_acousticBF_process,
  .control                    = NULL,                                     // user defined routine
  .checkConsistency           = s_acousticBF_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/

static int32_t s_acousticBF_init(audio_algo_t *const pAlgo)
{
  int32_t                                  error              = AUDIO_ERR_MGNT_NONE;
  acousticBF_context_t                    *pContext           = NULL;
  audio_chain_utilities_t           *const pUtilsHandle       = AudioAlgo_getUtilsHdle(pAlgo);
  audio_chunk_t                     *const pChunkIn           = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t                     *const pChunkOut          = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t                    *const pBuffIn            = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t                    *const pBuffOut           = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                           const fsIn               = AudioBuffer_getFs(pBuffIn) / 1000UL;
  uint32_t                           const fsOut              = AudioBuffer_getFs(pBuffOut) / 1000UL;
  uint8_t                            const nbChOut            = AudioBuffer_getNbChannels(pBuffOut);
  uint8_t                            const nbChIn             = AudioBuffer_getNbChannels(pBuffIn);
  acousticBF_static_config_t  const *const pStaticConfig      = (acousticBF_static_config_t *)AudioAlgo_getStaticConfig(pAlgo);
  acousticBF_dynamic_config_t const *const pDynamicConfig     = (acousticBF_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  size_t                             const allocSize          = sizeof(acousticBF_context_t) + sizeof(AcousticBF_Control_t);
  AcousticBF_Handler_t                    *pAcousticBFHandler = NULL;
  memPool_t                                memPool            = AUDIO_MEM_UNKNOWN;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static or dynamic config !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool  = (memPool_t)pStaticConfig->ramType;
    pContext = (acousticBF_context_t *)AudioAlgo_malloc(allocSize, memPool);
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
    AudioAlgo_setCtrlData(pAlgo, &pContext[1]);
    /* Saving info in context to have more efficient dataInOut & process routine*/
    pContext->memPool        = memPool;
    pContext->pChunkIn       = pChunkIn;
    pContext->pChunkOut      = pChunkOut;
    pContext->pUtilsHandle   = pUtilsHandle;
    pContext->nbSamplesOut   = AudioBuffer_getNbElements(pBuffIn);
    pContext->nbChOut        = nbChOut;
    pContext->ref_select     = pDynamicConfig->ref_select;
    pContext->controlCnt     = pStaticConfig->control_refresh;

    uint32_t duration = 0UL;
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
    pAcousticBFHandler->mixer_enable        = pStaticConfig->mixer_enable;
    pAcousticBFHandler->thresh_low_db       = pStaticConfig->tLowDb;
    pAcousticBFHandler->thresh_high_db      = pStaticConfig->tHighDb;
    pAcousticBFHandler->algorithm_type_init = pStaticConfig->type;
    pAcousticBFHandler->ref_mic_enable      = pDynamicConfig->ref_select; //(uint32_t)nbChOut - 1UL; //ACOUSTIC_BF_REF_OPPOSITE_ANTENNA; //(uint32_t)nbChOut - 1;
    pAcousticBFHandler->ptr_out_channels    = nbChOut;
    pAcousticBFHandler->sampling_frequency  = fsIn;
    pAcousticBFHandler->delay_enable        = (uint8_t)ACOUSTIC_BF_DELAY_ENABLE; //(uint8_t)ACOUSTIC_BF_DELAY_DISABLE; //pStaticConfig->delay_enable;
    pAcousticBFHandler->ptr_M1_channels     = nbChIn;
    pAcousticBFHandler->ptr_M2_channels     = nbChIn;
    switch (AudioBuffer_getType(pBuffIn))
    {
      case ABUFF_FORMAT_PDM_LSB_FIRST:
        pAcousticBFHandler->data_format = ACOUSTIC_BF_DATA_FORMAT_PDM_LSB;
        break;
      case ABUFF_FORMAT_PDM_MSB_FIRST:
        pAcousticBFHandler->data_format = ACOUSTIC_BF_DATA_FORMAT_PDM_MSB;
        break;
      case ABUFF_FORMAT_FIXED16:
        pAcousticBFHandler->data_format = ACOUSTIC_BF_DATA_FORMAT_PCM;
        break;
      default:
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported input sample format !");
        error = AUDIO_ERR_MGNT_INIT;
        break;
    }
  }

  if (AudioError_isOk(error))
  {
    error = s_acousticBF_initLowLevel(pAlgo, pAcousticBFHandler, memPool);
  }

  if (AudioError_isOk(error))
  {
    error = s_acousticBF_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_acousticBF_deinit(pAlgo);
  }

  return error;
}


static int32_t s_acousticBF_deinit(audio_algo_t *const pAlgo)
{
  int32_t                     error    = AUDIO_ERR_MGNT_NONE;
  acousticBF_context_t *const pContext = (acousticBF_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    AcousticBF_Handler_t *const pAcousticBFHandler = &pContext->lowLevelHdle;
    memPool_t             const memPool            = pContext->memPool;

    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_setCtrlData(pAlgo, NULL);

    if (pAcousticBFHandler != NULL)
    {
      uint32_t errorBf = AcousticBF_DeInit(pAcousticBFHandler);

      if (errorBf != 0UL)
      {
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "DeInit failed with error = %d; (check in header acoustic_bf.h for code)", errorBf);
        error = AUDIO_ERR_MGNT_INIT;
      }

      if (pAcousticBFHandler->pInternalMemory != NULL)
      {
        AudioAlgo_free(pAcousticBFHandler->pInternalMemory, memPool);
      }
    }
    AudioAlgo_free(pContext, memPool);
  }

  return error;
}


static int32_t s_acousticBF_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                           error         = AUDIO_ERR_MGNT_NONE;
  //  acousticBF_context_t        *const pContext           = (acousticBF_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  acousticBF_dynamic_config_t *const pDynamicConfig     = (acousticBF_dynamic_config_t *)AudioAlgo_getDynamicConfig4Check(pAlgo);
  acousticBF_static_config_t  *const pStaticConfig      = (acousticBF_static_config_t *)AudioAlgo_getStaticConfig4Check(pAlgo);
  //  audio_chain_utilities_t     *const pUtilsHandle       = AudioAlgo_getUtilsHdle(pAlgo);
  audio_chunk_t               *const pChunkIn           = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t               *const pChunkOut          = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t              *const pBuffIn            = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t              *const pBuffOut           = AudioChunk_getBuffInfo(pChunkOut);
  //  uint32_t                     const fsIn               = AudioBuffer_getFs(pBuffIn) / 1000UL;
  //  uint32_t                     const fsOut              = AudioBuffer_getFs(pBuffOut) / 1000UL;
  uint8_t                      const nbChOut            = AudioBuffer_getNbChannels(pBuffOut);
  //  uint8_t                      const nbChIn             = AudioBuffer_getNbChannels(pBuffIn);

  if ((pDynamicConfig->ref_select > ACOUSTIC_BF_REF_DISABLE) && (nbChOut < 2U))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Using a reference channels requires a stereo output channels! please verify the output chunk config");
  }

  if ((pStaticConfig->control_refresh > 0U) && (pStaticConfig->mixer_enable == 0U))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "control_refresh only makes sens if mixer is enabled");
  }

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


static int32_t s_acousticBF_configure(audio_algo_t *const pAlgo)
{
  int32_t                            error              = AUDIO_ERR_MGNT_NONE;
  acousticBF_context_t        *const pContext           = (acousticBF_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  acousticBF_dynamic_config_t *const pDynamicConfig     = (acousticBF_dynamic_config_t *)AudioAlgo_getDynamicConfig(pAlgo);
  acousticBF_static_config_t  *const pStaticConfig      = (acousticBF_static_config_t *)AudioAlgo_getStaticConfig(pAlgo);
  AcousticBF_Config_t         *const pAcousticBFConfig  = &pContext->lowLevelCfg;
  AcousticBF_Handler_t              *pAcousticBFHandler = &pContext->lowLevelHdle;

  if ((pContext->ref_select == ACOUSTIC_BF_REF_DISABLE) && (pDynamicConfig->ref_select > ACOUSTIC_BF_REF_DISABLE))
  {
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
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
    pAcousticBFConfig->mic_distance   = pStaticConfig->mic_distance;
    pAcousticBFConfig->algorithm_type = pStaticConfig->type;
    pAcousticBFConfig->volume         = pDynamicConfig->volume;
    pAcousticBFConfig->M2_gain        = 0.0f; /* Force automatic mode */

    error = s_acousticBF_configureLowLevel(pAlgo, pAcousticBFHandler, pAcousticBFConfig);
  }
  return error;
}


static int32_t s_acousticBF_configureLowLevel(audio_algo_t *const pAlgo, AcousticBF_Handler_t *const pAcousticBFHandler, AcousticBF_Config_t *const pAcousticBFConfig)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  /* Set lowlevel configuration from user params */
  uint32_t errorBf = AcousticBF_setConfig(pAcousticBFHandler, pAcousticBFConfig);
  //  errorBf = AcousticBF_getConfig(pAcousticBFHandler, pAcousticBFConfig);

  if (errorBf != 0UL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Config failed with error = %d! (check in header acoustic_bf.h for code)", errorBf);
    error = AUDIO_ERR_MGNT_INIT;
  }

  return error;
}


static int32_t s_acousticBF_initLowLevel(audio_algo_t *const pAlgo, AcousticBF_Handler_t *const pAcousticBFHandler, memPool_t const memPool)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if (AcousticBF_getMemorySize(pAcousticBFHandler) != 0UL)
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
    uint32_t errorBf = AcousticBF_Init(pAcousticBFHandler);

    if (errorBf != 0UL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Init failed with error = %d; (check in header acoustic_bf.h for code)", errorBf);
      error = AUDIO_ERR_MGNT_INIT;
    }
  }
  return error;
}


static int32_t s_acousticBF_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                     error    = AUDIO_ERR_MGNT_NONE;
  acousticBF_context_t *const pContext = (acousticBF_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    audio_chunk_t        *const pChunkIn            = pContext->pChunkIn;
    audio_chunk_t        *const pChunkOut           = pContext->pChunkOut;
    uint8_t              *const pMic1Base           = (uint8_t *)AudioChunk_getReadPtr(pChunkIn, pContext->idMic1, 0UL);
    uint8_t              *const pMic2Base           = (uint8_t *)AudioChunk_getReadPtr(pChunkIn, pContext->idMic2, 0UL);
    uint8_t              *const pOut1Base           = (uint8_t *)AudioChunk_getWritePtr(pChunkOut, 0U, 0UL);
    AcousticBF_Handler_t *const pAcousticBFHandler  = &pContext->lowLevelHdle;
    uint32_t                    inOffset            = 0UL;
    uint32_t                    outOffset           = 0UL;

    for (uint8_t i = 0U; i < pContext->nbFirstStepCalls; i++)
    {
      uint8_t *const pMic1 = &pMic1Base[inOffset];
      uint8_t *const pMic2 = &pMic2Base[inOffset];
      uint8_t *const pOut1 = &pOut1Base[outOffset];

      if (AcousticBF_FirstStep(pMic1, pMic2, pOut1, pAcousticBFHandler))
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


static int32_t s_acousticBF_process(audio_algo_t *const pAlgo)
{
  int32_t                            error         = AUDIO_ERR_MGNT_NONE;
  acousticBF_context_t        *const pContext      = (acousticBF_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  acousticBF_static_config_t  *const pStaticConfig = (acousticBF_static_config_t *)AudioAlgo_getStaticConfig(pAlgo);

  if (pContext != NULL)
  {
    AcousticBF_Handler_t *const pAcousticBFHandler = &pContext->lowLevelHdle;
    error = (int32_t)AcousticBF_SecondStep(pAcousticBFHandler);

    if (pContext->controlCnt > 0U)
    {
      pContext->controlCnt--;
      if (pContext->controlCnt == 0U)
      {
        AcousticBF_Control_t  *const pCtrl = (AcousticBF_Control_t *)AudioAlgo_getCtrlData(pAlgo);
        if (ACOUSTIC_BF_TYPE_ERROR_NONE == AcousticBF_getControl(pAcousticBFHandler, pCtrl))
        {
          AudioAlgo_incReadyForControl(pAlgo);
          pContext->controlCnt = pStaticConfig->control_refresh;
        }
      }
    }

  }
  else
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  return error;
}

