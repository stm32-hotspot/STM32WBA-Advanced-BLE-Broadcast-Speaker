/**
******************************************************************************
* @file          st_os_mem.h
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



#ifndef ST_OS_MEM_H
#define ST_OS_MEM_H

/* Includes ------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdbool.h>
#include "st_os_mem_conf.h"
#include "st_pmem.h"
#ifdef __cplusplus
extern "C"
{
#endif


/* Exported types ------------------------------------------------------------*/

/*!
@brief Memory pool definitions
@ingroup enum
*/
typedef enum st_mem_type
{
  ST_Mem_Type_POOL1, /*!< A memory pool */
  ST_Mem_Type_POOL2, /*!< A memory pool */
  ST_Mem_Type_POOL3, /*!< A memory pool */
  ST_Mem_Type_POOL4, /*!< A memory pool */
  ST_Mem_Type_POOL5, /*!< A memory pool */
  ST_Mem_Type_POOL6, /*!< A memory pool */
  ST_Mem_Type_POOL7, /*!< A memory pool */
  ST_Mem_Type_POOL8, /*!< A memory pool */
  ST_Mem_Type_POOL9, /*!< A memory pool */
  ST_Mem_Type_POOL10, /*!< A memory pool */
  ST_Mem_Type_POOL11, /*!< A memory pool */
  ST_Mem_Type_POOL12, /*!< A memory pool */
  ST_Mem_Type_POOL13, /*!< A memory pool */
  ST_Mem_Type_POOL14, /*!< A memory pool */
  ST_Mem_Type_POOL15, /*!< A memory pool */
  ST_Mem_Type_HEAP,  /*!< Normal heap */
  ST_Mem_Type_MAX,   /*!< Nb Pool max */
  ST_Mem_Type_ANY_FAST = 0x80U,  /*!< Try to alloc the best pool*/
  ST_Mem_Type_ANY_SLOW = 0x81U,  /*!< Try to alloc from the slower to the faster */
  ST_Mem_Type_UNMAPPED  /*!< The pool is unmapped */
} st_mem_type;


#define ST_Mem_is_any(pool) (((pool) == ST_Mem_Type_ANY_FAST) || ((pool) == ST_Mem_Type_ANY_SLOW))


/*!
@brief returns some information concerning the system
@ingroup struct
Returns some information concerning the system

*/
#define st_os_mem_unmapped      1UL
#define st_os_mem_hidden        2UL

typedef struct st_os_mem_pool
{
  uint32_t iPoolFreeSize; /*!< free size in the pool*/
  uint32_t iPoolSize;     /*!< max size in the pool*/
  uint32_t iPoolMaxAlloc; /*!< max size allocated in the pool*/
  uint8_t  iPoolEmbedded; /*!< if the pool is embedded in another pool*/
  uint32_t iPoolFrag;     /*!< number of fragment */
  uint32_t iFlags;
  uint32_t iPoolOffset;
  uint32_t iPerf;

  char     *pName;
} st_os_mem_pool;



typedef struct st_os_mem_status
{
  uint32_t iMemBssSpace;    /*!< BSS Section size */
  uint32_t iMemFlashSpace;  /*!< Flash Section size */
  uint32_t iMemDataSpace;   /*!< DATA Section size */
  uint32_t iMemHiddenSpace; /*!< Memory hard mapped and not visible by the system*/
  st_os_mem_pool tPools[ST_Mem_Type_MAX];
  st_os_mem_pool tStaticPools[3];

} st_os_mem_status;




/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
struct st_os_mem_info;
/* OS abstraction */

/* memory API */
pmem_pool_t *st_os_mem_pool_from_ptr(void *pMem);
pmem_pool_t *st_os_mem_pool_from_type(st_mem_type type);
void         st_os_mem_generic_free(void *ptr);
void        *st_os_mem_generic_malloc(size_t size);
void        *st_os_mem_generic_malloc_slow(size_t size);
void        *st_os_mem_generic_realloc_fast(void *pMemory, size_t size);
void        *st_os_mem_generic_realloc_slow(void *pMemory, size_t size);
void         st_os_mem_init(void);
void         st_os_mem_term(void);
void         st_os_mem_free(void *pMemToFree);
void         st_os_mem_reset(st_mem_type type);
void        *st_os_mem_alloc_named(st_mem_type type, size_t size, char *pString, uint32_t line);
void        *st_os_mem_calloc_named(st_mem_type type, size_t size, size_t elem, char *pString, uint32_t line);
void        *st_os_mem_realloc_named(st_mem_type type, void *pBuffer, size_t size, char *pString, uint32_t line);
void        *st_os_mem_check_corruption(void *pBase);
st_mem_type  st_os_mem_type_from_name(const char *pPoolName);
void         st_os_mem_info(struct st_os_mem_status **ppSysInfo);
void         st_os_mem_update_string(char **pString, const char *pNewString);
bool         st_os_mem_leak_detector(st_mem_type type, uint32_t flag, uint32_t tag);
bool         st_os_mem_check_ptr(st_mem_type type, void *pPtr);
bool         st_os_mem_check_heap_ptr(void *pHeap);
uint32_t     st_os_mem_size_ptr(void *pMem);
void         st_os_mem_unlock(void);
void         st_os_mem_lock(void);
bool         st_os_mem_create_pool(st_mem_type osPool, uint32_t offPool, uint32_t szPool, char *pPoolName, uint32_t flgAlign);
void         st_os_mem_init_ext(int32_t extIndexPool);
void         st_os_mem_set_ext_pool_index(int32_t extIndexPool);


// pool management interface
bool         st_os_mem_section_info(struct st_os_mem_status **ppSysInfo);
int32_t      st_os_mem_pool_max(void);
int32_t      st_os_mem_any_max(void);
int32_t      st_os_find_pool(st_mem_type type);
pmem_pool_t *st_os_mem_pool_index(int32_t index);
bool         st_os_mem_pool_is_valid(pmem_pool_t *pPool);



/* Pass the name of the object to FreeRtos in debug for trace analyzer*/
#ifdef ST_USE_DEBUG
#define st_os_mem_alloc(type, size)            st_os_mem_alloc_named(type, size, __FILE__, __LINE__)
#define st_os_mem_calloc(type, size, elem)     st_os_mem_calloc_named(type, size, elem, __FILE__, __LINE__)
#define st_os_mem_realloc(type, pBuffer, size) st_os_mem_realloc_named(type, pBuffer, size, __FILE__, __LINE__)
#else
#define st_os_mem_alloc(type, size)            st_os_mem_alloc_named(type, size, NULL, 0)
#define st_os_mem_calloc(type, size, elem)     st_os_mem_calloc_named(type, size, elem, NULL, 0)
#define st_os_mem_realloc(type, pBuffer, size) st_os_mem_realloc_named(type, pBuffer, size, NULL, 0)
#endif
#ifdef __cplusplus
};
#endif



#endif

