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
#ifdef STM32H735xx
  #include "stm32h7xx_hal.h"
  #include "stm32h735g_discovery.h"
  #include "stm32h735g_discovery_ospi.h"
#elif defined(STM32H573xx)
  #include "stm32h5xx_hal.h"
  //#include "stm32h573i_discovery.h"
  //#include "stm32h573i_discovery_ospi.h"
#else
  #error "Platform is not supported by LiveTune's Designer"
#endif

#include "st_os.h"
#include "st_flash_storage.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


/* Private defines -----------------------------------------------------------*/
/*#define TEST_FLASH_EXT*/

#define KILO                               (1024UL)

#define FLASHEXT_BASE_ADDRESS              OCTOSPI1_BASE
#define FLASHEXT_INSTANCE                  0

#if defined(STM32H735xx)
  #define H7_SETUP
#elif defined(STM32H573xx)
  #define H5_SETUP
#else
  #error "flash storage Not validated "
#endif


#if defined(H7_SETUP)
  #define DISABLE_DCACHE_DURING_EXTFLASH_ACCESS
  #define MEMORYMAP_WORKAROUND
  #define DUAL_BANK_H7
  #define PROG_PAGE_SIZE_INT                    (32U)
  #define PROG_PAGE_SIZE_EXT                    (256)
  #define FLASH_TYPE_PROG                       FLASH_TYPEPROGRAM_FLASHWORD
  #define FLASH_VOLTAGE(str)                    (str).VoltageRange = FLASH_VOLTAGE_RANGE_4

#elif defined(H5_SETUP)
  #define MEMORYMAP_WORKAROUND
  #define DUAL_BANK_H5
  #define PROG_PAGE_SIZE_INT                    (16U)
  #define PROG_PAGE_SIZE_EXT                    (256)
  #define FLASH_TYPE_PROG                       FLASH_TYPEPROGRAM_QUADWORD
  #define FLASH_VOLTAGE(a)

  /* when using internal flash, the cache must be disabled when performing write then read operation on the flash  */
  #define DISABLE_CACHE()      CLEAR_BIT(ICACHE->CR, ICACHE_CR_EN); \
                                  __SEV();

  #define ENABLE_CACHE()       SET_BIT(ICACHE->CR, ICACHE_CR_EN);
#else
  #error "Not supported "
#endif


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
/* sector distribution for this board */
bool iFlashInitialized = false;
/* flash ext  info */
static st_mutex   hMutexStorage;
#if !defined(ST_NO_FLASHEXT)
  static BSP_OSPI_NOR_Info_t           hInfoFlashExt;
  static BSP_OSPI_NOR_Init_t           hInstanceFlashExt;
#endif

/* Partition handles */
static flash_storage_t tFlashStorage[st_flash_storage_MAX];


/* Private functions ------------------------------------------------------- */

static uint32_t platform_persistent_size_partition(st_flash_storage_type type);
static uint32_t platform_persistent_ptr_partition(st_flash_storage_type type, uint32_t sector);
static uint32_t platform_persistent_is_flashExt(st_flash_storage_type type);
#if !defined(ST_NO_FLASHEXT)
static uint32_t platform_persistent_align_offset(uint32_t offset, uint32_t blocksize);
#endif
static ST_Storage_Result platform_persistent_storage_write_flashExt(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
static ST_Storage_Result platform_persistent_storage_read_flashExt(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
static ST_Storage_Result platform_persistent_storage_erase_flashExt(st_flash_storage_type type);
static ST_Storage_Result platform_persistent_storage_erase_internal_flash(st_flash_storage_type type);
static ST_Storage_Result platform_persistent_storage_read_internal_flash(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
static ST_Storage_Result platform_persistent_storage_write_internal_flash(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
ST_Storage_Result  platform_persistent_storage_is_locked(st_flash_storage_type type, st_flash_storage_mode mode);
static ST_Storage_Result platform_check_flash_erased(uint8_t *pByte, uint32_t size);


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
  if (!iFlashInitialized)
  {
    #if !defined(ST_NO_FLASHEXT)
    /* Flash ext  device configuration */
    hInstanceFlashExt.InterfaceMode = BSP_OSPI_NOR_OPI_MODE;
    hInstanceFlashExt.TransferRate  = BSP_OSPI_NOR_DTR_TRANSFER;
    /* the default is memory flash locked as map */
    iFlashExtLocked = st_flash_storage_LOCK_MAP;
    /* the flash ext must exist, otherwise assert */
    ST_STORAGE_VERIFY(BSP_OSPI_NOR_Init(FLASHEXT_INSTANCE, &hInstanceFlashExt) == BSP_ERROR_NONE);
    //    HAL_Delay(100U);
    ST_STORAGE_VERIFY(BSP_OSPI_NOR_EnableMemoryMappedMode(FLASHEXT_INSTANCE) == BSP_ERROR_NONE);
    #endif
    iFlashInitialized = true;
  }

  return ST_STORAGE_OK;
}



/**
* @brief  Configure external flash uses the Flash ext
*/
ST_Storage_Result st_flash_storage_init(void)
{
  ST_STORAGE_VERIFY(st_os_mutex_create(&hMutexStorage) == ST_OS_OK);
  ST_Storage_Result result = st_flash_storage_pre_init();
  if (result == ST_STORAGE_OK)
  {
    iFlashInitialized = true;
  }
  #ifdef TEST_FLASH_EXT
  platform_test_flash_conf();
  #endif
  return result;
}


#ifdef DUAL_BANK_H5

/**
  * @brief  Gets the sector of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The sector of a given address
  */
static uint32_t platform_config_persistent_storage_get_sector(st_flash_storage_type type, uint32_t sector)
{
  uint32_t Address = platform_persistent_ptr_partition(type, sector);
  if ((Address >= FLASH_BASE) && (Address < (FLASH_BASE + FLASH_BANK_SIZE)))
  {
    sector = (Address & ~FLASH_BASE) / FLASH_SECTOR_SIZE;
  }
  else if ((Address >= (FLASH_BASE + FLASH_BANK_SIZE)) && (Address < (FLASH_BASE + FLASH_SIZE)))
  {
    sector = ((Address & ~FLASH_BASE) - FLASH_BANK_SIZE) / FLASH_SECTOR_SIZE;
  }
  else
  {
    sector = 0xFFFFFFFF; /* Address out of range */
  }

  return sector;
}


/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t platform_config_persistent_storage_get_bank(st_flash_storage_type type, uint32_t sector)
{
  uint32_t Address = platform_persistent_ptr_partition(type, sector);
  uint32_t bank    = 0;

  if (READ_BIT(FLASH->OPTSR_CUR, FLASH_OPTSR_SWAP_BANK) == 0U)
  {
    /* No Bank swap */
    if (Address < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_1;
    }
    else
    {
      bank = FLASH_BANK_2;
    }
  }
  else
  {
    /* Bank swap */
    if (Address < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_2;
    }
    else
    {
      bank = FLASH_BANK_1;
    }
  }

  return bank;
}

#endif // DUAL_BANK_H5


#ifdef DUAL_BANK_H7

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

#endif // DUAL_BANK_H7


/**
* @brief  Primitive erase internal flash
* @param  curSec  sector to erase
* @retval error code
*/
static ST_Storage_Result platform_erase_internal_sector(st_flash_storage_type type, uint32_t curSec)
{
  uint32_t SectorError = 0;

  FLASH_EraseInitTypeDef EraseInitStruct = {0};
  EraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
  FLASH_VOLTAGE(EraseInitStruct);
  EraseInitStruct.Sector       = platform_config_persistent_storage_get_sector(type, curSec);
  EraseInitStruct.Banks        = platform_config_persistent_storage_get_bank(type, curSec);
  EraseInitStruct.NbSectors    = 1;
  HAL_FLASH_Unlock();
  HAL_StatusTypeDef result = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
  HAL_FLASH_Lock();
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
  if (HAL_FLASH_Program(FLASH_TYPE_PROG, offset, (uint32_t)pSlot) != HAL_OK)/*cstat !MISRAC2012-Rule-11.6 the HAL API expect an offset rather than a pointer, ie we need to cast a pointer as int */
  {
    ret = ST_STORAGE_ERROR;
  }
  else
  {
    ret = ST_STORAGE_OK;
  }

  return ret;
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

#if !defined(ST_NO_FLASHEXT)
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
#endif

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

  uint32_t flash_base_sector;
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);


  uint8_t *pStorage   = (uint8_t *)pData;
  uint32_t szStorage  = szData; /* Number of 64 bits words*/
  /* translate the offset to the absolute position */
  uint32_t offsetFlashExt = baseOffset + (flash_base_sector * tFlashStorage[type].sectorSize * KILO);

  /* Erase and read/write functions doesn't work while mapped mode is on */
  /* We must make sure nobody will access the flashExt while Erase and read/write  so, disable irq */
  /* BSP FLASHEXT doesn't provide "disable mapping", so deinit / init */

  ST_Storage_Result ret = ST_STORAGE_OK;
  uint8_t slot[PROG_PAGE_SIZE_EXT];

  /* write while size is done */
  #ifdef DISABLE_DCACHE_DURING_EXTFLASH_ACCESS
  SCB_DisableDCache();
  #endif
  while (szStorage)
  {
    /* Compute base and index*/
    uint32_t offsetBase = platform_persistent_align_offset(offsetFlashExt, hInfoFlashExt.ProgPageSize);
    uint32_t indexBase  = offsetFlashExt - offsetBase;

    /* Read the prog slot */
    if (BSP_OSPI_NOR_Read(FLASHEXT_INSTANCE, slot, offsetBase, hInfoFlashExt.ProgPageSize) != BSP_ERROR_NONE)
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
    if (BSP_OSPI_NOR_Write(FLASHEXT_INSTANCE, slot, offsetBase, sizeof(slot)) != BSP_ERROR_NONE)
    {
      ret = ST_STORAGE_ERROR;
      break;
    }
    szStorage      -= szSlot;
    pStorage       += szSlot;
    offsetFlashExt += szSlot;
  }
  #ifdef DISABLE_DCACHE_DURING_EXTFLASH_ACCESS
  SCB_EnableDCache();
  #endif

  if (ret == ST_STORAGE_OK)
  {
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
      if (BSP_OSPI_NOR_Read(FLASHEXT_INSTANCE, tBlk, off, blk) != BSP_ERROR_NONE)
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
  ST_Storage_Result ret = ST_STORAGE_OK;

  #if !defined(ST_NO_FLASHEXT)
  st_flash_storage_mode flashMode = iFlashExtLocked;
  if (flashMode != st_flash_storage_LOCK_PROG)
  {
    ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
  }

  if (ret == ST_STORAGE_OK)
  {
    uint32_t szPartition = platform_persistent_size_partition(type);
    if ((((int32_t)baseOffset) < 0) || (baseOffset > (szPartition * KILO)))
    {
      ST_STORAGE_TRACE_ERROR("OverWrite flash partition");
      ret = ST_STORAGE_ERROR;
    }
  }

  if (ret == ST_STORAGE_OK)
  {
    uint32_t flash_base_sector;
    st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);
    int32_t result = BSP_OSPI_NOR_Read(FLASHEXT_INSTANCE, (uint8_t *)pData, (uint32_t)((flash_base_sector * tFlashStorage[type].sectorSize * KILO) + baseOffset), szData);
    if (result != BSP_ERROR_NONE)
    {
      ST_STORAGE_TRACE_ERROR("failed while reading at 0x%x", baseOffset);
      ret = ST_STORAGE_ERROR;
    }
  }

  if (ret == ST_STORAGE_OK)
  {
    ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, flashMode);
  }
  #endif
  return ret;
}


/**
* @brief  erase data in a partition  external flash  using flashExt
* @param  type    slot type
* @param  fnProgress   sector callback
* @retval error code
*/

static ST_Storage_Result platform_persistent_storage_erase_flashExt(st_flash_storage_type type)
{
  #if !defined(ST_NO_FLASHEXT)

  uint32_t          flash_base_sector;
  uint32_t          flash_sector_nb;
  ST_Storage_Result ret = ST_STORAGE_OK;

  if (iFlashExtLocked != st_flash_storage_LOCK_PROG)
  {
    ST_STORAGE_TRACE_ERROR("Lock FlashExt");
    return ST_STORAGE_OK;
  }
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, &flash_sector_nb, NULL);

  /* Size partition */
  uint32_t szErase = flash_sector_nb;

  /* Get the absolute flashExt partition and  align on the erase block size */
  uint32_t offset_base = platform_persistent_align_offset(flash_base_sector * tFlashStorage[type].sectorSize * KILO, hInfoFlashExt.EraseSectorSize);
  uint32_t posSector   = offset_base;
  /* Erase while all is not done */
  uint32_t eraseSectorSize = 4UL * KILO;
  while (szErase > 0U)
  {
    uint32_t EraseTryCount = 10UL;
    while (EraseTryCount)
    {
      int32_t result = BSP_OSPI_NOR_Erase_Block(FLASHEXT_INSTANCE, posSector, MX25LM51245G_ERASE_4K);
      if (result == BSP_ERROR_NONE)
      {
        break;
      }
      EraseTryCount--;
      ST_STORAGE_TRACE_INFO("Re-try to erase the FLASHEXT flash");
    }
    while (BSP_OSPI_NOR_GetStatus(FLASHEXT_INSTANCE) == BSP_ERROR_BUSY);
    if (EraseTryCount == 0UL)
    {
      ST_STORAGE_TRACE_ERROR("Cannot erase the flash");
      ret = ST_STORAGE_ERROR;
      break;
    }
    szErase--;
    posSector += eraseSectorSize;
  }

  if (ret == ST_STORAGE_OK)
  {
    uint32_t szBlk = flash_sector_nb * KILO;
    uint32_t off   = (flash_base_sector * tFlashStorage[type].sectorSize * KILO);
    while ((szBlk != 0UL) && (ret == ST_STORAGE_OK))
    {
      uint8_t  tBlk[64];
      uint32_t blk = sizeof(tBlk);
      if (szBlk  < blk)
      {
        blk = szBlk;
      }
      int32_t result = BSP_OSPI_NOR_Read(FLASHEXT_INSTANCE, tBlk, off, blk);
      if (result != BSP_ERROR_NONE)
      {
        ret = ST_STORAGE_ERROR;
        break;
      }
      for (uint32_t a = 0UL; a < blk ; a++)
      {
        if (tBlk[a] != 0xFFU)
        {
          ret = ST_STORAGE_ERROR;
          break;
        }
      }
      off   += blk;
      szBlk -= blk;
    }
  }

  return ret;
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
  uint32_t curSect   = 0UL;
  uint32_t szCurrent = 0UL;
  while (flash_sector_nb > 0UL)
  {
    uint32_t EraseTryCount = 10UL;

    DISABLE_CACHE();
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
    ENABLE_CACHE();

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

  uint8_t *pFlash = (uint8_t *)platform_persistent_ptr_partition(type, 0);
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

  DISABLE_CACHE();
  if (memcmp((char *)baseOffset, (const char *)pData, (size_t)szData) != 0) /*cstat  !MISRAC2012-Rule-21.16  false positif */
  {
    ST_STORAGE_TRACE_ERROR("Write Persistent: Verify error");
    ENABLE_CACHE();
    return ST_STORAGE_ERROR;

  }
  ENABLE_CACHE();
  return ST_STORAGE_OK;
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
  ST_Storage_Result ret = ST_STORAGE_OK;
  st_os_mutex_lock(&hMutexStorage);
  if (platform_persistent_is_flashExt(type))
  {
    st_flash_storage_mode flashMode = iFlashExtLocked;
    if (flashMode != st_flash_storage_LOCK_PROG)
    {
      ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
    }
    if (ret == ST_STORAGE_OK)
    {
      ret = platform_persistent_storage_write_flashExt(type, flash_base_address, pData, szData);
    }
    if (ret == ST_STORAGE_OK)
    {
      ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, flashMode);
    }
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
  ST_Storage_Result ret = ST_STORAGE_OK;
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
  ST_Storage_Result ret = ST_STORAGE_OK;
  st_os_mutex_lock(&hMutexStorage);

  if (platform_persistent_is_flashExt(type))
  {
    st_flash_storage_mode flashMode = iFlashExtLocked;
    if (flashMode != st_flash_storage_LOCK_PROG)
    {
      ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
    }
    if (ret == ST_STORAGE_OK)
    {
      ret = platform_persistent_storage_erase_flashExt(type);
    }
    if (ret == ST_STORAGE_OK)
    {
      ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, flashMode);
    }
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
  ST_Storage_Result ret = ST_STORAGE_OK;
  st_os_mutex_lock(&hMutexStorage);
  uint32_t     flash_base_sector;

  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);

  if (platform_persistent_is_flashExt(type))
  {
#if !defined(ST_NO_FLASHEXT)
    if (iFlashExtLocked == st_flash_storage_LOCK_PROG)
    {
      st_flash_storage_mode flashMode = iFlashExtLocked;
      if (flashMode != st_flash_storage_LOCK_PROG)
      {
        ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
      }

      if (ret == ST_STORAGE_OK)
      {
        int32_t result = BSP_OSPI_NOR_Erase_Block(FLASHEXT_INSTANCE, flash_base_sector + indexSector, MX25LM51245G_ERASE_4K);

        ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, flashMode);

        if ((ret == ST_STORAGE_OK) && (result != BSP_ERROR_NONE))
        {
          ret = ST_STORAGE_ERROR;
        }
      }
    }
#else
    ret =  ST_STORAGE_ERROR;
#endif

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
  ST_Storage_Result ret = ST_STORAGE_OK;
  st_os_mutex_lock(&hMutexStorage);
  uint32_t     sizeSector = tFlashStorage[type].sectorSize * KILO;
  uint32_t     flash_base_sector;
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);

  if (platform_persistent_is_flashExt(type))
  {
#if !defined(ST_NO_FLASHEXT)
    if (iFlashExtLocked == st_flash_storage_LOCK_PROG)
    {
      int32_t result = BSP_ERROR_NONE;
      uint32_t offset = (flash_base_sector * tFlashStorage[type].sectorSize * KILO) + (indexSector * sizeSector);
      while ((szData != 0U) && (result == BSP_ERROR_NONE))
      {
        uint32_t blk = sizeSector;
        if (blk > szData)
        {
          blk = szData;
        }
        result = BSP_OSPI_NOR_Read(FLASHEXT_INSTANCE, (uint8_t *)pData, offset, blk);
        szData -= blk;
        offset += blk;
        pData = (uint8_t *)pData + blk;
      }
      if (result != BSP_ERROR_NONE)
      {
        ret = ST_STORAGE_ERROR;
      }
    }
#else
    UNUSED(sizeSector);
    ret =  ST_STORAGE_ERROR;
#endif
  }
  else
  {
    ret = ST_STORAGE_ERROR;
  }
  st_os_mutex_unlock(&hMutexStorage);
  return ret;
}


ST_Storage_Result st_flash_storage_write_sector(st_flash_storage_type type, uint32_t indexSector, void *pData, uint32_t szData)
{
  ST_Storage_Result ret = ST_STORAGE_OK;
  st_os_mutex_lock(&hMutexStorage);
  uint32_t     sizeSector = tFlashStorage[type].sectorSize * KILO;
  uint32_t     flash_base_sector;

  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);

  if (platform_persistent_is_flashExt(type))
  {
#if !defined(ST_NO_FLASHEXT)
    st_flash_storage_mode flashMode = iFlashExtLocked;
    if (flashMode != st_flash_storage_LOCK_PROG)
    {
      ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
    }

    if (ret == ST_STORAGE_OK)
    {
      int32_t result = BSP_ERROR_NONE;
      uint32_t offset = (flash_base_sector * tFlashStorage[type].sectorSize * KILO) + (indexSector * sizeSector);
      while ((szData != 0U) && (result == BSP_ERROR_NONE))
      {
        uint32_t blk = sizeSector;
        if (blk > szData)
        {
          blk = szData;
        }
        result = BSP_OSPI_NOR_Erase_Block(FLASHEXT_INSTANCE, offset, MX25LM51245G_ERASE_4K);
        if (result == BSP_ERROR_NONE)
        {
          result = BSP_OSPI_NOR_Write(FLASHEXT_INSTANCE, (uint8_t *)pData, offset, blk);
        }
        szData -= blk;
        offset += blk;
        pData   = (uint8_t *)pData + blk;
      }
      if (result != BSP_ERROR_NONE)
      {
        ret = ST_STORAGE_ERROR;
      }
    }
    if (ret == ST_STORAGE_OK)
    {
      ret = st_flash_storage_lock(st_flash_storage_FLASH_EXT, iFlashExtLocked);
    }
#else
    UNUSED(sizeSector);
    ret =  ST_STORAGE_ERROR;
#endif
  }
  else
  {
    /* not implemented yet */
    ret = ST_STORAGE_ERROR;
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
            if (iFlashInitialized)
            {
              iFlashInitialized = false;
              BSP_OSPI_NOR_DeInit(FLASHEXT_INSTANCE);
            }
            if (BSP_OSPI_NOR_Init(FLASHEXT_INSTANCE, &hInstanceFlashExt) != BSP_ERROR_NONE)
            {
              ret = ST_STORAGE_ERROR;
            }
            else
            {
              iFlashInitialized = true;
            }
            #else
            if (BSP_OSPI_NOR_DisableMemoryMappedMode(FLASHEXT_INSTANCE) != BSP_ERROR_NONE)
            {
              ret = ST_STORAGE_ERROR;
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
            /* on H7, BSP_OSPI_NOR_DisableMemoryMappedMode/BSP_OSPI_NOR_EnableMemoryMappedMode doesn't work, we need to init from scratch the flash */
            if (iFlashInitialized)
            {
              iFlashInitialized = false;
              BSP_OSPI_NOR_DeInit(FLASHEXT_INSTANCE);
            }
            if (BSP_OSPI_NOR_Init(FLASHEXT_INSTANCE, &hInstanceFlashExt) != BSP_ERROR_NONE)
            {
              ret = ST_STORAGE_ERROR;
            }
            else
            {
              iFlashInitialized = true;
            }
            #endif // MEMORYMAP_WORKAROUND
            if (ret == ST_STORAGE_OK)
            {
              if (BSP_OSPI_NOR_EnableMemoryMappedMode(FLASHEXT_INSTANCE) != BSP_ERROR_NONE)
              {
                ret = ST_STORAGE_ERROR;
              }
            }
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
  BSP_OSPI_NOR_GetInfo(FLASHEXT_INSTANCE, &hInfoFlashExt);
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
