/**
  ******************************************************************************
  * @file        st_os_pool_conf_h7xxx_template.c
  * @author      MCD Application Team
  * @brief       Manage mem pool for the designer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "st_os.h"
#include "st_os_mem.h"

#ifdef ST_USE_PMEM

#include "string.h"
#include "platform_setup_conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#ifndef ST_OS_POOL_DTCM_ENABLE
  #define ST_OS_POOL_DTCM_ENABLE 0UL
#endif

#ifndef ST_OS_POOL_ITCM_ENABLE
  #define ST_OS_POOL_ITCM_ENABLE 0UL
#endif

#ifndef ST_OS_POOL_RAMx24M_ENABLE // by default disabled because left for HEAP STACK and RW_content
  #define ST_OS_POOL_RAMx24M_ENABLE (uint32_t)PMEM_FLAG_DISABLED
#endif

#ifndef ST_OS_POOL_RAMx38M_ENABLE
  #define ST_OS_POOL_RAMx38M_ENABLE (uint32_t)PMEM_FLAG_DISABLED
#endif

#ifndef ST_OS_POOL_RAMx30M_ENABLE
  #define ST_OS_POOL_RAMx30M_ENABLE 0UL
#endif

#ifndef ST_OS_POOL_HEAP_ENABLE
  #define ST_OS_POOL_HEAP_ENABLE (uint32_t)PMEM_FLAG_DISABLED
#endif

#ifndef ST_OS_POOL_RAMEXT_ENABLE
  #define ST_OS_POOL_RAMEXT_ENABLE 0UL
#endif

/* Private macros ------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static int32_t  pmem_nbPools;

static pmem_pool_t tPools[ST_Mem_Type_MAX];      /* Pool instances */
static bool        bMemPoolInt;                  /* true if mem init is done */


int32_t st_os_mem_pool_max(void)
{
  return pmem_nbPools;
}

int32_t st_os_mem_any_max(void)
{
  return pmem_nbPools;
}

pmem_pool_t *st_os_mem_pool_index(int32_t index)
{
  pmem_pool_t *pPool = NULL;
  if (index < st_os_mem_pool_max())
  {
    pPool = &tPools[index];
  }
  return pPool;
}

bool st_os_mem_pool_is_valid(pmem_pool_t *pPool)
{
  bool bResult = false;
  if (pPool)
  {
    if (((pPool->m_flags & PMEM_FLAG_DISABLED) == 0U) && (pPool->m_iBaseSize != 0U))
    {
      bResult = true;
    }
  }

  return bResult;
}


/*----------------------------------------------------------------------------*/
/*-- __weak functions overwriting - connects to st_core -------------------*/
/*----------------------------------------------------------------------------*/

/**
 * @brief  return the os type from the local pool type
 *
 * @param type memory type
 */

int32_t st_os_find_pool(st_mem_type type)
{
  for (int32_t index = 0; index < st_os_mem_pool_max(); index++)
  {
    if (tPools[index].m_alias == (uint32_t)type)
    {
      return index;
    }
  }
  return -1;
}



void st_os_mem_set_ext_pool_index(int32_t extIndexPool)
{
  pmem_nbPools = extIndexPool;
}



bool st_os_mem_create_pool(st_mem_type osPool, uint32_t offPool, uint32_t szPool, char *pPoolName, uint32_t flgAlign)
{
  bool bResult = false;
  if (szPool)
  {
    void *pPool = (void *)offPool; /*cstat !MISRAC2012-Rule-11.6 the offset comes from the memory mapping and is perfectly aligned */
    pmem_init(&tPools[pmem_nbPools], pPool, szPool, flgAlign);
    tPools[pmem_nbPools].m_pName = pPoolName;
    tPools[pmem_nbPools].m_alias = (uint32_t)osPool;
    pmem_compute_performance_index(&tPools[pmem_nbPools], HAL_GetCpuClockFreq());
    pmem_nbPools++;
  }
  return bResult;
}

/**
 * @brief  overloaded: default Pool creation for H7 family
          st_board.h must be configured according to the board

 *
 */

void st_os_mem_init(void)
{
  if (bMemPoolInt)
  {
    return;
  }
  bMemPoolInt = true;


  /* Create pools */
  st_os_mem_create_pool(ST_Mem_Type_POOL1, POOL_DTCM_BASE,    POOL_DTCM_SIZE,    "DTCM",    ST_OS_POOL_DTCM_ENABLE);
  st_os_mem_create_pool(ST_Mem_Type_POOL2, POOL_ITCM_BASE,    POOL_ITCM_SIZE,    "ITCM",    ST_OS_POOL_ITCM_ENABLE);
  st_os_mem_create_pool(ST_Mem_Type_POOL3, POOL_RAMx38M_BASE, POOL_RAMx38M_SIZE, "RAMINT1", ST_OS_POOL_RAMx38M_ENABLE);
  st_os_mem_create_pool(ST_Mem_Type_POOL4, POOL_RAMx30M_BASE, POOL_RAMx30M_SIZE, "RAMINT2", ST_OS_POOL_RAMx30M_ENABLE);
  st_os_mem_create_pool(ST_Mem_Type_POOL5, POOL_RAMx24M_BASE, POOL_RAMx24M_SIZE, "RAMINT3", ST_OS_POOL_RAMx24M_ENABLE);
  st_os_mem_create_pool(ST_Mem_Type_POOL6, RAMEXT_BASE,       RAMEXT_SIZE,       "RAMEXT",  ST_OS_POOL_RAMEXT_ENABLE);
  st_os_mem_create_pool(ST_Mem_Type_HEAP,  0UL,               0UL,               "HEAP",    ST_OS_POOL_HEAP_ENABLE);

  st_os_mem_init_ext(pmem_nbPools);
}

/**
 * @brief overloaded: Term mem pools
 *
 */

void st_os_mem_term(void)
{
  for (int32_t a = 0; a < st_os_mem_pool_max(); a++)
  {
    pmem_term(&tPools[a]);
  }
  bMemPoolInt = false;
}

/**
 * @brief  overloaded: returns the pool according to the pointer
 *
 * @param pMem
 * @return pmem_pool_t*
 */

pmem_pool_t *st_os_mem_pool_from_ptr(void *pMem)
{
  /* Check Block allocated first otherwise return the wrong pool */
  uint32_t szPool = 0xFFFFFFFF;
  pmem_pool_t *pPool = NULL;

  for (int32_t index = 0; index < st_os_mem_pool_max(); index++)
  {
    if (pmem_check_ptr(&tPools[index], pMem))
    {
      /* manage the fact that a pool can be inside another pool, hence, several solutions
         so, the winner is the smaller */
      if (szPool >= tPools[index].m_iBaseSize)
      {
        szPool  = tPools[index].m_iBaseSize;
        pPool   = &tPools[index];
      }
    }
  }
  return pPool;
}
#endif
