/**
******************************************************************************
* @file          st_os_mem.c
* @author        MCD Application Team
* @brief         implement multi memory pool management
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
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <cmsis_compiler.h>
#include "st_os_mem.h"
#include "st_os.h"

#ifdef USE_THREADX
  #include "tx_api.h"
#endif

/* Pre-declaration ----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
#ifdef USE_THREADS
static st_mutex hLockMemory = {.hMutex = NULL};
#endif

/* Private defines -----------------------------------------------------------*/
#define MAX_STACK_SLOT (30)

/* Private macros ------------------------------------------------------------*/
#ifndef ST_MIN
  #define ST_MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  ALLOC_API,
  CALLOC_API
} alloc_api;

/* function declaration --------------------------------------------------------*/

__WEAK void st_os_mem_lock(void)
{
  #ifdef USE_THREADS
  if (hLockMemory.hMutex == NULL)
  {
    /* mutex creation */
    ST_VERIFY((st_os_mutex_create(&hLockMemory) == ST_OS_OK));
  }
  /* lock mutex */
  st_os_mutex_lock(&hLockMemory);
  #else
  ST_OS_DISABLE_IRQ();
  #endif
}

__WEAK void st_os_mem_unlock(void)
{
  #ifdef USE_THREADS
  ST_VERIFY(hLockMemory.hMutex != NULL);
  /* unlock mutex */
  st_os_mutex_unlock(&hLockMemory);
  #else
  ST_OS_ENABLE_IRQ();
  #endif
}

__WEAK bool st_os_mem_section_info(struct st_os_mem_status **ppSysInfo)
{
  static bool             done = false;
  static st_os_mem_status sysInfo;
  st_os_mem_status       *pSysInfo = &sysInfo;

  if (!done)
  {
    memset(pSysInfo, 0, sizeof(*pSysInfo));
    done = true;
  }

  *ppSysInfo = pSysInfo;

  return false;
}

__WEAK int32_t st_os_mem_pool_max(void)
{
  return 0;
}

__WEAK int32_t st_os_mem_any_max(void)
{
  return 0;
}

__WEAK pmem_pool_t *st_os_mem_pool_index(int32_t index)
{
  return NULL;
}

__WEAK bool st_os_mem_pool_is_valid(pmem_pool_t *pPool)
{
  return false;
}

__WEAK int32_t st_os_find_pool(st_mem_type type)
{
  return -1;
}

__WEAK bool st_os_mem_create_pool(st_mem_type osPool, uint32_t offPool, uint32_t szPool, char *pPoolName, uint32_t flgAlign)
{
  return false;
}

__WEAK void st_os_mem_init_ext(int32_t extIndexPool)
{
}

__WEAK void st_os_mem_set_ext_pool_index(int32_t extIndexPool)
{
}




/**
* @brief Checks if ptr is a heap block.   ptr must be the pointer returned by malloc.
 * @param pHeap a pointer
 * @return bool
 */

__WEAK bool st_os_mem_check_heap_ptr(void *pHeap)
{
  return true;
}




/**
 * @brief default Pool creation, this function could be overloaded according to the board or the platform
 *
 * @return
 */
__WEAK void st_os_mem_init(void)
{
  /* All in the Heap */
}

/**
 * @brief Term mem pools
 *
 * @return
 */
__WEAK void st_os_mem_term(void)
{
}



st_mem_type st_os_mem_type_from_name(const char *pPoolName)
{
  st_os_mem_init();

  if (strcmp(pPoolName, "HEAP") == 0)
  {
    return ST_Mem_Type_HEAP;
  }

  #ifdef ST_USE_PMEM
  for (int32_t iPool = 0; iPool < st_os_mem_pool_max(); iPool++)
  {
    pmem_pool_t *pPool = st_os_mem_pool_index(iPool);
    if ((pPool != NULL)  && (pPool->m_pName != NULL))
    {
      if (strcmp(pPoolName, pPool->m_pName) == 0)
      {
        return (st_mem_type)pPool->m_alias;
      }
    }
  }
  #endif
  return ST_Mem_Type_UNMAPPED;
}






/**
 * @brief default all in the heap
 *
 * @param pMem the block pointer
 * @return
 */
#ifdef ST_USE_PMEM

__WEAK uint32_t st_os_mem_size_ptr(void *pMem)
{
  uint32_t size = 0;

  pmem_pool_t *pPool = st_os_mem_pool_from_ptr(pMem);
  if (pPool)
  {
    size = pmem_get_size(pPool, pMem);
  }
  return size;
}



__WEAK pmem_pool_t *st_os_mem_pool_from_ptr(void *pMem)
{
  return NULL;
}



pmem_pool_t *st_os_mem_pool_from_type(st_mem_type type)
{
  pmem_pool_t *pPool = NULL;

  int32_t index = st_os_find_pool(type);
  if (index != -1)
  {
    pPool = st_os_mem_pool_index(index);
    if (st_os_mem_pool_is_valid(pPool))
    {
      return pPool;
    }
  }

  return NULL;
}



/**
 * @brief pmem_reset the pool to empty , all blocks will be lost
 *
 * @param type the pool type
 * @return __weak st_os_mem_reset
 */
__WEAK void st_os_mem_reset(st_mem_type type)
{
  pmem_pool_t *pPool = NULL;
  if (!ST_Mem_is_any(type))
  {
    st_os_mem_init();
    int32_t index = st_os_find_pool(type);
    if (index != -1)
    {
      pPool = st_os_mem_pool_index(index);
      if (st_os_mem_pool_is_valid(pPool))
      {
        pmem_reset(pPool);
      }
    }
  }
}

#endif

static void *s_mem_direct_pool(st_mem_type type, alloc_api allocApi, size_t size, size_t elem, char *pString, uint32_t line)
{
  void *ptr = NULL;
  #ifdef ST_USE_PMEM
  int32_t index = st_os_find_pool(type);
  if (index != -1)
  {
    pmem_pool_t *pPool = st_os_mem_pool_index(index);
    if (st_os_mem_pool_is_valid(pPool))
    {
      /* try on the explicit pool */
      st_os_mem_lock();
      if (allocApi == CALLOC_API)
      {
        ptr  = pmem_calloc_named(pPool, size, elem, pString, line); /*cstat !MISRAC2012-Rule-22.1_a pPtr wil be freed later */
      }
      if (allocApi == ALLOC_API)
      {
        ptr  = pmem_alloc_named(pPool, size, pString, line);        /*cstat !MISRAC2012-Rule-22.1_a pPtr wil be freed later */
      }
      st_os_mem_unlock();
    }
  }
  #endif
  return ptr;
}



/**
 * @brief Alloc or calloc from the fastest pool to the slowest
 *
 * @param allocApi alloc/calloc
 * @param size the block size
 * @param elem the elem size
 * @param pString the string for trace
 * @param line  the line for trace
 * @return
 */
static void *s_scan_for_fast(alloc_api allocApi, size_t size, size_t elem, char *pString, uint32_t line)
{
  void *pPtr = NULL;
  for (int32_t iPool = 0; iPool < st_os_mem_any_max(); iPool++)
  {
    pmem_pool_t *pPool = st_os_mem_pool_index(iPool);
    if (pPool)
    {
      if (st_os_mem_pool_is_valid(pPool))
      {
        st_mem_type ptype = (st_mem_type)pPool->m_alias;
        /* Alloc in a pool  */
        pPtr  = s_mem_direct_pool(ptype, allocApi, size, elem, pString, line);      /*cstat !MISRAC2012-Rule-22.1_a pPtr wil be freed later */
        if (pPtr != NULL)
        {
          /* if the block is ok*/
          return pPtr;
        }
      }
    }
  }
  return pPtr;
}


/**
 * @brief Alloc or calloc from the fastest pool to the slowest
 *
 * @param allocApi alloc/calloc
 * @param size the block size
 * @param elem the elem size
 * @param pString the string for trace
 * @param line  the line for trace
 * @return
 */
static void *s_scan_for_slow(alloc_api allocApi, size_t size, size_t elem, char *pString, uint32_t line)
{
  void *pPtr = NULL;

  for (int32_t iPool = st_os_mem_any_max() - 1; iPool >= 0; iPool--)
  {
    pmem_pool_t *pPool = st_os_mem_pool_index(iPool);
    if (pPool)
    {
      if (st_os_mem_pool_is_valid(pPool))
      {
        st_mem_type ptype = (st_mem_type)pPool->m_alias;
        /* Alloc in a pool  */
        pPtr  = s_mem_direct_pool(ptype, allocApi, size, elem, pString, line);      /*cstat !MISRAC2012-Rule-22.1_a pPtr wil be freed later */
        if (pPtr != NULL)
        {
          /* if the block is ok*/
          return pPtr;
        }
      }
    }
  }

  return pPtr;
}





/**
 * @brief Alloc a memory according to the desired type using traces
 *
 * @param type mem type
 * @param size the block size
 * @param pString the string for trace
 * @param line  the line for trace
 * @return
 */
__WEAK void *st_os_mem_alloc_named(st_mem_type type, size_t size, char *pString, uint32_t line)
{
  void *p = NULL;
  st_os_mem_init();

  if (type >= ST_Mem_Type_UNMAPPED)
  {
    return p;
  }

  /* Check if Best, ie try on a series of pools */
  if (type == ST_Mem_Type_ANY_FAST)
  {
    p = s_scan_for_fast(ALLOC_API, size, 1, pString, line);
    if (p)
    {
      return p;
    }
    /* if not found, default is heap */
    return st_os_mem_alloc_named(ST_Mem_Type_HEAP, size, pString, line);
  }
  if (type == ST_Mem_Type_ANY_SLOW)
  {
    p = s_scan_for_slow(ALLOC_API, size, 1, pString, line);
    if (p)
    {
      return p;
    }
    /* if not found, default is heap */
    return st_os_mem_alloc_named(ST_Mem_Type_HEAP, size, pString, line);
  }

  #ifdef ST_USE_PMEM
  p = s_mem_direct_pool(type, ALLOC_API, size, 1, pString, line);
  if (p)
  {
    return p;
  }
  #endif
  #if POOL_HEAP_LIMIT != 0
  /* We check the remaining memory to decide if we use the heap for livetune_db_find_instance_index_from_name or if we leave enough space for third parties APIs such as opus,vla,etc...
  ST_Result can switch on an other pool in case if the HEAP pool is full */

  st_os_mem_status *pSysInfo;
  if (st_os_mem_section_info(&pSysInfo))
  {
    int32_t rsize = (int32_t)pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolFreeSize - (int32_t)size;
    if (rsize < (int32_t)POOL_HEAP_LIMIT)
    {
      return NULL;
    }
  }
  else
  {
    ST_TRACE_OS_ERROR("Error, no heap guard limits API, cannot get Heap allocation info ");
  }

  #endif

  /* Fallback is heap */
  st_os_mem_lock();
  p = malloc(size);
  st_os_mem_unlock();
  return p;
}

/**
 * @brief Calloc a memory according to the desired type using traces
 *
 * @param type mem type
 * @param size the block size
 * @param pString the string for trace
 * @param line  the line for trace
 * @return
 */

__WEAK void *st_os_mem_calloc_named(st_mem_type type, size_t size, size_t elem, char *pString, uint32_t line)
{
  void *p = NULL;
  st_os_mem_init();

  if (type >= ST_Mem_Type_UNMAPPED)
  {
    return p;
  }

  /* Check if Best, ie try on a series of pools */
  if (type == ST_Mem_Type_ANY_FAST)
  {
    p = s_scan_for_fast(CALLOC_API, size, elem, pString, line);
    if (p)
    {
      return p;
    }
    /* if not found, default is heap */
    return st_os_mem_calloc_named(ST_Mem_Type_HEAP, size, elem, pString, line);
  }

  if (type == ST_Mem_Type_ANY_SLOW)
  {
    p = s_scan_for_slow(CALLOC_API, size, 1, pString, line);
    if (p)
    {
      return p;
    }
    /* if not found, default is heap */
    return st_os_mem_alloc_named(ST_Mem_Type_HEAP, size, pString, line);
  }

  #ifdef ST_USE_PMEM
  p = s_mem_direct_pool(type, CALLOC_API, size, 1, pString, line);
  if (p)
  {
    return p;
  }
  #endif

  #if POOL_HEAP_LIMIT != 0
  /* We check the remaining memory to decide if we use the heap for livetune_db_find_instance_index_from_name or if we leave enough space for third parties APIs such as opus,vla,etc...
  ST_Result can switch on an other pool in case if the HEAP pool is full */

  st_os_mem_status *pSysInfo;
  if (st_os_mem_section_info(&pSysInfo))
  {
    uint32_t rsize = pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolFreeSize - (size * elem);
    if (rsize < (uint32_t)POOL_HEAP_LIMIT)
    {
      return NULL;
    }
  }
  else
  {
    ST_TRACE_OS_ERROR("Error, no heap garde limits API,cannot get Heap allocation info ");
  }
  #endif
  /* Fallback is heap */
  st_os_mem_lock();
  p = calloc(size, elem);
  st_os_mem_unlock();
  return p;
}

/**
 * @brief realloc  a memory according to the desired type using traces
 *
 * @param pBuffer base pointer
 * @param size  size
 * @param pString debug string
 * @param line   line debug
 * @return
 */

__WEAK void *st_os_mem_realloc_named(st_mem_type type, void *pBuffer, size_t size, char *pString, uint32_t line)
{
  void *p = NULL;
  st_os_mem_init();

  if (pBuffer == NULL)
  {
    /* always preferred best in case if NULL (ie malloc) */
    return st_os_mem_alloc_named(type, size, pString, line);
  }

  #ifdef ST_USE_PMEM
  pmem_pool_t *pPool = st_os_mem_pool_from_ptr(pBuffer); /* In case the ptr is not 0, we realloc from the right pool */
  if (pPool)
  {
    /* try to re-allco in the same pool */
    uint32_t prevsize = pmem_get_size(pPool, pBuffer);
    st_os_mem_lock();
    p = pmem_realloc_named(pPool, pBuffer, size, pString, line);
    st_os_mem_unlock();
    if (p == NULL)
    {
      /* We can't realloc in the same pool */
      /* let's try in another pool before to give up */
      p = st_os_mem_alloc_named(type, size, pString, line);
      if (p)
      {
        uint32_t sz = ST_MIN(prevsize, size);
        memcpy(p, pBuffer, sz);
        st_os_mem_free(pBuffer);
      }
    }
    /* notice pBuffer is not freed */
    return p;
  }
  #endif
  /* Fall-back is the heap */
  st_os_mem_lock();
  p = realloc(pBuffer, size);
  st_os_mem_unlock();
  return p;
}


/**
 * @brief Free a memory block
 *
 * @param pMemToFree the pointer to free
 * @return __weak st_os_mem_free
 */
__WEAK void st_os_mem_free(void *pMemToFree)
{
  st_os_mem_init();

  /* TLS seems to free NULL pointer */
  if (pMemToFree == NULL)
  {
    return;
  }
  #ifdef ST_USE_PMEM
  pmem_pool_t *pPool = st_os_mem_pool_from_ptr(pMemToFree);
  if (pPool != NULL)
  {
    st_os_mem_lock();
    pmem_free(pPool, pMemToFree);
    st_os_mem_unlock();
    return;
  }
  #endif
  /* Fall back is the heap */

  st_os_mem_lock();
  free(pMemToFree);
  st_os_mem_unlock();
}


/**
 * @brief   These allocators are used by custom configuration allocator
             in order to be more flexible  when the HEAP is full
                replace malloc/free but allocate an all pools
 *
 * @param ptr the ptr
 */
void st_os_mem_generic_free(void *ptr)
{
  st_os_mem_free(ptr);
}
/**
 * @brief   These allocators are used by custom configuration allocator
             in order to be more flexible  when the HEAP is full
                replace malloc/free but allocate an all pools
 *
 * @param ptr the ptr
 */
void *st_os_mem_generic_malloc(size_t size)
{
  return st_os_mem_alloc(ST_Mem_Type_ANY_FAST, size);
}
void *st_os_mem_generic_malloc_slow(size_t size)
{
  return st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, size);
}

/**
 * @brief   These allocators are used by custom configuration allocator
             in order to be more flexible  when the HEAP is full
                replace malloc/free but allocate an all pools
 *
 * @param ptr the ptr
 */
static void *s_generic_realloc(st_mem_type type, void *pMemory, size_t size)
{
  void *pMem = st_os_mem_realloc(type, pMemory, size);
  if (pMem == NULL)
  {
    if (pMemory)
    {
      st_os_mem_free(pMemory); /*cstat !MISRAC2012-Dir-4.13_e !MISRAC2012-Rule-1.3_p !MISRAC2012-Rule-22.2_b  false positive, the pointer pMemory is freed because realloc st_os_mem_realloc, hence, we need to free it to prevent leak before realloc the pointer */
      pMemory = NULL;
    }
    /* allow the Pool change */
    pMem = st_os_mem_alloc(type, size);
  }
  return pMem;
}

void *st_os_mem_generic_realloc_fast(void *pMemory, size_t size)
{
  return s_generic_realloc(ST_Mem_Type_ANY_FAST, pMemory, size);
}

void *st_os_mem_generic_realloc_slow(void *pMemory, size_t size)
{
  return s_generic_realloc(ST_Mem_Type_ANY_SLOW, pMemory, size);
}



/**
 * @brief update  malloced   string
 *
 * @param pString the string
 * @param pNewString the new string
 */
void st_os_mem_update_string(char **pString, const char *pNewString)
{
  if (pNewString)
  {
    char  *pOld      = *pString;
    size_t szRealloc = strlen(pNewString) + 1U;

    (*pString) = st_os_mem_realloc(ST_Mem_Type_ANY_SLOW, pOld, szRealloc);
    strcpy(*pString, pNewString);
  }
  else
  {
    if ((*pString))
    {
      st_os_mem_free((*pString));
    }
    (*pString) = NULL;
  }
}


/**
 * @brief Allow to check a link
 *
 * @param type mem type
 * @param flag  0 set all blocks to a tag 1 check if not tagged block ar present ( if not tagged are present, there is a memory leak)
 * @param tag    any value but not 0
 * @return true or false
 */
__WEAK bool st_os_mem_leak_detector(st_mem_type type, uint32_t flag, uint32_t tag)
{
  bool error = false;
  #ifdef ST_USE_PMEM
  if (type != ST_Mem_Type_HEAP)
  {
    if (ST_Mem_is_any(type))
    {
      for (int32_t indexPool = 0; indexPool < st_os_mem_any_max(); indexPool++)
      {
        pmem_pool_t *pPool = st_os_mem_pool_index(indexPool);
        if (st_os_mem_pool_is_valid(pPool))
        {
          pmem_leak_detector(pPool, flag, tag);
        }
      }
    }
    error = true;
  }
  else
  {
    int32_t index = st_os_find_pool(ST_Mem_Type_HEAP);
    if (index != -1)
    {
      pmem_pool_t *pPool = st_os_mem_pool_index(index);
      if (pPool != NULL)
      {
        pmem_leak_detector(pPool, flag, tag);
        error = true;
      }
    }
    else
    {
      error = true;
    }
  }
  #endif

  return error;
}



/**
 * @brief Check the pointer validity
 *
 * @param type mem type
 * @param pPtr a valid pointer
 * @return true or false
 */
bool st_os_mem_check_ptr(st_mem_type type, void *pPtr)
{
  bool ret = false;

  /* In case no pool is created we still want to check that the pointer
  * allocated thru stlib is valid */
  if (st_os_mem_pool_max() == 0)
  {
    type = ST_Mem_Type_HEAP;
  }

  switch (type)
  {
    case ST_Mem_Type_HEAP:
      /* if HEAP direct, call to  the compiler support */
      ret = st_os_mem_check_heap_ptr(pPtr);
      break;
    case ST_Mem_Type_ANY_FAST:
    case ST_Mem_Type_ANY_SLOW:
      /* if any, we scan all pools */
      for (int32_t indexPool = 0; indexPool < st_os_mem_pool_max(); indexPool++)
      {
        pmem_pool_t *pPool = st_os_mem_pool_index(indexPool);
        if (st_os_mem_pool_is_valid(pPool))
        {
          #ifdef ST_USE_PMEM
          /* if not the HEAP call pmem */
          if (pPool != NULL)
          {
            ret = pmem_check_pool_ptr(pPool, pPtr);
            if (ret)
            {
              /* found */
              break;
            }
          }
          #endif
        }
        else
        {
          /* if HEAP direct, call to  the compiler support */
          ret = st_os_mem_check_heap_ptr(pPtr);
          if (ret)
          {
            /* found */
            break;
          }
        }
      }
      break;

    case ST_Mem_Type_MAX:
    case ST_Mem_Type_UNMAPPED:
      break;
    default: /* ST_Mem_Type_POOL1  to max number = ST_Mem_Type_HEAP-1 */
    {
      #ifdef ST_USE_PMEM
      /* otherwise it is is a pmem */
      int32_t index = st_os_find_pool(type);
      if (index != -1)
      {
        pmem_pool_t *pPool = st_os_mem_pool_index(index);
        if (st_os_mem_pool_is_valid(pPool))
        {
          ret = pmem_check_pool_ptr(pPool, pPtr);
        }
      }
      #endif
      break;
    }
  }

  return ret;
}




/**
 * @brief return the memory info
 *
 * @param pSysInfo the sysinfo instance
 * @return int
 */
void st_os_mem_info(st_os_mem_status **ppSysInfo)
{
  st_os_mem_section_info(ppSysInfo);
  st_os_mem_status *pSysInfo = *ppSysInfo;

  for (uint32_t indexMem = 0; indexMem < (uint32_t)ST_Mem_Type_MAX; indexMem++)
  {
    if (indexMem != (uint32_t)ST_Mem_Type_HEAP)
    {
      pSysInfo->tPools[indexMem].iFlags = st_os_mem_hidden;
    }
  }

  #ifdef ST_USE_PMEM
  for (int32_t indexMem = 0; indexMem < st_os_mem_pool_max(); indexMem++)
  {
    pmem_pool_t *pPool = st_os_mem_pool_index(indexMem);
    if ((pPool != NULL) && (pPool->m_pBaseMalloc != NULL))
    {
      pSysInfo->tPools[indexMem].iPoolFreeSize = (pPool->m_iBaseSize - (uint32_t)pPool->m_globalAlloc);
      pSysInfo->tPools[indexMem].iPoolSize     = pPool->m_iBaseSize;
      pSysInfo->tPools[indexMem].iPoolMaxAlloc = (uint32_t)pPool->m_maxAlloc;
      pSysInfo->tPools[indexMem].pName         = pPool->m_pName;
      pSysInfo->tPools[indexMem].iPoolFrag     = pPool->m_nbFrags;
      pSysInfo->tPools[indexMem].iPerf         = pPool->m_perfIndex;

      pSysInfo->tPools[indexMem].iFlags        = ((pPool->m_flags & PMEM_FLAG_DISABLED) != 0UL) ? st_os_mem_unmapped : 0UL;
      pSysInfo->tPools[indexMem].iPoolOffset   = (uint32_t)pPool->m_pBaseMalloc;  /*cstat !MISRAC2012-Rule-11.6 can't solve issue without changing iPoolOffset type to pointer*/
    }
  }
  #endif

  #ifdef USE_THREADX
  extern TX_BYTE_POOL HeapBytePool;
  extern TX_BYTE_POOL StackBytePool;

  pSysInfo->tStaticPools[0].pName         = "TX Heap"; //HeapBytePool.tx_byte_pool_name;
  pSysInfo->tStaticPools[0].iPoolFreeSize = HeapBytePool.tx_byte_pool_available;
  pSysInfo->tStaticPools[0].iPoolSize     = HeapBytePool.tx_byte_pool_size;
  pSysInfo->tStaticPools[0].iPoolFrag     = HeapBytePool.tx_byte_pool_fragments;
  pSysInfo->tStaticPools[0].iPoolFrag     = HeapBytePool.tx_byte_pool_fragments;

  pSysInfo->tStaticPools[1].pName         = "TX Stack"; //StackBytePool.tx_byte_pool_name;
  pSysInfo->tStaticPools[1].iPoolFreeSize = StackBytePool.tx_byte_pool_available;
  pSysInfo->tStaticPools[1].iPoolSize     = StackBytePool.tx_byte_pool_size;
  pSysInfo->tStaticPools[1].iPoolFrag     = StackBytePool.tx_byte_pool_fragments;
  pSysInfo->tStaticPools[1].iPoolFrag     = StackBytePool.tx_byte_pool_fragments;
  #endif
}

