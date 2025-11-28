/**
******************************************************************************
* @file    audio_chunk.h
* @author  MCD Application Team
* @brief   Header for audio_chunk structure and routines for basic processing
*          & generic pointers update.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_CHUNK_H
#define __AUDIO_CHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "audio_buffer.h"
#include "audio_chain_utilities.h"
#include "sfc.h"
/* Exported constants --------------------------------------------------------*/
#define AUDIO_CHUNK_SIGNATURE                      0x4B554843U /* 'CHUK' */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  AUDIO_CHUNK_TYPE_RESET      = 0,
  AUDIO_CHUNK_TYPE_SYSTEM     = 1,
  AUDIO_CHUNK_TYPE_USER       = 2,
  AUDIO_CHUNK_TYPE_SRC        = 4,
  AUDIO_CHUNK_TYPE_SINK       = 8,
  AUDIO_CHUNK_TYPE_SYS_IN     = (AUDIO_CHUNK_TYPE_SYSTEM | AUDIO_CHUNK_TYPE_SRC),                           // system src
  AUDIO_CHUNK_TYPE_SYS_OUT    = (AUDIO_CHUNK_TYPE_SYSTEM                        | AUDIO_CHUNK_TYPE_SINK),   // system sink
  AUDIO_CHUNK_TYPE_SYS_INOUT  = (AUDIO_CHUNK_TYPE_SYSTEM | AUDIO_CHUNK_TYPE_SRC | AUDIO_CHUNK_TYPE_SINK),   // system in/out
  AUDIO_CHUNK_TYPE_USER_SRC   = (AUDIO_CHUNK_TYPE_USER   | AUDIO_CHUNK_TYPE_SRC),                           // user src
  AUDIO_CHUNK_TYPE_USER_SINK  = (AUDIO_CHUNK_TYPE_USER                          | AUDIO_CHUNK_TYPE_SINK),   // user sink
  AUDIO_CHUNK_TYPE_USER_INOUT = (AUDIO_CHUNK_TYPE_USER   | AUDIO_CHUNK_TYPE_SRC | AUDIO_CHUNK_TYPE_SINK),   // user in/out
  AUDIO_CHUNK_TYPE_ALL        = (AUDIO_CHUNK_TYPE_SYS_INOUT | AUDIO_CHUNK_TYPE_USER_INOUT)                  // all types
} audio_chunk_type_t;

/* WARNING: livetune_chunk_conf_t must be aligned with audio_chunk_conf_t;
            in case of update of audio_chunk_conf_t,
            livetune_chunk_conf_t must be updated accordingly */
typedef struct audio_chunk_conf
{
  uint8_t     chunkType;
  uint8_t     timeFreq;
  uint8_t     bufferType;
  uint8_t     interleaved;
  uint8_t     nbFrames;
  uint8_t     nbChannels;
  uint32_t    nbElements;
  uint32_t    fs;
  const char *pName;
} audio_chunk_conf_t;

typedef struct audio_chunk
{
  uint32_t    signature;
  char const *pName;
  char const *pDescription;
  void       *pInternalMem;
} audio_chunk_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t AudioChunk_init(audio_chunk_t                      *const pChunk,
                        char                         const *const pName,
                        audio_chain_utilities_t            *const pUtilsHandle,
                        memPool_t                           const memPool);

int32_t AudioChunk_config(audio_chunk_t                    *const pChunk,
                          audio_chunk_type_t                const chunkType,
                          uint8_t                           const nbChannels,
                          uint32_t                          const fs,
                          uint32_t                          const nbElements,
                          uint8_t                           const nbFrames,
                          audio_buffer_time_freq_t          const timeFreq,
                          audio_buffer_type_t               const bufferType,
                          audio_buffer_interleaved_t        const interleaved);

int32_t AudioChunk_initAndConfig(audio_chunk_t             *const pChunk,
                                 audio_chunk_type_t         const chunkType,
                                 uint8_t                    const nbChannels,
                                 uint32_t                   const fs,
                                 uint32_t                   const nbElements,
                                 uint8_t                    const nbFrames,
                                 audio_buffer_time_freq_t   const timeFreq,
                                 audio_buffer_type_t        const bufferType,
                                 audio_buffer_interleaved_t const interleaved,
                                 char                const *const pName,
                                 audio_chain_utilities_t   *const pUtilsHandle,
                                 memPool_t                  const memPool);

int32_t AudioChunk_deinit(audio_chunk_t *const pChunk);

int32_t AudioChunk_initAtAddr(audio_chunk_t               **const ppChunk,
                              audio_chunk_type_t            const chunkType,
                              audio_buffer_t         const *const pBuffConfig,
                              uint8_t                       const nbFrames,
                              char                   const *const pName,
                              audio_chain_utilities_t      *const pUtilsHandle,
                              void                         *const pAllocatedMemory,
                              uint32_t                     *const pSize,
                              memPool_t                     const memPool);

int32_t AudioChunk_deinitAtAddr(audio_chunk_t              *const pChunk);
int32_t AudioChunk_create(audio_chunk_t                   **const ppChunk,
                          char                       const *const pName,
                          audio_chain_utilities_t          *const pUtilsHandle,
                          memPool_t                         const memPool);


bool    AudioChunk_isSystem(audio_chunk_t const *const pChunk);


void                    *AudioChunk_getPdata(audio_chunk_t                const *const pChunk);
int32_t                  AudioChunk_setPdata(audio_chunk_t                      *const pChunk,   void *const pData);
void                     AudioChunk_setDescription(audio_chunk_t                *const pChunk,   char const *const pDescription);
int32_t                  AudioChunk_reset(audio_chunk_t                         *const pChunk);
int32_t                  AudioChunk_getMemorySize(audio_buffer_t          const *const pBuffer,  uint8_t const nbFrames, uint32_t *const pMemorySize);
int32_t                  AudioChunk_allocateCh(audio_chunk_t                    *const pChunk,   memPool_t const memPool);
int32_t                  AudioChunk_allocateAndSet(audio_chunk_t                *const pChunk,   memPool_t const memPool, uint8_t const nbFrames, char const *const pName);
int32_t                  AudioChunk_copyConfig(audio_chunk_t              const *const pChunkIn, audio_chunk_t *const pChunkOut);
int32_t                  AudioChunk_setChannelsBuffPtr(audio_chunk_t            *const pChunk);
int32_t                  AudioChunk_setAudioBufPtrFromRdPtr(audio_chunk_t const *const pChunk,   audio_buffer_t *const pBuff);
int32_t                  AudioChunk_setAudioBufPtrFromWrPtr(audio_chunk_t const *const pChunk,   audio_buffer_t *const pBuff);
uint32_t                 AudioChunk_getAvailableSamples(audio_chunk_t     const *const pChunk);
uint32_t                 AudioChunk_getFreeSamples(audio_chunk_t          const *const pChunk);
void                    *AudioChunk_getWritePtr0(audio_chunk_t            const *const pChunk);
void                    *AudioChunk_getReadPtr0(audio_chunk_t             const *const pChunk);
void                    *AudioChunk_getWritePtr(audio_chunk_t             const *const pChunk,   uint8_t  const chId, uint32_t const spleId);
void                    *AudioChunk_getReadPtr(audio_chunk_t              const *const pChunk,   uint8_t  const chId, uint32_t const spleId);
int32_t                  AudioChunk_updateReadPtr(audio_chunk_t                 *const pChunk,   uint32_t const incr);
int32_t                  AudioChunk_defaultUpdateReadPtr(audio_chunk_t          *const pChunk);
int32_t                  AudioChunk_updateWritePtr(audio_chunk_t                *const pChunk,   uint32_t const incr);
int32_t                  AudioChunk_defaultUpdateWritePtr(audio_chunk_t         *const pChunk);
uint32_t                 AudioChunk_downSample(audio_chunk_t              const *const pChunkIn, audio_chunk_t *const pChunkOut);
audio_buffer_t          *AudioChunk_getBuffInfo(audio_chunk_t             const *const pChunk);
int                      AudioChunk_getDescriptionString(audio_chunk_t    const *const pChunk,   char *const pString, size_t const n);
void                     AudioChunk_traceConfig(audio_chunk_t             const *const pChunk);
bool                     AudioChunk_getFrameReady(audio_chunk_t           const *const pChunk);
bool                     AudioChunk_getFrameSpace(audio_chunk_t           const *const pChunk);
audio_chunk_type_t       AudioChunk_getType(audio_chunk_t                 const *const pChunk);
void                     AudioChunk_setType(audio_chunk_t                 const *const pChunk, audio_chunk_type_t type);
void                     AudioChunk_registerUtilities(audio_chunk_t       const *const pChunk, audio_chain_utilities_t *const pUtilsHandle);
void                     AudioChunk_setInstanceAllocated(audio_chunk_t    const *const pChunk, bool const state);
bool                     AudioChunk_isConfigured(audio_chunk_t            const *const pChunk);
int32_t                  AudioChunk_setParamConfig(audio_chunk_t                *const pChunk, const char *pKey, const char *pValue);
audio_chunk_conf_t      *AudioChunk_getConf(audio_chunk_t           const *const pChunk);

audio_chain_utilities_t *AudioChunk_getUtilities(audio_chunk_t      const *const pChunk);

void                     AudioChunk_setAllocStatsPtr(memAllocStat_t       *const pAllocStats);


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHUNK_H */
