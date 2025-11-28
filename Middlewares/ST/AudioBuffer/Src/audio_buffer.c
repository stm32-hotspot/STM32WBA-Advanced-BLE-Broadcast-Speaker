/**
******************************************************************************
* @file    audio_buffer.c
* @author  MCD Application Team
* @brief   audio buffers management
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
#include <stdio.h>
#include <stdint.h>
#include "audio_buffer.h"
#include "audio_mem_mgnt.h"


#ifdef AUDIO_MEM_CONF_TRACK_MALLOC
  static void *s_mallocDebug(size_t const size, memPool_t const memPool, char const *const file, int const line);
  static void  s_freeDebug(void *const pMemToFree, memPool_t const memPool);
  #define s_malloc(size, memPool)     s_mallocDebug((size), (memPool), __FILE__, __LINE__)
  #define s_free(pMemToFree, memPool) s_freeDebug((pMemToFree), (memPool))
#else
  static void *s_malloc(size_t const size, memPool_t const memPool);
  static void  s_free(void *const pMemToFree, memPool_t const memPool);
  // audio_mem_mgnt.c may be build with AUDIO_MEM_CONF_TRACK_MALLOC defined
  // thus if audio_algo.c is build with AUDIO_MEM_CONF_TRACK_MALLOC undefined
  // it must call audio_mem_mgnt.c routines with no debug
  //
  // thus we must undef AudioMalloc & AudioFree
  // and redefine them as external routines from audio_mem_mgnt.c
  #undef AudioMalloc
  #undef AudioFree
  void *AudioMalloc(size_t const size, memPool_t const memPool);
  void  AudioFree(void *const pMemToFree, memPool_t const memPool);
#endif
static void s_addBufferAllocStat(void *const pAlloc);
static void s_subBufferAllocStat(void *const pAlloc);


/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  void                     **pDataPtr;              /* samples buffer */
  uint32_t                   channelsOffset;        /* offset between 2 successive samples  of the same channel (interleaved : 1, non-interleaved : nbElements or nbElements * nbFrames in case audio_buffer comes from an audio_chunk) */
  uint8_t                    samplesOffset;         /* offset between 2 successive channels of the same sample  (interleaved : nbChannels, non-interleaved : 1) */
  uint8_t                    sampleSize;
  uint8_t                    sampleFullSize;
  memPool_t                  memPool;
} audio_buffer_context_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#define AUDIO_BUFFER_GET_CONTEXT_FIELD(field, valIfNoContext) (((pBuff == NULL) || (pBuff->pInternalMem == NULL)) ? valIfNoContext : ((audio_buffer_context_t *)pBuff->pInternalMem)->field)
#define AUDIO_BUFFER_GET_CONTEXT_FIELD_PTR(field) (((pBuff == NULL) || (pBuff->pInternalMem == NULL)) ? NULL : &((audio_buffer_context_t *)pBuff->pInternalMem)->field)

#define AUDIO_BUFFER_SET_CONTEXT_FIELD(field, value, errorMsg)                    \
  if ((pBuff != NULL) && (pBuff->pInternalMem != NULL))                           \
  {                                                                               \
    ((audio_buffer_context_t *)pBuff->pInternalMem)->field = value;               \
  }                                                                               \
  else if (errorMsg != NULL)                                                      \
  {                                                                               \
    AudioChunk_trace(pBuff, "AUDIO_BUFFER", TRACE_LVL_ERROR, NULL, 0, errorMsg);  \
  }


/* Private variables ---------------------------------------------------------*/
static memAllocStat_t *pAudioBufferAllocStats = NULL;

/* Private function prototypes -----------------------------------------------*/
static int32_t s_allocPdata(audio_buffer_t           *const pBuff);
static int32_t s_updateInternalConfig(audio_buffer_t *const pBuff);

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief  Reset an audio buffer.
 * @param  pBuff: pointer to the audio buffer
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_reset(audio_buffer_t *const pBuff)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if (pBuff == NULL)
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    memset(pBuff, 0, sizeof(audio_buffer_t));
  }

  return error;
}


/**
 * @brief  Initialize an audio buffer.
 * @param  pBuff:   pointer to the audio buffer
 * @param  memPool: memory pool
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_init(audio_buffer_t *const pBuff, memPool_t const memPool)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if (pBuff == NULL)
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    audio_buffer_context_t *const pContext = (audio_buffer_context_t *)s_malloc(sizeof(audio_buffer_context_t), memPool);

    if (pContext == NULL)
    {
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
    else
    {
      memset(pContext, 0, sizeof(audio_buffer_context_t));
      pBuff->interleaved       = ABUFF_FORMAT_INTERLEAVED;
      pBuff->timeFreq          = ABUFF_FORMAT_TIME;
      pBuff->type              = ABUFF_FORMAT_FIXED16;
      pContext->sampleSize     = (uint8_t)sizeof(int16_t);
      pContext->sampleFullSize = (uint8_t)sizeof(int16_t);     /* same as sampleSize because time format */
      pContext->memPool        = memPool;
      pBuff->pInternalMem      = pContext;
    }
  }

  return error;
}


/**
 * @brief  deallocate an audio buffer
 * @param  pBuff:       pointer to the audio buffer
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_deinit(audio_buffer_t *const pBuff)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if ((pBuff == NULL) || (pBuff->pInternalMem == NULL))
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    audio_buffer_context_t *const pContext = (audio_buffer_context_t *)pBuff->pInternalMem;

    if (pContext->pDataPtr != NULL)
    {
      s_free(pContext->pDataPtr, pContext->memPool);
    }
    s_free(pContext, pContext->memPool);
    pBuff->pInternalMem = NULL;
  }

  return error;
}


/**
 * @brief  allocate an audio buffer based on its config
 * @param  pBuff:       pointer to the audio buffer
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_allocate(audio_buffer_t *const pBuff)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if ((pBuff == NULL) || (pBuff->pInternalMem == NULL))
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    audio_buffer_context_t *const pContext = (audio_buffer_context_t *)pBuff->pInternalMem;

    if (pContext->pDataPtr != NULL)
    {
      error = AUDIO_ERR_MGNT_NOT_DONE;
    }
    else if (pBuff->nbElements == 0U)
    {
      error = AUDIO_ERR_MGNT_ERROR;
    }
    else
    {
      size_t const size1   = (size_t)pBuff->nbChannels * sizeof(void *);
      size_t const size2   = AudioBuffer_getBufferSize(pBuff);
      size_t const size    = size1 + size2;
      void  *const pMalloc = s_malloc(size, pContext->memPool);

      if (pMalloc == NULL)
      {
        error = AUDIO_ERR_MGNT_ALLOCATION;
      }
      else
      {
        void *const pDataPtr = pMalloc;
        void *const pData    = (uint8_t *)pDataPtr + size1;

        pContext->pDataPtr = pDataPtr;
        memset(pDataPtr, 0, size1);
        memset(pData, (int)AudioBuffer_getSilenceFillByte(pBuff), size2);
        error = AudioBuffer_setPdata(pBuff, pData);
      }
    }
  }

  return error;
}


/**
 * @brief  set audio buffer config
 * @param  pBuff:        pointer to the audio buffer
 * @param  nbChannels:   number of channels
 * @param  fs:           sampling frequency
 * @param  nbElements:   number of elements per frame
 * @param  nbFrames:     number of frames
 * @param  timeFreq:     time or frequnency domain buffer
 * @param  type:         samples format (fixed-point, floating-point, etc...)
 * @param  interleaved:  interleaved or non-interleaved buffer
 * @param  memPool:      memory pool
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_create(audio_buffer_t            *const pBuff,
                           uint8_t                    const nbChannels,
                           uint32_t                   const fs,
                           uint32_t                   const nbElements,
                           audio_buffer_time_freq_t   const timeFreq,
                           audio_buffer_type_t        const type,
                           audio_buffer_interleaved_t const interleaved,
                           memPool_t                  const memPool)
{
  int32_t error = AudioBuffer_init(pBuff, memPool);

  if (AudioError_isOk(error))
  {
    error = AudioBuffer_configAndAllocate(pBuff, nbChannels, fs, nbElements, timeFreq, type, interleaved);
  }

  return error;
}


/**
 * @brief  set audio buffer config
 * @param  pBuff:        pointer to the audio buffer
 * @param  nbChannels:   number of channels
 * @param  fs:           sampling frequency
 * @param  nbElements:   number of elements per frame
 * @param  nbFrames:     number of frames
 * @param  timeFreq:     time or frequnency domain buffer
 * @param  type:         samples format (fixed-point, floating-point, etc...)
 * @param  interleaved:  interleaved or non-interleaved buffer
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_config(audio_buffer_t *const pBuff, uint8_t const nbChannels, uint32_t const fs, uint32_t const nbElements, audio_buffer_time_freq_t const timeFreq, audio_buffer_type_t const type, audio_buffer_interleaved_t const interleaved)
{
  int32_t                          error          = AUDIO_ERR_MGNT_NONE;
  uint8_t                    const newNbChannels  = (nbChannels  == ABUFF_CONFIG_NO_CHANGE)             ? pBuff->nbChannels                : nbChannels;
  uint32_t                   const newFs          = (fs          == ABUFF_CONFIG_NO_CHANGE)             ? pBuff->fs                        : fs;
  uint32_t                   const newNbElements  = (nbElements  == ABUFF_CONFIG_NO_CHANGE)             ? AudioBuffer_getNbElements(pBuff) : nbElements;
  audio_buffer_time_freq_t   const newTimeFreq    = (timeFreq    == ABUFF_FORMAT_TIME_FREQ_NO_CHANGE)   ? pBuff->timeFreq                  : timeFreq;
  audio_buffer_type_t        const newType        = (type        == ABUFF_FORMAT_TYPE_NO_CHANGE)        ? pBuff->type                      : type;
  audio_buffer_interleaved_t const newInterleaved = (interleaved == ABUFF_FORMAT_INTERLEAVED_NO_CHANGE) ? pBuff->interleaved               : interleaved;

  pBuff->nbChannels  = newNbChannels;
  pBuff->fs          = newFs;
  pBuff->timeFreq    = newTimeFreq;
  pBuff->type        = newType;
  pBuff->interleaved = newInterleaved;
  pBuff->nbElements  = newNbElements;
  error              = s_updateInternalConfig(pBuff);

  return error;
}


/**
 * @brief  set audio buffer config and allocate samples buffer
 * @param  pBuff:        pointer to the audio buffer
 * @param  nbChannels:   number of channels
 * @param  fs:           sampling frequency
 * @param  nbElements:   number of elements per frame
 * @param  nbFrames:     number of frames
 * @param  timeFreq:     time or frequnency domain buffer
 * @param  type:         samples format (fixed-point, floating-point, etc...)
 * @param  interleaved:  interleaved or non-interleaved buffer
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_configAndAllocate(audio_buffer_t *const pBuff, uint8_t const nbChannels, uint32_t const fs, uint32_t const nbElements, audio_buffer_time_freq_t const timeFreq, audio_buffer_type_t const type, audio_buffer_interleaved_t const interleaved)
{
  int32_t error = AudioBuffer_config(pBuff, nbChannels, fs, nbElements, timeFreq, type, interleaved);

  if (AudioError_isOk(error))
  {
    error = AudioBuffer_allocate(pBuff);
  }

  return error;
}


/**
 * @brief  copy audio buffer config
 * @param  pBuffIn:     pointer to the input audio buffer (from which the config is read)
 * @param  pBuffOut:    pointer to the output audio buffer (into which the config is set)
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_copyConfig(audio_buffer_t const *const pBuffIn, audio_buffer_t *const pBuffOut)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if ((pBuffIn == NULL) || (pBuffIn->pInternalMem == NULL) || (pBuffOut == NULL))
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    audio_buffer_context_t *pContextIn = (audio_buffer_context_t *)pBuffIn->pInternalMem;
    audio_buffer_context_t *pContextOut;

    if (pBuffOut->pInternalMem == NULL)
    {
      error = AudioBuffer_init(pBuffOut, pContextIn->memPool);
    }
    if (AudioError_isOk(error))
    {
      pContextOut                 = (audio_buffer_context_t *)pBuffOut->pInternalMem;
      pBuffOut->fs                = pBuffIn->fs;
      pBuffOut->nbChannels        = pBuffIn->nbChannels;
      pBuffOut->nbElements        = pBuffIn->nbElements;
      pBuffOut->interleaved       = pBuffIn->interleaved;
      pBuffOut->timeFreq          = pBuffIn->timeFreq;
      pBuffOut->type              = pBuffIn->type;
      pContextOut->samplesOffset  = pContextIn->samplesOffset;
      pContextOut->channelsOffset = pContextIn->channelsOffset;
      pContextOut->sampleSize     = pContextIn->sampleSize;
      pContextOut->sampleFullSize = pContextIn->sampleFullSize;
    }
  }

  return error;
}


audio_buffer_interleaved_t AudioBuffer_getInterleaved(audio_buffer_t const *const pBuff)
{
  return pBuff->interleaved;
}


audio_buffer_time_freq_t AudioBuffer_getTimeFreq(audio_buffer_t const *const pBuff)
{
  return pBuff->timeFreq;
}


audio_buffer_type_t AudioBuffer_getType(audio_buffer_t const *const pBuff)
{
  return pBuff->type;
}

uint32_t AudioBuffer_convTypeToBit(audio_buffer_type_t const type)
{
  uint32_t ret;

  switch (type)
  {
    case ABUFF_FORMAT_PDM_LSB_FIRST:
      ret = 1UL;
      break;
    case ABUFF_FORMAT_PDM_MSB_FIRST:
      ret = 1UL;
      break;
    case ABUFF_FORMAT_G711_ALAW:
      ret = 8UL;
      break;
    case ABUFF_FORMAT_G711_MULAW:
      ret = 8UL;
      break;
    case ABUFF_FORMAT_FIXED16:
      ret = 16UL;
      break;
    case ABUFF_FORMAT_FIXED32:
      ret = 32UL;
      break;
    case ABUFF_FORMAT_FLOAT:
      ret = 32UL;
      break;
    default:
      ret = 0UL;
      break;
  }

  return ret;

}

bool AudioBuffer_isPdmType(audio_buffer_t const *const pBuff)
{
  return (pBuff->type == ABUFF_FORMAT_PDM_LSB_FIRST) || (pBuff->type == ABUFF_FORMAT_PDM_MSB_FIRST);
}


bool AudioBuffer_isG711Type(audio_buffer_t const *const pBuff)
{
  return (pBuff->type == ABUFF_FORMAT_G711_ALAW) || (pBuff->type == ABUFF_FORMAT_G711_MULAW);
}


uint8_t AudioBuffer_getNbChannels(audio_buffer_t const *const pBuff)
{
  return pBuff->nbChannels;
}


uint32_t AudioBuffer_getFs(audio_buffer_t const *const pBuff)
{
  return pBuff->fs;
}


void *AudioBuffer_getPdata(audio_buffer_t const *const pBuff)
{
  void **const pDataPtr = AUDIO_BUFFER_GET_CONTEXT_FIELD(pDataPtr, NULL);

  return (pDataPtr == NULL) ? NULL : pDataPtr[0];
}


void *AudioBuffer_getPdataCh(audio_buffer_t const *const pBuff, uint8_t const ch)
{
  void **const pDataPtr = AUDIO_BUFFER_GET_CONTEXT_FIELD(pDataPtr, NULL);

  return (pDataPtr == NULL) ? NULL : pDataPtr[ch];
}


/**
 * @brief  update channels offset (internal hidden audio buffer structure) from channels pointers.
 * @param  pBuff: pointer to the audio buffer
 * @retval None
 */
void AudioBuffer_updateChannelsOffsetFromPtr(audio_buffer_t *const pBuff)
{
  if ((pBuff != NULL) && (pBuff->pInternalMem != NULL))
  {
    audio_buffer_context_t *const pContext = (audio_buffer_context_t *)pBuff->pInternalMem;

    if (pBuff->nbChannels > 1U)
    {
      if (pBuff->interleaved == ABUFF_FORMAT_NON_INTERLEAVED)
      {
        pContext->channelsOffset = ((uint32_t)pContext->pDataPtr[1] - (uint32_t)pContext->pDataPtr[0]) / pContext->sampleFullSize;  /*cstat !MISRAC2012-Rule-10.3 !MISRAC2012-Rule-11.6 Suppress pointer to arithmertic type conversion*/
      }
      else
      {
        pContext->channelsOffset = 1UL;
      }
    }
    else
    {
      pContext->channelsOffset = 0UL;
    }
  }
}


/**
 * @brief  get channels offset (offset between 2 successive channels of the same sample)
 * @param  pBuff: pointer to the audio buffer
 * @retval channels offset (including *2 for complex (spectral) samples)
 */
uint32_t AudioBuffer_getChannelsOffset(audio_buffer_t const *const pBuff)
{
  return AUDIO_BUFFER_GET_CONTEXT_FIELD(channelsOffset, 0U) << AudioBuffer_getRealComplexFactor(pBuff->timeFreq);
}


/**
 * @brief  get samples offset (offset between 2 successive samples of the same channel)
 * @param  pBuff: pointer to the audio buffer
 * @retval samples offset (including *2 for complex (spectral) samples)
 */
uint32_t AudioBuffer_getSamplesOffset(audio_buffer_t const *const pBuff)
{
  return (uint32_t)AUDIO_BUFFER_GET_CONTEXT_FIELD(samplesOffset, 0U) << AudioBuffer_getRealComplexFactor(pBuff->timeFreq);
}


/**
 * @brief  get samples offset (offset between 2 successive samples of the same channel)
 * @param  pBuff: pointer to the audio buffer
 * @retval samples offset (NOT including *2 for complex (spectral) samples)
 */
uint32_t AudioBuffer_getSamplesOffset0(audio_buffer_t const *const pBuff)
{
  return (uint32_t)AUDIO_BUFFER_GET_CONTEXT_FIELD(samplesOffset, 0U);
}


/**
 * @brief  get samples size (bytes)
 * @param  pBuff: pointer to the audio buffer
 * @retval samples size (NOT including *2 for complex (spectral) samples)
 */
uint8_t AudioBuffer_getSampleSize(audio_buffer_t const *const pBuff)
{
  return AUDIO_BUFFER_GET_CONTEXT_FIELD(sampleSize, 0U);
}


/**
 * @brief  get samples size (bytes)
 * @param  pBuff: pointer to the audio buffer
 * @retval samples size (including *2 for complex (spectral) samples)
 */
uint8_t AudioBuffer_getSampleFullSize(audio_buffer_t const *const pBuff)
{
  return AUDIO_BUFFER_GET_CONTEXT_FIELD(sampleFullSize, 0U);
}


/**
 * @brief  get number of element of a buffer
 * @param  pBuff: pointer to the audio buffer
 * @retval number of samples (including *2 for complex (spectral) samples)
 */
uint32_t AudioBuffer_getNbCells(audio_buffer_t const *const pBuff)
{
  return pBuff->nbElements << AudioBuffer_getRealComplexFactor(pBuff->timeFreq);
}


/**
 * @brief  get number of elements of an audio buffer. An element is a sample in real domain and a bin in freq domain; a bin is complex so it has two floats: 1 for real 1 for imaginary part.
 * @param  pBuff: pointer to the audio buffer
 * @retval number of elements (doesn't include *2 for complex (spectral) samples)
 */
uint32_t AudioBuffer_getNbElements(audio_buffer_t const *const pBuff)
{
  return pBuff->nbElements;
}


/**
 * @brief  get number of samples of an audio buffer (kept for legacy USB stack)
 * @param  pBuff: pointer to the audio buffer
 * @retval number of elements (doesn't include *2 for complex (spectral) samples)
 */
uint32_t AudioBuffer_getNbSamples(audio_buffer_t const *const pBuff)
{
  return AudioBuffer_getNbElements(pBuff);
}


/**
 * @brief  get duration in nanoseconds
 * @param  pBuff: pointer to the audio buffer
 * @retval duration
 */
int32_t AudioBuffer_getDurationNs(audio_buffer_t const *const pBuff, uint32_t *const pDuration)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if (AudioBuffer_getTimeFreq(pBuff) == ABUFF_FORMAT_TIME)
  {
    *pDuration = (uint32_t)((((uint64_t)pBuff->nbElements * 1000000000ULL) + ((uint64_t)pBuff->fs / 2ULL)) / (uint64_t)pBuff->fs);   // +fs/2 for rounding
  }
  else
  {
    error = AUDIO_ERR_MGNT_TIME_FREQ;
  }
  return error;
}


/**
 * @brief  reset audio buffer samples
 * @param  pBuff:       pointer to the audio buffer
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_resetData(audio_buffer_t const *const pBuff)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  void *pData = AudioBuffer_getPdata(pBuff);

  if (pData != NULL)
  {
    memset(pData, (int)AudioBuffer_getSilenceFillByte(pBuff), AudioBuffer_getBufferSize(pBuff));
  }
  else
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  return error;
}


/**
 * @brief  set audio buffer interleaved/non-interleaved samples format
 * @param  pBuff:       pointer to the audio buffer
 * @param  interleaved: interleaved/non-interleaved
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_setInterleaved(audio_buffer_t *const pBuff, audio_buffer_interleaved_t const interleaved)
{
  pBuff->interleaved = interleaved;

  return s_updateInternalConfig(pBuff);
}


/**
 * @brief  set audio buffer time/frequency samples format
 * @param  pBuff:       pointer to the audio buffer
 * @param  timeFreq:    time/frequency
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_setTimeFreq(audio_buffer_t *const pBuff, audio_buffer_time_freq_t const timeFreq)
{
  pBuff->timeFreq = timeFreq;

  return s_updateInternalConfig(pBuff);
}


/**
 * @brief  set audio buffer sample type (fixed-point, floating-point, etc...) format
 * @param  pBuff:       pointer to the audio buffer
 * @param  type:        sample type
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_setType(audio_buffer_t *const pBuff, audio_buffer_type_t const type)
{
  pBuff->type = type;

  return s_updateInternalConfig(pBuff);
}


/**
 * @brief  set audio buffer number of channels
 * @param  pBuff:       pointer to the audio buffer
 * @param  nbChannels:  number of channels
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_setNbChannels(audio_buffer_t *const pBuff, uint8_t const nbChannels)
{
  pBuff->nbChannels = nbChannels;

  return s_updateInternalConfig(pBuff);
}


/**
 * @brief  set audio buffer channels pointers
 * @note   allocates channels pointers if not already done
 *         set all channels pointers to NULL if pData is NULL, else set them from pData according to audio buffer config
 * @param  pBuff:       pointer to the audio buffer
 * @param  pData:       samples buffer pointer
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_setPdata(audio_buffer_t *const pBuff, void *const pData)
{
  int32_t error = s_allocPdata(pBuff);

  if (AudioError_isOk(error))
  {
    audio_buffer_context_t *const pContext            = (audio_buffer_context_t *)pBuff->pInternalMem;
    uint32_t                const channelsOffsetBytes = (pData == NULL) ? 0UL : (AudioBuffer_getChannelsOffset(pBuff) * (uint32_t)pContext->sampleSize);  // if pData == NULL: set all channel pointers to NULL

    for (uint8_t ch = 0U; ch < pBuff->nbChannels; ch++)
    {
      pContext->pDataPtr[ch] = (uint8_t *)pData + ((uint32_t)ch * channelsOffsetBytes);
    }
  }

  return error;
}


/**
 * @brief  set audio buffer specific channel pointers
 * @note   allocates channels pointers if not already done
 *         set specific channel pointers to pData
 * @param  pBuff:       pointer to the audio buffer
 * @param  chId:        specific channel id
 * @param  pData:       samples buffer pointer
 * @param  memPool:     memory pool
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_setPdataCh(audio_buffer_t *const pBuff, uint8_t const chId, void *const pData)
{
  int32_t error = s_allocPdata(pBuff);

  if (AudioError_isOk(error))
  {
    audio_buffer_context_t *const pContext = (audio_buffer_context_t *)pBuff->pInternalMem;

    pContext->pDataPtr[chId] = pData;
  }

  return error;
}


/**
 * @brief  get audio buffer samples buffer size (bytes)
 * @param  pBuff:       pointer to the audio buffer
 * @retval samples buffer size
 */
uint32_t AudioBuffer_getBufferSize(audio_buffer_t const *const pBuff)
{
  uint32_t ret = 0U;

  if ((pBuff != NULL) && (pBuff->pInternalMem != NULL))
  {
    audio_buffer_context_t const *const pContext = (audio_buffer_context_t const *)pBuff->pInternalMem;

    if ((pBuff->type == ABUFF_FORMAT_PDM_LSB_FIRST) || (pBuff->type == ABUFF_FORMAT_PDM_MSB_FIRST))
    {
      uint32_t const channelSize = (pBuff->nbElements >> 3U); // nbElements is a multiple of 8: see s_updateInternalConfig

      ret = (uint32_t)pBuff->nbChannels * channelSize;
    }
    else
    {
      ret = (uint32_t)pBuff->nbChannels * pBuff->nbElements * (uint32_t)pContext->sampleFullSize;
    }
  }

  return ret;
}


/**
 * @brief  check audio buffer compatibility between 2 audio buffers
 * @note   allocates channels pointers if not already done
 *         set specific channel pointers to pData
 * @param  pBuff1:      pointer to the first audio buffer
 * @param  pBuff2:      pointer to the second audio buffer
 * @param  toBeTested   mask to specify what param to check
 * @retval Error; AUDIO_ERR_MGNT_NONE if no compatible
 */
int32_t AudioBuffer_checkAudioBuffersCompatibility(audio_buffer_t const *const pBuff1, audio_buffer_t const *const pBuff2, audio_buffer_param_list_t const toBeTested)
{
  int32_t        error         = AUDIO_ERR_MGNT_NONE;
  uint32_t const toBeTestedU32 = (uint32_t)toBeTested;

  if (AudioError_isOk(error))
  {
    void *const pBuff1Data = AudioBuffer_getPdata(pBuff1);
    void *const pBuff2Data = AudioBuffer_getPdata(pBuff2);

    if ((pBuff1Data == NULL) || (pBuff2Data == NULL))
    {
      error = AUDIO_ERR_MGNT_PTR_NULL;
    }
  }

  if (AudioError_isOk(error))
  {
    if (((uint32_t)ABUFF_PARAM_CH & toBeTestedU32) != 0UL)
    {
      uint8_t const nbChBuff1 = AudioBuffer_getNbChannels(pBuff1);
      uint8_t const nbChBuff2 = AudioBuffer_getNbChannels(pBuff2);

      if (nbChBuff1 != nbChBuff2)
      {
        error = AUDIO_ERR_MGNT_NB_CH;
      }
    }
  }

  if (AudioError_isOk(error))
  {
    if (((uint32_t)ABUFF_PARAM_FS & toBeTestedU32) != 0UL)
    {
      uint32_t const fsBuff1 = AudioBuffer_getFs(pBuff1);
      uint32_t const fsBuff2 = AudioBuffer_getFs(pBuff2);

      if (fsBuff1 != fsBuff2)
      {
        error = AUDIO_ERR_MGNT_FS;
      }
    }
  }

  if (AudioError_isOk(error))
  {
    if (((uint32_t)ABUFF_PARAM_TIME_FREQ & toBeTestedU32) != 0UL)
    {
      audio_buffer_time_freq_t const timeFreqBuff1 = AudioBuffer_getTimeFreq(pBuff1);
      audio_buffer_time_freq_t const timeFreqBuff2 = AudioBuffer_getTimeFreq(pBuff2);

      if (timeFreqBuff1 != timeFreqBuff2)
      {
        error = AUDIO_ERR_MGNT_TIME_FREQ;
      }
    }
  }

  if (AudioError_isOk(error))
  {
    if (((uint32_t)ABUFF_PARAM_TYPE & toBeTestedU32) != 0UL)
    {
      audio_buffer_type_t const typeBuff1 = AudioBuffer_getType(pBuff1);
      audio_buffer_type_t const typeBuff2 = AudioBuffer_getType(pBuff2);

      if (typeBuff1 != typeBuff2)
      {
        error = AUDIO_ERR_MGNT_TYPE;
      }
    }
  }

  if (AudioError_isOk(error))
  {
    if (((uint32_t)ABUFF_PARAM_INTERLEAVING & toBeTestedU32) != 0UL)
    {
      audio_buffer_interleaved_t const inteleavedBuff1 = AudioBuffer_getInterleaved(pBuff1);
      audio_buffer_interleaved_t const inteleavedBuff2 = AudioBuffer_getInterleaved(pBuff2);

      if (inteleavedBuff1 != inteleavedBuff2)
      {
        error = AUDIO_ERR_MGNT_INTERLEAVING;
      }
    }
  }

  if (AudioError_isOk(error))
  {
    if (((uint32_t)ABUFF_PARAM_NB_ELEMENTS & toBeTestedU32) != 0UL)
    {
      uint32_t const nbElementsBuff1 = AudioBuffer_getNbElements(pBuff1);
      uint32_t const nbElementsBuff2 = AudioBuffer_getNbElements(pBuff2);

      if (nbElementsBuff1 != nbElementsBuff2)
      {
        error = AUDIO_ERR_MGNT_NB_ELEMENTS;
      }
    }
  }

  if (AudioError_isOk(error))
  {
    if (((uint32_t)ABUFF_PARAM_DURATION & toBeTestedU32) != 0UL)
    {
      uint32_t   durationNs1 = 0UL;
      uint32_t   durationNs2 = 0UL;
      bool const ok1         = AudioError_isOk(AudioBuffer_getDurationNs(pBuff1, &durationNs1));
      bool const ok2         = AudioError_isOk(AudioBuffer_getDurationNs(pBuff2, &durationNs2));

      if (ok1 && ok2 && (durationNs1 != durationNs2))
      {
        error = AUDIO_ERR_MGNT_DURATION;
      }
    }
  }

  return error;
}


/**
 * @brief  merge 2 mono audio buffers into a stereo audio buffer
 * @param  pBuffIn1: pointer to the first input mono audio buffer
 * @param  pBuffIn2: pointer to the second input mono audio buffer
 * @param  pBuffOut: pointer to the output stereo audio buffer
 * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
 */
int32_t AudioBuffer_merge2Stereo(audio_buffer_t const *const pBuffIn1, audio_buffer_t const *const pBuffIn2, audio_buffer_t *const pBuffOut)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if ((pBuffIn1 == NULL) || (pBuffIn1->pInternalMem == NULL) || (pBuffIn2 == NULL) || (pBuffIn2->pInternalMem == NULL) || (pBuffOut == NULL) || (pBuffOut->pInternalMem == NULL))
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    audio_buffer_context_t *const pContextIn1 = (audio_buffer_context_t *)pBuffIn1->pInternalMem;
    audio_buffer_context_t *const pContextIn2 = (audio_buffer_context_t *)pBuffIn2->pInternalMem;
    audio_buffer_context_t *const pContextOut = (audio_buffer_context_t *)pBuffOut->pInternalMem;

    if ((pContextIn1->pDataPtr == NULL)                     ||
        (pContextIn2->pDataPtr == NULL)                     ||
        (pContextOut->pDataPtr == NULL)                     ||
        (pBuffIn1->timeFreq    != ABUFF_FORMAT_TIME)        ||
        (pBuffIn2->timeFreq    != ABUFF_FORMAT_TIME)        ||
        (pBuffOut->timeFreq    != ABUFF_FORMAT_TIME)        ||
        (pBuffIn2->type        != pBuffIn1->type)           ||
        (pBuffOut->type        != pBuffIn1->type)           ||
        (pBuffIn1->nbChannels  != 1U)                       ||
        (pBuffIn2->nbChannels  != 1U)                       ||
        (pBuffOut->nbChannels  != 2U)                       ||
        (pBuffIn2->nbElements  != pBuffIn1->nbElements)     ||
        (pBuffOut->nbElements  != pBuffIn1->nbElements))
    {
      error = AUDIO_ERR_MGNT_ERROR;
    }
    else
    {
      switch (AudioBuffer_getSampleSize(pBuffIn1))
      {
        case 1U:
        {
          uint32_t const  n        = ((pBuffIn1->type == ABUFF_FORMAT_PDM_LSB_FIRST) || (pBuffIn1->type == ABUFF_FORMAT_PDM_MSB_FIRST)) ? (pBuffIn1->nbElements >> 3U) : pBuffIn1->nbElements;
          int8_t   const *pIn1Int8 = (int8_t const *)pContextIn1->pDataPtr[0];
          int8_t   const *pIn2Int8 = (int8_t const *)pContextIn2->pDataPtr[0];
          int8_t         *pOutInt8 = (int8_t *)      pContextOut->pDataPtr[0];

          for (uint32_t i = 0UL; i < n; i++)
          {
            *pOutInt8 = *pIn1Int8;  pOutInt8++;  pIn1Int8++;
            *pOutInt8 = *pIn2Int8;  pOutInt8++;  pIn2Int8++;
          }
          break;
        }

        case 2U:
        {
          int16_t const *pIn1Int16 = (int16_t const *)pContextIn1->pDataPtr[0];
          int16_t const *pIn2Int16 = (int16_t const *)pContextIn2->pDataPtr[0];
          int16_t       *pOutInt16 = (int16_t *)      pContextOut->pDataPtr[0];

          for (uint32_t i = 0UL; i < pBuffIn1->nbElements; i++)
          {
            *pOutInt16 = *pIn1Int16;  pOutInt16++;  pIn1Int16++;
            *pOutInt16 = *pIn2Int16;  pOutInt16++;  pIn2Int16++;
          }
          break;
        }

        case 4U:
        {
          int32_t const *pIn1Int32 = (int32_t const *)pContextIn1->pDataPtr[0];
          int32_t const *pIn2Int32 = (int32_t const *)pContextIn2->pDataPtr[0];
          int32_t       *pOutInt32 = (int32_t *)      pContextOut->pDataPtr[0];

          for (uint32_t i = 0UL; i < pBuffIn1->nbElements; i++)
          {
            *pOutInt32 = *pIn1Int32;  pOutInt32++;  pIn1Int32++;
            *pOutInt32 = *pIn2Int32;  pOutInt32++;  pIn2Int32++;
          }
          break;
        }

        default:
          break;
      }
    }
  }

  return error;
}


/**
* @brief  get audio buffer sample address
* @param  pBuff:    pointer to the audio buffer
* @param  chId:     channel id
* @param  sampleId: sample id
* @retval sample address
*/
void *AudioBuffer_getSampleAddress(audio_buffer_t const *const pBuff, uint8_t const chId, uint32_t const sampleId)
{
  void *pSample = NULL;

  if ((pBuff != NULL) && (pBuff->pInternalMem != NULL))
  {
    audio_buffer_context_t *const pContext = (audio_buffer_context_t *)pBuff->pInternalMem;

    if (pContext->pDataPtr != NULL)
    {
      uint8_t *const pSampleBase = (uint8_t *)pContext->pDataPtr[chId];
      uint32_t const pos         = ((pBuff->type == ABUFF_FORMAT_PDM_LSB_FIRST) || (pBuff->type == ABUFF_FORMAT_PDM_MSB_FIRST)) ? (sampleId >> 3U) : sampleId;

      pSample = pSampleBase + ((uint32_t)pContext->sampleSize * pos * (uint32_t)pContext->samplesOffset);
    }
  }

  return pSample;
}


/**
* @brief  get audio buffer silence fill byte depending on sample type (byte to use in memset to reset samples buffer)
* @param  pBuff:    pointer to the audio buffer
* @retval silence fill byte
*/
uint8_t AudioBuffer_getSilenceFillByte(audio_buffer_t const *const pBuff)
{
  static uint8_t const s_silenceFillByte[ABUFF_FORMAT_NB] =
  {
    [ABUFF_FORMAT_PDM_LSB_FIRST] = 0x55U,
    [ABUFF_FORMAT_PDM_MSB_FIRST] = 0x55U,
    [ABUFF_FORMAT_G711_ALAW]     = 0x55U,
    [ABUFF_FORMAT_G711_MULAW]    = 0x7FU,
    [ABUFF_FORMAT_FIXED16]       = 0x00U,
    [ABUFF_FORMAT_FIXED32]       = 0x00U,
    [ABUFF_FORMAT_FLOAT]         = 0x00U
  };
  audio_buffer_type_t const sampleType      = AudioBuffer_getType(pBuff);
  uint8_t                   silenceFillByte = 0x00U;

  if (sampleType < ABUFF_FORMAT_NB)
  {
    silenceFillByte = s_silenceFillByte[sampleType];
  }

  return silenceFillByte;
}


/**
* @brief  get audio buffer time/frequency type string
* @param  pBuff:    pointer to the audio buffer
* @retval audio buffer time/frequency type string
*/
char const *AudioBuffer_getTimeFreqString(audio_buffer_t const *const pBuff)
{
  char const *pString;

  switch (AudioBuffer_getTimeFreq(pBuff))
  {
    case ABUFF_FORMAT_TIME:
      pString = "time sample";
      break;
    case ABUFF_FORMAT_FREQ:
      pString = "freq sample";
      break;
    default:
      pString = "time/freq ???";
      break;
  }

  return pString;
}


/**
* @brief  get audio buffer sample type string
* @param  pBuff:    pointer to the audio buffer
* @retval audio buffer sample type string
*/
char const *AudioBuffer_getTypeString(audio_buffer_t const *const pBuff)
{
  char const *pString;

  switch (AudioBuffer_getType(pBuff))
  {
    case ABUFF_FORMAT_PDM_LSB_FIRST:
      pString = "PDM LSB first";
      break;
    case ABUFF_FORMAT_PDM_MSB_FIRST:
      pString = "PDM MSB first";
      break;
    case ABUFF_FORMAT_G711_ALAW:
      pString = "G711 A law";
      break;
    case ABUFF_FORMAT_G711_MULAW:
      pString = "G711 mu law";
      break;
    case ABUFF_FORMAT_FIXED16:
      pString = "fixed-point 16";
      break;
    case ABUFF_FORMAT_FIXED32:
      pString = "fixed-point 32";
      break;
    case ABUFF_FORMAT_FLOAT:
      pString = "floating-point";
      break;
    default:
      pString = "format type ???";
      break;
  }

  return pString;
}


/**
* @brief  get audio buffer interleaved/non-interleaved string
* @param  pBuff:    pointer to the audio buffer
* @retval audio buffer interleaved/non-interleaved string
*/
char const *AudioBuffer_getInterleavedString(audio_buffer_t const *const pBuff)
{
  char const *pString;

  switch (AudioBuffer_getInterleaved(pBuff))
  {
    case ABUFF_FORMAT_NON_INTERLEAVED:
      pString = "non-interleaved";
      break;
    case ABUFF_FORMAT_INTERLEAVED:
      pString = "interleaved";
      break;
    default:
      pString = "interleaved ???";
      break;
  }

  return pString;
}


/**
* @brief  get audio buffer description string
* @param  pBuff:    pointer to the audio buffer
* @param  pString:  pointer to the string
* @param  n:        size of the string
* @retval number of characters of the output string
*/
int AudioBuffer_getDescriptionString(audio_buffer_t const *const pBuff, char *const pString, size_t const n)
{
  char const *const pTimeFreqString = AudioBuffer_getTimeFreqString(pBuff);
  uint32_t    const fs              = AudioBuffer_getFs(pBuff);
  uint8_t     const nbChannels      = AudioBuffer_getNbChannels(pBuff);
  uint32_t    const nbElements      = AudioBuffer_getNbElements(pBuff);

  return snprintf(pString, n, "domain:%s, freq:%uHz, channel:%u, nb samples:%u", pTimeFreqString, fs, nbChannels, nbElements);
}


/**
* @brief  get left shift (depending on timeFreq sample format) to apply to obtain an offset in a sample buffer
* @param  timeFreq:    time/freq sample format
* @retval left shift
*/
uint8_t AudioBuffer_getRealComplexFactor(audio_buffer_time_freq_t const timeFreq)
{
  return (uint8_t)timeFreq;
}


void AudioBuffer_setAllocStatsPtr(memAllocStat_t *const pAllocStats)
{
  pAudioBufferAllocStats = pAllocStats;
}


/* Private routines ----------------------------------------------------------*/

static int32_t s_allocPdata(audio_buffer_t *const pBuff)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if ((pBuff == NULL) || (pBuff->pInternalMem == NULL))
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    audio_buffer_context_t *const pContext = (audio_buffer_context_t *)pBuff->pInternalMem;

    if (pContext->pDataPtr == NULL)
    {
      void *const pMalloc = s_malloc((size_t)pBuff->nbChannels * sizeof(void *), pContext->memPool);

      if (pMalloc == NULL)
      {
        error = AUDIO_ERR_MGNT_ALLOCATION;
      }
      else
      {
        pContext->pDataPtr = pMalloc;
      }
    }
  }

  return error;
}


static int32_t s_updateInternalConfig(audio_buffer_t *const pBuff)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if ((pBuff == NULL) || (pBuff->pInternalMem == NULL))
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    audio_buffer_context_t *const pContext       = (audio_buffer_context_t *)pBuff->pInternalMem;
    bool                    const nonInterleaved = (pBuff->interleaved == ABUFF_FORMAT_NON_INTERLEAVED);

    if ((pBuff->type == ABUFF_FORMAT_PDM_LSB_FIRST) || (pBuff->type == ABUFF_FORMAT_PDM_MSB_FIRST))
    {
      if (((pBuff->nbElements % 8UL) != 0UL) || (pBuff->timeFreq != ABUFF_FORMAT_TIME))
      {
        // nbElements must be multiple of 8 and timeFreq must be ABUFF_FORMAT_TIME samples for PDM samples
        error = AUDIO_ERR_MGNT_ERROR;
      }
      else
      {
        // values for 8 PDM samples
        pContext->sampleSize     = 1U;
        pContext->sampleFullSize = 1U;
        if (nonInterleaved)
        {
          pContext->channelsOffset = (pBuff->nbElements >> 3U);
          pContext->samplesOffset  = 1U;
        }
        else
        {
          pContext->channelsOffset = 1UL;
          pContext->samplesOffset  = pBuff->nbChannels;
        }
      }
    }
    else
    {
      if (nonInterleaved)
      {
        pContext->channelsOffset = pBuff->nbElements;
        pContext->samplesOffset  = 1U;
      }
      else
      {
        pContext->channelsOffset = 1UL;
        pContext->samplesOffset  = pBuff->nbChannels;
      }

      switch (pBuff->type)
      {
        case ABUFF_FORMAT_G711_ALAW:
        case ABUFF_FORMAT_G711_MULAW:
          pContext->sampleSize = 1U;
          break;
        case ABUFF_FORMAT_FIXED16:
          pContext->sampleSize = 2U;
          break;
        case ABUFF_FORMAT_FIXED32:
        case ABUFF_FORMAT_FLOAT:
          pContext->sampleSize = 4U;
          break;
        default:
          // unsupported sample format type
          error = AUDIO_ERR_MGNT_ERROR;
          break;
      }

      if (AudioError_isOk(error))
      {
        pContext->sampleFullSize = (pContext->sampleSize << AudioBuffer_getRealComplexFactor(pBuff->timeFreq));
      }
    }
  }

  return error;
}


static void s_addBufferAllocStat(void *const pAlloc)
{
  if ((pAlloc != NULL) && (pAudioBufferAllocStats != NULL))
  {
    memAllocInfo_t allocInfo;

    AudioMalloc_getInfo(pAlloc, &allocInfo);

    pAudioBufferAllocStats[allocInfo.requestedMemoryPool].totalRequestedAllocSize += allocInfo.size;
    pAudioBufferAllocStats[allocInfo.requestedMemoryPool].nbRequestedMalloc++;
    pAudioBufferAllocStats[allocInfo.allocatedMemoryPool].totalAllocatedAllocSize += allocInfo.size;
    pAudioBufferAllocStats[allocInfo.allocatedMemoryPool].nbAllocatedMalloc++;
  }
}


static void s_subBufferAllocStat(void *const pAlloc)
{
  if ((pAlloc != NULL) && (pAudioBufferAllocStats != NULL))
  {
    memAllocInfo_t allocInfo;

    AudioMalloc_getInfo(pAlloc, &allocInfo);

    pAudioBufferAllocStats[allocInfo.requestedMemoryPool].totalRequestedAllocSize -= allocInfo.size;
    pAudioBufferAllocStats[allocInfo.requestedMemoryPool].nbRequestedMalloc--;
    pAudioBufferAllocStats[allocInfo.allocatedMemoryPool].totalAllocatedAllocSize -= allocInfo.size;
    pAudioBufferAllocStats[allocInfo.allocatedMemoryPool].nbAllocatedMalloc--;
  }
}


#ifdef AUDIO_MEM_CONF_TRACK_MALLOC

void *s_mallocDebug(size_t const size, memPool_t const memPool, char const *const file, int const line)
{
  void *const ptr = AudioMallocDebug(size, memPool, file, line);

  s_addBufferAllocStat(ptr);

  return ptr;
}


void s_freeDebug(void *const pMemToFree, memPool_t const memPool)
{
  if (pMemToFree != NULL)
  {
    s_subBufferAllocStat(pMemToFree);

    AudioFreeDebug(pMemToFree, memPool);
  }
}

#else /* AUDIO_MEM_CONF_TRACK_MALLOC */

static void *s_malloc(size_t const size, memPool_t const memPool)
{
  void *const ptr = AudioMalloc(size, memPool);

  s_addBufferAllocStat(ptr);

  return ptr;
}


void s_free(void *const pMemToFree, memPool_t const memPool)
{
  if (pMemToFree != NULL)
  {
    s_subBufferAllocStat(pMemToFree);

    AudioFree(pMemToFree, memPool);
  }
}

#endif /* AUDIO_MEM_CONF_TRACK_MALLOC */
