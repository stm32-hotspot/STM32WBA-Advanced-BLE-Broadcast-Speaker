/**
  ******************************************************************************
  * @file    speexAlloc.c
  * @author  MCD Application Team
  * @brief   speex alloc routines wrapper
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
#include "common/speexAlloc.h"
#include "audio_algo.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined(OVERRIDE_SPEEX_ALLOC) && defined(OVERRIDE_SPEEX_FREE)
  static memPool_t speexMemPool = AUDIO_MEM_TCM;
#endif

/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

#ifdef OVERRIDE_SPEEX_ALLOC
/** Speex wrapper for calloc. To do your own dynamic allocation, all you need to do is replace this function, speex_realloc and speex_alloc */
void *speex_alloc(int size)
{
  /* WARNING: this is not equivalent to malloc(). If you want to use malloc()
     or your own allocator, YOU NEED TO CLEAR THE MEMORY ALLOCATED. Otherwise
     you will experience strange bugs */
  void *pAlloc = AudioAlgo_malloc((size_t)size, speexMemPool);
  if (pAlloc != NULL)
  {
    memset(pAlloc, 0, (size_t)size);
  }
  return pAlloc;
}
#endif


#ifdef OVERRIDE_SPEEX_FREE
void speex_free(void *ptr)
{
  AudioAlgo_free(ptr, speexMemPool);
}
#endif


#if defined(OVERRIDE_SPEEX_ALLOC) && defined(OVERRIDE_SPEEX_FREE)
void speex_setMemPool(memPool_t const memPool)
{
  speexMemPool = memPool;
}
#endif