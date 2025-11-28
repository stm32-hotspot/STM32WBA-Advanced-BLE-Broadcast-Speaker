/**
******************************************************************************
* @file          st_flash_storage.c
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


#include "st_flash_storage.h"
#include "string.h"
#include <cmsis_compiler.h>






/**
* @brief reset the registry
*
* @param hHandle  instance handle
* @param pFactory  factory instance
* @return error code
*/


__WEAK ST_Storage_Result st_flash_storage_init(void)
{
  return ST_STORAGE_ERROR;
}



/**
* @brief Term  persistent ( never called )
*/


__WEAK ST_Storage_Result st_flash_storage_term(void)
{
  return ST_STORAGE_ERROR;
}
/**
* @brief Lock storage
*
* @param type  type of storage
* @param mode  type of lock
* @return error code
*/


__WEAK ST_Storage_Result st_flash_storage_lock(st_flash_storage_type type, st_flash_storage_mode mode)
{
  return ST_STORAGE_ERROR;
}



/**
* @brief Read a storage
*
* @param type  type of storage
* @param flash_base_adress  offset sector
* @param pData              buffer pointer
* @param szData             buffer pointer size
* @return error code
*/

__WEAK ST_Storage_Result st_flash_storage_read(st_flash_storage_type type, uint32_t flash_base_adress, void *pData, uint32_t szData)
{
  return ST_STORAGE_ERROR;
}

/**
* @brief Erase  storage
*
* @param type               type of storage
* @return error code
*/

__WEAK ST_Storage_Result st_flash_storage_erase(st_flash_storage_type type)
{
  return ST_STORAGE_ERROR;
}

/**
* @brief Erase  sector
*
* @param type               type of storage
* @param indexSector        num sector
* @return error code
*/

__WEAK ST_Storage_Result st_flash_storage_erase_sector(st_flash_storage_type type, uint32_t indexSector)
{
  return ST_STORAGE_ERROR;
}


/**
* @brief read  sector
*@return error code
*/

__WEAK ST_Storage_Result st_flash_storage_read_sector(st_flash_storage_type type, uint32_t indexSector, void *pData, uint32_t szData)
{
  return ST_STORAGE_ERROR;
}

/**
* @brief write  sector
*@return error code
*/

__WEAK ST_Storage_Result st_flash_storage_write_sector(st_flash_storage_type type, uint32_t indexSector, void *pData, uint32_t szData)
{
  return ST_STORAGE_ERROR;
}

__WEAK ST_Storage_Result st_flash_storage_write(st_flash_storage_type type, uint32_t flash_base_adress, void *pData, uint32_t szData)
{
  return ST_STORAGE_ERROR;
}


__WEAK ST_Storage_Result  st_flash_storage_set_partition(st_flash_storage_type type, uint32_t offset, uint32_t sector, uint32_t sectorNb, uint32_t sectorSize)
{
  return ST_STORAGE_ERROR;
}
__WEAK ST_Storage_Result  st_flash_storage_get_partition(st_flash_storage_type type, uint32_t *pOffset, uint32_t *pSector, uint32_t *pSectorNb, uint32_t *pSectorSize)
{
  return ST_STORAGE_ERROR;
}

__WEAK ST_Storage_Result st_flash_storage_pre_init(void)
{
  return ST_STORAGE_ERROR;
}
