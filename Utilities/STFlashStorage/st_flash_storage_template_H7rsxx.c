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

#include "main_hooks.h"
#include "stm32_extmem_conf.h"
#include "stm32_extmem.h"
#include "stm32_extmem_type.h"
#include "extmem_manager.h"

#include "stm32h7s78_discovery.h"
#include "stm32h7s78_discovery_xspi.h"
//#include "stm32_sfdp_driver_api.h"
#include  "st_os.h"
#include  "st_flash_storage.h"
#include  "string.h"
#include "stdlib.h"
#include "stdbool.h"

#define   USE_INTERNAL_FLASH    0


/* Private defines -----------------------------------------------------------*/
//#define TEST_FLASH_EXT
#define Error_Handler main_hooks_error_handler
#define KILO                               (1024U)
#define ERASE_BLOC_SIZE                     0x1000U           /*!< 4 Kbytes */

#define FLASHEXT_BASE_ADDRESS              XSPI2_BASE // could be XSPI2_BASE XSPI3_BASE
#define FLASHEXT_INSTANCE                  0

//#define MEMORYMAP_WOARKAROUND

#define DUAL_BANK_H7
#define PROG_PAGE_SIZE_INT                 32
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
/* sector distribution for this board */
int8_t       iFlashInitialized = 0;
/* flash ext  info */
static st_mutex   hMutexStorage;
#if !defined(ST_NO_FLASHEXT)
  static BSP_XSPI_NOR_Info_t               hInfoFlashExt;
#endif

/* Partition handles */
static flash_storage_t tFlashStorage[st_flash_storage_MAX];

XSPI_HandleTypeDef hxspi2;
DMA_HandleTypeDef hdmatx;

/* Private functions ------------------------------------------------------- */
void Error_Handler(void);

static uint32_t platform_persistent_size_partition(st_flash_storage_type type);
static uint32_t platform_persistent_ptr_partition(st_flash_storage_type type, uint32_t sector);
static uint32_t platform_persistent_is_flashExt(st_flash_storage_type type);
static ST_Storage_Result platform_persistent_storage_write_flashExt(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
static ST_Storage_Result platform_persistent_storage_read_flashExt(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
static ST_Storage_Result platform_persistent_storage_erase_flashExt(st_flash_storage_type type);
static ST_Storage_Result platform_persistent_storage_erase_internal_flash(st_flash_storage_type type);
static ST_Storage_Result platform_persistent_storage_read_internal_flash(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
static ST_Storage_Result platform_persistent_storage_write_internal_flash(st_flash_storage_type type, uint32_t baseOffset, void *pData, uint32_t szData);
ST_Storage_Result  platform_persistent_storage_is_locked(st_flash_storage_type type, st_flash_storage_mode mode);
static ST_Storage_Result platform_check_flash_erased(uint8_t *pByte, uint32_t size);


#ifdef TEST_FLASH_EXT

void EXTMEM_TRACE(uint8_t *Message);
void EXTMEM_TRACE(uint8_t *Message)
{
  return;
}


static ST_Storage_Result  platform_check_pattern(uint8_t *pSector, uint32_t szSector)
{
  ST_Storage_Result err = ST_STORAGE_OK;
  int32_t count = 0;
  for (int32_t sz = 0; sz < szSector ; sz++)
  {
    if (pSector[sz] != 'a' + (count ++ % 24))
    {
      ST_STORAGE_TRACE_ERROR("Check  flash not written : %08x != %0xx", &pSector[sz], 'a' + (sz % 24));
      err  = ST_STORAGE_ERROR;
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
    ST_STORAGE_VERIFY(pSector  != NULL);
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

#endif


/**
  * @brief XSPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_XSPI2_Init(void)
{

  /* USER CODE BEGIN XSPI2_Init 0 */

  /* USER CODE END XSPI2_Init 0 */

  /* USER CODE BEGIN XSPI2_Init 1 */

  /* USER CODE END XSPI2_Init 1 */
  /* XSPI2 parameter configuration*/
  hxspi2.Instance = XSPI2;
  hxspi2.Init.FifoThresholdByte = 4;
  hxspi2.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
  hxspi2.Init.MemoryType = HAL_XSPI_MEMTYPE_MACRONIX;
  hxspi2.Init.MemorySize = HAL_XSPI_SIZE_32GB;
  hxspi2.Init.ChipSelectHighTimeCycle = 2;
  hxspi2.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi2.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
  hxspi2.Init.WrapSize = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hxspi2.Init.ClockPrescaler = 0;
  hxspi2.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi2.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_ENABLE;
  hxspi2.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_NONE;
  hxspi2.Init.MaxTran = 0;
  hxspi2.Init.Refresh = 0;
  hxspi2.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;
  if (HAL_XSPI_Init(&hxspi2) != HAL_OK)
  {
    Error_Handler();
  }


  /* USER CODE BEGIN XSPI2_Init 2 */

  /* USER CODE END XSPI2_Init 2 */

}


/**
  * @brief  This function does an erase of n (depends on Length) pages in external OSPI flash
  * @param  Address: flash address to be erased
  * @param  Length: number of bytes
  * @retval ST_Storage_Result ST_STORAGE_OK if successful, ST_STORAGE_ERROR otherwise.
  */
ST_Storage_Result FLASH_EXT_Erase_Size(uint32_t Address, uint32_t Length);
ST_Storage_Result FLASH_EXT_Erase_Size(uint32_t Address, uint32_t Length)
{
  ST_Storage_Result e_ret_status    = ST_STORAGE_OK;
  uint32_t            block_index   = 0U;
  uint32_t            start_address = 0U;

  /* flash address to erase is the offset from begin of external flash */
  start_address = Address & 0x0FFFFFFFU;

  /* Loop on 64KBytes block */
  for (block_index = 0U; (e_ret_status == ST_STORAGE_OK) && (block_index < ((Length - 1U) / ERASE_BLOC_SIZE) + 1U); block_index++)
  {
    st_os_enter_critical_section();

    /* Disable memory mapped mode */
    if (EXTMEM_MemoryMappedMode(EXTMEMORY_1, EXTMEM_DISABLE) != EXTMEM_OK)
    {
      e_ret_status = ST_STORAGE_ERROR;
    }

    if (e_ret_status == ST_STORAGE_OK)
    {
      if (EXTMEM_EraseSector(EXTMEMORY_1, start_address, ERASE_BLOC_SIZE) != EXTMEM_OK)
      {
        e_ret_status = ST_STORAGE_ERROR;
      }
    }

    /* Enable back memory mapped mode (even in case of writing error) */
    if (EXTMEM_MemoryMappedMode(EXTMEMORY_1, EXTMEM_ENABLE) != EXTMEM_OK)
    {
      e_ret_status = ST_STORAGE_ERROR;
    }

    st_os_exit_critical_section();

    /* next 64KBytes block */
    start_address += ERASE_BLOC_SIZE;
  }

  return e_ret_status;
}


/**
  * @brief  This function writes a data buffer in external QSPI flash.
  * @param  WriteAddress: flash address to write
  * @param  pData: pointer on buffer with data to write
  * @param  Length: number of bytes
  * @retval STIROT_ILOADER_ErrorStatus STIROT_ILOADER_SUCCESS if successful, STIROT_ILOADER_ERROR otherwise.
  */
ST_Storage_Result FLASH_EXT_Write(uint32_t WriteAddress, const uint8_t *pData, uint32_t Length);
ST_Storage_Result FLASH_EXT_Write(uint32_t WriteAddress, const uint8_t *pData, uint32_t Length)
{
  ST_Storage_Result e_ret_status = ST_STORAGE_OK;

  //  EXTMEM_DRIVER_NOR_SFDP_InfoTypeDef FlashInfo;

  /* Do nothing if Length equal to 0 */
  if (Length == 0U)
  {
    return ST_STORAGE_OK;
  }

  st_os_enter_critical_section();

  /* Disable memory mapped mode */
  if (EXTMEM_MemoryMappedMode(EXTMEMORY_1, EXTMEM_DISABLE) != EXTMEM_OK)
  {
    /* Return here as following step cannot be run */
    e_ret_status = ST_STORAGE_ERROR;
  }

  /* Get flash information */
  //EXTMEM_GetInfo(EXTMEMORY_1, &FlashInfo);

  /* Manage the write of the application in mapped mode */
  if (e_ret_status == ST_STORAGE_OK)
  {
    if (EXTMEM_WriteInMappedMode(EXTMEMORY_1, WriteAddress, (uint8_t *) pData, Length) != EXTMEM_OK)
    {
      e_ret_status = ST_STORAGE_ERROR;
    }
  }

  /* Enable back memory mapped mode (even in case of writing error) */
  if (EXTMEM_MemoryMappedMode(EXTMEMORY_1, EXTMEM_ENABLE) != EXTMEM_OK)
  {
    e_ret_status = ST_STORAGE_ERROR;
  }

  st_os_exit_critical_section();

  return e_ret_status;
}


/**
  * @brief  This function is used to write data with DMA.
  * @param  None
  * @retval None
  */
void FLASH_EXT_XSPI_DMA_Init(void);
void FLASH_EXT_XSPI_DMA_Init(void)
{
  /* Clear the handler of DMA */
  memset((void *)&hdmatx, 0U, sizeof(DMA_HandleTypeDef));

  HAL_NVIC_SetPriority(HPDMA1_Channel12_IRQn, 0x0FU, 0U);
  HAL_NVIC_EnableIRQ(HPDMA1_Channel12_IRQn);

  /* Configure the XSPI DMA transmit */
  hdmatx.Init.Request               = DMA_REQUEST_SW;
  hdmatx.Init.Direction             = DMA_MEMORY_TO_MEMORY;
  hdmatx.Init.SrcInc                = DMA_SINC_INCREMENTED;
  hdmatx.Init.DestInc               = DMA_DINC_INCREMENTED;
  hdmatx.Init.Priority              = DMA_HIGH_PRIORITY;
  hdmatx.Init.SrcBurstLength        = 16U;
  hdmatx.Init.DestBurstLength       = 16U;
  hdmatx.Init.TransferEventMode     = DMA_TCEM_BLOCK_TRANSFER;
  hdmatx.Init.SrcDataWidth          = DMA_SRC_DATAWIDTH_WORD;
  hdmatx.Init.DestDataWidth         = DMA_DEST_DATAWIDTH_WORD;
  hdmatx.Init.BlkHWRequest          = DMA_BREQ_SINGLE_BURST;
  hdmatx.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;

  hdmatx.Instance                   = HPDMA1_Channel12;

  /* Enable the GPDMA clock */
  __HAL_RCC_HPDMA1_CLK_ENABLE();

  /* Initialize the DMA channel */
  HAL_DMA_Init(&hdmatx);
}


/**
  * @brief  This function does a memory copy with DMA on XSPI
  * @param  destination_Address destination address
  * @param  ptrData source address
  * @param  DataSize data size to copy
  * @retval None.
  */
void EXTMEM_MemCopy(uint32_t *destination_Address, const uint8_t *ptrData, uint32_t DataSize);
void EXTMEM_MemCopy(uint32_t *destination_Address, const uint8_t *ptrData, uint32_t DataSize)
{
  /* Ensure memory access are done before the use of DMA */
  __DMB();

  HAL_DMA_Start_IT(&hdmatx, (uint32_t)ptrData, (uint32_t)destination_Address, DataSize);
  HAL_DMA_PollForTransfer(&hdmatx, HAL_DMA_FULL_TRANSFER, 1000U);
}

/**

* @brief  Pre-init is used to initialize the low level Flash Ext without mutex, because Thread doesn't
allow to create mutext before the kernel is started
and we need a mutex toprevent write/erase concurance
*/
ST_Storage_Result st_flash_storage_pre_init(void)
{
  if (iFlashInitialized == 0)
  {
    iFlashInitialized++;

    #if !defined(ST_NO_FLASHEXT)
    /* the default is memory flash locked as map */
    iFlashExtLocked = st_flash_storage_LOCK_MAP;


    HAL_RCCEx_EnableClockProtection(RCC_CLOCKPROTECT_XSPI);

    /* Initialization of the memory parameters */
    memset(extmem_list_config, 0x0, sizeof(extmem_list_config));
    /* EXTMEMORY_1 */
    extmem_list_config[0].MemType = EXTMEM_NOR_SFDP;
    extmem_list_config[0].Handle = (void *)&hxspi2;
    extmem_list_config[0].ConfigType = EXTMEM_LINK_CONFIG_8LINES;

    MX_XSPI2_Init();

    if (EXTMEM_Init(EXTMEMORY_1, HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI2)) != EXTMEM_OK)
    {
      Error_Handler();
    }

    /* Enable default memory mapped mode */
    if (EXTMEM_MemoryMappedMode(EXTMEMORY_1, EXTMEM_ENABLE) != EXTMEM_OK)
    {
      Error_Handler();
    }

    /* Initialize DMA for the external memory */
    FLASH_EXT_XSPI_DMA_Init();
    #endif
    /* the flash ext must exist, otherwise assert */
    BSP_XSPI_NOR_GetInfo(FLASHEXT_INSTANCE, &hInfoFlashExt);
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
  iFlashInitialized++;
  #ifdef TEST_FLASH_EXT
  platform_test_flash_conf();
  #endif
  return result;
}


#if USE_INTERNAL_FLASH ==1

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

#endif


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

  ST_Storage_Result ret = ST_STORAGE_OK;
  uint32_t     write_address = tFlashStorage[type].offset + baseOffset;

  //ST_STORAGE_TRACE_INFO("Write %d Bytes in extFlash at 0x%x", szData, write_address);

  SCB_DisableDCache();
  ret = FLASH_EXT_Write(write_address, pData, szData);
  SCB_EnableDCache();

  /* Check that sector hase been written */
  for (uint32_t i = 0; i < szData; i++)
  {
    if (((uint8_t *)pData)[i] != *(uint8_t *)(write_address + i))
    {
      ST_STORAGE_TRACE_ERROR("Check flash write failed at 0x%x : %d instead of %d", write_address + i, *(uint8_t *)(write_address + i), ((uint8_t *)pData)[i]);
      ret =  ST_STORAGE_ERROR;
      break;
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
static ST_Storage_Result platform_persistent_storage_read_flashExt(st_flash_storage_type type, uint32_t baseOffset, void  *pData, uint32_t szData)
{
  #if !defined(ST_NO_FLASHEXT)

  st_flash_storage_mode flashMode = iFlashExtLocked;
  if (flashMode != st_flash_storage_LOCK_PROG)
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
  #endif
  st_flash_storage_lock(st_flash_storage_FLASH_EXT, flashMode);

  return ST_STORAGE_OK;
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

  uint32_t     EraseTryCount = 10;
  uint32_t     flash_base_sector;
  uint32_t     flash_sector_nb;
  ST_Storage_Result err = ST_STORAGE_OK;

  if (iFlashExtLocked != st_flash_storage_LOCK_PROG)
  {
    ST_STORAGE_TRACE_ERROR("Lock FlashExt");
    return  ST_STORAGE_OK;
  }
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, &flash_sector_nb, NULL);

  uint32_t eraseBytesSize = tFlashStorage[type].sectorNb * tFlashStorage[type].sectorSize * KILO;

  //ST_STORAGE_TRACE_INFO("Erase flash required : 0x%x - %d KB", tFlashStorage[type].offset, eraseBytesSize / 1024);

  SCB_DisableDCache();
  while (EraseTryCount)
  {
    ST_Storage_Result err = FLASH_EXT_Erase_Size(tFlashStorage[type].offset, eraseBytesSize);

    if (err == ST_STORAGE_OK)
    {
      break;
    }
    EraseTryCount--;
    //ST_STORAGE_TRACE_INFO("Re-try to erase the FLASHEXT flash");
  }
  SCB_EnableDCache();

  if (EraseTryCount == 0U)
  {
    ST_STORAGE_TRACE_ERROR("Cannot erase the flash");
    return  ST_STORAGE_ERROR;
  }

  /* Check that sector have been erased*/
  for (uint32_t i = 0; i < eraseBytesSize; i++)
  {
    if (0xff != *(uint8_t *)(tFlashStorage[type].offset + i))
    {
      ST_STORAGE_TRACE_ERROR("Check flash erase failed");
      return ST_STORAGE_ERROR;
    }
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
  uint32_t     flash_base_address;
  uint32_t     flash_base_sector;
  uint32_t     flash_sector_nb;

  st_flash_storage_get_partition(type, &flash_base_address, &flash_base_sector, &flash_sector_nb, NULL);
  uint32_t EraseTryCount = 10;
  uint32_t curSect = 0;
  uint32_t szCurrent = 0;
  while (((int32_t)flash_sector_nb) > 0)
  {
    while (EraseTryCount)
    {
      if (platform_erase_internal_sector(type, curSect) == ST_STORAGE_OK)
      {
        uint8_t *pMem = (uint8_t *) platform_persistent_ptr_partition(type, curSect);
        if (platform_check_flash_erased(pMem, tFlashStorage[type].sectorSize * KILO) == ST_STORAGE_OK)
        {
          break;
        }
      }
      EraseTryCount--;
      //ST_STORAGE_TRACE_INFO("Re-try to erase the flash");
    }

    if (EraseTryCount == 0U)
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

  uint32_t     flash_base_sector, flash_base;
  st_flash_storage_get_partition(type, &flash_base, &flash_base_sector, NULL, NULL);
  void *pFlash = (void *)platform_persistent_ptr_partition(type, 0);
  memcpy(pData, pFlash, szData);
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
  if ((((int32_t)baseOffset)  < 0) || (baseOffset > (szPartition * KILO)))
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
  uint8_t  slot[PROG_PAGE_SIZE_INT];
  while (szStorage > 0U)
  {
    /* Decompose flash base + offset */
    /* Remove lower bits */
    uint32_t msk = (sizeof(slot) - 1U);
    uint32_t alignSlot_base = flash_base_address & (~msk);
    /* Compute the offset */
    uint32_t alignSlot_off  = flash_base_address & msk;
    /* Fills with the pre-existing data */
    memcpy((void *)slot, (void *) alignSlot_base, sizeof(slot));/*cstat !MISRAC2012-Rule-11.6 the HAL API expect an offset rather than a pointer, ie we need to cast a pointer as int */

    /* Read existing data in flash */
    uint32_t szSlot = sizeof(slot);
    /* Clamps to the max size */
    if (szStorage  < szSlot)
    {
      szSlot = szStorage ;
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
    szStorage -= szSlot;
    pStorage  += szSlot;
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
  return  ST_STORAGE_OK;
  #else
  return  ST_STORAGE_ERROR;
  #endif
}


/**
* @brief  Generic write a data in a partition
* @param  type                 slot type
* @param  flash_base_address   offset
* @param  pData                sector pointer
* @param  szData               sector pointer size
* @retval error code
*/
ST_Storage_Result st_flash_storage_write(st_flash_storage_type type, uint32_t flash_base_address, void *pData, uint32_t szData)
{
#define BLOCK_SIZE 256UL
  uint8_t           block[BLOCK_SIZE];
  uint8_t          *pDataU8                      = (uint8_t *)pData;
  uint32_t          flash_address                = flash_base_address;
  uint32_t          flash_address_mod_block_size = flash_base_address % BLOCK_SIZE;
  uint32_t          flash_storage_size           = flash_address_mod_block_size + szData;
  uint32_t          flash_block_size             = (flash_storage_size > BLOCK_SIZE) ? BLOCK_SIZE : flash_storage_size;
  ST_Storage_Result ret                          = ST_STORAGE_OK;

  if (flash_address_mod_block_size > 0UL)
  {
    // retrieve start of first block in flash to avoid its modification
    flash_address -= flash_address_mod_block_size;
    memcpy(block, (uint8_t *)(tFlashStorage[type].offset + flash_address), flash_address_mod_block_size);
  }

  st_os_mutex_lock(&hMutexStorage);
  while ((ret == ST_STORAGE_OK) && (flash_storage_size > 0UL))
  {
    memcpy(&block[flash_address_mod_block_size], pDataU8, flash_block_size - flash_address_mod_block_size);
    pDataU8                     += flash_block_size - flash_address_mod_block_size;
    flash_address_mod_block_size = 0UL;
    if (flash_block_size < BLOCK_SIZE)
    {
      // retrieve end of last block in flash to avoid its modification
      memcpy(&block[flash_block_size], (uint8_t *)(tFlashStorage[type].offset + flash_base_address + szData), BLOCK_SIZE - flash_block_size);
    }
    if (platform_persistent_is_flashExt(type))
    {
      if (iFlashExtLocked != st_flash_storage_LOCK_PROG)
      {
        st_flash_storage_lock(st_flash_storage_FLASH_EXT, st_flash_storage_LOCK_PROG);
      }
      ret = platform_persistent_storage_write_flashExt(type, flash_address, block, BLOCK_SIZE);
      st_flash_storage_lock(st_flash_storage_FLASH_EXT, iFlashExtLocked);
    }
    else
    {
      ret = platform_persistent_storage_write_internal_flash(type, flash_address, block, BLOCK_SIZE);
    }
    flash_address      += flash_block_size;
    flash_storage_size -= flash_block_size;
    flash_block_size    = (flash_storage_size > BLOCK_SIZE) ? BLOCK_SIZE : flash_storage_size;
    memcpy(block, pDataU8, flash_block_size);
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
* @param  fnProgress   sector callback
* @retval error code
*/
ST_Storage_Result st_flash_storage_erase(st_flash_storage_type type)
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
  uint32_t     flash_base_sector;

  st_os_mutex_lock(&hMutexStorage);
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);

  if (platform_persistent_is_flashExt(type))
  {
    if (iFlashExtLocked  == st_flash_storage_LOCK_PROG)
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
    ret =  ST_STORAGE_ERROR;
  }
  st_os_mutex_unlock(&hMutexStorage);

  return ret;
}


ST_Storage_Result st_flash_storage_read_sector(st_flash_storage_type type, uint32_t indexSector, void *pData, uint32_t szData)
{
  ST_Storage_Result ret = ST_STORAGE_ERROR;
  uint32_t     sizeSector = tFlashStorage[type].sectorSize * KILO;
  uint32_t     flash_base_sector;

  st_os_mutex_lock(&hMutexStorage);
  st_flash_storage_get_partition(type, NULL, &flash_base_sector, NULL, NULL);

  if (platform_persistent_is_flashExt(type))
  {
    if (iFlashExtLocked  == st_flash_storage_LOCK_PROG)
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
        result = (BSP_XSPI_NOR_Read(FLASHEXT_INSTANCE, pData, offset, blk) == BSP_ERROR_NONE) ? true : false;
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
  uint32_t     sizeSector = tFlashStorage[type].sectorSize * KILO;
  uint32_t     flash_base_sector;

  st_os_mutex_lock(&hMutexStorage);
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
        result = (BSP_XSPI_NOR_Write(FLASHEXT_INSTANCE, pData, offset, blk) == BSP_ERROR_NONE) ? true : false;
      }
      szData -= blk;
      offset += blk;
      pData = (uint8_t *)pData + blk;
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
ST_Storage_Result platform_persistent_storage_is_locked(st_flash_storage_type type, st_flash_storage_mode mode)
{
  if (type == st_flash_storage_FLASH_EXT)
  {
    return (iFlashExtLocked == mode) ? ST_STORAGE_OK : ST_STORAGE_ERROR;
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
      /* lock prg for internal flash, always mapped */
      break;

    case st_flash_storage_FLASH_EXT:
      #if !defined(ST_NO_FLASHEXT)
      if (iFlashExtLocked != mode)
      {
        switch (mode)
        {
          case st_flash_storage_LOCK_PROG:
            #ifdef MEMORYMAP_WOARKAROUND
            if (iFlashInitialized)
            {
              iFlashInitialized--;
              BSP_XSPI_NOR_DeInit(0);
            }
            if (BSP_XSPI_NOR_Init(FLASHEXT_INSTANCE, &hInstanceFlashExt) != BSP_ERROR_NONE)
            {
              ret = ST_STORAGE_ERROR;
            }
            iFlashInitialized++;
            #else
            if (BSP_XSPI_NOR_DisableMemoryMappedMode(FLASHEXT_INSTANCE) != BSP_ERROR_NONE)
            {
              ret = ST_STORAGE_ERROR;
            }
            #endif

            iFlashExtLocked = mode;
            break;

          case st_flash_storage_LOCK_MAP:
            #ifdef MEMORYMAP_WOARKAROUND
            /* on H7, BSP_OSPI_NOR_DisableMemoryMappedMode/BSP_OSPI_NOR_EnableMemoryMappedMode  doesn't work, we need to init from scratch the flash */
            if (iFlashInitialized)
            {
              iFlashInitialized--;
              BSP_XSPI_NOR_DeInit(0);
            }
            iFlashInitialized++;
            if (BSP_XSPI_NOR_Init(FLASHEXT_INSTANCE, &hInstanceFlashExt) != BSP_ERROR_NONE)
            {
              ret = ST_STORAGE_ERROR;
            }
            if (ret == ST_STORAGE_OK)
            {
              if (BSP_XSPI_NOR_EnableMemoryMappedMode(FLASHEXT_INSTANCE) != BSP_ERROR_NONE)
              {
                ret = ST_STORAGE_ERROR;
              }
            }
            #else
            if (BSP_XSPI_NOR_EnableMemoryMappedMode(FLASHEXT_INSTANCE) != BSP_ERROR_NONE)
            {
              ret = ST_STORAGE_ERROR;
            }
            #endif

            iFlashExtLocked = mode;
            break;
        }
      }
      #else
      ret = ST_STORAGE_ERROR;
      #endif
      break;

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
* @param  sectorSize    sector size
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

/*
return info concerning a flash partition
*/


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
  /* Other boards  may need to override this function*/
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
