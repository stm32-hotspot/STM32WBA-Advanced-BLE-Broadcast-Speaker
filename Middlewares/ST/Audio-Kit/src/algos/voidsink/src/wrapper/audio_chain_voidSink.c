/**
  ******************************************************************************
  * @file    audio_chain_voidSink.c
  * @author  MCD Application Team
  * @brief   void sink to be used when a chunk needs to be connected even if not used
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
#include "voidsink/audio_chain_voidSink.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define VOIDSINK_MEM_POOL AUDIO_MEM_RAMINT

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static int32_t s_voidSink_deinit(audio_algo_t    *const pAlgo);
static int32_t s_voidSink_init(audio_algo_t      *const pAlgo);
static int32_t s_voidSink_dataInOut(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_voidSink_common =
{
  .pName                     = "voidSink",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_NOT_APPLICABLE,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL_AND_CUSTOM,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME_FREQ,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_NONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL_AND_CUSTOM,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME_FREQ,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("consume input samples without any processing"),
  .misc.icon                 = 2        /* use a sink icon */
};

audio_algo_cbs_t AudioChainWrp_voidSink_cbs =
{
  .init                       = s_voidSink_init,
  .deinit                     = s_voidSink_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_voidSink_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_voidSink_init(audio_algo_t *const pAlgo)
{
  (void)pAlgo;  // unused parameter
  return AUDIO_ERR_MGNT_NONE;
}

static int32_t s_voidSink_deinit(audio_algo_t *const pAlgo)
{
  (void)pAlgo;  // unused parameter
  return AUDIO_ERR_MGNT_NONE;
}

int32_t s_voidSink_dataInOut(audio_algo_t *const pAlgo)
{
  (void)pAlgo;  // unused parameter
  return AUDIO_ERR_MGNT_NONE;
}
