/**
******************************************************************************
* @file    audio_chain_nlms.c
* @author  MCD Application Team
* @brief   wrapper of CMSIS nlms algo to match usage inside audio_chain.c
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
#include "nlms/audio_chain_nlms.h"
/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */

/* Private typedef -----------------------------------------------------------*/
typedef enum {SFC_IN0, SFC_IN1, SFC_OUT0, SFC_OUT1, NB_SFC_MAX} sfcBit_t;

typedef struct
{
  sfcContext_t              sfcContext;
  void                     *pSfcSamples;
} sfcStruct_t;

typedef struct
{
  uint16_t                  numTaps;
  arm_lms_norm_instance_f32 arm_hdle;
  audio_chunk_t            *pChunkIn0;      /* save pChunkIn0  in context to avoid to call AudioAlgo_getChunkPtrIn(pAlgo,  0U)                          each time in process */
  audio_chunk_t            *pChunkIn1;      /* save pChunkIn1  in context to avoid to call AudioAlgo_getChunkPtrIn(pAlgo,  1U)                          each time in process */
  audio_chunk_t            *pChunkOut0;     /* save pChunkOut0 in context to avoid to call AudioAlgo_getChunkPtrOut(pAlgo, 0U)                          each time in process */
  audio_chunk_t            *pChunkOut1;     /* save pChunkOut0 in context to avoid to call AudioAlgo_getChunkPtrOut(pAlgo, 1U)                          each time in process */
  uint32_t                  nbSamples;      /* save nbSamples  in context to avoid to call AudioBuffer_getNbElements(AudioChunk_getBuffInfo(pChunkIn0)) each time in process */
  sfcStruct_t              *pSfcStruct;     /* optional if sfc needed on IO chunks */
  sfcStruct_t              *pSfcStructTable[NB_SFC_MAX];
  memPool_t                 memPool;
}
NLMS_Handler_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_nlms_deinit(audio_algo_t    *const pAlgo);
static int32_t s_nlms_init(audio_algo_t      *const pAlgo);
static int32_t s_nlms_configure(audio_algo_t *const pAlgo);
static int32_t s_nlms_dataInOut(audio_algo_t *const pAlgo);
static int32_t s_nlms_process(audio_algo_t   *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_nlms_common =
{
  .pName                     = "nlms",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_TYPE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_TYPE,
  .chunks_consistency.in_out = ABUFF_PARAM_NOT_TYPE,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_TWO,
  .iosIn.nbChan              = AUDIO_CAPABILITY_1CH,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,
  .iosIn.pPinNames           = AUDIO_ALGO_OPT_STR("Src\0Ref\0"),
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("Source signal\0Reference signal\0"),

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_TWO,
  .iosOut.nbChan             = AUDIO_CAPABILITY_1CH,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,
  .iosOut.pPinNames          = AUDIO_ALGO_OPT_STR("Out\0Err\0"),
  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("Output signal (filtered Src)\0Error signal (Ref - filtered Src)\0"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("nlms"),

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Normalized Least Mean Squares adaptive algorithm: try to find the best FIR filter so that filtered Src matches Ref; i.e. minimization of Least Mean Squares of (Ref - filtered Src)")
};

audio_algo_cbs_t AudioChainWrp_nlms_cbs =
{
  .init                       = s_nlms_init,
  .deinit                     = s_nlms_deinit,
  .configure                  = s_nlms_configure,
  .dataInOut                  = s_nlms_dataInOut,
  .process                    = s_nlms_process,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_nlms_init(audio_algo_t *const pAlgo)
{
  int32_t                            error          = AUDIO_ERR_MGNT_NONE;
  NLMS_Handler_t                    *pNlmsHandler   = NULL;
  nlms_static_config_t  const *const pStaticConfig  = (nlms_static_config_t  const *)AudioAlgo_getStaticConfig(pAlgo);
  nlms_dynamic_config_t const *const pDynamicConfig = (nlms_dynamic_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chain_utilities_t     *const pUtilsHandle   = AudioAlgo_getUtilsHdle(pAlgo);
  audio_chunk_t               *const pChunkIn0      = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t               *const pChunkIn1      = AudioAlgo_getChunkPtrIn(pAlgo,  1U);
  audio_chunk_t               *const pChunkOut0     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_chunk_t               *const pChunkOut1     = AudioAlgo_getChunkPtrOut(pAlgo, 1U);
  audio_buffer_t              *const pBuffIn0       = AudioChunk_getBuffInfo(pChunkIn0);
  audio_buffer_t              *const pBuffIn1       = AudioChunk_getBuffInfo(pChunkIn1);
  audio_buffer_t              *const pBuffOut0      = AudioChunk_getBuffInfo(pChunkOut0);
  audio_buffer_t              *const pBuffOut1      = AudioChunk_getBuffInfo(pChunkOut1);
  uint32_t                     const fs             = AudioBuffer_getFs(pBuffIn0);
  uint32_t                     const nbSamples      = AudioBuffer_getNbElements(pBuffIn0);
  float                             *pCoeffs        = NULL;
  float                             *pState         = NULL;
  size_t                             allocSize      = 0UL;
  size_t                             allocOffset    = 0UL;
  uint8_t                            nbSfcContexts  = 0U;
  uint8_t                            sfcMask        = 0U;
  uint16_t                           numTaps        = 0U;
  memPool_t                          memPool        = AUDIO_MEM_UNKNOWN;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static or dynamic config !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    float const numTapsFloat = pStaticConfig->echo_length * (float)fs;

    allocSize = sizeof(NLMS_Handler_t);

    /* allocate room for pState*/
    numTaps    = (uint16_t)numTapsFloat;
    allocSize += ((size_t)numTaps + ((size_t)nbSamples - 1U)) * sizeof(float);

    /* allocate room for pCoeffs*/
    allocSize += numTaps * sizeof(float);

    /* allocate room sfc chunks if input ones are not in the right format */
    sfcMask      |= (AudioBuffer_getType(pBuffIn0)  != ABUFF_FORMAT_FLOAT) ? (1U << SFC_IN0)  : 0U;
    sfcMask      |= (AudioBuffer_getType(pBuffIn1)  != ABUFF_FORMAT_FLOAT) ? (1U << SFC_IN1)  : 0U;
    sfcMask      |= (AudioBuffer_getType(pBuffOut0) != ABUFF_FORMAT_FLOAT) ? (1U << SFC_OUT0) : 0U;
    sfcMask      |= (AudioBuffer_getType(pBuffOut1) != ABUFF_FORMAT_FLOAT) ? (1U << SFC_OUT1) : 0U;
    nbSfcContexts = ((sfcMask >> SFC_IN0) & 1U) + ((sfcMask >> SFC_IN1) & 1U) + ((sfcMask >> SFC_OUT0) & 1U) + ((sfcMask >> SFC_OUT1) & 1U);
    allocSize    += nbSfcContexts * (sizeof(sfcStruct_t) + ((size_t)nbSamples * sizeof(float)));  /* sfc context(s) & sfc samples buffer(s) */

    memPool      = (memPool_t)pStaticConfig->ramType;
    pNlmsHandler = (NLMS_Handler_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pNlmsHandler == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }

  if (AudioError_isOk(error))
  {
    memset(pNlmsHandler, 0, allocSize);
    AudioAlgo_setWrapperContext(pAlgo, pNlmsHandler);

    pNlmsHandler->memPool    = memPool;
    pNlmsHandler->numTaps    = numTaps;
    pNlmsHandler->pChunkIn0  = pChunkIn0;
    pNlmsHandler->pChunkIn1  = pChunkIn1;
    pNlmsHandler->pChunkOut0 = pChunkOut0;
    pNlmsHandler->pChunkOut1 = pChunkOut1;
    pNlmsHandler->nbSamples  = nbSamples;

    /* set ptr for pCoeffs */
    allocOffset  = sizeof(NLMS_Handler_t);
    pCoeffs      = (float *)((uint8_t *)pNlmsHandler + allocOffset);
    allocOffset += numTaps * sizeof(float);

    /* set ptr for pState */
    pState       = (float *)((uint8_t *)pNlmsHandler + allocOffset);
    allocOffset += ((size_t)numTaps + ((size_t)nbSamples - 1U)) * sizeof(float);

    if (nbSfcContexts > 0U)
    {
      /* set ptr for sfcContext & samples if needed for inputs & outputs (if input or outputs are not float) */
      audio_buffer_t bufferConfig;

      error = AudioBuffer_init(&bufferConfig, memPool);
      if (AudioError_isOk(error))
      {
        /* set bufferConfig for sfcSetContext purpose */
        error = AudioBuffer_config(&bufferConfig,
                                   1U,
                                   fs,
                                   nbSamples,
                                   ABUFF_FORMAT_TIME,
                                   ABUFF_FORMAT_FLOAT,
                                   ABUFF_FORMAT_NON_INTERLEAVED);
      }

      if (AudioError_isOk(error))
      {
        audio_buffer_t *const pBuffTable[NB_SFC_MAX][2] =
        {
          [SFC_IN0]  = {pBuffIn0,      &bufferConfig},
          [SFC_IN1]  = {pBuffIn1,      &bufferConfig},
          [SFC_OUT0] = {&bufferConfig, pBuffOut0},
          [SFC_OUT1] = {&bufferConfig, pBuffOut1}
        };
        sfcStruct_t *pSfcStruct = (sfcStruct_t *)((uint8_t *)pNlmsHandler + allocOffset);

        pNlmsHandler->pSfcStruct = pSfcStruct;
        allocOffset             += (size_t)nbSfcContexts * sizeof(sfcStruct_t);

        for (uint8_t scfBit = (uint8_t)SFC_IN0; AudioError_isOk(error) && (scfBit <= (uint8_t)SFC_OUT1); scfBit++)
        {
          if ((sfcMask & (1U << scfBit)) != 0U)
          {
            sfcResetContext(&pSfcStruct->sfcContext);
            error = sfcSetContext(&pSfcStruct->sfcContext, pBuffTable[scfBit][0], pBuffTable[scfBit][1], false, 1.0f, pUtilsHandle);
            if (AudioError_isError(error))
            {
              AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfc issue for %s%d !", (scfBit < (uint8_t)SFC_OUT0) ? "input" : "output", scfBit & 1U);
            }
            else
            {
              pSfcStruct->pSfcSamples               = (float *)((uint8_t *)pNlmsHandler + allocOffset);
              allocOffset                          += (size_t)nbSamples * sizeof(float);
              pNlmsHandler->pSfcStructTable[scfBit] = pSfcStruct;
            }
            pSfcStruct++;
          }
        }

        AudioBuffer_deinit(&bufferConfig);
      }
    }
  }

  if (AudioError_isOk(error))
  {
    //    pNlmsHandler->pCoeffs[0] = 1;
    arm_lms_norm_init_f32(&pNlmsHandler->arm_hdle, numTaps, pCoeffs, pState, pDynamicConfig->mu, nbSamples);

    error = s_nlms_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    s_nlms_deinit(pAlgo);
  }

  return error;
}


static int32_t s_nlms_deinit(audio_algo_t *const pAlgo)
{
  int32_t         error        = AUDIO_ERR_MGNT_NONE;
  NLMS_Handler_t *pNlmsHandler = (NLMS_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pNlmsHandler != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pNlmsHandler->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pNlmsHandler, memPool);
  }

  return error;
}


static int32_t s_nlms_configure(audio_algo_t *const pAlgo)
{
  int32_t                            error          = AUDIO_ERR_MGNT_NONE;
  NLMS_Handler_t                    *pNlmsHandler   = (NLMS_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);
  nlms_dynamic_config_t const *const pDynamicConfig = (nlms_dynamic_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);

  if (pDynamicConfig == NULL)
  {
    error = AUDIO_ERR_MGNT_INIT;
  }
  else
  {
    /* update step size that controls filter coefficient updates */
    pNlmsHandler->arm_hdle.mu = pDynamicConfig->mu;
  }
  return error;
}


static int32_t s_nlms_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  /* no need to disable irq because process task is lower priority */
  AudioAlgo_incReadyForProcess(pAlgo);

  return error;
}


static int32_t s_nlms_process(audio_algo_t *const pAlgo)
{
  NLMS_Handler_t *const pNlmsHandler  = (NLMS_Handler_t *)AudioAlgo_getWrapperContext(pAlgo);
  void           *const pSamplesIn0   = AudioChunk_getReadPtr0(pNlmsHandler->pChunkIn0);
  void           *const pSamplesIn1   = AudioChunk_getReadPtr0(pNlmsHandler->pChunkIn1);
  void           *const pSamplesOut0  = AudioChunk_getWritePtr0(pNlmsHandler->pChunkOut0);
  void           *const pSamplesOut1  = AudioChunk_getWritePtr0(pNlmsHandler->pChunkOut1);
  void                 *pSfcSamplesTable[NB_SFC_MAX][2] =
  {
    [SFC_IN0]  = {pSamplesIn0,  pSamplesIn0},
    [SFC_IN1]  = {pSamplesIn1,  pSamplesIn1},
    [SFC_OUT0] = {pSamplesOut0, pSamplesOut0},
    [SFC_OUT1] = {pSamplesOut1, pSamplesOut1}
  };

  /* sfc for input samples */
  for (uint8_t scfBit = (uint8_t)SFC_IN0; scfBit <= (uint8_t)SFC_IN1; scfBit++)
  {
    if (pNlmsHandler->pSfcStructTable[scfBit] != NULL)
    {
      pSfcSamplesTable[scfBit][1] = pNlmsHandler->pSfcStructTable[scfBit]->pSfcSamples;
      sfcSampleBufferConvert(&pNlmsHandler->pSfcStructTable[scfBit]->sfcContext, pSfcSamplesTable[scfBit][0], pSfcSamplesTable[scfBit][1], 1, (int)pNlmsHandler->nbSamples);
    }
  }

  /* set samples pointers for sfc out */
  for (uint8_t scfBit = (uint8_t)SFC_OUT0; scfBit <= (uint8_t)SFC_OUT1; scfBit++)
  {
    if (pNlmsHandler->pSfcStructTable[scfBit] != NULL)
    {
      pSfcSamplesTable[scfBit][0] = pNlmsHandler->pSfcStructTable[scfBit]->pSfcSamples;
    }
  }

  /* CMSIS nlms processing */
  arm_lms_norm_f32(&pNlmsHandler->arm_hdle,
                   (float32_t *)pSfcSamplesTable[SFC_IN0] [1],
                   (float32_t *)pSfcSamplesTable[SFC_IN1] [1],
                   (float32_t *)pSfcSamplesTable[SFC_OUT0][0],
                   (float32_t *)pSfcSamplesTable[SFC_OUT1][0],
                   pNlmsHandler->nbSamples);

  /* sfc for output samples */
  for (uint8_t scfBit = (uint8_t)SFC_OUT0; scfBit <= (uint8_t)SFC_OUT1; scfBit++)
  {
    if (pNlmsHandler->pSfcStructTable[scfBit] != NULL)
    {
      sfcSampleBufferConvert(&pNlmsHandler->pSfcStructTable[scfBit]->sfcContext, pSfcSamplesTable[scfBit][0], pSfcSamplesTable[scfBit][1], 1, (int)pNlmsHandler->nbSamples);
    }
  }

  return AUDIO_ERR_MGNT_NONE;
}
