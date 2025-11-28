/**
******************************************************************************
* @file    audio_mem_mgnt.c
* @author  MCD Application Team
* @brief   audio chain os abstraction layer for bare metal implementation.
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
#include <stdlib.h>     /* malloc, free, rand */
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include "audio_mem_mgnt.h"
#include "irq_utils.h"

#ifdef AUDIO_MEM_CONF_STOS_USED
#include "st_os_mem.h"
#include "stm32xxx_voice_bsp.h" /* Needed for the define identifying the family
*                                  such as STM32N6, STM32H7, etc... 
*                                  wiz cubeMx this define is removed from pre 
*                                  processor define list thus we need to get it 
*                                  from stm32n6xx.h, stm32h7xx.h, etc... 
*                                  In our audio projects, these includes are 
*                                  handled by stm32xxx_voice_bsp.h.
*/
#else

#if defined (VALIDATION_X86)
static inline bool st_os_mem_check_heap_ptr(void *pHeap)
{
  (void)pHeap;
  return true;
}

#elif defined ( __GNUC__  )
static inline bool st_os_mem_check_heap_ptr(void *pHeap)
{
  extern char _sheap;
  extern char _eheap;
  return ((pHeap >= (void *)&_sheap) && (pHeap <= (void *)&_eheap));
}

#elif defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma section = "HEAP"
static inline bool st_os_mem_check_heap_ptr(void *pHeap)
{
  void *heap_start = __section_begin("HEAP");
  void *heap_limit = __section_end("HEAP");
  return ((pHeap >= heap_start) && (pHeap <= heap_limit));
}

#else
static inline bool st_os_mem_check_heap_ptr(void *pHeap)
{
  return true;
}

#endif /* __GNUC__ vs __ICCARM__ vs VALIDATION_X86*/
#endif /* AUDIO_MEM_CONF_STOS_USED */

#include "traces.h"

#ifdef AUDIO_MEM_CONF_TRACK_MALLOC
  #undef AudioMalloc
  #undef AudioCalloc
  #undef AudioRealloc
  #undef AudioFree
  void *AudioMalloc(size_t const size, memPool_t const memPool);
  void *AudioCalloc(size_t const nbElements, size_t const elementSize, memPool_t const memPool);
  void *AudioRealloc(void *const ptr, size_t const size, memPool_t const memPool);
  void  AudioFree(void *const pMemToFree, memPool_t const memPool);
#endif

#define AUDIO_MEM_DISABLE_IRQ disable_irq_with_cnt
#define AUDIO_MEM_ENABLE_IRQ  enable_irq_with_cnt

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
#ifdef AUDIO_MEM_CONF_TRACK_MALLOC
typedef struct
{
  memPool_t     memPool;
  uint64_t      cumulatedAllocSize;
  size_t        maxTotalAllocSize;
  size_t        totalAllocSize;
  int           nbMalloc;
  int           maxNbMalloc;
} memPoolStat_t;
#endif

typedef struct allocBuffer
{
  #ifdef AUDIO_MEM_CONF_TRACK_MALLOC
  uint32_t            header;
  int                 id;
  char         const *file;
  int                 line;
  struct allocBuffer *pNext;
  struct allocBuffer *pPrev;
  uint32_t            separator;
  memAllocInfo_t      allocInfo;
  // useful data is here just after allocInfo (size bytes)
  uint32_t            dummyFooter;  // not at the right place: added just for allocation size purpose (dummyFooter field is never used)
  #else
  memAllocInfo_t allocInfo;
  // useful data is here just after allocInfo (size bytes)
  #endif
} allocBuffer_t;

#ifdef AUDIO_MEM_CONF_STOS_USED
typedef struct
{
  char const *poolName;
  st_mem_type poolType;
  memPool_t   memPool;
} poolWrapping_t;
#endif

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#ifdef AUDIO_MEM_CONF_ASSERT_ENABLED
  #define MEM_ASSERT(cond, ...) s_assert((cond), #cond, __FILE__, __LINE__, __VA_ARGS__)
#else
  #define MEM_ASSERT(cond, ...) do { int a = (cond); (void)a; } while(0)    // cond is executed but no assert is issued
#endif

/* Private variables ---------------------------------------------------------*/
#ifdef AUDIO_MEM_CONF_TRACK_MALLOC
  static          allocBuffer_t *s_pAllocRoot            = NULL;
  static          allocBuffer_t *s_pAllocLast            = NULL;
  static          uint64_t       s_cumulatedAllocSize    = 0ULL;
  static          size_t         s_maxTotalAllocSize     = 0UL;
  static          size_t         s_totalAllocSize        = 0UL;
  static          int            s_nbMalloc              = 0;
  static          int            s_maxNbMalloc           = 0;
  static          int            s_mallocId              = 0;
  static volatile bool           s_doNotCheckConsistency = false;
  static const    uint32_t       s_allocHeader           = 0x3EA92C10UL;
  static const    uint32_t       s_allocSeparator        = 0x9C03F857UL;
  static const    uint32_t       s_allocFooter           = 0xB470AD26UL;
  static memPoolStat_t           s_memPoolStat[AUDIO_MEM_NB_POOL];
#endif

/* Private function prototypes -----------------------------------------------*/
#ifdef AUDIO_MEM_CONF_STOS_USED
  static poolWrapping_t *s_getPoolWrapping(void);
  static st_mem_type     s_getNextPoolType(poolWrapping_t **const ppPoolWrapping, memPool_t const memPool);
  static memPool_t       s_getMemPool(allocBuffer_t *const pAllocBuffer);
#endif
#ifdef AUDIO_MEM_CONF_TRACK_MALLOC
  static void s_checkResetMemPoolStat(void);
  static bool s_displayMallocDescr(allocBuffer_t const *const pAllocBuffer);
  static void s_trace(traceLvl_t const level, char const *const pFormat, ...);
#endif
#ifdef AUDIO_MEM_CONF_ASSERT_ENABLED
  static void s_assert(bool const cond, char const *const condString, char const *const file, int const line, char const *const pFormat, ...);
#endif

/* Functions Definition ------------------------------------------------------*/

#ifdef AUDIO_MEM_CONF_STOS_USED

static poolWrapping_t *s_getPoolWrapping(void)
{
  static poolWrapping_t poolWrapping[] =
  {
    {"DTCM",    ST_Mem_Type_UNMAPPED, AUDIO_MEM_TCM},
    {"ITCM",    ST_Mem_Type_UNMAPPED, AUDIO_MEM_TCM},
    {"RAMINT",  ST_Mem_Type_UNMAPPED, AUDIO_MEM_RAMINT},
    {"RAMINT1", ST_Mem_Type_UNMAPPED, AUDIO_MEM_RAMINT},
    {"RAMINT2", ST_Mem_Type_UNMAPPED, AUDIO_MEM_RAMINT},
    {"RAMINT3", ST_Mem_Type_UNMAPPED, AUDIO_MEM_RAMINT},
    {"RAMEXT",  ST_Mem_Type_UNMAPPED, AUDIO_MEM_RAMEXT},
    #ifdef AUDIO_MEM_NOCACHED_ENABLED
    {"DMA",     ST_Mem_Type_UNMAPPED, AUDIO_MEM_NOCACHED},
    #endif
    {"HEAP",    ST_Mem_Type_UNMAPPED, AUDIO_MEM_RAMINT},
    {NULL,      ST_Mem_Type_UNMAPPED, AUDIO_MEM_UNKNOWN}
  };
  static bool firstTime = true;

  if (firstTime)
  {
    for (poolWrapping_t *pPoolWrapping = poolWrapping; pPoolWrapping->poolName != NULL; pPoolWrapping++)
    {
      st_mem_type poolType = st_os_mem_type_from_name((char *)pPoolWrapping->poolName);

      if ((poolType != ST_Mem_Type_HEAP) && (poolType != ST_Mem_Type_UNMAPPED))
      {
        // ST_Mem_Type_HEAP type is OK whatever its validity (enabled as a STPmem pool or not) => keep its type in any case
        // else if type is mapped, check if it is valid (enabled as a STPmem pool) and if not, consider it as unmapped for AudioMalloc
        int32_t index = st_os_find_pool(poolType);

        if (index >= 0)
        {
          if (!st_os_mem_pool_is_valid(st_os_mem_pool_index(index)))
          {
            poolType = ST_Mem_Type_UNMAPPED;
          }
        }
        else
        {
          poolType = ST_Mem_Type_UNMAPPED;
        }
      }
      pPoolWrapping->poolType = poolType;
    }
    firstTime = false;
  }

  return poolWrapping;
}


static st_mem_type s_getNextPoolType(poolWrapping_t **const ppPoolWrapping, memPool_t const memPool)
{
  st_mem_type     poolType = ST_Mem_Type_UNMAPPED;
  poolWrapping_t *pPoolWrapping;

  for (pPoolWrapping = (*ppPoolWrapping == NULL) ? s_getPoolWrapping() : *ppPoolWrapping;
       pPoolWrapping->poolName != NULL;
       pPoolWrapping++)
  {
    if ((pPoolWrapping->memPool == memPool) && (pPoolWrapping->poolType != ST_Mem_Type_UNMAPPED))
    {
      poolType = pPoolWrapping->poolType;
      pPoolWrapping++;
      break;
    }
  }
  *ppPoolWrapping = pPoolWrapping;

  return poolType;
}


static memPool_t s_getMemPool(allocBuffer_t *const pAllocBuffer)
{
  memPool_t allocatedMemPool = AUDIO_MEM_UNKNOWN;

  for (poolWrapping_t *pPoolWrapping = s_getPoolWrapping(); pPoolWrapping->poolName != NULL; pPoolWrapping++)
  {
    if (st_os_mem_check_ptr(pPoolWrapping->poolType, pAllocBuffer) != 0UL)
    {
      allocatedMemPool = pPoolWrapping->memPool;
      break;
    }
  }

  return allocatedMemPool;
}

#endif // AUDIO_MEM_CONF_STOS_USED


bool AudioMallocCheckPtr(void *ptr, int const type)
{
  bool ret = false;

  if (ptr != NULL)
  {
    char *pAlloc = NULL;

    pAlloc  = (char *)ptr;
    pAlloc -= offsetof(allocBuffer_t, allocInfo) + sizeof(memAllocInfo_t);

    #ifdef AUDIO_MEM_CONF_STOS_USED
    ret = st_os_mem_check_ptr((st_mem_type)type, pAlloc);
    #else  // AUDIO_MEM_CONF_STOS_USED
    (void)type;
    ret = st_os_mem_check_heap_ptr(pAlloc);
    #endif // AUDIO_MEM_CONF_STOS_USED
  }

  return ret;
}


void *AudioMalloc(size_t const size, memPool_t const memPool)
{
  memPool_t     allocatedMemPool = AUDIO_MEM_UNKNOWN;
  size_t const  allocSize        = sizeof(allocBuffer_t) + AUDIO_MEM_SIZE_ALIGN(size);
  allocBuffer_t *pAllocBuffer    = NULL;
  void          *ptr             = NULL;

  #ifdef AUDIO_MEM_CONF_STOS_USED

  /* We privilege a memory placement at the right place according to the memory speed requested */
  poolWrapping_t *pPoolWrapping = NULL;

  while (pAllocBuffer == NULL)
  {
    st_mem_type poolType = s_getNextPoolType(&pPoolWrapping, memPool);

    if (poolType == ST_Mem_Type_UNMAPPED)
    {
      /* poolType not found */
      break;
    }
    else
    {
      /* poolType found */
      pAllocBuffer = (allocBuffer_t *)st_os_mem_alloc(poolType, allocSize);
      if (pAllocBuffer != NULL)
      {
        /* enough space in this poolType */
        break;
      }
    }
  }

  if (pAllocBuffer == NULL)
  {
    /* If free space not found in the requested pool type, we try ST_Mem_Type_ANY_SLOW if AUDIO_MEM_RAMEXT requested or ST_Mem_Type_ANY_FAST in any other case */
    switch (memPool)
    {
      case AUDIO_MEM_RAMEXT:
        pAllocBuffer = (allocBuffer_t *)st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, allocSize);
        break;

      case AUDIO_MEM_RAMINT:
      case AUDIO_MEM_TCM:
        pAllocBuffer = (allocBuffer_t *)st_os_mem_alloc(ST_Mem_Type_ANY_FAST, allocSize);
        break;

      default:
        break;
    }
  }

  if (pAllocBuffer != NULL)
  {
    // memory is allocated and allocatedMemPool is still unknown => search where it has been allocated
    allocatedMemPool = s_getMemPool(pAllocBuffer);
  }

  #else // AUDIO_MEM_CONF_STOS_USED

  pAllocBuffer     = (allocBuffer_t *)malloc(allocSize);  /*cstat !MISRAC2012-Dir-4.13_b pAllocBuffer will be freed by application*/
  allocatedMemPool = AUDIO_MEM_RAMINT;

  #endif // AUDIO_MEM_CONF_STOS_USED

  if (pAllocBuffer != NULL)
  {
    memAllocInfo_t *const pAllocInfo = &pAllocBuffer->allocInfo;

    pAllocInfo->requestedMemoryPool = (uint32_t)memPool;
    pAllocInfo->allocatedMemoryPool = (uint32_t)allocatedMemPool;
    pAllocInfo->size                = size;
    ptr                             = &pAllocInfo[1]; // useful data is just after allocInfo
  }

  return ptr;
}


void *AudioCalloc(size_t const nbElements, size_t const elementSize, memPool_t const memPool)
{
  size_t const size = nbElements * elementSize;
  void  *const ptr  = AudioMalloc(size, memPool);

  if (ptr != NULL)
  {
    memset(ptr, 0, size);
  }

  return ptr;
}


void *AudioRealloc(void *const ptr, size_t const size, memPool_t const memPool)
{
  void *newPtr = NULL;

  if (ptr == NULL)
  {
    newPtr = AudioMalloc(size, memPool);
  }
  else if (size > 0UL)
  {
    size_t const   allocSize       = sizeof(allocBuffer_t) + AUDIO_MEM_SIZE_ALIGN(size);
    char          *pChar           = (char *)ptr;
    allocBuffer_t *pOldAllocBuffer = NULL;
    allocBuffer_t *pNewAllocBuffer = NULL;

    pChar          -= offsetof(allocBuffer_t, allocInfo) + sizeof(memAllocInfo_t);
    pOldAllocBuffer = (allocBuffer_t *)pChar;

    #ifdef AUDIO_MEM_CONF_STOS_USED
    pNewAllocBuffer = (allocBuffer_t *)st_os_mem_realloc(ST_Mem_Type_ANY_FAST, pOldAllocBuffer, allocSize);
    #else // AUDIO_MEM_CONF_STOS_USED
    pNewAllocBuffer = (allocBuffer_t *)realloc(pOldAllocBuffer, allocSize); /*cstat !MISRAC2012-Dir-4.13_b pNewAllocBuffer will be freed by application*/
    #endif // AUDIO_MEM_CONF_STOS_USED

    if (pNewAllocBuffer != NULL)
    {
      memPool_t             allocatedMemPool = AUDIO_MEM_UNKNOWN;
      memAllocInfo_t *const pAllocInfo      = &pNewAllocBuffer->allocInfo;

      #ifdef AUDIO_MEM_CONF_STOS_USED
      // memory is allocated => search where it has been allocated
      allocatedMemPool = s_getMemPool(pNewAllocBuffer);
      #else // AUDIO_MEM_CONF_STOS_USED
      allocatedMemPool = memPool;
      #endif // AUDIO_MEM_CONF_STOS_USED

      pAllocInfo->requestedMemoryPool = (uint32_t)memPool;
      pAllocInfo->allocatedMemoryPool = (uint32_t)allocatedMemPool;
      pAllocInfo->size                = size;
      newPtr                          = &pAllocInfo[1];
    }
  }
  else
  {
    // size = 0
    AudioFree(ptr, memPool);
  }

  return newPtr;
}


void AudioFree(void *const pMemToFree, memPool_t const memPool)
{
  char *pAlloc = NULL;

  (void)memPool;

  if (pMemToFree != NULL)
  {
    pAlloc  = (char *)pMemToFree;
    pAlloc -= offsetof(allocBuffer_t, allocInfo) + sizeof(memAllocInfo_t);
  }

  #ifdef AUDIO_MEM_CONF_STOS_USED
  st_os_mem_free(pAlloc);
  #else // AUDIO_MEM_CONF_STOS_USED
  free(pAlloc); /*cstat !MISRAC2012-Dir-4.13_g pAllocBuffer has been allocated by application*/
  #endif // AUDIO_MEM_CONF_STOS_USED
}


void AudioMalloc_getInfo(void const *const ptr, memAllocInfo_t *const pMemAllocInfo)
{
  if ((ptr != NULL) || (pMemAllocInfo != NULL))
  {
    char           const *pChar = (const char *)ptr;
    memAllocInfo_t const *pAllocInfo;

    pChar         -= sizeof(memAllocInfo_t);  // allocInfo is just before data
    pAllocInfo     = (memAllocInfo_t *)pChar;
    *pMemAllocInfo = *pAllocInfo;
  }
}


#ifdef AUDIO_MEM_CONF_TRACK_MALLOC

void *AudioMallocDebug(size_t const size, memPool_t const memPool, char const *const file, int const line)
{
  void *pAlloc = NULL;

  MEM_ASSERT(((int)memPool < (int)AUDIO_MEM_NB_POOL), "memPool=%d, AUDIO_MEM_NB_POOL=%d", memPool, AUDIO_MEM_NB_POOL);

  s_checkResetMemPoolStat();

  if (size > 0UL)
  {
    pAlloc = AudioMalloc(size, memPool);
    if (pAlloc == NULL)
    {
      s_trace(TRACE_LVL_ERROR,
              "AudioMalloc(%d, %d) FAILED in %s, line %d",
              size,
              memPool,
              file,
              line);
    }
    else
    {
      char           *pChar;
      allocBuffer_t  *pAllocBuffer;
      memAllocInfo_t *pAllocInfo;
      uint32_t       *pAllocHeader, *pAllocSeparator, *pAllocFooter;

      pChar              = (char *)pAlloc;
      pChar             -= offsetof(allocBuffer_t, allocInfo) + sizeof(memAllocInfo_t);
      pAllocBuffer       = (allocBuffer_t *)pChar;
      pAllocInfo         = &pAllocBuffer->allocInfo;
      pChar              = (char *)pAlloc;
      pChar             += AUDIO_MEM_SIZE_ALIGN(size);
      pAllocHeader       = &pAllocBuffer->header;
      pAllocSeparator    = &pAllocBuffer->separator;
      pAllocFooter       = (uint32_t *)pChar;
      pAllocBuffer->id   = s_mallocId++;
      pAllocBuffer->file = file;
      pAllocBuffer->line = line;

      // copy header, separator and footer masks (to check memory corruption: see AudioMallocCheckConsistency())
      *pAllocHeader    = s_allocHeader;
      *pAllocSeparator = s_allocSeparator;
      *pAllocFooter    = s_allocFooter;

      AUDIO_MEM_DISABLE_IRQ();
      s_doNotCheckConsistency = true;
      pAllocBuffer->pPrev     = s_pAllocLast;
      pAllocBuffer->pNext     = NULL;
      if (s_pAllocRoot == NULL)
      {
        s_pAllocRoot        = pAllocBuffer;
      }
      else
      {
        s_pAllocLast->pNext = pAllocBuffer;
      }
      s_pAllocLast          = pAllocBuffer;

      // update global alloc statistics
      s_totalAllocSize     += size;
      s_cumulatedAllocSize += (uint64_t)size;
      s_nbMalloc++;
      s_maxTotalAllocSize   = (s_totalAllocSize > s_maxTotalAllocSize) ? s_totalAllocSize : s_maxTotalAllocSize;
      s_maxNbMalloc         = (s_nbMalloc       > s_maxNbMalloc)       ? s_nbMalloc       : s_maxNbMalloc;

      // update memPool alloc statistics
      s_memPoolStat[pAllocInfo->allocatedMemoryPool].cumulatedAllocSize += (uint64_t)size;
      s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize     += size;
      s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc++;
      s_memPoolStat[pAllocInfo->allocatedMemoryPool].maxTotalAllocSize   = (s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize > s_memPoolStat[pAllocInfo->allocatedMemoryPool].maxTotalAllocSize) ? s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize : s_memPoolStat[pAllocInfo->allocatedMemoryPool].maxTotalAllocSize;
      s_memPoolStat[pAllocInfo->allocatedMemoryPool].maxNbMalloc         = (s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc       > s_memPoolStat[pAllocInfo->allocatedMemoryPool].maxNbMalloc)       ? s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc       : s_memPoolStat[pAllocInfo->allocatedMemoryPool].maxNbMalloc;
      AUDIO_MEM_ENABLE_IRQ();

      #ifdef AUDIO_MEM_CONF_TRACK_MALLOC_VERBOSE2
      s_trace(TRACE_LVL_INFO,
              "malloc(%d)=0x%p, memPool=%d, id=%d, totalAllocSize=%d, nbMalloc=%d",
              size,
              pAlloc,
              pAllocInfo->allocatedMemoryPool,
              pAllocBuffer->id,
              s_totalAllocSize,
              s_nbMalloc);
      #endif
    }
  }

  return pAlloc;
}


void *AudioCallocDebug(size_t const nbElements, size_t const elementSize, memPool_t const memPool, char const *const file, int const line)
{
  size_t const size = nbElements * elementSize;
  void  *const ptr  = AudioMallocDebug(size, memPool, file, line);

  if (ptr != NULL)
  {
    memset(ptr, 0, size);
  }

  return ptr;
}


void *AudioReallocDebug(void *const ptr, size_t const size, memPool_t const memPool, char const *const file, int const line)
{
  void *pAlloc = NULL;

  MEM_ASSERT(((int)memPool < (int)AUDIO_MEM_NB_POOL), "memPool=%d, AUDIO_MEM_NB_POOL=%d", memPool, AUDIO_MEM_NB_POOL);

  s_checkResetMemPoolStat();

  if (ptr == NULL)
  {
    pAlloc = AudioMallocDebug(size, memPool, file, line);
  }
  else
  {
    char           *pChar;
    allocBuffer_t  *pOldAllocBuffer, *pNewAllocBuffer;
    memAllocInfo_t *pAllocInfo;
    uint32_t       *pAllocHeader, *pAllocSeparator, *pAllocFooter;
    size_t          oldSize, newSize;
    memPool_t       oldAllocMemPool, oldReqMemPool, newAllocMemPool;

    pChar           = (char *)ptr;
    pChar          -= offsetof(allocBuffer_t, allocInfo) + sizeof(memAllocInfo_t);
    pOldAllocBuffer = (allocBuffer_t *)pChar;
    pAllocInfo      = &pOldAllocBuffer->allocInfo;
    oldSize         = (size_t)pAllocInfo->size;
    oldReqMemPool   = (memPool_t)pAllocInfo->requestedMemoryPool;
    oldAllocMemPool = (memPool_t)pAllocInfo->allocatedMemoryPool;
    pChar           = (char *)ptr;
    pChar          += AUDIO_MEM_SIZE_ALIGN(oldSize);
    pAllocHeader    = &pOldAllocBuffer->header;
    pAllocSeparator = &pOldAllocBuffer->separator;
    pAllocFooter    = (uint32_t *)pChar;

    if ((*pAllocHeader    != s_allocHeader)    ||
        (*pAllocSeparator != s_allocSeparator) ||
        (*pAllocFooter    != s_allocFooter))
    {
      MEM_ASSERT(false, "AudioReallocDebug: old ptr head, separator or footer mask issue");
    }
    else
    {
      if (size > 0UL)
      {
        allocBuffer_t *const pOldAllocBufferPrev = pOldAllocBuffer->pPrev;  // copy pOldAllocBuffer->pPrev because it may be destroyed by AudioRealloc
        allocBuffer_t *const pOldAllocBufferNext = pOldAllocBuffer->pNext;  // copy pOldAllocBuffer->pNext because it may be destroyed by AudioRealloc
        void          *const newPtr              = AudioRealloc(ptr, size, memPool);

        if (newPtr == NULL)
        {
          s_trace(TRACE_LVL_ERROR, "AudioRealloc(0x%p, %d, %d) FAILED in %s, line %d", ptr, size, memPool, file, line); /*cstat !MISRAC2012-Dir-4.13_d !MISRAC2012-Dir-4.13_e !MISRAC2012-Rule-1.3_o !MISRAC2012-Rule-1.3_p ptr is only used for trace*/
        }
        else
        {
          pAlloc                = newPtr;
          pChar                 = (char *)newPtr;
          pChar                -= offsetof(allocBuffer_t, allocInfo) + sizeof(memAllocInfo_t);
          pNewAllocBuffer       = (allocBuffer_t *)pChar;
          pAllocHeader          = &pNewAllocBuffer->header;
          pAllocSeparator       = &pNewAllocBuffer->separator;
          pAllocInfo            = &pNewAllocBuffer->allocInfo;
          newSize               = (size_t)pAllocInfo->size;
          newAllocMemPool       = (memPool_t)pAllocInfo->allocatedMemoryPool;
          pChar                 = (char *)newPtr;
          pChar                += AUDIO_MEM_SIZE_ALIGN(size);
          pAllocFooter          = (uint32_t *)pChar;
          pNewAllocBuffer->id   = pOldAllocBuffer->id;
          pNewAllocBuffer->file = file;
          pNewAllocBuffer->line = line;

          // copy header, separator and footer masks (to check memory corruption: see AudioMallocCheckConsistency())
          *pAllocHeader    = s_allocHeader;
          *pAllocSeparator = s_allocSeparator;
          *pAllocFooter    = s_allocFooter;

          AUDIO_MEM_DISABLE_IRQ();
          s_doNotCheckConsistency = true;
          if (pOldAllocBufferPrev == NULL)
          {
            s_pAllocRoot           = pNewAllocBuffer;
            pNewAllocBuffer->pPrev = NULL;
            pNewAllocBuffer->pNext = pOldAllocBufferNext;
            if (pOldAllocBufferNext != NULL)
            {
              pOldAllocBufferNext->pPrev = pNewAllocBuffer;
            }
          }
          else if (pOldAllocBufferNext == NULL)
          {
            s_pAllocLast           = pNewAllocBuffer;
            pNewAllocBuffer->pPrev = pOldAllocBufferPrev;
            pNewAllocBuffer->pNext = NULL;
            if (pOldAllocBufferPrev != NULL)
            {
              pOldAllocBufferPrev->pNext = pNewAllocBuffer;
            }
          }
          else
          {
            pNewAllocBuffer->pPrev     = pOldAllocBufferPrev;
            pNewAllocBuffer->pNext     = pOldAllocBufferNext;
            pOldAllocBufferNext->pPrev = pNewAllocBuffer;
            pOldAllocBufferPrev->pNext = pNewAllocBuffer;
          }

          // update global alloc statistics
          s_totalAllocSize     -= oldSize;
          s_totalAllocSize     += newSize;
          s_cumulatedAllocSize -= (uint64_t)oldSize;
          s_cumulatedAllocSize += (uint64_t)newSize;
          s_maxTotalAllocSize   = (s_totalAllocSize > s_maxTotalAllocSize) ? s_totalAllocSize : s_maxTotalAllocSize;

          // update memPool alloc statistics
          s_memPoolStat[oldAllocMemPool].cumulatedAllocSize -= (uint64_t)oldSize;
          s_memPoolStat[newAllocMemPool].cumulatedAllocSize += (uint64_t)newSize;
          s_memPoolStat[oldAllocMemPool].totalAllocSize     -= oldSize;
          s_memPoolStat[newAllocMemPool].totalAllocSize     += newSize;
          s_memPoolStat[newAllocMemPool].maxTotalAllocSize   = (s_memPoolStat[newAllocMemPool].totalAllocSize > s_memPoolStat[newAllocMemPool].maxTotalAllocSize) ? s_memPoolStat[newAllocMemPool].totalAllocSize : s_memPoolStat[newAllocMemPool].maxTotalAllocSize;
          AUDIO_MEM_ENABLE_IRQ();

          #ifdef AUDIO_MEM_CONF_TRACK_MALLOC_VERBOSE2
          s_trace(TRACE_LVL_INFO,
                  "realloc(0x%p, %d)=0x%p, memPool=%d, id=%d, totalAllocSize=%d, nbMalloc=%d",
                  ptr,
                  size,
                  pAlloc,
                  memPool,
                  pNewAllocBuffer->id,
                  s_totalAllocSize,
                  s_nbMalloc);
          #endif
        }
      }
      else
      {
        // size = 0
        AudioFreeDebug(ptr, oldReqMemPool);
        pAlloc = NULL;
      }
    }
  }

  return pAlloc;
}


void AudioFreeDebug(void *const pMemToFree, memPool_t const memPool)
{
  MEM_ASSERT((memPool < AUDIO_MEM_NB_POOL), "memPool=%d, AUDIO_MEM_NB_POOL=%d", memPool, AUDIO_MEM_NB_POOL);

  s_checkResetMemPoolStat();

  if (pMemToFree != NULL)
  {
    char           *pChar;
    allocBuffer_t  *pAllocBuffer2Free, *pAllocBuffer;
    memAllocInfo_t *pAllocInfo;
    uint32_t       *pAllocHeader, *pAllocSeparator, *pAllocFooter;

    pChar             = (char *)pMemToFree;
    pChar            -= offsetof(allocBuffer_t, allocInfo) + sizeof(memAllocInfo_t);
    pAllocBuffer2Free = (allocBuffer_t *)pChar;
    pAllocInfo        = &pAllocBuffer2Free->allocInfo;
    pChar             = (char *)pMemToFree;
    pChar            += AUDIO_MEM_SIZE_ALIGN(pAllocInfo->size);
    pAllocHeader      = &pAllocBuffer2Free->header;
    pAllocSeparator   = &pAllocBuffer2Free->separator;
    pAllocFooter      = (uint32_t *)pChar;

    if ((*pAllocHeader    != s_allocHeader)    ||
        (*pAllocSeparator != s_allocSeparator) ||
        (*pAllocFooter    != s_allocFooter))
    {
      MEM_ASSERT(false, "AudioFreeDebug: ptr head, separator or footer mask issue");
    }
    else
    {
      AUDIO_MEM_DISABLE_IRQ();
      s_doNotCheckConsistency = true;
      for (pAllocBuffer = s_pAllocRoot; pAllocBuffer != NULL; pAllocBuffer = pAllocBuffer->pNext)
      {
        if (pAllocBuffer == pAllocBuffer2Free)
        {
          break;
        }
      }

      if (pAllocBuffer == NULL)
      {
        // pMemToFree has not been found in allocCheck list !
        MEM_ASSERT(false, "free(0x%p) not found", pMemToFree);
        AUDIO_MEM_ENABLE_IRQ();
      }
      else
      {
        // pMemToFree has been found in allocCheck list: remove it from the list and update global alloc statistics
        if (pAllocBuffer2Free->pPrev == NULL)
        {
          if (pAllocBuffer2Free->pNext == NULL)
          {
            s_pAllocRoot = NULL;
            s_pAllocLast = NULL;
          }
          else
          {
            s_pAllocRoot        = pAllocBuffer2Free->pNext;
            s_pAllocRoot->pPrev = NULL;
          }
        }
        else if (pAllocBuffer2Free->pNext == NULL)
        {
          s_pAllocLast        = pAllocBuffer2Free->pPrev;
          s_pAllocLast->pNext = NULL;
        }
        else
        {
          pAllocBuffer2Free->pPrev->pNext = pAllocBuffer2Free->pNext;
          pAllocBuffer2Free->pNext->pPrev = pAllocBuffer2Free->pPrev;
        }

        MEM_ASSERT(s_totalAllocSize >= pAllocInfo->size, "totalAllocSize=%d, pAllocInfo->size=%d", s_totalAllocSize, pAllocInfo->size);
        MEM_ASSERT(s_nbMalloc > 0, "s_nbMalloc=%d", s_nbMalloc);
        s_totalAllocSize -= pAllocInfo->size;
        s_nbMalloc--;
        if (s_nbMalloc == 0)
        {
          MEM_ASSERT(s_totalAllocSize == 0UL, "totalAllocSize=%d", s_totalAllocSize);
        }
        if (s_totalAllocSize == 0UL)
        {
          MEM_ASSERT(s_nbMalloc == 0, "nbMalloc=%d", s_nbMalloc);
        }

        // update memPool alloc statistics
        MEM_ASSERT(s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize >= pAllocInfo->size, "s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize=%d, pAllocInfo->size=%d", s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize, pAllocInfo->size);
        MEM_ASSERT(s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc > 0, "s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc=%d", s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc);
        s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize -= pAllocInfo->size;
        s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc--;
        if (s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc == 0)
        {
          MEM_ASSERT(s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize == 0UL, "s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize=%d", s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize);
        }
        if (s_memPoolStat[pAllocInfo->allocatedMemoryPool].totalAllocSize == 0UL)
        {
          MEM_ASSERT(s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc == 0, "s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc=%d", s_memPoolStat[pAllocInfo->allocatedMemoryPool].nbMalloc);
        }
        AUDIO_MEM_ENABLE_IRQ();

        if (memPool == (memPool_t)pAllocInfo->requestedMemoryPool)
        {
          #ifdef AUDIO_MEM_CONF_TRACK_MALLOC_VERBOSE2
          s_trace(TRACE_LVL_INFO,
                  "free(0x%p), memPool=%d, id=%d, size=%d (allocated in \"%s\" line %d), totalAllocSize=%d, nbMalloc=%d",
                  pMemToFree,
                  memPool,
                  pAllocBuffer2Free->id,
                  pAllocInfo->size,
                  pAllocBuffer2Free->file,
                  pAllocBuffer2Free->line,
                  s_totalAllocSize,
                  s_nbMalloc);
          #endif
        }
        else
        {
          s_trace(TRACE_LVL_WARNING,
                  "WARNING: free(0x%p), memPool=%d instead of expected %d, id=%d, size=%d (allocated in \"%s\" line %d), totalAllocSize=%d, nbMalloc=%d",
                  pMemToFree,
                  pAllocInfo->requestedMemoryPool,
                  memPool,
                  pAllocBuffer2Free->id,
                  pAllocInfo->size,
                  pAllocBuffer2Free->file,
                  pAllocBuffer2Free->line,
                  s_totalAllocSize,
                  s_nbMalloc);
        }
        AudioFree(pMemToFree, memPool);
      }
    }
  }
}


memError_t AudioMallocCheckConsistency(void)
{
  memError_t error = AUDIO_MEM_ERROR_NONE;
  bool       ok    = true;

  s_checkResetMemPoolStat();

  for (allocBuffer_t const *pAllocBuffer = s_pAllocRoot; pAllocBuffer != NULL; pAllocBuffer = pAllocBuffer->pNext)
  {
    AUDIO_MEM_DISABLE_IRQ();
    if (!s_doNotCheckConsistency)
    {
      memAllocInfo_t const *pAllocInfo = &pAllocBuffer->allocInfo;
      uint32_t       const *pAllocHeader, *pAllocSeparator, *pAllocFooter;
      char           const *pChar, *pAlloc;

      pChar           = (char *)pAllocBuffer;
      pChar          += offsetof(allocBuffer_t, allocInfo) + sizeof(memAllocInfo_t);
      pAlloc          = pChar;
      pChar          += AUDIO_MEM_SIZE_ALIGN(pAllocInfo->size);
      pAllocHeader    = &pAllocBuffer->header;
      pAllocSeparator = &pAllocBuffer->separator;
      pAllocFooter    = (uint32_t *)pChar;

      if ((*pAllocHeader    != s_allocHeader)    ||
          (*pAllocSeparator != s_allocSeparator) ||
          (*pAllocFooter    != s_allocFooter))
      {
        // rewind to display all heap structure before issue
        s_trace(TRACE_LVL_ERROR, "Heap structure in allocation order:");
        for (allocBuffer_t const *pAllocLoop = s_pAllocRoot; pAllocLoop != NULL; pAllocLoop = pAllocLoop->pNext)
        {
          if (s_displayMallocDescr(pAllocLoop) || (pAllocLoop == pAllocBuffer))
          {
            break;
          }
        }
        // display it in reverse order too
        s_trace(TRACE_LVL_ERROR, "Heap structure in reversed allocation order:");
        for (allocBuffer_t const *pAllocLoop = s_pAllocLast; pAllocLoop != NULL; pAllocLoop = pAllocLoop->pPrev)
        {
          if (s_displayMallocDescr(pAllocLoop) || (pAllocLoop == pAllocBuffer))
          {
            break;
          }
        }
        if (*pAllocHeader    != s_allocHeader)
        {
          s_trace(TRACE_LVL_ERROR, "pAlloc=0x%p: allocHeader (at 0x%p)=0x%08X instead of 0x%08X",    pAlloc, pAllocHeader,    *pAllocHeader,    s_allocHeader);
          error = AUDIO_MEM_ERROR_ALLOCATION;
        }
        if (*pAllocSeparator != s_allocSeparator)
        {
          s_trace(TRACE_LVL_ERROR, "pAlloc=0x%p: allocSeparator (at 0x%p)=0x%08X instead of 0x%08X", pAlloc, pAllocSeparator, *pAllocSeparator, s_allocSeparator);
          error = AUDIO_MEM_ERROR_ALLOCATION;
        }
        if (*pAllocFooter    != s_allocFooter)
        {
          s_trace(TRACE_LVL_ERROR, "pAlloc=0x%p: allocFooter (at 0x%p)=0x%08X instead of 0x%08X",    pAlloc, pAllocFooter,    *pAllocFooter,    s_allocFooter);
          error = AUDIO_MEM_ERROR_ALLOCATION;
        }
        ok = false;
        AUDIO_MEM_ENABLE_IRQ();
        break;
      }
    }
    AUDIO_MEM_ENABLE_IRQ();
  }

  AUDIO_MEM_DISABLE_IRQ();
  if (!s_doNotCheckConsistency && ok)
  {
    size_t localTotalAllocSize = 0UL;
    int    localNbMalloc       = 0;

    for (allocBuffer_t const *pAllocBuffer = s_pAllocRoot; pAllocBuffer != NULL; pAllocBuffer = pAllocBuffer->pNext)
    {
      localTotalAllocSize += pAllocBuffer->allocInfo.size;
      localNbMalloc++;
    }
    if (localTotalAllocSize != s_totalAllocSize)
    {
      s_trace(TRACE_LVL_ERROR, "localTotalAllocSize=%d, totalAllocSize=%d", localTotalAllocSize, s_totalAllocSize);
      error = AUDIO_MEM_ERROR_ALLOCATION;
    }
    if (localNbMalloc != s_nbMalloc)
    {
      s_trace(TRACE_LVL_ERROR, "localNbMalloc=%d, nbMalloc=%d", localNbMalloc, s_nbMalloc);
      error = AUDIO_MEM_ERROR_ALLOCATION;
    }

    localTotalAllocSize = 0UL;
    localNbMalloc       = 0;
    for (int idx = 0; idx < (int)AUDIO_MEM_NB_POOL; idx++)
    {
      localTotalAllocSize += s_memPoolStat[idx].totalAllocSize;
      localNbMalloc       += s_memPoolStat[idx].nbMalloc;
    }
    if (localTotalAllocSize != s_totalAllocSize)
    {
      s_trace(TRACE_LVL_ERROR, "localTotalAllocSize=%d, totalAllocSize=%d", localTotalAllocSize, s_totalAllocSize);
      error = AUDIO_MEM_ERROR_ALLOCATION;
    }
    if (localNbMalloc != s_nbMalloc)
    {
      s_trace(TRACE_LVL_ERROR, "localNbMalloc=%d, nbMalloc=%d", localNbMalloc, s_nbMalloc);
      error = AUDIO_MEM_ERROR_ALLOCATION;
    }
  }
  s_doNotCheckConsistency = false;
  AUDIO_MEM_ENABLE_IRQ();

  return error;
}


memError_t AudioMallocDisplayRemainingMalloc(int const resetMax)
{
  memError_t error = AUDIO_MEM_ERROR_NONE;

  s_checkResetMemPoolStat();

  // display remaining alloc if any
  if (s_pAllocRoot == NULL)
  {
    s_trace(TRACE_LVL_INFO, "no remaining malloc");
    if (s_nbMalloc != 0)
    {
      s_trace(TRACE_LVL_ERROR, "nbMalloc=%d", s_nbMalloc);
      error = AUDIO_MEM_ERROR_ALLOCATION;
    }
    if (s_totalAllocSize != 0UL)
    {
      s_trace(TRACE_LVL_ERROR, "totalAllocSize=%d", s_totalAllocSize);
      error = AUDIO_MEM_ERROR_ALLOCATION;
    }
  }
  else
  {
    #ifdef AUDIO_MEM_CONF_TRACK_MALLOC_VERBOSE1
    for (allocBuffer_t const *pAllocBuffer = s_pAllocRoot; pAllocBuffer != NULL; pAllocBuffer = pAllocBuffer->pNext)
    {
      memAllocInfo_t const *const pAllocInfo = &pAllocBuffer->allocInfo;

      s_trace(TRACE_LVL_INFO,
              "malloc(%d)=0x%p, memPool=%d, id=%d (allocated in \"%s\" line %d) not freed",
              pAllocBuffer->allocInfo.size,
              (char *)pAllocInfo + sizeof(pAllocBuffer->allocInfo),
              pAllocInfo->allocatedMemoryPool,
              pAllocBuffer->id,
              pAllocBuffer->file,
              pAllocBuffer->line);
    }
    #endif
    error = AUDIO_MEM_ERROR_MEMORY_LEAK;
  }

  // display statistics summary
  s_trace(TRACE_LVL_INFO,
          "%d malloc done for a cumulated alloc size of %lld; at a given time maxNbMalloc=%d, maxTotalAllocSize=%d",
          s_mallocId,
          s_cumulatedAllocSize,
          s_maxNbMalloc,
          s_maxTotalAllocSize);
  for (int idx = 0; idx < (int)AUDIO_MEM_NB_POOL; idx++)
  {
    s_trace(TRACE_LVL_INFO,
            "memPool %d: cumulated alloc size of %lld; at a given time maxNbMalloc=%d, maxTotalAllocSize=%d; remaining %d malloc for total alloc size of %d",
            idx,
            s_memPoolStat[idx].cumulatedAllocSize,
            s_memPoolStat[idx].maxNbMalloc,
            s_memPoolStat[idx].maxTotalAllocSize,
            s_memPoolStat[idx].nbMalloc,
            s_memPoolStat[idx].totalAllocSize);
  }

  if (resetMax)
  {
    s_maxTotalAllocSize = s_totalAllocSize;
    s_maxNbMalloc       = s_nbMalloc;
    for (int idx = 0; idx < (int)AUDIO_MEM_NB_POOL; idx++)
    {
      s_memPoolStat[idx].maxTotalAllocSize = s_memPoolStat[idx].totalAllocSize;
      s_memPoolStat[idx].maxNbMalloc       = s_memPoolStat[idx].nbMalloc;
    }
  }

  return error;
}


/* Private Functions Definition ------------------------------------------------------*/

static void s_checkResetMemPoolStat(void)
{
  static bool firstTime = true;

  AUDIO_MEM_DISABLE_IRQ();
  if (firstTime)
  {
    memset(s_memPoolStat, 0, sizeof(s_memPoolStat));
    firstTime = false;
  }
  AUDIO_MEM_ENABLE_IRQ();
}


static bool s_displayMallocDescr(allocBuffer_t const *const pAllocBuffer)
{
  memAllocInfo_t const *const pAllocInfo = &pAllocBuffer->allocInfo;
  char           const       *pChar      = (const char *)pAllocBuffer;
  uint32_t       const       *pAllocHeader, *pAllocSeparator, *pAllocFooter;
  char           const       *pAlloc;

  pChar          += offsetof(allocBuffer_t, allocInfo) + sizeof(memAllocInfo_t);
  pAlloc          = pChar;
  pChar          += AUDIO_MEM_SIZE_ALIGN(pAllocInfo->size);
  pAllocHeader    = &pAllocBuffer->header;
  pAllocSeparator = &pAllocBuffer->separator;
  pAllocFooter    = (const uint32_t *)pChar;

  s_trace(TRACE_LVL_INFO,
          "pAlloc=0x%p: size=%d, memPool=%d, id=%d (allocated in \"%s\" line %d), s_allocHeader (at 0x%p) = 0x%08X, s_allocSeparator (at 0x%p) = 0x%08X, s_allocFooter (at 0x%p) = 0x%08X",
          pAlloc,
          pAllocInfo->size,
          pAllocInfo->allocatedMemoryPool,
          pAllocBuffer->id,
          pAllocBuffer->file,
          pAllocBuffer->line,
          pAllocHeader,
          *((uint32_t *)pAllocHeader),
          pAllocSeparator,
          *((uint32_t *)pAllocSeparator),
          pAllocFooter,
          *((uint32_t *)pAllocFooter));

  return ((*pAllocHeader    != s_allocHeader)    ||
          (*pAllocSeparator != s_allocSeparator) ||
          (*pAllocFooter    != s_allocFooter));
}


static void s_trace(traceLvl_t const level, char const *const pFormat, ...)
{
  char    tmpBuff[256];
  va_list args;

  snprintf(tmpBuff, sizeof(tmpBuff), "AUDIO_MALLOC %s\n", pFormat);
  va_start(args, pFormat);
  trace_print_args(TRACE_OUTPUT_UART, level, tmpBuff, args);
  va_end(args);
}

#else // AUDIO_MEM_CONF_TRACK_MALLOC

void *AudioMallocDebug(size_t const size, memPool_t const memPool, char const *const file, int const line)
{
  (void)file;
  (void)line;
  return AudioMalloc(size, memPool);
}


void *AudioReallocDebug(void *const ptr, size_t const size, memPool_t const memPool, char const *const file, int const line)
{
  (void)file;
  (void)line;
  return AudioRealloc(ptr, size, memPool);
}


void AudioFreeDebug(void *const pMemToFree, memPool_t const memPool)
{
  AudioFree(pMemToFree, memPool);
}


memError_t AudioMallocCheckConsistency(void)
{
  return AUDIO_MEM_ERROR_NONE;
}


memError_t AudioMallocDisplayRemainingMalloc(int const resetMax)
{
  (void)resetMax;
  return AUDIO_MEM_ERROR_NONE;
}

#endif // AUDIO_MEM_CONF_TRACK_MALLOC


/* Private Functions Definition ------------------------------------------------------*/

#ifdef AUDIO_MEM_CONF_ASSERT_ENABLED
static void s_assert(bool const cond, char const *const condString, char const *const file, int const line, char const *const pFormat, ...)
{
  if (!cond)
  {
    const char *p;
    char        tmpBuff[256];
    va_list     args;

    /* Remove the full path */
    for (p = &file[strlen(file)]; p > file; p--)
    {
      if ((*p == '/') || (*p == '\\'))
      {
        p++;
        break;
      }
    }

    snprintf(tmpBuff, sizeof(tmpBuff), "AUDIO_MEM assert(%s) failed in file %s, line %d: %s\n", condString, p, line, pFormat);
    va_start(args, pFormat);
    trace_print_args(TRACE_OUTPUT_UART, TRACE_LVL_ERROR, tmpBuff, args);
    va_end(args);

    while (1)
    {
      /* lock FW */
    }
  }
}
#endif // AUDIO_MEM_CONF_ASSERT_ENABLED
