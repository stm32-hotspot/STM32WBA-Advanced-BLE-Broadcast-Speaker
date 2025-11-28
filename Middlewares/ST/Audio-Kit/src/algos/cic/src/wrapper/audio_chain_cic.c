/**
******************************************************************************
* @file    audio_chain_cic.c
* @author  MCD Application Team
* @brief   wrapper of cic algo to match usage inside audio_chain.c
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
#include "cic/audio_chain_cic.h"
#include "cic/src/cic.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_cic_deinit(audio_algo_t    *const pAlgo);
static int32_t s_cic_init(audio_algo_t      *const pAlgo);
//static int32_t s_cic_dataInOut(audio_algo_t *const pAlgo);
static int32_t s_cic_process(audio_algo_t   *const pAlgo);
static int32_t s_cic_configure(audio_algo_t *const pAlgo);
static int32_t s_cic_checkConsistency(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_cic_common =
{
  .pName                     = "cic",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = (audio_buffer_param_list_t)((uint32_t)ABUFF_PARAM_CH | (uint32_t)ABUFF_PARAM_DURATION | (uint32_t)ABUFF_PARAM_TIME_FREQ),

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PDM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_YES,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = (audio_capability_type_t)((uint32_t)AUDIO_CAPABILITY_TYPE_PDM_LSB_FIRST | (uint32_t)AUDIO_CAPABILITY_TYPE_PDM_MSB_FIRST),
  .iosIn.pDescs              = AUDIO_ALGO_OPT_STR("PDM data\0"),

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_YES,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32,
  .iosOut.pDescs             = AUDIO_ALGO_OPT_STR("PCM data\0"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("cic"),
  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Performs similar processing as the dfsdm")
};

audio_algo_cbs_t AudioChainWrp_cic_cbs =
{
  .init                       = s_cic_init,
  .deinit                     = s_cic_deinit,
  .configure                  = s_cic_configure,
  .dataInOut                  = s_cic_process, //s_cic_dataInOut, // don't move it to process it doesn't work a 1ms
  //  .process                    = s_cic_process,
  .control                    = NULL,
  .checkConsistency           = s_cic_checkConsistency,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_cic_init(audio_algo_t *const pAlgo)
{
  return cic_init(pAlgo);
}


static int32_t s_cic_deinit(audio_algo_t *const pAlgo)
{
  return cic_deinit(pAlgo);
}


static int32_t s_cic_configure(audio_algo_t *const pAlgo)
{
  return cic_configure(pAlgo);
}


//static int32_t s_cic_dataInOut(audio_algo_t *const pAlgo)
//{
//  int32_t error = AUDIO_ERR_MGNT_NONE;
//
//  /* no need to disable irq because process task is lower priority */
//  AudioAlgo_incReadyForProcess(pAlgo);
//
//  return error;
//}


static int32_t s_cic_process(audio_algo_t *const pAlgo)
{
  return cic_process(pAlgo);
}

static int32_t s_cic_checkConsistency(audio_algo_t *const pAlgo)
{
  int32_t                          error         = AUDIO_ERR_MGNT_NONE;
  audio_chunk_t             *const pChunkIn      = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_chunk_t             *const pChunkOut     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t      const *const pBuffIn       = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t      const *const pBuffOut      = AudioChunk_getBuffInfo(pChunkOut);
  uint32_t                   const fsIn          = AudioBuffer_getFs(pBuffIn);
  uint32_t                   const fsOut         = AudioBuffer_getFs(pBuffOut);
  cic_static_config_t const *const pStaticConfig = AudioAlgo_getStaticConfig(pAlgo);

  if (fsOut == 0UL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "fsOut should be > 0 !");
    error = AUDIO_ERR_MGNT_INIT;
  }
  else
  {
    if ((fsIn % fsOut) != 0UL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "fsIn / fsOut should be an integer value!");
      error = AUDIO_ERR_MGNT_INIT;
    }
    if ((fsIn % (fsOut * 8UL)) != 0UL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "fsIn / (fsOut * 8) should be an integer value because CIC decimation is done by accumulation of bytes packets !");
      error = AUDIO_ERR_MGNT_INIT;
    }
  }
  if (pStaticConfig->order > 5U)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "SINC order is 5 maximum!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  return error;
}