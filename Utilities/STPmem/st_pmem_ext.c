/**
******************************************************************************
* @file    st_pmem_ext.c
* @author  MCD Application Team
* @brief   This module implements collection and services objects
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

#include <string.h>
#include <stdbool.h>
#include "st_pmem.h"

#undef STVS_USE_GLOBAL_CORRUPTION_TRACKER

/*
  Warning:

  The st_pmem allocator is significantly slower than the original allocator

  Add  those options in the linker command line
  --redirect malloc=SAFE_malloc
  --redirect free=SAFE_free
  --redirect realloc=SAFE_realloc
  --redirect calloc=SAFE_calloc

*/


static bool bFirstAlloc = false;
pmem_pool_t hCorruptionPool;

#define SAFE_LOCK_ALLOC()   do {} while(0)
#define SAFE_UNLOCK_ALLOC() do {} while(0)


#define USE_WHOLE_HEAP

void SAFE_Init_Heap(void);


/* IAR : SAFE_alloc may use the whole c heap*/
#if defined(__ICCARM__) && defined(USE_WHOLE_HEAP)

#pragma section = "HEAP"
const char   *heapStart = __section_begin("HEAP");
const char   *heapEnd   = __section_end("HEAP");
#define STVS_HEAP_CORRUPTION_SIZE ((uint32_t)__section_end("HEAP") - (uint32_t)__section_begin("HEAP"))
#define tCorruptionHeap ((char *)__section_begin("HEAP"))

void SAFE_Init_Heap(void)
{
  if (!bFirstAlloc)
  {
    /* Init the pool */
    pmem_init(&hCorruptionPool, tCorruptionHeap, STVS_HEAP_CORRUPTION_SIZE, 4); /*cstat !MISRAC2012-Rule-11.6 no issue with STVS_HEAP_CORRUPTION_SIZE (difference of pointers)*/
    /* The app does a lot of malloc/free, check the corruption every 1000 allocs */
    hCorruptionPool.m_checkFreq = 0;
    bFirstAlloc = true;
  }
}
#else
void SAFE_Init_Heap(void)
{
}

#endif


/**
 * @brief Init the safe pool heap
 *
 */


void SAFE_Init_External_Heap(char *heap, size_t size);
void SAFE_Init_External_Heap(char *heap, size_t size)
{
  if (!bFirstAlloc)
  {
    /* Init the pool */
    pmem_init(&hCorruptionPool, heap, size, 4);
    /* The app does a lot of malloc/free, check the corruption every 1000 allocs */
    hCorruptionPool.m_checkFreq = 0;
    bFirstAlloc = true;
  }
}
/**
 * @brief  * @brief safe pool equivalent to malloc

 *
 * @return void*
 */
void *SAFE_malloc(uint32_t size);
void *SAFE_malloc(uint32_t size)
{
  SAFE_Init_Heap();
  SAFE_LOCK_ALLOC();
  void *pMalloc = pmem_alloc(&hCorruptionPool, size);
  SAFE_UNLOCK_ALLOC();
  return pMalloc;
}


/**
 * @brief  * @brief safe pool equivalent to free
 *
 */
void SAFE_free(void *pAlloc);
void SAFE_free(void *pAlloc)
{
  SAFE_Init_Heap();
  SAFE_LOCK_ALLOC();
  pmem_free(&hCorruptionPool, pAlloc);
  SAFE_UNLOCK_ALLOC();
}


/**
  * @brief safe pool equivalent to realloc
*
 * @return void*
 */
void *SAFE_realloc(void *pAlloc, uint32_t size);
void *SAFE_realloc(void *pAlloc, uint32_t size)
{
  SAFE_Init_Heap();
  SAFE_LOCK_ALLOC();
  void *pMalloc = pmem_realloc(&hCorruptionPool, pAlloc, size);
  SAFE_UNLOCK_ALLOC();
  return pMalloc;
}


/**
 * @brief safe pool equivalent to calloc

 *
 * @param num
 * @param size
 * @return void*
 */
void *SAFE_calloc(uint32_t num, uint32_t size);
void *SAFE_calloc(uint32_t num, uint32_t size)
{
  SAFE_Init_Heap();
  SAFE_LOCK_ALLOC();
  void *pMalloc = pmem_alloc(&hCorruptionPool, num * size);
  if (pMalloc)
  {
    memset(pMalloc, 0, num * size);
  }
  SAFE_UNLOCK_ALLOC();
  return pMalloc;
}


