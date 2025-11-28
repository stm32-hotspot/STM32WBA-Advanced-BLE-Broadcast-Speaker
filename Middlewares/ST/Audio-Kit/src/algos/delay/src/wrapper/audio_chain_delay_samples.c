/**
******************************************************************************
* @file    audio_chain_delay_samples.c
* @author  MCD Application Team
* @brief   wrapper of delay_samples algo to match usage inside audio_chain.c
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
#include "delay/audio_chain_delay_samples.h"
#include "delay/src/delay.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_delay_samples_deinit(audio_algo_t    *const pAlgo);
static int32_t s_delay_samples_init(audio_algo_t      *const pAlgo);
static int32_t s_delay_samples_dataInOut(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_delay_samples_common =
{
  .pName                     = "delay-samples",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_ALL,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_ALL,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_ALL,
  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Add a delay of N samples")
};

audio_algo_cbs_t AudioChainWrp_delay_samples_cbs =
{
  .init                       = s_delay_samples_init,
  .deinit                     = s_delay_samples_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_delay_samples_dataInOut,
  .process                    = NULL,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_delay_samples_init(audio_algo_t *const pAlgo)
{
  return delay_init(pAlgo, false);
}


static int32_t s_delay_samples_deinit(audio_algo_t *const pAlgo)
{
  return delay_deinit(pAlgo);
}


static int32_t s_delay_samples_dataInOut(audio_algo_t *const pAlgo)
{
  return delay_dataInOut(pAlgo);
}
