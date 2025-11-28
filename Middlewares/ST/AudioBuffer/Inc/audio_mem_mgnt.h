/**
******************************************************************************
* @file    audio_mem_mgnt.h
* @author  MCD Application Team
* @brief   malloc management
******************************************************************************
* @attention
*
* Copyright (c) 2019(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_MEM_MGNT_H
#define __AUDIO_MEM_MGNT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "audio_mem_mgnt_conf.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  AUDIO_MEM_TCM,
  AUDIO_MEM_RAMINT,
  AUDIO_MEM_RAMEXT,
  #ifdef AUDIO_MEM_NOCACHED_ENABLED
  AUDIO_MEM_NOCACHED,
  #endif
  AUDIO_MEM_NB_POOL,
  AUDIO_MEM_UNKNOWN
} memPool_t;

typedef enum
{
  AUDIO_MEM_ERROR_NONE,
  AUDIO_MEM_ERROR_ALLOCATION,
  AUDIO_MEM_ERROR_MEMORY_LEAK
} memError_t;

typedef struct
{
  uint32_t size                : 26;  // up to 64 MBytes
  uint32_t requestedMemoryPool : 3;
  uint32_t allocatedMemoryPool : 3;
} memAllocInfo_t;

typedef struct
{
  size_t   totalRequestedAllocSize;
  size_t   totalAllocatedAllocSize;
  uint16_t nbRequestedMalloc;
  uint16_t nbAllocatedMalloc;
} memAllocStat_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#define AUDIO_MEM_ALIGNED_SIZE         4UL
#define AUDIO_MEM_SIZE_ALIGN(size)     (((size) + AUDIO_MEM_ALIGNED_SIZE - 1UL) & (0xFFFFFFFFUL - (AUDIO_MEM_ALIGNED_SIZE - 1UL)))
#define AUDIO_MEM_SIZEOF_ALIGN(object) AUDIO_MEM_SIZE_ALIGN(sizeof(object))

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif /* __packed */
#endif /* __GNUC__ */

/* Exported functions ------------------------------------------------------- */

/* In HS mode and when the DMA is used, all variables and data structures dealing
   with the DMA during the transaction process should be 4-bytes aligned */

bool  AudioMallocCheckPtr(void *ptr, int const type);
void *AudioMallocDebug(size_t const size, memPool_t const memPool, char const *const file, int const line);
void *AudioCallocDebug(size_t const nbElements, size_t const elementSize, memPool_t const memPool, char const *const file, int const line);
void *AudioReallocDebug(void *const ptr, size_t const size, memPool_t const memPool, char const *const file, int const line);
void  AudioFreeDebug(void *const pMemToFree, memPool_t const memPool);
#ifdef AUDIO_MEM_CONF_TRACK_MALLOC
#define AudioMalloc(size, memPool)                    AudioMallocDebug((size), (memPool), __FILE__, __LINE__)
#define AudioCalloc(nbElements, elementSize, memPool) AudioCallocDebug((nbElements), (elementSize), (memPool), __FILE__, __LINE__)
#define AudioRealloc(ptr, size, memPool)              AudioReallocDebug((ptr), (size), (memPool), __FILE__, __LINE__)
#define AudioFree(pMemToFree, memPool)                AudioFreeDebug((pMemToFree), (memPool))
#else
void *AudioMalloc(size_t const size, memPool_t const memPool);
void *AudioCalloc(size_t const nbElements, size_t const elementSize, memPool_t const memPool);
void *AudioRealloc(void *const ptr, size_t const size, memPool_t const memPool);
void  AudioFree(void *const pMemToFree, memPool_t const memPool);
#endif
memError_t AudioMallocCheckConsistency(void);
memError_t AudioMallocDisplayRemainingMalloc(int const resetMax);
void AudioMalloc_getInfo(void const *const ptr, memAllocInfo_t *const pMemAllocInfo);


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_MEM_MGNT_H */
