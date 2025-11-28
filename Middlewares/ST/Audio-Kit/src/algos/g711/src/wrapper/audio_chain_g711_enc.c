/**
  ******************************************************************************
  * @file    audio_chain_g711_enc.c
  * @author  MCD Application Team
  * @brief   wrapper of G711 (encode) algo to match usage inside audio_chain.c
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
#include "g711_st.h"
#include "g711/audio_chain_g711.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t s_g711Enc_init(audio_algo_t      *const pAlgo);
static int32_t s_g711Enc_dataInOut(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
const audio_algo_common_t AudioChainWrp_g711_enc_common =
{
  .pName                     = "g711-enc",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_NOT_TYPE,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosIn.nbChan              = AUDIO_CAPABILITY_CH_ALL,
  .iosIn.fs                  = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosIn.interleaving        = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosIn.time_freq           = AUDIO_CAPABILITY_TIME,
  .iosIn.type                = AUDIO_CAPABILITY_TYPE_FIXED16,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_CH_ALL,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_BOTH,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_G711,

  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Generates g711 compressed stream from uncompressed PCM"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("g711enc")
};

audio_algo_cbs_t AudioChainWrp_g711_enc_cbs =
{
  .init                       = s_g711Enc_init,
  .deinit                     = NULL,                                 // defined below but useless
  .configure                  = NULL,
  .dataInOut                  = s_g711Enc_dataInOut,
  .process                    = NULL,                                 // defined below but useless
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/
static int32_t s_g711Enc_dataInOut(audio_algo_t *const pAlgo)
{
  audio_chunk_t      *const pChunkIn        = AudioAlgo_getChunkPtrIn(pAlgo,  0U);
  audio_chunk_t      *const pChunkOut       = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t     *const pBuffIn         = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t     *const pBuffOut        = AudioChunk_getBuffInfo(pChunkOut);
  uint8_t             const nbChannels      = AudioBuffer_getNbChannels(pBuffIn);
  int                 const buffInNbSamples = (int)AudioBuffer_getNbElements(pBuffIn);
  int                 const samplesOffset   = (int)AudioBuffer_getSamplesOffset(pBuffIn);
  audio_buffer_type_t const sampleType      = AudioBuffer_getType(pBuffOut);
  void (*const pG711Encode)(int16_t const * const input, uint8_t *const output, int const nb_samples, int const offset) = (sampleType == ABUFF_FORMAT_G711_ALAW) ? g711_alaw_buff_encode : g711_ulaw_buff_encode;

  for (uint8_t ch = 0U; ch < nbChannels; ch++)
  {
    int16_t *const ptrIn  = (int16_t *)AudioChunk_getReadPtr(pChunkIn,   ch, 0UL);
    uint8_t *const ptrOut = (uint8_t *)AudioChunk_getWritePtr(pChunkOut, ch, 0UL);

    /* encodes linear PCM samples to A-law or mu-law */
    (*pG711Encode)(ptrIn, ptrOut, buffInNbSamples, samplesOffset);
  }

  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_g711Enc_init(audio_algo_t *const pAlgo)
{
  (void)pAlgo;  // unused parameter
  return AUDIO_ERR_MGNT_NONE;
}
