/**
******************************************************************************
* @file          platform_flash.c
* @author        MCD Application Team
* @brief         Flash ext adaption

persistent  storage   adaptation
the flash management varies from a board to another
an app must overload service_persistent_erase_storage & service_persistent_erase_storage
to support the persistent storage
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
#include <stdlib.h>
#include <stdbool.h>
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_xspi.h"
#include "st_os.h"
#include "st_flash_storage.h"

#define   USE_INTERNAL_FLASH    0


/* Private defines -----------------------------------------------------------*/
/*#define TEST_FLASH_EXT*/

#define KILO                               (1024UL)

#define FLASHEXT_BASE_ADDRESS              XSPI2_BASE // could be XSPI2_BASE XSPI3_BASE
#define FLASHEXT_INSTANCE                  0


//#define MEMORYMAP_WORKAROUND

#define DUAL_BANK_H7
#define PROG_PAGE_SIZE_INT                 (32U)
#define PROG_PAGE_SIZE_EXT                 256
#define FLASH_TYPE_PROG                    FLASH_TYPEPROGRAM_FLASHWORD
#define FLASH_VOLTAGE(str)                 (str).VoltageRange = FLASH_VOLTAGE_RANGE_4



/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/


typedef struct t_flash_storage
{
  uint32_t offset;
  uint32_t sector;
  uint32_t sectorNb;
  uint32_t sectorSize;
} flash_storage_t;

/* Global variables ----------------------------------------------------------*/
static st_flash_storage_mode iFlashExtLocked = st_flash_storage_LOCK_PROG;
/* flash ext  info */
static st_mutex   hMutexStorage;

/* Partition handles */
static flash_storage_t tFlashStorage[st_flash_storage_MAX];


/* Private functions ------------------------------------------------------- */

static uint32_t platform_persistent_size_partition(st_flash_storage_type type);
static uint32_t platform_persistent_ptr_partition(st_flash_storage_type type, uint32_t sector);
static uint32_t platform_persistent_is_flashExt(st_flash_storage_type type);
static uint32_t platform_persistent_align_offset(uint32_t offset, uint32_t blocksize);
static ST_Storage_Result platform_persistent_storage_write_flashExt(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
static ST_Storage_Result platform_persistent_storage_read_flashExt(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
static ST_Storage_Result platform_persistent_storage_erase_flashExt(st_flash_storage_type type);
static ST_Storage_Result platform_persistent_storage_erase_internal_flash(st_flash_storage_type type);
static ST_Storage_Result platform_persistent_storage_read_internal_flash(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
static ST_Storage_Result platform_persistent_storage_write_internal_flash(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
ST_Storage_Result  platform_persistent_storage_is_locked(st_flash_storage_type type, st_flash_storage_mode mode);
static ST_Storage_Result platform_check_flash_erased(uint8_t *pByte, uint32_t size);



#if !defined(ST_NO_FLASHEXT)
static bool  bXSPI_InitDone = false;
static BSP_XSPI_NOR_Init_t hInstanceFlashExt;
static BSP_XSPI_NOR_Info_t hInfoFlashExt;
#define XSPI_INSTANCE 0UL



/**
* @brief  Initialize extFlash support.
* @param  None
* @retval None
*/

__weak void XSPI_Init(bool  enable_mem_map)
{
  if (!bXSPI_InitDone)
  {
    bXSPI_InitDone = true;

    #if !defined(ST_NO_FLASHEXT)
    /* Flash ext device configuration */
    hInstanceFlashExt.InterfaceMode = BSP_XSPI_NOR_OPI_MODE;
    hInstanceFlashExt.TransferRate  = BSP_XSPI_NOR_DTR_TRANSFER;
    /* the flash ext must exist, otherwise assert */
    if (BSP_XSPI_NOR_Init(XSPI_INSTANCE, &hInstanceFlashExt) != BSP_ERROR_NONE)
    {
      while (1);
    }
    if (enable_mem_map == true)
    {
      if (BSP_XSPI_NOR_EnableMemoryMappedMode(XSPI_INSTANCE) != BSP_ERROR_NONE)
      {
        while (1);
      }
    }

    MODIFY_REG(XSPI2->CR, XSPI_CR_NOPREF, HAL_XSPI_AUTOMATIC_PREFETCH_DISABLE); // Hotfix for xspi: no prefetch


    /*  Note: before to set a risaf for a given IP, the IP
    *        should be cloked.
    */

    RISAF12_S->REG[0].CFGR    = 0x00000000;
    RISAF12_S->REG[1].CFGR    = 0x00000000;
    // risaf->REG[0].CIDCFGR  = 0x00FF00FF;   /* RW for everyone */
    RISAF12_S->REG[0].CIDCFGR = 0x000F000F;   /* RW for everyone */
    RISAF12_S->REG[0].ENDR    = 0xFFFFFFFF;   /* all-encompassing */
    RISAF12_S->REG[0].CFGR    = 0x00000101;   /* enabled, secure, unprivileged for everyone */
    RISAF12_S->REG[1].CIDCFGR = 0x00FF00FF;   /* RW for everyone */
    RISAF12_S->REG[1].ENDR    = 0xFFFFFFFF;   /* all-encompassing */
    RISAF12_S->REG[1].CFGR    = 0x00000001;   /* enabled, non-secure, unprivileged for everyone */
    BSP_XSPI_NOR_GetInfo(XSPI_INSTANCE, &hInfoFlashExt);
    #endif
  }
}


/**
* @brief  Initialize extFlash support.
* @param  None
* @retval None
*/
__weak void XSPI_DeInit(void)
{
  if (bXSPI_InitDone)
  {
    bXSPI_InitDone = false;
    #if !defined(ST_NO_FLASHEXT)
    if (BSP_XSPI_NOR_DeInit(XSPI_INSTANCE) != BSP_ERROR_NONE)
    {
      while (1);
    }
    #endif
  }
}

#endif


#ifdef TEST_FLASH_EXT

static ST_Storage_Result platform_check_pattern(uint8_t *pSector, uint32_t szSector)
{
  ST_Storage_Result err = ST_STORAGE_OK;
  for (int32_t sz = 0; sz < szSector ; sz++)
  {
    uint8_t check = 'a' + (sz % 24);
    if (pSector[sz] != check)
    {
      ST_STORAGE_TRACE_ERROR("Check flash not written : %08x = %02x instead of %02x", &pSector[sz], pSector[sz], check);
      err = ST_STORAGE_ERROR;
      break;
    }
  }
  return err;
}


static void platform_test_flash_conf(void)
{
  for (int32_t type = 0 ; type < st_flash_storage_MAX ; type++)
  {
    ST_STORAGE_TRACE_INFO("Test Partition %d", type);

    uint32_t offset, baseSector, nbSector, szSector;
    ST_STORAGE_VERIFY(st_flash_storage_get_partition((st_flash_storage_type)type, &offset, &baseSector, &nbSector, &szSector) == ST_STORAGE_OK);
    ST_STORAGE_VERIFY(st_flash_storage_erase((st_flash_storage_type)type) == ST_STORAGE_OK);
    ST_STORAGE_VERIFY(platform_check_flash_erased((void *)offset, nbSector * szSector * KILO) == ST_STORAGE_OK);
    uint8_t *pSector = malloc(szSector * KILO);
    ST_STORAGE_VERIFY(pSector != NULL);
    int32_t count = 0;

    for (uint32_t sector = 0; sector < nbSector ; sector++)
    {
      ST_STORAGE_TRACE_INFO("Write sector %d", sector);

      for (int32_t sz = 0; sz < szSector * KILO ; sz++) { pSector[sz] = 'a' + (count ++ % 24); }
      ST_STORAGE_VERIFY(st_flash_storage_write((st_flash_storage_type)type, sector * szSector * KILO, pSector, szSector * KILO) == ST_STORAGE_OK);
    }
    ST_Storage_Result err = platform_check_pattern((void *)offset, nbSector * szSector * KILO);
    ST_STORAGE_VERIFY(err == ST_STORAGE_OK);
    free(pSector);
    ST_STORAGE_TRACE_INFO("Test Finished");
    HAL_Delay(100);
  }
}

#endif // TEST_FLASH_EXT


/**
* @brief  Pre-init is used to initialize the low level Flash Ext without mutex, because Thread doesn't
allow to create mutext before the kernel is started
and we need a mutex toprevent write/erase concurance
*/
ST_Storage_Result st_flash_storage_pre_init(void)
{
  #if !defined(ST_NO_FLASHEXT)
  XSPI_Init(true);
  iFlashExtLocked = st_flash_storage_LOCK_MAP;
  #endif
  return ST_STORAGE_OK;
}


/**
* @brief  Configure external flash uses the Flash ext
*/
ST_Storage_Result st_flash_storage_init(void)
{
  ST_STORAGE_VERIFY(st_os_mutex_create(&hMutexStorage) == ST_OS_OK);
  ST_Storage_Result result = st_flash_storage_pre_init();
  #ifdef TEST_FLASH_EXT
  platform_test_flash_conf();
  #endif
  return result;
}


#if USE_INTERNAL_FLASH == 1

/*
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t platform_config_persistent_storage_get_bank(st_flash_storage_type type, uint32_t sector)
{
  return FLASH_BANK_1;
}

static uint32_t platform_config_persistent_storage_get_sector(st_flash_storage_type type, uint32_t sector)
{
  return sector;
}

#endif // USE_INTERNAL_FLASH == 1


/**
* @brief  Primitive erase internal flash
* @param  curSec  sector to erase
* @retval error code
*/
static ST_Storage_Result platform_erase_internal_sector(st_flash_storage_type type, uint32_t curSec)
{
  #if USE_INTERNAL_FLASH == 1
  uint32_t SectorError = 0;

  FLASH_EraseInitTypeDef EraseInitStruct = {0};
  EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
  FLASH_VOLTAGE(EraseInitStruct);
  EraseInitStruct.Sector       = platform_config_persistent_storage_get_sector(type, curSec);
  EraseInitStruct.Banks        = platform_config_persistent_storage_get_bank(type, curSec);
  EraseInitStruct.NbSectors    = 1;
  //st_os_enter_critical_section();
  HAL_FLASH_Unlock();
  HAL_StatusTypeDef result = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
  HAL_FLASH_Lock();
  //st_os_exit_critical_section();
  return (result == HAL_OK) ? ST_STORAGE_OK : ST_STORAGE_ERROR;
}


/**
* @brief  Primitive write internl flash
* @param  offset    offset to write
* @param  pSlot     data pointer
* @retval error code
*/
static ST_Storage_Result platform_write_internal_sector(uint32_t offset, void *pSlot)
{
  ST_Storage_Result ret;
  /* flash the slot */
  //st_os_enter_critical_section();
  if (HAL_FLASH_Program(FLASH_TYPE_PROG, offset, (uint32_t)pSlot) != HAL_OK)/*cstat !MISRAC2012-Rule-11.6 the HAL API expect an offset rather than a pointer, ie we need to cast a pointer as int */
  {
    ret = ST_STORAGE_ERROR;
  }
  else
  {
    ret = ST_STORAGE_OK;
  }
  //st_os_exit_critical_section();

  return ret;
  #else
  return  ST_STORAGE_ERROR;
  #endif
}


/**
* @brief  return true if the partition belongs to the external flash
* @param  type      slot index
* @retval 0 o 1
*/
static uint32_t platform_persistent_is_flashExt(st_flash_storage_type type)
{
  #if !defined(ST_NO_FLASHEXT)
  if ((tFlashStorage[type].offset >= FLASHEXT_BASE_ADDRESS) && (tFlashStorage[type].offset < (FLASHEXT_BASE_ADDRESS + hInfoFlashExt.FlashSize)))
  {
    return true;
  }
  #endif
  return false;
}


/**
* @brief  return the partition size
* @param  type    slot type
* @retval slot size
*/
static uint32_t platform_persistent_size_partition(st_flash_storage_type type)
{
  return tFlashStorage[type].sectorNb * tFlashStorage[type].sectorSize;
}


/**
* @brief  return the offset rounded to a sector
* @param  type    slot type
* @param  sector  sector offset
* @retval slot size
*/
static uint32_t platform_persistent_ptr_partition(st_flash_storage_type type, uint32_t sector)
{
  return tFlashStorage[type].offset + (sector * tFlashStorage[type].sectorSize * KILO);
}


/**
* @brief  Check if the flash is empty after erasing
* @param  pByte   pointer sector
* @param  size    sector size
* @retval error code
*/
static ST_Storage_Result platform_check_flash_erased(uint8_t *pByte, uint32_t size)
{
  for (uint32_t a = 0; a < size; a++)
  {
    if (pByte[a] != 0xFFU)
    {
      ST_STORAGE_TRACE_ERROR("Check  flash not erased : %08x = %x", &pByte[a], pByte[a]);
      return ST_STORAGE_ERROR;
    }
  }
  return ST_STORAGE_OK;
}


/**
* @brief  Align an address according to the block size
* @param  offset   offset sector
* @param  blocksize  sector size
* @retval the sector aligned
*/
static uint32_t platform_persistent_align_offset(uint32_t offset, uint32_t blocksize)
{
  uint32_t msk = ~(blocksize - 1U);
  return  offset & msk;
}


/**
* @brief  write data in a partition  external flash  using flashExt
* @param  type    slot type
* @param  baseOffset   offset sector
* @param  pData       sector pointer
* @param  szData   sector pointer size
* @retval error code
*/
static ST_Storage_Result platform_persistent_storage_write_flashExt(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData)
{
  #if !defined(ST_NO_FLASHEXT)

  if (iFlashExtLocked != st_flash_storage_LOCK_PROG)
  {
    ST_STORAGE_TRACE_ERROR("Lock FlashExt");
    return ST_STORAGE_OK;
  }

  uint32_t szPartition = platform_persistent_size_partition(type);
  if ((((int32_t)baseOffset) < 0) || (baseOffset > (szPartition * KILO)))
  {
    ST_STORAGE_TRACE_ERROR("OverWrite flash partition");
    return ST_STORAGE_ERROR;
  }

  uint32_t     flash_base_sector;
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);


  uint8_t *pStorage   = (uint8_t *)pData;
  uint32_t szStorage  = szData; /* Number of 64 bits words*/
  /* translate the offset to the absolute position */
  uint32_t offsetFlashExt = baseOffset + (flash_base_sector * tFlashStorage[type].sectorSize * KILO);

  /* Erase and read/write functions doesn't work while mapped mode is on */
  /* We must make sure nobody will access the flashExt while Erase and read/write  so, disable irq */
  /* BSP FLASHEXT doesn't provide "disable mapping", so deinit / init */

  //st_os_enter_critical_section();

  ST_Storage_Result ret = ST_STORAGE_OK;
  uint8_t  slot[PROG_PAGE_SIZE_EXT];

  SCB_DisableDCache();

  /* write while size is done */
  while (szStorage)
  {
    /* Compute base and index*/
    uint32_t offsetBase = platform_persistent_align_offset(offsetFlashExt, hInfoFlashExt.ProgPageSize);
    uint32_t indexBase  = offsetFlashExt - offsetBase;

    /* Read the prog slot */
    if (BSP_XSPI_NOR_Read(FLASHEXT_INSTANCE, slot, offsetBase, hInfoFlashExt.ProgPageSize) != BSP_ERROR_NONE)
    {
      ret = ST_STORAGE_ERROR;
      break;
    }

    /* Read the existing data from  flash */
    uint32_t szSlot = hInfoFlashExt.ProgPageSize;
    /* Clamps to the max size */
    if (szStorage  < szSlot)
    {
      szSlot = szStorage;
    }
    /* Clamps to the max slot */
    if ((indexBase + szStorage) > hInfoFlashExt.ProgPageSize)
    {
      szSlot = hInfoFlashExt.ProgPageSize - indexBase;
    }

    /* Fills  the modified part */
    memmove(&slot[indexBase], pStorage, szSlot);
    /* Write the slot */
    if (BSP_XSPI_NOR_Write(FLASHEXT_INSTANCE, slot, offsetBase, sizeof(slot)) != BSP_ERROR_NONE)
    {
      ret = ST_STORAGE_ERROR;
      break;
    }
    szStorage      -= szSlot;
    pStorage       += szSlot;
    offsetFlashExt += szSlot;
  }
  //st_os_exit_critical_section();
  SCB_EnableDCache();

  uint32_t szBlk = szData;
  uint8_t  *pBlk = pData;
  uint32_t  off  = baseOffset + (flash_base_sector * tFlashStorage[type].sectorSize * KILO);
  while (szBlk)
  {
    uint8_t  tBlk[64];
    uint32_t blk = sizeof(tBlk);
    if (szBlk < blk)
    {
      blk = szBlk;
    }
    if (BSP_XSPI_NOR_Read(FLASHEXT_INSTANCE, tBlk, off, blk) != BSP_ERROR_NONE)
    {
      ret = ST_STORAGE_ERROR;
      break;
    }
    if (memcmp(tBlk, pBlk, blk) != 0)
    {
      ret = ST_STORAGE_ERROR;
      break;
    }
    off   += blk;
    pBlk  += blk;
    szBlk -= blk;
  }
  return ret;
  #else
  return ST_STORAGE_ERROR;
  #endif
}


/**
* @brief  read data in a partition  external flash  using flashExt
* @param  type    slot type
* @param  baseOffset   offset sector
* @param  pData       sector pointer
* @param  szData   sector pointer size
* @retval error code
*/
static ST_Storage_Result platform_persistent_storage_read_flashExt(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData)
{
  #if !defined(ST_NO_FLASHEXT)

  st_flash_storage_mode flashMode = iFlashExtLocked;
  if (flashMode  != st_flash_storage_LOCK_PROG)
  {
    st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
  }

  uint32_t szPartition = platform_persistent_size_partition(type);
  if ((((int32_t)baseOffset)  < 0) || (baseOffset > (szPartition * KILO)))
  {
    ST_STORAGE_TRACE_ERROR("OverWrite flash partition");
    return ST_STORAGE_ERROR;
  }

  uint32_t     flash_base_sector;
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);
  int32_t ret = BSP_XSPI_NOR_Read(FLASHEXT_INSTANCE, (uint8_t *)pData, (uint32_t)((flash_base_sector * tFlashStorage[type].sectorSize * KILO) + baseOffset), szData);
  if (ret != BSP_ERROR_NONE)
  {
    ST_STORAGE_TRACE_ERROR("failed while reading at 0x%x", baseOffset);
    return ST_STORAGE_ERROR;
  }
  st_flash_storage_lock(st_flash_storage_FLASH_EXT, flashMode);
  #endif

  return ST_STORAGE_OK;
}


/**
* @brief  erase data in a partition external flash  using flashExt
* @param  type    slot type
* @param  fnProgress   sector callback
* @retval error code
*/
static ST_Storage_Result platform_persistent_storage_erase_flashExt(st_flash_storage_type type)
{
  #if !defined(ST_NO_FLASHEXT)

  uint32_t          flash_base_sector;
  uint32_t          flash_sector_nb;
  ST_Storage_Result err = ST_STORAGE_OK;

  if (iFlashExtLocked != st_flash_storage_LOCK_PROG)
  {
    ST_STORAGE_TRACE_ERROR("Lock FlashExt");
    return  ST_STORAGE_OK;
  }
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, &flash_sector_nb, NULL);

  /* Size partition */
  uint32_t szErase = flash_sector_nb;

  /* Get the absolute flashExt partition and  align on the erase block size */
  uint32_t offset_base = platform_persistent_align_offset(flash_base_sector * tFlashStorage[type].sectorSize * KILO, hInfoFlashExt.EraseSectorSize);
  uint32_t posSector   = offset_base;
  /* Erase while all is not done */
  uint32_t EraseTryCount   = 10UL;
  uint32_t eraseSectorSize = 4UL * KILO;
  while (szErase > 0U)
  {
    while (EraseTryCount)
    {
      //st_os_enter_critical_section();
      int32_t result = BSP_XSPI_NOR_Erase_Block(FLASHEXT_INSTANCE, posSector, BSP_XSPI_NOR_ERASE_4K);
      //st_os_exit_critical_section();
      if (result == BSP_ERROR_NONE)
      {
        break;
      }
      EraseTryCount--;
      ST_STORAGE_TRACE_INFO("Re-try to erase the FLASHEXT flash");
    }
    while (BSP_XSPI_NOR_GetStatus(0) == BSP_ERROR_BUSY);
    if (EraseTryCount == 0UL)
    {
      ST_STORAGE_TRACE_ERROR("Cannot erase the flash");
      err = ST_STORAGE_ERROR;
      break;
    }
    szErase--;
    posSector += eraseSectorSize;
  }
  uint32_t szBlk = flash_sector_nb * KILO;
  uint32_t off   = (flash_base_sector * tFlashStorage[type].sectorSize * KILO);
  while ((szBlk != 0UL) && (err == ST_STORAGE_OK))
  {
    uint8_t  tBlk[64];
    uint32_t blk = sizeof(tBlk);
    if (szBlk  < blk)
    {
      blk = szBlk;
    }
    int32_t ret = BSP_XSPI_NOR_Read(FLASHEXT_INSTANCE, tBlk, off, blk);
    if (ret != BSP_ERROR_NONE)
    {
      err = ST_STORAGE_ERROR;
      break;
    }
    for (uint32_t a = 0UL; a < blk; a++)
    {
      if (tBlk[a] != 0xFFU)
      {
        err = ST_STORAGE_ERROR;
        break;
      }
    }
    off   += blk;
    szBlk -= blk;
  }
  return err;
  #else
  return ST_STORAGE_ERROR;
  #endif
}


/**
* @brief  Erase  partition internal flash
* @param  type    slot type
* @param  fnProgress   sector callback
* @retval error code
*/
static ST_Storage_Result platform_persistent_storage_erase_internal_flash(st_flash_storage_type type)
{
  uint32_t flash_base_address;
  uint32_t flash_base_sector;
  uint32_t flash_sector_nb;

  st_flash_storage_get_partition(type, &flash_base_address, &flash_base_sector, &flash_sector_nb, NULL);
  uint32_t EraseTryCount = 10UL;
  uint32_t curSect       = 0UL;
  uint32_t szCurrent     = 0UL;
  while (flash_sector_nb > 0UL)
  {
    while (EraseTryCount)
    {
      if (platform_erase_internal_sector(type, curSect) == ST_STORAGE_OK)
      {
        uint8_t *pMem = (uint8_t *)platform_persistent_ptr_partition(type, curSect);
        if (platform_check_flash_erased(pMem, tFlashStorage[type].sectorSize * KILO) == ST_STORAGE_OK)
        {
          break;
        }
      }
      EraseTryCount--;
      ST_STORAGE_TRACE_INFO("Re-try to erase the flash");
    }

    if (EraseTryCount == 0UL)
    {
      ST_STORAGE_TRACE_ERROR("Cannot erase the flash");
      return ST_STORAGE_ERROR;
    }
    flash_sector_nb--;
    szCurrent += tFlashStorage[type].sectorSize;
    curSect++;
  }
  return ST_STORAGE_OK;
}


/**
* @brief  Read a data in a partition internal flash
* @param  type    slot type
* @param  baseOffset   offset sector
* @param  pData       sector pointer
* @param  szData   sector pointer size
* @retval error code
*/
static ST_Storage_Result platform_persistent_storage_read_internal_flash(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData)
{
  uint32_t szPartition = platform_persistent_size_partition(type);

  if ((((int32_t)baseOffset) < 0) || (baseOffset > (szPartition * KILO)))
  {
    ST_STORAGE_TRACE_ERROR("OverWrite flash partition");
    return ST_STORAGE_ERROR;
  }

  uint32_t flash_base_sector, flash_base;
  st_flash_storage_get_partition(type, &flash_base, &flash_base_sector, NULL, NULL);

  uint8_t *pFlash = (uint8_t *)(platform_persistent_ptr_partition(type, 0));
  memcpy(pData, (void *)pFlash, szData);

  return ST_STORAGE_OK;
}


/**
* @brief  Write a data in a partition internal flash
* @param  type    slot type
* @param  baseOffset   offset sector
* @param  pData       sector pointer
* @param  szData   sector pointer size
* @retval error code
*/
static ST_Storage_Result platform_persistent_storage_write_internal_flash(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData)
{
  #if USE_INTERNAL_FLASH == 1
  uint32_t szPartition = platform_persistent_size_partition(type);

  if ((((int32_t)baseOffset) < 0) || (baseOffset > (szPartition * KILO)))
  {
    ST_STORAGE_TRACE_ERROR("OverWrite flash partition");
    return ST_STORAGE_ERROR;
  }
  baseOffset += tFlashStorage[type].offset;
  uint32_t flash_base_address = baseOffset;

  uint8_t *pStorage = (uint8_t *)pData;
  uint32_t szStorage = szData; /* Number of 64 bits words*/
  /* Write the storage in the flash sector */
  HAL_FLASH_Unlock();

  ST_Storage_Result ret = ST_STORAGE_OK;
  /* on H7 the slot is qword 32 bytes */
  uint8_t slot[PROG_PAGE_SIZE_INT];
  while (szStorage > 0UL)
  {
    /* Decompose flash base + offset */
    /* Remove lower bits */
    uint32_t msk            = (sizeof(slot) - 1U);
    uint32_t alignSlot_base = flash_base_address & (~msk);
    /* Compute the offset */
    uint32_t alignSlot_off  = flash_base_address & msk;
    /* Fills with the pre-existing data */
    memcpy((void *)slot, (void *)alignSlot_base, sizeof(slot));/*cstat !MISRAC2012-Rule-11.6 the HAL API expect an offset rather than a pointer, ie we need to cast a pointer as int */

    /* Read existing data in flash */
    uint32_t szSlot = sizeof(slot);
    /* Clamps to the max size */
    if (szStorage < szSlot)
    {
      szSlot = szStorage;
    }
    /* Clamps to the max slot */
    if ((alignSlot_off + szStorage) > sizeof(slot))
    {
      szSlot = sizeof(slot) - alignSlot_off;
    }

    /* Updates the modified part */
    memmove(&slot[alignSlot_off], pStorage, szSlot);
    /* flash the slot */
    if (platform_write_internal_sector(alignSlot_base, &slot) != ST_STORAGE_OK)
    {
      ret = ST_STORAGE_ERROR;
      break;
    }
    szStorage          -= szSlot;
    pStorage           += szSlot;
    flash_base_address += szSlot;
  }
  HAL_FLASH_Lock();
  if (ret != ST_STORAGE_OK)
  {
    ST_STORAGE_TRACE_ERROR("HAL_FLASH_Program failed while writing at 0x%x", flash_base_address);
    return ST_STORAGE_ERROR;
  }

  if (memcmp((char *)baseOffset, (const char *)pData, (size_t)szData) != 0) /*cstat  !MISRAC2012-Rule-21.16  false positif */
  {
    ST_STORAGE_TRACE_ERROR("Write Persistent: Verify error");
    return ST_STORAGE_ERROR;

  }
  return ST_STORAGE_OK;
  #else // USE_INTERNAL_FLASH == 1
  return ST_STORAGE_ERROR;
  #endif // USE_INTERNAL_FLASH == 1
}


/**
* @brief  Generic write a data in a partition
* @param  type    slot type
* @param  flash_base_address   offset
* @param  pData       sector pointer
* @param  szData   sector pointer size
* @retval error code
*/
ST_Storage_Result st_flash_storage_write(st_flash_storage_type type, uint32_t flash_base_address, void *pData, uint32_t szData)
{
  ST_Storage_Result ret;
  st_os_mutex_lock(&hMutexStorage);
  if (platform_persistent_is_flashExt(type))
  {
    st_flash_storage_mode flashMode = iFlashExtLocked;
    if (flashMode != st_flash_storage_LOCK_PROG)
    {
      st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
    }
    ret = platform_persistent_storage_write_flashExt(type, flash_base_address, pData, szData);
    st_flash_storage_lock(st_flash_storage_FLASH_EXT, flashMode);
  }
  else
  {
    ret = platform_persistent_storage_write_internal_flash(type, flash_base_address, pData, szData);
  }
  st_os_mutex_unlock(&hMutexStorage);
  return ret;
}


/**
* @brief  Generic Read a data in a partition
* @param  type    slot type
* @param  flash_base_address   offset
* @param  pData       sector pointer
* @param  szData   sector pointer size
* @retval error code
*/
ST_Storage_Result st_flash_storage_read(st_flash_storage_type type, uint32_t flash_base_address, void *pData, uint32_t szData)
{
  ST_Storage_Result ret;
  st_os_mutex_lock(&hMutexStorage);

  if (platform_persistent_is_flashExt(type))
  {
    ret = platform_persistent_storage_read_flashExt(type, flash_base_address, pData, szData);
  }
  else
  {
    ret = platform_persistent_storage_read_internal_flash(type, flash_base_address, pData, szData);
  }
  st_os_mutex_unlock(&hMutexStorage);
  return ret;
}


/**
* @brief  Generic erase data in a partition
* @param  type    slot type
* @retval error code
*/
ST_Storage_Result st_flash_storage_erase(st_flash_storage_type type)
{
  ST_Storage_Result ret;
  st_os_mutex_lock(&hMutexStorage);

  if (platform_persistent_is_flashExt(type))
  {
    st_flash_storage_mode flashMode = iFlashExtLocked;
    if (flashMode  != st_flash_storage_LOCK_PROG)
    {
      st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
    }
    ret = platform_persistent_storage_erase_flashExt(type);
    st_flash_storage_lock(st_flash_storage_FLASH_EXT, flashMode);
  }
  else
  {
    ret = platform_persistent_storage_erase_internal_flash(type);
  }
  st_os_mutex_unlock(&hMutexStorage);
  return ret;
}


ST_Storage_Result st_flash_storage_erase_sector(st_flash_storage_type type, uint32_t indexSector)
{
  ST_Storage_Result ret = ST_STORAGE_ERROR;
  st_os_mutex_lock(&hMutexStorage);
  uint32_t     flash_base_sector ;
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);

  if (platform_persistent_is_flashExt(type))
  {
    if (iFlashExtLocked == st_flash_storage_LOCK_PROG)
    {
      st_flash_storage_mode flashMode = iFlashExtLocked;
      if (flashMode  != st_flash_storage_LOCK_PROG)
      {
        st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
      }

      //st_os_enter_critical_section();
      int32_t result = BSP_XSPI_NOR_Erase_Block(FLASHEXT_INSTANCE, flash_base_sector + indexSector, BSP_XSPI_NOR_ERASE_4K);
      //st_os_exit_critical_section();
      st_flash_storage_lock(st_flash_storage_FLASH_EXT, flashMode);

      if (result == BSP_ERROR_NONE)
      {
        ret = ST_STORAGE_OK;
      }
    }
  }
  else
  {
    ret = ST_STORAGE_ERROR;
  }
  st_os_mutex_unlock(&hMutexStorage);
  return ret;
}


ST_Storage_Result st_flash_storage_read_sector(st_flash_storage_type type, uint32_t indexSector, void *pData, uint32_t szData)
{
  ST_Storage_Result ret = ST_STORAGE_ERROR;
  st_os_mutex_lock(&hMutexStorage);
  uint32_t     sizeSector = tFlashStorage[type].sectorSize * KILO;
  uint32_t     flash_base_sector;

  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);

  if (platform_persistent_is_flashExt(type))
  {
    if (iFlashExtLocked == st_flash_storage_LOCK_PROG)
    {
      bool result = true;
      //st_os_enter_critical_section();
      uint32_t offset = (flash_base_sector * tFlashStorage[type].sectorSize * KILO) + (indexSector * sizeSector);
      while ((szData != 0U) && (result == true))
      {
        uint32_t blk = sizeSector;
        if (blk > szData)
        {
          blk = szData;
        }
        result = (BSP_XSPI_NOR_Read(FLASHEXT_INSTANCE, (uint8_t *)pData, offset, blk) == BSP_ERROR_NONE) ? true : false;
        szData -= blk;
        offset += blk;
        pData = (uint8_t *)pData + blk;
      }
      //st_os_exit_critical_section();
      if (result == BSP_ERROR_NONE)
      {
        ret = ST_STORAGE_OK;
      }
    }
  }
  else
  {
    ret =  ST_STORAGE_ERROR;
  }
  st_os_mutex_unlock(&hMutexStorage);
  return ret;
}


ST_Storage_Result st_flash_storage_write_sector(st_flash_storage_type type, uint32_t indexSector, void *pData, uint32_t szData)
{
  ST_Storage_Result ret = ST_STORAGE_ERROR;
  st_os_mutex_lock(&hMutexStorage);
  uint32_t     sizeSector = tFlashStorage[type].sectorSize * KILO;
  uint32_t     flash_base_sector;

  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);

  if (platform_persistent_is_flashExt(type))
  {
    st_flash_storage_mode flashMode = iFlashExtLocked;
    if (flashMode  != st_flash_storage_LOCK_PROG)
    {
      st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
    }

    bool result = true;
    //st_os_enter_critical_section();
    uint32_t offset = (flash_base_sector * tFlashStorage[type].sectorSize * KILO) + (indexSector * sizeSector);
    while ((szData != 0U) && (result == true))
    {
      uint32_t blk = sizeSector;
      if (blk > szData)
      {
        blk = szData;
      }
      result = (BSP_XSPI_NOR_Erase_Block(FLASHEXT_INSTANCE, offset, BSP_XSPI_NOR_ERASE_4K) == BSP_ERROR_NONE) ? true : false;
      if (result == true)
      {
        result = (BSP_XSPI_NOR_Write(FLASHEXT_INSTANCE, (uint8_t *)pData, offset, blk) == BSP_ERROR_NONE) ? true : false;
      }
      szData -= blk;
      offset += blk;
      pData   = (uint8_t *)pData + blk;
    }
    //st_os_exit_critical_section();
    if (result == BSP_ERROR_NONE)
    {
      ret = ST_STORAGE_OK;
    }
    st_flash_storage_lock(st_flash_storage_FLASH_EXT, iFlashExtLocked);
  }
  else
  {
    /* not implemented yet */
    ret =  ST_STORAGE_ERROR;
  }
  st_os_mutex_unlock(&hMutexStorage);
  return ret;
}


/**
* @brief  Check if the slot is locked
* @param  type    slot type
* @param  mode    type de lock
* @retval error code
*/
ST_Storage_Result  platform_persistent_storage_is_locked(st_flash_storage_type type, st_flash_storage_mode mode)
{
  if (type == st_flash_storage_FLASH_EXT)
  {
    return (iFlashExtLocked == (st_flash_storage_mode)mode) ? ST_STORAGE_OK : ST_STORAGE_ERROR;
  }
  else
  {
    return ST_STORAGE_OK;
  }
}


/**
* @brief  Lock the slot
* @param  type    slot type
* @param  mode    type de lock
* @retval error code
*/
ST_Storage_Result st_flash_storage_lock(st_flash_storage_type type, st_flash_storage_mode mode)
{
  ST_Storage_Result ret = ST_STORAGE_OK;
  st_os_mutex_lock(&hMutexStorage);

  switch (type)
  {
    case st_flash_storage_REGISTRY:
    {
      /* lock prg for internal flash, always mapped */
      break;
    }

    case st_flash_storage_FLASH_EXT:
    {
      #if !defined(ST_NO_FLASHEXT)
      switch (mode)
      {
        case st_flash_storage_LOCK_PROG:
        {
          if (iFlashExtLocked != (st_flash_storage_mode) mode)
          {
            #ifdef MEMORYMAP_WORKAROUND
            XSPI_DeInit();
            XSPI_Init(false);
            #else
            if (BSP_XSPI_NOR_DisableMemoryMappedMode(FLASHEXT_INSTANCE) != BSP_ERROR_NONE)
            {
              ret =  ST_STORAGE_ERROR;
            }
            #endif
          }

          iFlashExtLocked = (st_flash_storage_mode)mode;
          break;
        }

        case st_flash_storage_LOCK_MAP:
        {
          if (iFlashExtLocked != (st_flash_storage_mode)mode)
          {
            #ifdef MEMORYMAP_WORKAROUND
            /* on H7, BSP_OSPI_NOR_DisableMemoryMappedMode/BSP_OSPI_NOR_EnableMemoryMappedMode  doesn't work, we need to init from scratch the flash */
            XSPI_DeInit();
            XSPI_Init(true);
            #else // MEMORYMAP_WORKAROUND
            if (ret == ST_STORAGE_OK)
            {
              if (BSP_XSPI_NOR_EnableMemoryMappedMode(FLASHEXT_INSTANCE) != BSP_ERROR_NONE)
              {
                ret =  ST_STORAGE_ERROR;
              }
            }
            #endif // MEMORYMAP_WORKAROUND
          }

          iFlashExtLocked = (st_flash_storage_mode)mode;
          break;
        }
      }
      #else // !defined(ST_NO_FLASHEXT)
      ret = ST_STORAGE_ERROR;
      #endif // !defined(ST_NO_FLASHEXT)
      break;
    }

    default:
      break;
  }
  st_os_mutex_unlock(&hMutexStorage);
  /* Locked by default */
  return ret;
}


/**
* @brief  return the offset rounded to a sector
* @param  type    slot type
* @param  sector  sector offset
* @param  sector  sector offset
* @param  sectorNb      sector number
* @param  sectorSize      sector size
*/
ST_Storage_Result st_flash_storage_set_partition(st_flash_storage_type type, uint32_t offset, uint32_t sector, uint32_t sectorNb, uint32_t sectorSize)
{
  #if !defined(ST_NO_FLASHEXT)
  BSP_XSPI_NOR_GetInfo(FLASHEXT_INSTANCE, &hInfoFlashExt);
  #endif
  tFlashStorage[type].offset     = offset;
  tFlashStorage[type].sector     = sector;
  tFlashStorage[type].sectorNb   = sectorNb;
  tFlashStorage[type].sectorSize = sectorSize;
  return ST_STORAGE_OK;
}


/**
* @brief  return info concerning a flash partition
* @param  type    slot type
* @param  pOffset  pointer sector offset
* @param  pSector  pointer sector offset
* @param  pSectorNb   pointer sector number
* @param  pSectorSize   pointer sector Size
*/
ST_Storage_Result st_flash_storage_get_partition(st_flash_storage_type type, uint32_t *pOffset, uint32_t *pSector, uint32_t *pSectorNb, uint32_t *pSectorSize)
{
  /* Other boards may need to override this function */
  if (pOffset != NULL)
  {
    *pOffset = tFlashStorage[type].offset;
  }
  if (pSector != NULL)
  {
    *pSector = tFlashStorage[type].sector;
  }
  if (pSectorNb != NULL)
  {
    *pSectorNb = tFlashStorage[type].sectorNb;
  }
  if (pSectorSize != NULL)
  {
    *pSectorSize = tFlashStorage[type].sectorSize;
  }
  return ST_STORAGE_OK;
}

