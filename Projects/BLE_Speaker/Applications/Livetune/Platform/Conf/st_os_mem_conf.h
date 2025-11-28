/**
******************************************************************************
* @file          st_os_mem_conf.h
* @author        MCD Application Team
* @brief         some defines and includes for memory support
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



#ifndef __ST_OS_MEM_CONF_H_
#define __ST_OS_MEM_CONF_H_

#if defined(STM32H573xx)


  #define RAMEXT_BASE_THF 0UL   // (0x70000000UL + (1UL * 1024UL * 1024UL)) /* 1 MB reserved THF */
  #define RAMEXT_SIZE_THF 0UL   // (1UL * 1024UL * 1024UL)

  #define RAMEXT_BASE     0UL   // (0x70000000UL + (1UL * 1024UL * 1024UL)) /* 5 MB reserved for the designer*/
  #define RAMEXT_SIZE     0UL   // (1UL * 1024UL * 1024UL)

  #define RAMEXT_BASE_TR  0UL   // (0x70000000UL + (2UL * 1024UL * 1024UL)) /* 10 MB for trace alyzer (7060 0000) */
  #define RAMEXT_SIZE_TR  0UL   // (10UL * 1024UL * 1024UL)


  /*
  Some MW use malloc to get memory blocks, ST_Result use a memory manager able to
  allocate memory in several pools, one of these pools is the HEAP and use malloc.
  To prevent the heap empty, and MW allocating using malloc gets a NULL pointer
  the ST_Result  allocator, allocs until the pool is full less POOL_HEAP_LIMIT
  in order do not starve some MW using malloc. This mechanism guarantees that the POOL_HEAP_LIMIT space will be available
  for MW and ST_Result won't use it.For example, WHD wifi driver uses mallocs.
  In the current ST_Result context POOL_HEAP_LIMIT value is the best compromise.
  The exact usage of the HEAP can be checked with the GUI memory pools usage tab

  */
  #define POOL_HEAP_LIMIT (10 * 1024) /* disable if 0 , size of heap memory not allocated by livetune_db_find_instance_index_from_name and left to the standard API using malloc */
#else
  #error "platform not supported"
#endif

#endif



