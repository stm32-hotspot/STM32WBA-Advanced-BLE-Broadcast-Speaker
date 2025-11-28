/**
******************************************************************************
* @file          st_flash_storage.h
* @author        MCD Application Team
* @brief         persistent storage
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

#ifndef ST_FLASH_STORAGE_H
#define ST_FLASH_STORAGE_H
/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
#include "st_flash_storage_conf.h"
#include "stdarg.h"
#include "stdint.h"
#include "stdio.h"


#ifdef __cplusplus
extern "C"
{
#endif
#ifndef ST_STORAGE_OK
#define   ST_STORAGE_OK     1
#endif
#ifndef ST_STORAGE_ERROR
#define   ST_STORAGE_ERROR  0
#endif

#ifndef ST_STORAGE_TRACE_ERROR
#define ST_STORAGE_TRACE_ERROR(...) {printf(__VA_ARGS__);printf("\n");}
#endif
#ifndef  ST_STORAGE_TRACE_INFO
#define ST_STORAGE_TRACE_INFO(...)  {printf(__VA_ARGS__);printf("\n");}
#endif

#ifndef ST_STORAGE_VERIFY
#define ST_STORAGE_VERIFY(a)  ((void)(a))
#endif

#ifndef ST_STORAGE_ASSERT
#define ST_STORAGE_ASSERT(a)
#endif


typedef int32_t ST_Storage_Result ;


/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/


typedef enum st_flash_storage_mode
{
  st_flash_storage_LOCK_PROG,
  st_flash_storage_LOCK_MAP,
} st_flash_storage_mode;



/* Global variables ----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */
ST_Storage_Result st_flash_storage_lock(st_flash_storage_type type, st_flash_storage_mode mode);
ST_Storage_Result st_flash_storage_read(st_flash_storage_type type, uint32_t flash_base_adress, void *pData, uint32_t szData);
ST_Storage_Result st_flash_storage_read_sector(st_flash_storage_type type, uint32_t indexSector, void *pData, uint32_t szData);
ST_Storage_Result st_flash_storage_erase(st_flash_storage_type type);
ST_Storage_Result st_flash_storage_erase_sector(st_flash_storage_type type, uint32_t indexSector);
ST_Storage_Result st_flash_storage_write(st_flash_storage_type type, uint32_t flash_base_adress, void *pData, uint32_t szData);
ST_Storage_Result st_flash_storage_write_sector(st_flash_storage_type type, uint32_t indexSector, void *pData, uint32_t szData);
ST_Storage_Result st_flash_storage_pre_init(void);
ST_Storage_Result st_flash_storage_init(void);
ST_Storage_Result st_flash_storage_term(void);
ST_Storage_Result st_flash_storage_set_partition(st_flash_storage_type type, uint32_t offset, uint32_t sector, uint32_t size, uint32_t sectorSize);
ST_Storage_Result st_flash_storage_get_partition(st_flash_storage_type type, uint32_t *pOffset, uint32_t *pSector, uint32_t *pPartitionSize, uint32_t *pSectorSize);

/* overload init XSPI */
void XSPI_DeInit(void);
void XSPI_Init(bool enable_mem_map);


#ifdef __cplusplus
};
#endif
#endif //PERSISTANT_STORAGE_H


