/**
******************************************************************************
* @file          st_registry.c
* @author        MCD Application Team
* @brief         registry management
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
#include <string.h>
#include "st_registry.h"
#include "st_os_mem.h"
#include "st_flash_storage.h"
#include <stdio.h>



/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#ifndef ST_PERSISTENT_REGISTRY
  #error  "The partition ST_PERSISTENT_REGISTRY must be defined"
#endif

/* Private typedef -----------------------------------------------------------*/


#ifdef ST_USER_REGISTRY_CHECKSUM

/**
 * @brief compute a checksum
 *
 * @param pCheck  the data to check
 * @param szCheck  the data size
 * @return uint32_t
 */
static uint32_t st_registry_checksum(uint8_t *pCheck, uint32_t szCheck)
{
  uint32_t checksum = 0;
  for (uint32_t a = 0; a < szCheck; a++)
  {
    checksum += *pCheck++;
  }
  return checksum;
}
#endif


__weak ST_Registry_Result st_registry_reset_default(st_registry *pHandle, st_persist_sys *pRegistry)
{
  return ST_REGISTRY_OK;
}



/**
 * @brief init the core registry
 *
 * @param pHandle the instance handle
 * @return ST_Registry_Result
 */
ST_Registry_Result st_registry_create(st_registry *pHandle)
{
  ST_ASSERT(pHandle != NULL);

  memset(pHandle, 0, sizeof(*pHandle));
  ST_VERIFY((st_os_mutex_create(&pHandle->systemLock) == ST_OS_OK));
  return ST_REGISTRY_OK;
}


ST_Registry_Result       st_registry_is_valid(st_registry *pHandle)
{
  ST_Registry_Result result = ST_REGISTRY_OK;
  st_persist_sys *pRegistry = st_registry_lock_sys(pHandle);
  if (pRegistry == NULL)
  {
    return ST_REGISTRY_ERROR;
  }
  result = (pRegistry->iSignature == ST_PERSIST_SIGNATURE) ? ST_REGISTRY_OK : ST_REGISTRY_ERROR;
  st_registry_unlock_sys(pHandle, 0);
  return result;
}

/**
 * @brief terminate the core registry
 *
 * @param pHandle the instance handle
 * @return ST_Registry_Result
 */
ST_Registry_Result st_registry_delete(st_registry *pHandle)
{
  st_os_mutex_delete(&pHandle->systemLock);
  return ST_REGISTRY_OK;
}

ST_Registry_Result st_registry_erase(st_registry *pHandle)
{
  ST_Registry_Result result    = ST_REGISTRY_OK;
  st_persist_sys    *pRegistry = NULL;
  ST_Storage_Result  res       = st_flash_storage_erase(ST_PERSISTENT_REGISTRY);

  if (res == ST_STORAGE_OK)
  {
    pRegistry = st_os_mem_alloc(ST_Mem_Type_Registry, sizeof(st_persist_sys));
    if (pRegistry == NULL)
    {
      result = ST_REGISTRY_ERROR;
    }
  }
  else
  {
    result = ST_REGISTRY_ERROR;
  }
  if (result == ST_REGISTRY_OK)
  {
    memset(pRegistry, 0, sizeof(*pRegistry));
    pRegistry->iSignature = ST_PERSIST_SIGNATURE;
    st_registry_reset_default(pHandle, pRegistry);

    #ifdef ST_USER_REGISTRY_CHECKSUM
    uint32_t sizeSys = (uint32_t)&pRegistry->end_encryption - (uint32_t)pRegistry->pSystemRAM;
    /*
      We write the registry , so we compute the checksum
      When we will boot the board, we will be able to know if the registry has been corrupted
    */
    pHandle->pSystemRAM->checksum = st_registry_checksum(pRegistry, sizeSys);
    #endif
    result = (st_flash_storage_write(ST_PERSISTENT_REGISTRY, 0, pRegistry, sizeof(*pRegistry)) == ST_STORAGE_OK) ? ST_REGISTRY_OK : ST_REGISTRY_ERROR;
  }
  st_os_mem_free(pRegistry);
  pRegistry = NULL;

  return result;
}


/**
 * @brief Locks the registry for read or write
 *
 * @param pHandle  the instance handle
 * @return st_persist_sys*
 */

st_persist_sys *st_registry_lock_sys(st_registry *pHandle)
{
  if (pHandle->pSystemRAM == 0)
  {
    st_persist_sys *pRegistry = st_os_mem_alloc(ST_Mem_Type_Registry, sizeof(st_persist_sys));
    if (pRegistry == NULL)
    {
      return NULL;
    }
    st_persist_sys *pRegistryBase;
    ST_Storage_Result err = st_flash_storage_get_partition(ST_PERSISTENT_REGISTRY, (uint32_t *)&pRegistryBase, NULL, NULL, NULL);
    if (err !=  ST_STORAGE_OK)
    {
      st_os_mem_free(pRegistry);
      return NULL;
    }
    memcpy(pRegistry, pRegistryBase, sizeof(st_persist_sys));
    pHandle->pSystemRAM = pRegistry;
  }
  st_os_mutex_lock(&pHandle->systemLock);
  return pHandle->pSystemRAM;
}

/**
 * @brief unlocks the registry  and write if it is modified
 *
 * @param pHandle the instance handle
 * @param bModified true if modified
 * @return ST_Registry_Result
 */

ST_Registry_Result st_registry_unlock_sys(st_registry *pHandle, uint32_t bModified)
{
  ST_Registry_Result result = ST_REGISTRY_OK;

  if ((bModified != 0UL))
  {
    #ifdef ST_USER_REGISTRY_CHECKSUM
    uint32_t sizeSys = (uint32_t)&pHandle->pSystemRAM->end_encryption - (uint32_t)pHandle->pSystemRAM;
    /*
    We write the registry , so we compute the checksum
    When we will boot the board, we will be able to know if the registry has been corrupted
    */
    uint8_t *pCheck               = (uint8_t *)pHandle->pSystemRAM;
    pHandle->pSystemRAM->checksum = st_registry_checksum(pCheck, sizeSys);
    #endif
    ST_Storage_Result err = st_flash_storage_erase(ST_PERSISTENT_REGISTRY);
    if (err == ST_STORAGE_OK)
    {
      err = st_flash_storage_write(ST_PERSISTENT_REGISTRY, 0, pHandle->pSystemRAM, sizeof(*pHandle->pSystemRAM));
    }
    if (err  == ST_STORAGE_OK)
    {
      result = ST_REGISTRY_OK;
    }
    ST_REGISTRY_TRACE_DEBUG("Write system registry %d", result);
  }
  if (pHandle->pSystemRAM)
  {
    st_os_mem_free(pHandle->pSystemRAM);
    pHandle->pSystemRAM = NULL;
  }
  st_os_mutex_unlock(&pHandle->systemLock);
  return result;
}



