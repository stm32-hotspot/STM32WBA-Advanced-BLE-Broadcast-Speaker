/**
******************************************************************************
* @file    delay.c
* @author  MCD Application Team
* @brief   wrapper of delay algo to match usage inside audio_chain.c
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
#include "delay/audio_chain_delay.h"
#include "delay/audio_chain_delay_samples.h"
#include "delay/src/delay.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  bool      isPdmType;          // true if samples are PDM
  bool      isPdmMsbFirst;      // true if samples are PDM MSB first
  bool      isPdmInterleaved;   // true if samples are PDM interleaved (and nbChannels > 1)
  uint8_t   sampleSize;         // PCM: sample size multiplied by nbChannels if all channels may be copied by block (if interleaved or 1 channel); PDM: 1
  uint8_t   nbChannels;         // number of channels of input/output buffer or 1 if all channels may be copied by block (if interleaved or 1 channel)
  uint32_t  nbSamples;          // number of samples of input/output buffer
  uint32_t  samplesOffset;      // samples offset of input/output buffer
  uint32_t  delaySamples;       // current delay value
  uint32_t  allocDelaySamples;  // allocated delay size in samples (must be multiplied by nbChannels and sample size to obtain allocated buffer size)
  uint32_t  chDelaySize;        // byte size of 1 channel of delay buffer
  uint32_t  writePos;           // write position inside delay buffer
  uint8_t  *pBuff;              // delay buffer pointer
  memPool_t memPool;
} delay_context_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
int32_t delay_init(audio_algo_t *const pAlgo, bool const delayInSeconds)
{
  int32_t                     error             = AUDIO_ERR_MGNT_NONE;
  void           const *const pStaticConfigVoid = AudioAlgo_getStaticConfig(pAlgo);
  audio_chunk_t  const *const pChunkIn          = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_buffer_t const *const pBuffIn           = AudioChunk_getBuffInfo(pChunkIn);
  uint8_t               const sampleSize        = AudioBuffer_getSampleSize(pBuffIn);
  uint8_t               const nbChannels        = AudioBuffer_getNbChannels(pBuffIn);
  uint32_t              const nbSamples         = AudioBuffer_getNbSamples(pBuffIn);
  uint32_t              const samplesOffset     = AudioBuffer_getSamplesOffset(pBuffIn);
  uint32_t              const fs                = AudioBuffer_getFs(pBuffIn);
  audio_buffer_type_t   const sampleType        = AudioBuffer_getType(pBuffIn);
  bool                  const isPdmType         = AudioBuffer_isPdmType(pBuffIn);
  bool                  const isPdmMsbFirst     = (sampleType == ABUFF_FORMAT_PDM_MSB_FIRST);
  bool                  const isInterleaved     = (AudioBuffer_getInterleaved(pBuffIn) == ABUFF_FORMAT_INTERLEAVED);
  bool                  const blockCopy         = !isPdmType && (isInterleaved || (nbChannels == 1U));  // block copy is not possible for PDM samples
  uint32_t                    delaySamples      = 0UL;
  uint32_t                    allocDelaySamples = 0UL;
  uint32_t                    chDelaySize       = 0UL;
  uint32_t                    buffSize          = 0UL;
  size_t                      allocSize         = sizeof(delay_context_t);
  delay_context_t            *pContext          = NULL;
  memPool_t                   memPool           = AUDIO_MEM_UNKNOWN;

  if (pStaticConfigVoid == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static conf !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    if (delayInSeconds)
    {
      delay_static_config_t const *const pStaticConfig     = (delay_static_config_t const *)pStaticConfigVoid;
      float                        const delaySamplesFloat = pStaticConfig->delay * (float)fs;

      delaySamples = (uint32_t)delaySamplesFloat;
      memPool      = (memPool_t)pStaticConfig->ramType;
    }
    else
    {
      delay_samples_static_config_t const *const pStaticConfig = (delay_samples_static_config_t const *)pStaticConfigVoid;

      delaySamples = pStaticConfig->delay;
      memPool      = (memPool_t)pStaticConfig->ramType;
    }

    // compute delay buffer allocation size
    allocDelaySamples = nbSamples + (isPdmType ? (((delaySamples + 7UL) >> 3U) << 3U) : delaySamples);  // round it to upper multiple of 8 in case of PDM samples
    chDelaySize       = isPdmType ? (allocDelaySamples >> 3U) : (allocDelaySamples * (uint32_t)sampleSize);
    buffSize          = (size_t)nbChannels * (size_t)chDelaySize;
    allocSize        += buffSize;
    pContext          = (delay_context_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pContext == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    memset(pContext, 0, sizeof(delay_context_t));
    pContext->memPool           = memPool;
    pContext->pBuff             = (uint8_t *)&pContext[1];
    pContext->isPdmType         = isPdmType;
    pContext->isPdmMsbFirst     = isPdmMsbFirst;
    pContext->isPdmInterleaved  = isPdmType && isInterleaved && (nbChannels > 1U);    // to know if PDM samples of the same channel are interleaved or not
    pContext->chDelaySize       = chDelaySize;
    pContext->delaySamples      = delaySamples;
    pContext->allocDelaySamples = allocDelaySamples;
    pContext->sampleSize        = blockCopy ? (sampleSize * nbChannels) : sampleSize; // for PCM samples only (see blockCopy definition):
    pContext->nbChannels        = blockCopy ? 1U : nbChannels;                        // if blockCopy, consider 1 channel of nbChannels bigger sampleSize
    pContext->nbSamples         = nbSamples;
    pContext->samplesOffset     = samplesOffset;
    memset(pContext->pBuff, (int)AudioBuffer_getSilenceFillByte(pBuffIn), buffSize); // fill delay buffer with silence
    AudioAlgo_setWrapperContext(pAlgo, pContext);
  }

  if (AudioError_isError(error))
  {
    delay_deinit(pAlgo);
  }

  return error;
}


int32_t delay_deinit(audio_algo_t *const pAlgo)
{
  delay_context_t *const pContext = (delay_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t memPool = pContext->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, memPool);
  }

  return AUDIO_ERR_MGNT_NONE;
}


int32_t delay_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                    error             = AUDIO_ERR_MGNT_NONE;
  delay_context_t     *const pContext          = (delay_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  audio_chunk_t const *const pChunkIn          = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_chunk_t const *const pChunkOut         = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  uint8_t                   *pBuff             = pContext->pBuff;
  uint8_t              const nbChannels        = pContext->nbChannels;
  uint32_t             const nbSamples         = pContext->nbSamples;
  uint32_t             const delaySamples      = pContext->delaySamples;
  uint32_t             const allocDelaySamples = pContext->allocDelaySamples;
  uint32_t                   writePos          = pContext->writePos;
  uint32_t                   readPos           = (writePos < delaySamples) ? (writePos + allocDelaySamples - delaySamples) : (writePos - delaySamples);
  uint32_t                   readSize1, readSize2, writeSize1, writeSize2;

  if ((readPos + nbSamples) < allocDelaySamples)
  {
    // no circular buffer wrapping
    readSize1 = nbSamples;
    readSize2 = 0UL;
  }
  else
  {
    // circular buffer wrapping
    readSize1 = allocDelaySamples - readPos;
    readSize2 = nbSamples - readSize1;
  }

  if ((writePos + nbSamples) < allocDelaySamples)
  {
    // no circular buffer wrapping
    writeSize1         = nbSamples;
    writeSize2         = 0UL;
    pContext->writePos = writePos + nbSamples;
  }
  else
  {
    // circular buffer wrapping
    writeSize1         = allocDelaySamples - writePos;
    writeSize2         = nbSamples - writeSize1;
    pContext->writePos = writeSize2;
  }

  if (pContext->isPdmType)
  {
    // PDM samples
    // in case of PDM samples, whatever input/output interleaving, delay buffer is non-interleaved to facilitate delay processing
    uint32_t const samplesOffset = pContext->samplesOffset;
    uint32_t const writePosDiv8  = (writePos >> 3U);        // by construction, writePos is always multiple of 8
    uint32_t const readPosDiv8   = (readPos  >> 3U);
    uint32_t const readPosMod8   = readPos - (readPosDiv8 << 3U);
    uint32_t const pdmWriteSize1 = (writeSize1 >> 3U);
    uint32_t const pdmWriteSize2 = (writeSize2 >> 3U);
    uint32_t const pdmReadSize1  = ((readSize1 + readPosMod8 + 7UL) >> 3U);
    uint32_t const pdmReadSize2  = ((readSize2               + 7UL) >> 3U);

    // First step: copy from input buffer into delay buffer
    for (uint8_t ch = 0U; ch < nbChannels; ch++)
    {
      uint8_t const *pIn = (uint8_t const *)AudioChunk_getReadPtr(pChunkIn, ch, 0UL);

      // knowing that writePos is always multiple of 8 (see above), writeSize1 and writeSize2 are multpiple of 8 too
      // moreover in such case, whether PDM is LSB first or MSB first has no impact on buffer copy (shift is not needed)
      if (pContext->isPdmInterleaved)
      {
        // delay buffer pBuff is non-interleaved, thus if input is interleaved, input to pBuff copy must deinterleave samples
        uint8_t *pChBuff = pBuff + writePosDiv8;

        for (uint32_t i = 0UL; i < pdmWriteSize1; i++)
        {
          pChBuff[i] = *pIn;
          pIn       += samplesOffset;
        }
        pChBuff = pBuff;
        for (uint32_t i = 0UL; i < pdmWriteSize2; i++)
        {
          pChBuff[i] = *pIn;
          pIn       += samplesOffset;
        }
      }
      else
      {
        // delay buffer pBuff is non-interleaved, thus if input is non-interleaved too, a simple memcpy can be done
        memcpy(pBuff + writePosDiv8, pIn,                 pdmWriteSize1);
        memcpy(pBuff,                pIn + pdmWriteSize1, pdmWriteSize2);
      }
      pBuff += pContext->chDelaySize; // next delay buffer channel
    }

    // Second step: copy from delay buffer into output buffer
    pBuff = pContext->pBuff;
    for (uint8_t ch = 0U; ch < nbChannels; ch++)
    {
      uint8_t *pOut = (uint8_t *)AudioChunk_getWritePtr(pChunkOut, ch, 0UL);

      if (readPosMod8 == 0UL)
      {
        // when readPos is multiple of 8, readSize1 and readSize2 are multpiple of 8 too
        // moreover in such case, whether PDM is LSB first or MSB first has no impact on buffer copy (shift is not needed)
        if (pContext->isPdmInterleaved)
        {
          // delay buffer pBuff is non-interleaved, thus if output is interleaved, pBuff to output copy must interleave samples
          uint8_t const *pChBuff = pBuff + readPosDiv8;

          for (uint32_t i = 0UL; i < pdmReadSize1; i++)
          {
            *pOut = pChBuff[i];
            pOut += samplesOffset;
          }
          pChBuff = pBuff;
          for (uint32_t i = 0UL; i < pdmReadSize2; i++)
          {
            *pOut = pChBuff[i];
            pOut += samplesOffset;
          }
        }
        else
        {
          // delay buffer pBuff is non-interleaved, thus if output is non-interleaved too, a simple memcpy can be done
          memcpy(pOut,                pBuff + readPosDiv8, pdmReadSize1);
          memcpy(pOut + pdmReadSize1, pBuff,               pdmReadSize2);
        }
      }
      else
      {
        // readPos is not multiple of 8 => whether PDM is LSB first or MSB first has an impact on buffer copy (shift is needed)
        uint8_t const *pChBuff = pBuff + readPosDiv8;
        uint32_t       i;
        union
        {
          uint16_t u16;
          uint8_t  u8[2];
        } tmp1, tmp2;

        if (pContext->isPdmMsbFirst)
        {
          tmp1.u8[0] = pChBuff[0];
          for (i = 1UL; i < pdmReadSize1; i++)
          {
            tmp1.u8[1] = tmp1.u8[0];
            tmp1.u8[0] = pChBuff[i];
            tmp2.u16   = tmp1.u16 << readPosMod8;
            *pOut      = tmp2.u8[1];
            pOut      += samplesOffset;
          }
          pChBuff = pBuff;
          for (i = 0UL; i < pdmReadSize2; i++)
          {
            tmp1.u8[1] = tmp1.u8[0];
            tmp1.u8[0] = pChBuff[i];
            tmp2.u16   = tmp1.u16 << readPosMod8;
            *pOut      = tmp2.u8[1];
            pOut      += samplesOffset;
          }
        }
        else
        {
          tmp1.u8[1] = pChBuff[0];
          for (i = 1UL; i < pdmReadSize1; i++)
          {
            tmp1.u8[0] = tmp1.u8[1];
            tmp1.u8[1] = pChBuff[i];
            tmp2.u16   = tmp1.u16 >> readPosMod8;
            *pOut      = tmp2.u8[0];
            pOut      += samplesOffset;
          }
          pChBuff = pBuff;
          for (i = 0UL; i < pdmReadSize2; i++)
          {
            tmp1.u8[0] = tmp1.u8[1];
            tmp1.u8[1] = pChBuff[i];
            tmp2.u16   = tmp1.u16 >> readPosMod8;
            *pOut      = tmp2.u8[0];
            pOut      += samplesOffset;
          }
        }
      }
      pBuff += pContext->chDelaySize; // next delay buffer channel
    }
  }
  else
  {
    // PCM samples
    // convert readPos, readSize1, readSize2, writePos, writeSize1, writeSize2 from samples unit into bytes unit for memcpy
    uint32_t const sampleSize = (uint32_t)pContext->sampleSize;

    readPos    *= sampleSize;
    readSize1  *= sampleSize;
    readSize2  *= sampleSize;
    writePos   *= sampleSize;
    writeSize1 *= sampleSize;
    writeSize2 *= sampleSize;

    // copy first from input buffer into delay buffer then copy from delay buffer into output buffer
    // remind that sampleSize and nbChannels have been modified if all channels may be copied by block (if interleaved or 1 channel)
    // in such case sampleSize is multiplied by nbChannels and nbChannels is set to 1
    for (uint8_t ch = 0U; ch < nbChannels; ch++)
    {
      uint8_t const *const pIn  = (uint8_t const *)AudioChunk_getReadPtr(pChunkIn, ch, 0UL);
      uint8_t       *const pOut = (uint8_t *)AudioChunk_getWritePtr(pChunkOut, ch, 0UL);

      memcpy(pBuff + writePos, pIn,              writeSize1);
      memcpy(pBuff,            pIn + writeSize1, writeSize2);
      memcpy(pOut,             pBuff + readPos,  readSize1);
      memcpy(pOut + readSize1, pBuff,            readSize2);

      pBuff += pContext->chDelaySize; // next delay buffer channel
    }
  }

  return error;
}
