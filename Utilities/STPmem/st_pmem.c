/**
******************************************************************************
* @file    st_pmem.c
* @author  MCD Application Team
* @brief   implements a pool memory and instrumentation management to check
*          leak and corruptions
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


/*
  This module implements a memory manager similar to stdlib.
  but allocations occur in a memory pool and provides aligned memory options, the code is highly instrumented to
  detect memory corruptions  / leak / statistics /re-entrance

  Pay attention to the instrumentation that is very intrusive. Disable it for release


*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "st_pmem.h"

#ifndef MIN
  #define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define PMEM_BLK_FREE 1UL
#define PMEM_BLK_END  2UL
#define PMEM_BLK_MSK  3UL

#if defined(PMEM_CHECK_CORRUPTION)
#define PMEM_MEM_ASSERT(p,a) pmem_assert(p,((uint32_t)(a) & 0xFFFFFFFF), (const char*)#a, (const char*)__FILE__, __LINE__)
/* Check if we are thread safe */

#define CHECK_CORRUPTION(a, b) pmem_check((a), (b))
#define PMEM_MEM_PROLOG(a)           \
    (a)->m_active = 1
#define PMEM_MEM_EPILOG(a)          \
    (a)->m_active = 0
#else
#define PMEM_MEM_ASSERT(...)  ((void)0)
#define PMEM_MEM_PROLOG(...)  ((void)0)
#define PMEM_MEM_EPILOG(...)  ((void)0)
#define CHECK_CORRUPTION(...) ((void)0)
#endif

#ifdef PMEM_USE_SIGNATURE
  #define PMEM_MEM_CHECK_PMEM_SIGNATURE(h, p) pmem_check_signature(h, p)
  #define PMEM_MEM_NEW_BLK(p)                 {p->m_userTag = 0;}
#else
  #define PMEM_MEM_CHECK_PMEM_SIGNATURE(h, p)
  #define PMEM_MEM_NEW_BLK(p)
#endif /*PMEM_USE_SIGNATURE*/

#define REASONABLE_SIZE(a)              ((a)->m_iBaseSize)
#define PMEM_MEM_OFFSET(a)              ((uint32_t)(a))
#define PMEM_MEM_CHK_SIZE(h, cur, size) pmem_check_size_coherency((h), (cur), (size))

/* pre-definition */

static pmem_header_t *pmem_find_best_candidate(pmem_pool_t *pHandle, uint32_t size);
static uint32_t       pmem_align(pmem_pool_t *pHandle, int32_t size);
static pmem_header_t *pmem_get_header_pointer(pmem_pool_t *pHandle, void *pBlock);
static void           pmem_update_alloc(pmem_pool_t *pHandle, int32_t blk);
void                  pmem_assert(pmem_pool_t *pHandle, uint32_t condition, const char *pCondition, const char *pFile, int32_t line);
static void          *pmem_get_malloc_pointer(pmem_pool_t *pHandle, pmem_header_t *pBlock);
void                  pmem_check_size_coherency(pmem_pool_t *pHandle, void *pCur, uint32_t size);
void                  pmem_check_signature(pmem_pool_t *pHandle, pmem_header_t *pCur);
void                  pmem_check(pmem_pool_t *pHandle, uint32_t force);
static void           pmem_free_blk(pmem_pool_t *pHandle, void *pBlk);


/**
 * @brief set a flag
 *
 * @param pBlk the block instance
 * @param flags the flags
 */
static void pmem_set_blk_flags(pmem_header_t *pBlk, uint32_t flags)
{
  pBlk->m_szBlk |= flags;
}
/**
 * @brief clear a flag
 *
 * @param pBlk the block instance
 * @param flags the flag
 */
static void pmem_clear_blk_flags(pmem_header_t *pBlk, uint32_t flags)
{
  pBlk->m_szBlk &= ~flags;
}
/**
 * @brief returns the block size
 *
 * @param pBlk the block instance
 * @return uint32_t the size
 */
static uint32_t pmem_get_blk_size(pmem_header_t *pBlk)
{
  return pBlk->m_szBlk & ~PMEM_BLK_MSK;
}

/**
 * @brief set the size
 *
 * @param pBlk the block instance
 * @param void
 */
static void pmem_set_blk_size(pmem_header_t *pBlk, uint32_t size)
{
  pBlk->m_szBlk &= PMEM_BLK_MSK;
  size &= ~PMEM_BLK_MSK;
  pBlk->m_szBlk |= size;
}
/**
 * @brief return true if the flags is set
 *
 * @param pBlk the block instance
 * @param flags the flag
 * @return true or false
 */
static int32_t pmem_check_blk_flags(pmem_header_t *pBlk, uint32_t flags)
{
  return ((pBlk->m_szBlk & flags) != 0U)  ? 1 : 0;
}
/**
 * @brief gets the nex block or null if it is the end
 *
 * @param pHandle the pool install
 * @param pBlk pointer on the current block
 * @return the next block
 */
static pmem_header_t *pmem_get_blk_next(pmem_pool_t *pHandle, pmem_header_t *pBlk)
{
  if (pmem_check_blk_flags(pBlk, PMEM_BLK_END))
  {
    return NULL;
  }
  return (pmem_header_t *)(((uint8_t *)pBlk) + (pHandle->m_szHeader + pmem_get_blk_size(pBlk)));
}

/**
 * @brief check if the block is within the pool, otherwise corruption
 *
 * @param pHandle the  instance handle
 * @param pCur the current block
 * @param size the size to check
 */
void pmem_check_size_coherency(pmem_pool_t *pHandle, void *pCur, uint32_t size)
{
  if (((int32_t)size < 0) || ((((uint8_t *)pCur) + size) > (((uint8_t *)pHandle->m_pBaseMalloc) + pHandle->m_iBaseSize)))
  {
    PMEM_PRINTF("Mem corruption: at 0x%p\r\n", pCur);
    PMEM_MEM_ASSERT(pHandle, (((int32_t)size >= 0) && ((((uint8_t *)pCur) + size) <= (((uint8_t *)pHandle->m_pBaseMalloc) + pHandle->m_iBaseSize))) ? 1U : 0U);
  }
}
/**
 * @brief Check the block signature
 *
 * @param pCur the current block
 */
void pmem_check_signature(pmem_pool_t *pHandle, pmem_header_t *pCur)
{
  #ifdef PMEM_USE_SIGNATURE
  if (pCur->m_signature != PMEM_SIGNATURE)
  {
    PMEM_PRINTF("Mem corruption: at %p\r\n", pCur);
    #ifdef PMEM_SAVE_POOL_ASSERT
    char tName[50];
    snprintf(tName, sizeof(tName), "Pool_%p-%p.pool", pHandle->m_pBaseMalloc, (void *)pHandle->m_iBaseSize);

    FILE *pFile = fopen(tName, "wb");
    if (pFile)
    {
      fwrite(pHandle->m_pBaseMalloc, 1, pHandle->m_iBaseSize, pFile);
      fclose(pFile);
      printf("Save pool after corruption %s\r\n\r\n", tName);
    }
    #endif

    PMEM_MEM_ASSERT(pHandle, ((pCur->m_signature == PMEM_SIGNATURE) ? 1U : 0U));
  }
  #endif
}

/**
 * @brief return a aligned size
 *
 * @param pHandle the  instance handle
 * @param size  the size
 * @return uint32_t  the aligned size
 */
static uint32_t pmem_align(pmem_pool_t *pHandle, int32_t size)
{
  uint32_t newsize = ((uint32_t)size + pHandle->m_szAlign - 1U) & (~(pHandle->m_szAlign - 1U));
  return newsize;
}
/**
 * @brief returns the raw allocated pointer
 *
 * @param pHandle the instance handle
 * @param pBlock the block
 * @return void* the raw pointer
 */
static void *pmem_get_malloc_pointer(pmem_pool_t *pHandle, pmem_header_t *pBlock)
{
  uint8_t *pM = ((uint8_t *)pBlock) + pHandle->m_szHeader;
  return (void *)pM;
}
/**
 * @brief returns the block pointer from a raw pointer  allocated
 *
 * @param pHandle the instance handle
 * @param pBlock  the raw pointer
 * @return pmem_header_t* the block pointer
 */
static pmem_header_t *pmem_get_header_pointer(pmem_pool_t *pHandle, void *pBlock)
{
  uint8_t *pM = ((uint8_t *)pBlock) - pHandle->m_szHeader;
  return ((pmem_header_t *)pM);
}
/**
 * @brief collect statistics
 *
 * @param pHandle the instance handle
 * @param blk the current block
 */

static void pmem_update_alloc(pmem_pool_t *pHandle, int32_t blk)
{
  pHandle->m_globalAlloc += blk;
  PMEM_MEM_ASSERT(pHandle, ((uint32_t)pHandle->m_globalAlloc <= pHandle->m_iBaseSize) ? 1U : 0U);
  if (pHandle->m_maxAlloc < pHandle->m_globalAlloc)
  {
    pHandle->m_maxAlloc = pHandle->m_globalAlloc;
  }
}

/* ------------------------------------------------------------------------------------- */
/*! pmem_reset the pool to empty , all blocks will be lost
*
* \param pmem_pool_t *pHandle
*
* \return memBool
*/

void pmem_reset(pmem_pool_t *pHandle)
{
  if (pHandle->m_pBaseMalloc != 0)
  {
    PMEM_SET_PATTERN(pHandle->m_pBaseMalloc, pHandle->m_iBaseSize);

    if ((uint32_t)pHandle == PMEM_MEM_PATTERN_32BIT)
    {
      PMEM_PRINTF("A pool is being created where the RW_CONTENT is stored, please review both your linker file and ST_OS_Mem pool mapping.\n");
      PMEM_MEM_ASSERT(NULL, false);
    }

    pmem_header_t *pFirst = ((pmem_header_t *)pHandle->m_pBaseMalloc);
    memset(pFirst, 0, sizeof(pmem_header_t));
    pHandle->m_globalAlloc = (int32_t)pHandle->m_szHeader;
    pHandle->m_maxAlloc    = (int32_t)pHandle->m_globalAlloc;
    pHandle->m_nbFrags     = 1;
    pFirst->m_szBlk        = pHandle->m_iBaseSize - pHandle->m_szHeader;
    pmem_set_blk_flags(pFirst, PMEM_BLK_FREE | PMEM_BLK_END);
    #ifdef PMEM_INSTRUMENTATION
    #ifdef PMEM_USE_SIGNATURE
    pFirst->m_signature = PMEM_SIGNATURE;
    #endif
    #ifdef PMEM_CHECK_CORRUPTION
    pHandle->m_active = 0;
    #endif
    #endif
  }
}
/**
 * @brief gets the pool size
 *
 * @param pHandle the instance handle
 * @return int32_t the size
 */

int32_t pmem_get_free_size(pmem_pool_t *pHandle)
{
  return (int32_t)pHandle->m_iBaseSize - pHandle->m_globalAlloc + (int32_t)sizeof(pmem_header_t);
}


/**
 * @brief bench the pool
 *
 * @param pHandle
 */
__weak uint32_t pmem_compute_performance_index(pmem_pool_t *pHandle, uint32_t cpuFreq)
{
  pHandle->m_perfIndex = 0;
  return pHandle->m_perfIndex;
}


/**
 * @brief initializes the memory pool
 *
 * @param pHandle the instance handle
 * @param pBlock the base memory block
 * @param size  the base memory block size
 * @param align the alignment and  flags
 * @return bool true or false
 */
bool pmem_init(pmem_pool_t *pHandle, void *pBlock, uint32_t size, uint32_t flags)
{
  memset(pHandle, 0, sizeof(*pHandle));

  pHandle->m_szAlign = flags & PMEM_MSK_ALIGN;
  if (pHandle->m_szAlign < 4U)
  {
    pHandle->m_szAlign = PMEM_DEFAULT_SIZE_ALIGN;
  }
  uint32_t shiftAlign = 0;
  while ((pHandle->m_szAlign & 1U) == 0U)
  {
    pHandle->m_szAlign >>= 1U;
    shiftAlign++;
  }
  if (pHandle->m_szAlign != 1U)
  {
    /* m_szAlign is not a power of 2 */
    pHandle->m_szAlign = PMEM_DEFAULT_SIZE_ALIGN;
  }
  else
  {
    pHandle->m_szAlign = (uint32_t)(1UL << shiftAlign);
  }

  pHandle->m_szHeader = (uint8_t)pmem_align(pHandle, (int32_t)sizeof(pmem_header_t));
  pHandle->m_flags = (flags & (~PMEM_MSK_ALIGN));
  pHandle->m_iBaseSize   = size;
  pHandle->m_checkFreq   = 0;
  pHandle->m_pBaseMalloc = (pmem_header_t *)pBlock;

  if ((size > (uint32_t)pHandle->m_szHeader) && ((flags & PMEM_FLAG_DISABLED) == 0UL))
  {
    pmem_reset(pHandle);
  }

  return true;
}

/**
 * @brief terminate the pool instance
 *
 * @param pHandle
 */
void pmem_term(pmem_pool_t *pHandle)
{
}

/**
 * @brief finds a best candidate to be allocated
 *
 * @param pHandle the instance handle
 * @param size the size to alloc
 * @return pmem_header_t* the block or null
 */

static pmem_header_t *pmem_find_best_candidate(pmem_pool_t *pHandle, uint32_t size)
{
  pmem_header_t *pBestOne = NULL;
  uint32_t       bestSize = 0xFFFFFFFFUL;

  for (pmem_header_t *pCur = ((pmem_header_t *)pHandle->m_pBaseMalloc); pCur != NULL; pCur = pmem_get_blk_next(pHandle, pCur))
  {
    PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pCur);
    if (pmem_check_blk_flags(pCur, PMEM_BLK_FREE))
    {
      uint32_t curSize = pmem_get_blk_size(pCur);
      if (curSize == size)
      {
        /* we have found the exact size => no need to check further */
        pBestOne = pCur;
        break;
      }
      if (curSize > size)
      {
        /* Always prefer the smaller block */
        if (bestSize > curSize)
        {
          pBestOne = pCur;
          bestSize = curSize;
        }
      }
    }
  }
  return pBestOne;
}

/**
 * @brief Evaluate the possible free size for a new allocation
 *
 * @param pHandle the instance handle
 * @param pFirst the first block
 * @return uint32_t the size
 */

static uint32_t pmem_evaluate_free_size(pmem_pool_t *pHandle, pmem_header_t *pFirst)
{
  uint32_t       szFree = 0;
  pmem_header_t *pCur   = pFirst;
  while (pCur)
  {
    PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pCur);
    if (pmem_check_blk_flags(pCur, PMEM_BLK_FREE) == 0)
    {
      break;
    }
    szFree += pmem_get_blk_size(pCur);
    szFree += pHandle->m_szHeader;
    pCur = pmem_get_blk_next(pHandle, pCur);
  }
  return szFree;
}
/**
 * @brief merges  blocks free in a single block
 *
 * @param pHandle the instance handle
 * @param pFirst the first block
 */
static void pmem_collapse_free_continuous_blk(pmem_pool_t *pHandle, pmem_header_t *pFirst)
{
  uint32_t       collapsedBlockSize = pmem_get_blk_size(pFirst);
  uint32_t       nbCollapse         = 0;
  pmem_header_t *pNext              = NULL;
  pmem_header_t *pCur               = pmem_get_blk_next(pHandle, pFirst);
  while (pCur)
  {
    PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pCur);
    if (pmem_check_blk_flags(pCur, PMEM_BLK_FREE) == 0)
    {
      /* this block is not free */
      /* stops the collapse */
      break;
    }

    /* delete this block and collapse with the first */
    collapsedBlockSize += pHandle->m_szHeader + pmem_get_blk_size(pCur);
    nbCollapse++;

    /* Fetch Next and flags before to delete it */
    pNext = pmem_get_blk_next(pHandle, pCur);

    /* remove any header mark */
    PMEM_SET_PATTERN(pCur, pHandle->m_szHeader);

    pCur = pNext;
  }
  if (pCur == NULL)
  {
    /* the block is the end, the parent becomes the last block and stop */
    pmem_set_blk_flags(pFirst, PMEM_BLK_END);
  }
  /* transmit the new size to the parent */
  pmem_set_blk_size(pFirst, collapsedBlockSize);
  /* remove the headers size */
  pmem_update_alloc(pHandle, - ((int32_t)nbCollapse * (int32_t)pHandle->m_szHeader));
  pHandle->m_nbFrags -= nbCollapse;
}

/**
 * @brief   merges  all pool blocks together
 *
 * @param pHandle the instance handle
 */

static void pmem_collapse_free_blk(pmem_pool_t *pHandle)
{
  pmem_header_t *pCur = (pmem_header_t *)pHandle->m_pBaseMalloc;
  if (pmem_check_blk_flags(pCur, PMEM_BLK_END))
  {
    return;
  }
  while (pCur)
  {
    PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pCur);
    if (pmem_check_blk_flags(pCur, PMEM_BLK_FREE))
    {
      /* found a free block */
      /* collapse it */
      pmem_collapse_free_continuous_blk(pHandle, pCur);
    }
    pCur = pmem_get_blk_next(pHandle, pCur);
  }
}

/**
 * @brief pooled calloc
 *
 * @param pHandle the instance handle
 * @param size the element size
 * @param elem the number of elements
 * @return void* the raw pointer
 */
void *pmem_calloc(pmem_pool_t *pHandle, uint32_t size, uint32_t elem)
{
  void *pMalloc = pmem_alloc(pHandle, size * elem);
  if (pMalloc != NULL)
  {
    memset(pMalloc, 0, size * elem);
  }
  return pMalloc;
}

/**
 * @brief pooled malloc
 *
 * @param pHandle the instance handle
 * @param size the  size
 * @return void* the raw pointer
 */
void *pmem_alloc(pmem_pool_t *pHandle, uint32_t size)
{

  if ((pHandle->m_iBaseSize == 0U) || ((pHandle->m_flags & PMEM_FLAG_DISABLED) != 0UL))
  {
    return NULL;
  }

  PMEM_MEM_PROLOG(pHandle);
  CHECK_CORRUPTION(pHandle, FALSE);


  uint8_t       *pMallocPtr = NULL;
  pmem_header_t *pBlockNext = NULL;
  uint32_t        dwLastFree;
  uint32_t       sizeAligned = pmem_align(pHandle, (int32_t)size);
  //  pmem_collapse_free_blk(pHandle);
  pmem_header_t *pCandidate = pmem_find_best_candidate(pHandle, sizeAligned);
  if (pCandidate == NULL)
  {
    PMEM_MEM_EPILOG(pHandle);
    return NULL;
  }
  /* preserve size */
  /* size include m_szHeader + payload */
  dwLastFree = pmem_get_blk_size(pCandidate);
  /* return the pointer right after the header */
  pMallocPtr = (uint8_t *)pmem_get_malloc_pointer(pHandle, pCandidate);

  if ((dwLastFree - sizeAligned) <= pHandle->m_szHeader)
  {
    /* not enough room for a new fragment => simply use pCandidate with its current size and declare it as not free */
    pmem_clear_blk_flags(pCandidate, PMEM_BLK_FREE);
    pmem_update_alloc(pHandle, (int32_t)dwLastFree); /* do not add pHandle->m_szHeader: it was already counted inside pHandle->m_globalAlloc when pCandidate was a free block */
    PMEM_MEM_CHK_SIZE(pHandle, pCandidate, pmem_get_blk_size(pCandidate));
  }
  else
  {
    /* Compute the next entry at the end of the coppered block size */
    pBlockNext = (pmem_header_t *)(pMallocPtr + sizeAligned);
    //memset(pBlockNext, 0, pHandle->m_szHeader); // useless: will be overwritten by "*pBlockNext = *pCandidate;" just below
    pHandle->m_nbFrags++;
    /* copy flags */
    *pBlockNext = *pCandidate;
    PMEM_MEM_NEW_BLK(pBlockNext);

    /* We have heated all memory of the candidate */
    pmem_set_blk_size(pCandidate, sizeAligned);
    /* The next block gets the block size */
    pmem_set_blk_size(pBlockNext, (uint32_t)(dwLastFree - (sizeAligned + pHandle->m_szHeader)));

    pmem_clear_blk_flags(pCandidate, PMEM_BLK_FREE | PMEM_BLK_END);
    pmem_set_blk_flags(pBlockNext, PMEM_BLK_FREE);
    PMEM_MEM_CHK_SIZE(pHandle, pCandidate, pmem_get_blk_size(pCandidate));

    PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pBlockNext);
    PMEM_MEM_CHK_SIZE(pHandle, pBlockNext, pmem_get_blk_size(pBlockNext));
    pmem_update_alloc(pHandle, ((int32_t)sizeAligned + (int32_t)pHandle->m_szHeader));
  }
  PMEM_MEM_EPILOG(pHandle);

  return pMallocPtr;
}

/**
 * @brief pooled realloc
 *
 * @param pHandle the instance handle
 *  @param pBlock the base block
 * @param sizeMalloc the  size
 * @return void* the raw pointer
 */
void *pmem_realloc(pmem_pool_t *pHandle, void *pBlock, uint32_t sizeMalloc)
{
  if ((pHandle->m_iBaseSize == 0U) || ((pHandle->m_flags & PMEM_FLAG_DISABLED) != 0UL))
  {
    return NULL;
  }

  if (pBlock == NULL)
  {
    return pmem_alloc(pHandle, sizeMalloc);
  }

  PMEM_MEM_PROLOG(pHandle);
  CHECK_CORRUPTION(pHandle, FALSE);


  /* make free as mush as possible continue free block */
  pmem_collapse_free_blk(pHandle);

  pmem_header_t *pCandidate = pmem_get_header_pointer(pHandle, pBlock);
  PMEM_MEM_CHK_SIZE(pHandle, pCandidate, pmem_get_blk_size(pCandidate));
  /* fetch the size before realloc */
  uint32_t prevSize = pmem_get_blk_size(pCandidate);
  /* size must be aligned */
  uint32_t sizeAligned = pmem_align(pHandle, (int32_t)sizeMalloc);

  /* we need at least sizeAligned + sizeof(header) to create a new free block*/
  /* if same size, quit  */
  if (sizeAligned == prevSize)
  {
    PMEM_MEM_EPILOG(pHandle);
    return pBlock;
  }

  /* we have to decide if we keep the block or free the block and realloc a new one */
  pmem_header_t *pNext = pmem_get_blk_next(pHandle, pCandidate);
  /* Take in account the current block size */
  uint32_t maxFreeBlock = prevSize;
  if (pNext)
  {
    PMEM_MEM_CHK_SIZE(pHandle, pNext, pmem_get_blk_size(pNext));
    /* if the nex block is a free block, add it the potential size of the current block */
    maxFreeBlock = prevSize + pmem_evaluate_free_size(pHandle, pNext);
  }
  /* if we have a size that fit the free block , we need to update the block and create a free block after the new size */
  if (maxFreeBlock >= (sizeAligned + pHandle->m_szHeader))
  {
    /* compute the free block size that will be created after the resizeed block */
    int32_t newFreeSize = (int32_t)maxFreeBlock - (int32_t)sizeAligned - (int32_t)pHandle->m_szHeader;

    /* set the block free, data are not changed except blk headers filled with a pattern*/
    pmem_set_blk_flags(pCandidate, PMEM_BLK_FREE);
    /* update the freed mem*/
    pmem_update_alloc(pHandle, -(int32_t)prevSize);
    /* merge all continuous free block in this blk, the pmem_update_alloc is done in this function   */
    pmem_collapse_free_continuous_blk(pHandle, pCandidate);
    if (newFreeSize <= 0)
    {
      /* not enough room for a new fragment => simply use pCandidate with its collapsed size slightly bigger than necessary */
      /* restore the prevsize */
      prevSize = pmem_get_blk_size(pCandidate);
      /* remove the free flag */
      pmem_clear_blk_flags(pCandidate, PMEM_BLK_FREE);
      /* the size is alorady set in the pCandidate, just update the global size */
      pmem_update_alloc(pHandle, (int32_t)prevSize);
    }
    else
    {
      /* set the new size  and create a free block between the current and the next*/
      pmem_set_blk_size(pCandidate, sizeAligned);
      pmem_update_alloc(pHandle, (int32_t)sizeAligned);

      /* create the new free block */
      pNext = (pmem_header_t *)(((char *)pCandidate) + (pHandle->m_szHeader + sizeAligned));
      /* propagate end and other flags */
      *pNext = *pCandidate;
      PMEM_MEM_NEW_BLK(pNext);

      /* set the new free size */
      pmem_set_blk_size(pNext, (uint32_t)newFreeSize);
      /* remove the free flag and end copied in the next */
      pmem_clear_blk_flags(pCandidate, PMEM_BLK_FREE | PMEM_BLK_END);
      /* set the new block as free */
      pmem_set_blk_flags(pNext, PMEM_BLK_FREE);
      /* update the new allocated size */
      pmem_update_alloc(pHandle, (int32_t)pHandle->m_szHeader); /* payload and the free blk header */
      pHandle->m_nbFrags++;
      PMEM_MEM_CHK_SIZE(pHandle, pNext, pmem_get_blk_size(pNext));
    }
    PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pCandidate);
    PMEM_MEM_CHK_SIZE(pHandle, pCandidate, pmem_get_blk_size(pCandidate));
    PMEM_MEM_EPILOG(pHandle);
    return pmem_get_malloc_pointer(pHandle, pCandidate);
  }
  else
  {
    /* Else free the block & realloc */
    /* if it was not possible to increase block size to fit requested size, it would have been done above
    thus at this point we are sure it wasn't possible, thus freeing pBlock before allocating a new block
    won't increase the chance to find enough room for this new block, thus we can free the old block
    after having allocated the new one */
    PMEM_MEM_EPILOG(pHandle);
    void *pNewBlk = pmem_alloc(pHandle, sizeMalloc);
    if (pNewBlk)
    {
      PMEM_MEM_PROLOG(pHandle);
      pCandidate = pmem_get_header_pointer(pHandle, pNewBlk);
      PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pCandidate);
      PMEM_MEM_CHK_SIZE(pHandle, pCandidate, pmem_get_blk_size(pCandidate));
      memmove(pNewBlk, pBlock, MIN((prevSize), (sizeMalloc)));
      PMEM_MEM_EPILOG(pHandle);

      pmem_free(pHandle, pBlock);
    }
    return pNewBlk;
  }
}
/**
 * @brief returns the allocated size
 *
 * @param pHandle the instance handle
 * @param pBlk the block
 * @return uint32_t the size
 */

uint32_t pmem_get_size(pmem_pool_t *pHandle, void *pBlk)
{
  if ((pHandle->m_iBaseSize == 0U) || ((pHandle->m_flags & PMEM_FLAG_DISABLED) != 0UL))
  {
    return 0;
  }
  if (pBlk == NULL)
  {
    return 0;
  }
  pmem_header_t *pNext = pmem_get_header_pointer(pHandle, pBlk);
  return pmem_get_blk_size(pNext);
}

/**
 * @brief pooled free
 *
 * @param pHandle the instance handle
 * @param pBlk the raw allocated pointer
 */
void pmem_free(pmem_pool_t *pHandle, void *pBlk)
{
  if ((pHandle->m_iBaseSize == 0U) || ((pHandle->m_flags & PMEM_FLAG_DISABLED) != 0UL))
  {
    return;
  }

  /* The standard says that we can free a null pointer, if ptr is a null pointer, no action occurs. */
  if (pBlk == NULL)
  {
    return;
  }

  PMEM_MEM_PROLOG(pHandle);
  CHECK_CORRUPTION(pHandle, TRUE);

  pmem_free_blk(pHandle, pBlk);

  /* Take this opportunity to collapse blocks */
  pmem_collapse_free_blk(pHandle);
  PMEM_MEM_EPILOG(pHandle);
}



static void pmem_free_blk(pmem_pool_t *pHandle, void *pBlk)
{
  pmem_header_t *pCur = pmem_get_header_pointer(pHandle, pBlk);
  CHECK_CORRUPTION(pHandle, FALSE);
  PMEM_MEM_ASSERT(pHandle, pHandle->m_nbFrags);
  PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pCur);
  PMEM_MEM_ASSERT(pHandle, pmem_check_blk_flags(pCur, PMEM_BLK_FREE) == 0);


  /* remove blk size from the global alloc tracker */
  pmem_update_alloc(pHandle, -(int32_t)(pmem_get_blk_size(pCur)));
  /* the the block as free*/
  pmem_set_blk_flags(pCur, PMEM_BLK_FREE);
  /* Unlink block */
  //  if (pHandle->m_checkFreq)
  {
    /* If we are debugging the alloc, we fill a pattern to see freed blk in the dump mem */
    PMEM_SET_PATTERN(pBlk, pmem_get_blk_size(pCur));
  }
  PMEM_MEM_CHK_SIZE(pHandle, pCur, pmem_get_blk_size(pCur));
}



/**
 * @brief checks if the raw pointer belongs to the pool
 *
 * @param pHandle the instance handle
 * @param ptr the raw pointer
 * @return bool true or false
 */

bool pmem_check_ptr(pmem_pool_t *pHandle, void *ptr)
{
  if (pHandle == NULL)
  {
    return false;
  }
  if (ptr == NULL)
  {
    return false;
  }
  if (pHandle->m_pBaseMalloc == NULL)
  {
    return false;
  }
  if (ptr < pHandle->m_pBaseMalloc)
  {
    return false;
  }
  if ((uint8_t *)ptr > (((uint8_t *)pHandle->m_pBaseMalloc) + pHandle->m_iBaseSize))
  {
    return false;
  }
  #ifdef PMEM_USE_SIGNATURE
  if (pmem_get_header_pointer(pHandle, ptr)->m_signature != PMEM_SIGNATURE)
  {
    return false;
  }
  #endif

  return true;
}

/**
 * @brief fill info concerning the pool
 *
 * @param pHandle the instance handle
 * @param pInfo the information struct
 */

void pmem_get_info(pmem_pool_t *pHandle, pmem_Info_t *pInfo)
{
  pmem_header_t *pCur   = NULL;
  pInfo->sizeOccuped    = 0UL;
  pInfo->sizeFree       = 0UL;
  pInfo->AllocMax       = 0UL;
  pInfo->NumFrag        = 0UL;
  pmem_header_t *pFirst = ((pmem_header_t *)pHandle->m_pBaseMalloc);

  pCur = pFirst;
  while (1)
  {
    uint32_t blkSize = pmem_get_blk_size(pCur);
    if (pmem_check_blk_flags(pCur, PMEM_BLK_FREE))
    {
      pInfo->sizeFree += blkSize;
      if ((blkSize - pHandle->m_szHeader) > pInfo->AllocMax)
      {
        pInfo->AllocMax = blkSize;
      }
    }
    else
    {
      pInfo->sizeOccuped += blkSize;
    }
    pInfo->NumFrag++;
    if (pmem_check_blk_flags(pCur, PMEM_BLK_END))
    {
      break;
    }
    pCur = pmem_get_blk_next(pHandle, pCur);
  }
}

/**
 * @brief checks error and corruptions
 *
 * @param pHandle the instance handle
 * @param force force the check
 */
void pmem_check(pmem_pool_t *pHandle, uint32_t force)
{
  pHandle->m_checkCount++;
  if (force == 0U)
  {
    if (pHandle->m_checkFreq == 0U)
    {
      return;
    }
    if ((pHandle->m_checkCount % pHandle->m_checkFreq) != 0U)
    {
      return;
    }
  }

  uint32_t       nbFrags   = 0;
  uint32_t       allocSize = 0;
  uint32_t       freeSize  = 0;
  pmem_header_t *pCur      = ((pmem_header_t *)pHandle->m_pBaseMalloc);
  while (pCur)
  {
    PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pCur);
    uint32_t size = pmem_get_blk_size(pCur);
    PMEM_MEM_CHK_SIZE(pHandle, pCur, size);
    if (pmem_check_blk_flags(pCur, PMEM_BLK_FREE))
    {
      freeSize += size;
    }
    else
    {
      allocSize += size;
    }
    allocSize += pHandle->m_szHeader;
    nbFrags++;
    PMEM_MEM_ASSERT(pHandle, ((allocSize + freeSize) <= pHandle->m_iBaseSize) ? 1U : 0U);
    PMEM_MEM_ASSERT(pHandle, (allocSize <= (uint32_t)pHandle->m_globalAlloc) ? 1U : 0U);
    PMEM_MEM_ASSERT(pHandle, (nbFrags <= pHandle->m_nbFrags) ? 1U : 0U);
    pCur = pmem_get_blk_next(pHandle, pCur);
  }
  PMEM_MEM_ASSERT(pHandle, ((allocSize + freeSize) == pHandle->m_iBaseSize) ? 1U : 0U);
  PMEM_MEM_ASSERT(pHandle, (allocSize == (uint32_t)pHandle->m_globalAlloc) ? 1U : 0U);
  PMEM_MEM_ASSERT(pHandle, (nbFrags == pHandle->m_nbFrags) ? 1U : 0U);
}

/**
 * @brief checks if it is corrupted
 *
 * @param pHandle the instance handle
 */

void pmem_check_corruption(pmem_pool_t *pHandle)
{
  pmem_check(pHandle, TRUE);
}

/**
 * @brief checks the block corruption
 *
 * @param pHandle the instance handle
 * @param pBlk the block
 * @param size the size
 * @return bool  true or false
 */
bool pmem_check_corruption_blk(pmem_pool_t *pHandle, void *pBlk, int32_t size)
{
  pHandle->m_checkCount++;
  if (pHandle->m_checkFreq == 0U)
  {
    return false;
  }
  if ((pHandle->m_checkCount % pHandle->m_checkFreq) != 0U)
  {
    return false;
  }
  uint32_t maxFrags = pHandle->m_nbFrags;
  if (maxFrags == 0U)
  {
    return false;
  }
  pmem_header_t *pCur   = NULL;
  pmem_header_t *pFirst = ((pmem_header_t *)pHandle->m_pBaseMalloc);
  pCur                  = pFirst;
  uint32_t nbFrag       = 0;
  while ((pCur != NULL) && (nbFrag < (pHandle->m_nbFrags + 30U)))
  {
    pmem_header_t *pNext = pmem_get_blk_next(pHandle, pCur);
    PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pCur);
    PMEM_MEM_CHK_SIZE(pHandle, pCur, pmem_get_blk_size(pCur));
    if (pNext)
    {
      PMEM_MEM_CHECK_PMEM_SIGNATURE(pHandle, pNext);
      PMEM_MEM_CHK_SIZE(pHandle, pCur, pmem_get_blk_size(pNext));
    }
    /* New check the limits */
    uint8_t *pCurEnd = (uint8_t *)pmem_get_blk_next(pHandle, pCur);
    if (pCurEnd == NULL)
    {
      pCurEnd = (uint8_t *)pHandle->m_pBaseMalloc + pHandle->m_iBaseSize;
    }

    if ((pBlk >= (void *)pCur) && (pBlk < (void *)pCurEnd))
    {
      uint8_t *pEnd = (uint8_t *)pBlk + size;
      if (pBlk < (void *)&pCur[0])  /*cstat !MISRAC2012-Rule-18.3 probably a false positif because pointers are properly casted */
      {
        /* The block will corrupt the header */
        PMEM_MEM_ASSERT(pHandle, 0);

        return true;
      }

      /* Ok the block is in this check */
      /* Compute end of block to consider */
      if ((pCurEnd != NULL) && (pEnd > pCurEnd))
      {
        /* The block will corrupt the next chunk */
        PMEM_MEM_ASSERT(pHandle, 0);
        return true;
      }
    }
    nbFrag++;
    pCur = pmem_get_blk_next(pHandle, pCur);
    ;
  }
  PMEM_MEM_ASSERT(pHandle, (nbFrag <= (maxFrags + 1U)) ? 1U : 0U);
  PMEM_MEM_ASSERT(pHandle, (nbFrag != maxFrags) ? 1U : 0U);

  return false;
}

/**
 * @brief signals a corruption, signaled in asserts
 *
 * @param condition true ar false
 * @param pCondition string
 * @param pFile  source  file name __FILE__
 * @param line  num line __LINE__
 */
void pmem_assert(pmem_pool_t *pHandle, uint32_t condition, const char *pCondition, const char *pFile, int32_t line)
{
  if (condition == 0U)
  {
    if (pHandle == NULL)
    {
      PMEM_PRINTF("Mem corruption: condition %s : at %s:%d\r\n", pCondition, pFile, line);
    }
    else
    {
      /* Don't use pmem trace message due to the mutex */
      PMEM_PRINTF("Mem corruption:%s: condition %s : at %s:%d\r\n", (pHandle->m_pName != NULL) ? pHandle->m_pName : "", pCondition, pFile, line);
    }
    /* leave a small delay to allows DMA text dispatch */
    uint64_t     delay = (uint64_t)10000 * (uint64_t)100;
    volatile int flag  = 1;
    for (volatile uint64_t a = 0; a < delay; a++)
    {
    }
    while (flag)
      ;
  }
}

/**
 * @brief prints all fragments allocated ( debug)
 *
 * @param pHandle the instance handle
 * @param pTitle the reason of the dump
 */

void pmem_print_frags(pmem_pool_t *pHandle, const char *pTitle)
{
  uint32_t line    = 0;
  char  *pstring = NULL;
  PMEM_UNUSED(line);
  PMEM_UNUSED(pstring);

  PMEM_PRINTF("%s :  Current %lu frags, last %u frags total : %lu bytes\r\n", (pTitle == NULL) ? "Pool" : pTitle, pHandle->m_nbFrags, 0, pHandle->m_globalAlloc);

  pmem_header_t *pCur   = NULL;
  pmem_header_t *pFirst = ((pmem_header_t *)pHandle->m_pBaseMalloc);
  pCur                  = pFirst;
  int32_t nbFrag        = 0;
  while ((pCur != NULL) && (nbFrag < ((int32_t)pHandle->m_nbFrags + 30L)))
  {
    int32_t err = 1;
    PMEM_UNUSED(err);

    #ifdef PMEM_USE_SIGNATURE
    if (pCur->m_signature != PMEM_SIGNATURE)
    {
      err = 1;
    }
    #endif
    uint32_t szBlk = pmem_get_blk_size(pCur);
    if (((int32_t)szBlk < 0L) && (szBlk > pHandle->m_iBaseSize))
    {
      err = 1;
    }
    #ifdef PMEM_MALLOC_NAMED

    line    = pCur->m_line;
    pstring = pCur->m_pstring;
    #endif

    if (pmem_check_blk_flags(pCur, PMEM_BLK_FREE))
    {
      PMEM_PRINTF("poolFrag\t%s\t-\t%c\t%04ld\t%p\t%ld\t%s\t%lu\r\n",
                  pTitle,
                  (err == 0) ? '*' : ' ',
                  nbFrag,
                  pmem_get_malloc_pointer(pHandle, pCur),
                  szBlk,
                  (pstring == NULL) ? "None" : pstring,
                  line);
    }
    else
    {
      PMEM_PRINTF("poolFrag\t%s\t+\t%c\t%04ld\t%p\t%ld\t%s\t%lu\r\n",
                  pTitle,
                  (err == 0) ? '*' : ' ',
                  nbFrag,
                  pmem_get_malloc_pointer(pHandle, pCur),
                  szBlk,
                  (pstring == NULL) ? "None" : pstring,
                  line);
    }
    nbFrag++;
    pCur = pmem_get_blk_next(pHandle, pCur);
  }
}

/**
 * @brief free a block with debug options
 *
 * @param pHandle the instance handle
 * @param pBlock the block
 * @param pString the string maker
 * @param line  the integer marker
 */

void pmem_free_named(pmem_pool_t *pHandle, void *pBlock, char *pString, uint32_t  line)
{
  pmem_free(pHandle, pBlock);
}

/**
 * @brief realloc a block with debug options
 *
 * @param pHandle the instance handle
 * @param pBlock the base  block
 * @param sizeMalloc the size
 * @param pString the string maker
 * @param line  the integer marker
 */

void *pmem_realloc_named(pmem_pool_t *pHandle, void *pBlock, uint32_t sizeMalloc, char *pString, uint32_t  line)
{
  void *pMalloc = pmem_realloc(pHandle, pBlock, sizeMalloc);
  #ifdef PMEM_MALLOC_NAMED
  if (pMalloc)
  {
    pmem_get_header_pointer(pHandle, pMalloc)->m_pstring = pString;
    pmem_get_header_pointer(pHandle, pMalloc)->m_line    = line;
  }
  #endif
  return pMalloc;
}

/**
 * @brief malloc a block with debug options
 *
 * @param pHandle the instance handle
 * @param sizeMalloc the size
 * @param pString the string maker
 * @param line  the integer marker
 */

void *pmem_alloc_named(pmem_pool_t *pHandle, uint32_t sizeMalloc, char *pString, uint32_t line)
{
  void *pMalloc = pmem_alloc(pHandle, sizeMalloc);
  #ifdef PMEM_MALLOC_NAMED
  if (pMalloc)
  {
    pmem_get_header_pointer(pHandle, pMalloc)->m_pstring = pString;
    pmem_get_header_pointer(pHandle, pMalloc)->m_line    = line;
  }
  #endif
  return pMalloc;
}

/**
 * @brief calloc with debug options
 *
 * @param pHandle the instance handle
 * @param size  size element
 * @param elem  nb element
 * @param pString the string maker
 * @param line  the integer marker
 * @return void*
 */

void *pmem_calloc_named(pmem_pool_t *pHandle, uint32_t size, uint32_t elem, char *pString, uint32_t line)
{
  if (pHandle->m_iBaseSize == 0U)
  {
    return NULL;
  }

  void *pMalloc = pmem_alloc_named(pHandle, size * elem, pString, line);
  if (pMalloc == NULL)
  {
    return NULL;
  }
  memset(pMalloc, 0, size * elem);
  return pMalloc;
}


/**
* @brief Checks if ptr is a heap block.   ptr must be the pointer returned by malloc.
 * @param pHandle the instance handle
 * @param *pPtr a pointer
 * @return TRUE if found
 */

bool pmem_check_pool_ptr(pmem_pool_t *pHandle, void *pPtr)
{
  /* assume NULL is a valid pointer */
  if (pPtr == NULL)
  {
    return true;
  }
  bool result = false;
  uint8_t *pPtrU8 = (uint8_t *)pPtr;
  for (pmem_header_t *pCur = ((pmem_header_t *)pHandle->m_pBaseMalloc); pCur != NULL; pCur = pmem_get_blk_next(pHandle, pCur))
  {
    if (pmem_check_blk_flags(pCur, PMEM_BLK_FREE) == 0)
    {
      uint8_t *pBlk = pmem_get_malloc_pointer(pHandle, pCur);
      if ((pPtrU8 >= pBlk) && (pPtrU8 < (pBlk + pCur->m_szBlk)))  /*cstat !MISRAC2012-Rule-18.3 pointers comparison is OK*/
      {
        result = true;
        break;
      }
    }
  }
  return result;
}




/**
* @brief Check leak between to call of pmem_leak_detector, the first use FALSE + Tag, the second call use TRUE
 * @param pHandle the instance handle
 * @param flag  FALSE: Snap the pool situation, FALSE, print all record without the tag marker
 * @param tag   any tag != 0
 */

void pmem_leak_detector(pmem_pool_t *pHandle, uint32_t flag, uint32_t tag)
{
  #ifdef PMEM_INSTRUMENTATION
  if (pHandle->m_pBaseMalloc == NULL)
  {
    return;
  }
  if (flag == 0U)
  {
    for (pmem_header_t *pCur = ((pmem_header_t *)pHandle->m_pBaseMalloc); pCur != NULL; pCur = pmem_get_blk_next(pHandle, pCur))
    {
      pCur->m_userTag = (int16_t)tag;
    }
  }
  if (flag == 1U)
  {
    uint32_t count = 0;
    for (pmem_header_t *pCur = ((pmem_header_t *)pHandle->m_pBaseMalloc); pCur != NULL; pCur = pmem_get_blk_next(pHandle, pCur))
    {
      if ((pmem_check_blk_flags(pCur, PMEM_BLK_FREE) == 0) && (pCur->m_userTag != (int16_t)tag))
      {

        #ifdef PMEM_MALLOC_NAMED
        printf("%03d: from:%s:%d\n", count, (pCur->m_pstring == NULL) ? "" : pCur->m_pstring, pCur->m_line);
        #else
        printf("%03d: PMEM_MALLOC_NAMED unused\n", count);
        #endif

#define PMEM_NB_MAX_CHAR_TO_DUMP 10UL
        char     tString[PMEM_NB_MAX_CHAR_TO_DUMP + 1UL];
        char     tHex[(2UL * PMEM_NB_MAX_CHAR_TO_DUMP) + 1UL];
        uint8_t *pPtr         = pmem_get_malloc_pointer(pHandle, pCur);
        uint32_t nbCharToDump = PMEM_NB_MAX_CHAR_TO_DUMP;
        uint32_t size         = pmem_get_blk_size(pCur);

        if (size < nbCharToDump)
        {
          nbCharToDump = size;
        }

        for (uint32_t index = 0UL; index < nbCharToDump; index++)
        {
          sprintf(&tHex[2UL * index], "%02X", pPtr[index]);
          if (isprint(pPtr[index])) /*cstat !MISRAC2012-Dir-4.11_h !MISRAC2012-Rule-10.3  use of stdlib macro not misra compliant */
          {
            tString[index] = (char)pPtr[index];
          }
          else
          {
            tString[index] = '.';
          }
        }
        tHex[2UL * nbCharToDump] = '\0';
        tString[nbCharToDump] = '\0';
        PMEM_PRINTF("%s:%04d: %08x:%08x  %-11s %s\n", (pHandle->m_pName != NULL) ? pHandle->m_pName : "PMEM", count, pPtr, size, tString, tHex);
      }
      count++;
    }
  }
  #endif
}


#ifdef PMEM_TEST
/* This part is an app to test the API */

uint8_t     tPool[64 * 1024];
pmem_pool_t cPool;

void pmem_test()
{
  pmem_init(&cPool, tPool, sizeof(tPool), 0);
  pmem_print_frags(&cPool, "-- Start --");
  void *pAlloc = 0;
  void *pPtr;

  /* check corruption each for call*/
  cPool.m_checkFreq      = 1;
  uint32_t sizeFreeStart = pmem_get_size(&cPool, ((uint8_t *)cPool.m_pBaseMalloc) + cPool.m_szHeader);
  uint32_t sizeMaxSize   = cPool.m_iBaseSize - cPool.m_szHeader;

  void *pPtrBase = pmem_alloc(&cPool, sizeMaxSize / 2);
  if (pPtrBase != 0 && cPool.m_nbFrags != 2)
  {
    PMEM_PRINTF("Coherency error \r\n");
  }


  pPtrBase = pmem_realloc(&cPool, pPtrBase, (sizeMaxSize / 2) + 20);
  if (pPtrBase != 0 && cPool.m_nbFrags != 2)
  {
    PMEM_PRINTF("Coherency error \r\n");
  }
  pPtr = pPtrBase;

  pPtrBase = pmem_realloc(&cPool, pPtrBase, (sizeMaxSize / 2) - 20);
  if (pPtrBase != pPtr && cPool.m_nbFrags != 2)
  {
    PMEM_PRINTF("Coherency error \r\n");
  }


  pPtr = pmem_realloc(&cPool, pPtrBase, (sizeMaxSize) + 20);
  if (pPtr == 0 && cPool.m_nbFrags != 2)
  {
    PMEM_PRINTF("Coherency error \r\n");
  }
  pmem_free(&cPool, pPtrBase);
  if (cPool.m_nbFrags != 1)
  {
    PMEM_PRINTF("Coherency error \r\n");
  }

  pmem_print_frags(&cPool, "-- Check malloc/free--");
  uint32_t sizeFreeEnd = pmem_get_size(&cPool, ((uint8_t *)cPool.m_pBaseMalloc) + cPool.m_szHeader);

  if (cPool.m_nbFrags != 1 || sizeFreeEnd != sizeFreeStart || cPool.m_globalAlloc != cPool.m_szHeader)
  {
    PMEM_PRINTF("Coherency error \r\n");
  }
  else
  {
    PMEM_PRINTF("Coherency OK\r\n");
  }




  for (int a = 0; a < 1000; a++)
  {
    void    *pAlloc = 0;
    uint32_t size   = rand() % 1000;
    if ((size % 3) == 0)
    {
      pAlloc = pmem_alloc(&cPool, size);
      if (pAlloc == 0)
      {
        break;
      }
    }
    if ((size % 3) == 1)
    {
      pAlloc = pmem_realloc(&cPool, pAlloc, size * 2);
      if (pAlloc == 0)
      {
        break;
      }
    }

    if ((size % 3) == 2)
    {
      pAlloc = pmem_calloc(&cPool, 1, size);
      if (pAlloc == 0)
      {
        break;
      }
    }

    if ((size % 3) == 3)
    {
      pmem_free(&cPool, pAlloc);
    }
  }
  pmem_header_t *pFirst = ((pmem_header_t *)cPool.m_pBaseMalloc);
  pmem_header_t *pCur   = 0;
  pCur                  = pFirst;
  int32_t nbFrag        = 0;
  int     a             = 0;
  int     bQuit         = TRUE;
  /* free all allocated memory */

  while (bQuit)
  {
    pmem_header_t *pNext = pmem_get_blk_next(&cPool, pCur);
    bQuit                = !pmem_check_blk_flags(pNext, PMEM_BLK_END);
    pmem_free(&cPool, pmem_get_malloc_pointer(&cPool, pCur));
    pCur = pNext;
    a++;
  }
  pmem_print_frags(&cPool, "--- End Alloc --");
  sizeFreeEnd = pmem_get_size(&cPool, ((uint8_t *)cPool.m_pBaseMalloc) + cPool.m_szHeader);

  if (cPool.m_nbFrags != 1 || sizeFreeEnd != sizeFreeStart || cPool.m_globalAlloc != cPool.m_szHeader)
  {
    PMEM_PRINTF("Coherency error \r\n");
  }
  else
  {
    PMEM_PRINTF("Coherency OK\r\n");
  }
}
#endif

