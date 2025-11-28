/**
  ******************************************************************************
  * @file    st_pmem.h
  * @author  MCD Application Team
  * @brief   Header for st_pmem.c module
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

#ifndef __ST_PMEM_H
#define __ST_PMEM_H

#ifdef __cplusplus
extern "C"
{
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "st_pmem_conf.h"


/* Exported constants & macro --------------------------------------------------------*/
#define PMEM_SIGNATURE          ((uint16_t)0x4D50) /* "PM" */
#define PMEM_MEM_PATTERN        (0xCD)
#define PMEM_MEM_PATTERN_32BIT  (((uint32_t)PMEM_MEM_PATTERN << 24) | ((uint32_t)PMEM_MEM_PATTERN << 16) | ((uint32_t)PMEM_MEM_PATTERN << 8) | (uint32_t)PMEM_MEM_PATTERN)
#define PMEM_DEFAULT_SIZE_ALIGN 4U
#define PMEM_UNUSED(a)          ((void)(a))

#ifdef PMEM_INSTRUMENTATION
#ifndef PMEM_PERFORMANCE_INDEX
#define PMEM_PERFORMANCE_INDEX  1
#endif
#define PMEM_USE_SIGNATURE    /* Use signature and detects memory overrun */
#define PMEM_CHECK_CORRUPTION /* Check the memory corruption on fly and raise assert if corrupted*/
//#define PMEM_MALLOC_NAMED      /* Malloc + string and line to trace call origine*/
#define PMEM_USE_SIGNATURE /* Use signature and detect memory overrun */

#ifndef PMEM_PRINTF
#define PMEM_PRINTF printf
#endif
#define PMEM_SET_PATTERN(pAddr, size) memset(pAddr, PMEM_MEM_PATTERN, size);
#else // PMEM_INSTRUMENTATION
#define PMEM_PRINTF(...)      ((void)0)
#define PMEM_SET_PATTERN(...) ((void)0)

#endif // PMEM_INSTRUMENTATION

#ifndef TRUE
#define TRUE (1)
#define HAVE_TRUE
#endif

#ifndef FALSE
#define FALSE (0)
#define HAVE_FALSE
#endif

#define PMEM_MSK_ALIGN       0xFFU
#define PMEM_FLAG_EMBEDDED   0x8000U
#define PMEM_FLAG_DISABLED   0x4000U
#define PMEM_FLAG_READ_ONLY  0x2000U

#ifndef PMEM_DISABLE_IRQ
#define PMEM_DISABLE_IRQ() __disable_irq()
#endif

#ifndef PMEM_ENABLE_IRQ
#define PMEM_ENABLE_IRQ() __enable_irq()
#endif


/* Exported structures--------------------------------------------------------*/
/**
* @brief the pmem instance
*
*/
typedef struct t_pmem_pool
{
  void    *m_pBaseMalloc; /* pool base pointer */
  uint32_t m_iBaseSize;   /* total pool base */
  int32_t  m_globalAlloc; /* global size allocated in the pool */
  int32_t  m_maxAlloc;    /* Maximum size allocated in the pool */
  uint32_t m_nbFrags;     /* nb alloc fragment */
  uint32_t m_checkCount;  /* aPI call count */
  uint32_t m_checkFreq;   /* threshold api call to apply a corruption check */
  uint32_t m_flags;       /* global flags */
  uint32_t m_szAlign;     /* memory block alignment */
  uint8_t  m_szHeader;    /* block size header */
  uint32_t m_alias;       /* user field */
  uint32_t m_perfIndex;   /* nb read write ops per second */
  char    *m_pName;
  #ifdef PMEM_INSTRUMENTATION
  #ifdef PMEM_CHECK_CORRUPTION
  uint8_t m_active; /* to check pmem pool re-entrance */
  #endif
  uint8_t m_nbMaxMem2free;
  #endif
} pmem_pool_t;

/**
* @brief Info struct returned by the function pmem_get_info
*
*/
typedef struct t_pmem_Info
{
  uint32_t sizeOccuped; /* size allocated */
  uint32_t sizeFree;    /* size free */
  uint32_t AllocMax;    /* maximum allocated */
  uint32_t NumFrag;     /* number of fragments */
} pmem_Info_t;


/**
 * @brief Block header
 *
 */
typedef struct t_pmem_header
{
  #ifdef PMEM_USE_SIGNATURE
  uint16_t m_signature; /* signature to detect corruption */
  int16_t  m_userTag;
  #endif                /* PMEM_USE_SIGNATURE */
  uint32_t m_szBlk; /* block size, last 2 bits are used as flags, it is the real block size that exclude the header size */
  #ifdef PMEM_MALLOC_NAMED
  char  *m_pstring; /* if instrumented, allows to link a const string to the allocated block */
  uint32_t m_line;    /* if instrumented, allows to link a integer to the block */
  #endif
} pmem_header_t;


/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

/* Mem Pool API */
bool     pmem_check_pool_ptr(pmem_pool_t *pHandle, void *pPtr);
bool     pmem_init(pmem_pool_t *pHandle, void *pBlock, uint32_t size, uint32_t align);
int32_t  pmem_get_free_size(pmem_pool_t *pHandle);
void     pmem_reset(pmem_pool_t *pHandle);
void     pmem_term(pmem_pool_t *pHandle);
void    *pmem_alloc(pmem_pool_t *pHandle, uint32_t size);                       /* mustn't be called under interrupt */
void    *pmem_calloc(pmem_pool_t *pHandle, uint32_t size, uint32_t elem);       /* mustn't be called under interrupt */
void    *pmem_realloc(pmem_pool_t *pHandle, void *pBlock, uint32_t sizeMalloc); /* mustn't be called under interrupt */
void     pmem_free(pmem_pool_t *pHandle, void *pBlk);                           /* mustn't be called under interrupt */
uint32_t pmem_get_size(pmem_pool_t *pHandle, void *pBlk);
void     pmem_check_corruption(pmem_pool_t *pHandle);
bool     pmem_check_ptr(pmem_pool_t *pHandle, void *ptr);
bool     pmem_check_corruption_blk(pmem_pool_t *pHandle, void *pBlk, int32_t size);
void     pmem_print_frags(pmem_pool_t *pHandle, const char *pTitle);
void     pmem_get_info(pmem_pool_t *pHandle, pmem_Info_t *pInfo);
uint32_t pmem_compute_performance_index(pmem_pool_t *pHandle, uint32_t cpuFreq);
/* Optional using PMEM_MALLOC_NAMED */
void     pmem_free_named(pmem_pool_t *pHandle, void *pBlock, char *pString, uint32_t line);
void    *pmem_realloc_named(pmem_pool_t *pHandle, void *pBlock, uint32_t sizeMalloc, char *pString, uint32_t  line);
void    *pmem_alloc_named(pmem_pool_t *pHandle, uint32_t sizeMalloc, char *pString, uint32_t  line);
void    *pmem_calloc_named(pmem_pool_t *pHandle, uint32_t size, uint32_t elem, char *pString, uint32_t  line);
void     pmem_leak_detector(pmem_pool_t *pHandle, uint32_t flag, uint32_t tag);

#ifdef __cplusplus
}
#endif

#endif // __ST_PMEM_H
