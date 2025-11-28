/**
******************************************************************************
* @file        livetune_hook_hw_conf.c
* @author      MCD Application Team
* @brief       Hook the standard platform to add QSPI and Ram Ext
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
#include "platform_setup_conf.h"
#include "st_flash_storage.h"
#include "stm32_audio_conf.h"
#include "string.h"


#ifdef USE_LIVETUNE_DESIGNER

void MPU_Config_Hook(uint8_t regionNumber, uint8_t AttrbNumber);
void MPU_Config_Hook(uint8_t regionNumber, uint8_t AttrbNumber)
{
  #ifdef ST_FLASH_REGISTRY_BASE_ADDR

  MPU_Attributes_InitTypeDef   attrRegistry;
  MPU_Region_InitTypeDef       regionRegistry;
  /* Define cacheable memory via MPU */
  attrRegistry.Number             = AttrbNumber;
  attrRegistry.Attributes         = MPU_NOT_CACHEABLE;
  HAL_MPU_ConfigMemoryAttributes(&attrRegistry);

  /* BaseAddress-LimitAddress configuration */
  regionRegistry.Enable           = MPU_REGION_ENABLE;
  regionRegistry.Number           = regionNumber; /* last one */
  regionRegistry.AttributesIndex  = AttrbNumber;
  regionRegistry.BaseAddress      = ST_FLASH_REGISTRY_BASE_ADDR;
  regionRegistry.LimitAddress     = (ST_FLASH_REGISTRY_BASE_ADDR + (ST_FLASH_REGISTRY_BASE_SECTOR * ST_FLASH_REGISTRY_BASE_SECTOR_SIZE * 1024U));
  regionRegistry.AccessPermission = MPU_REGION_ALL_RW;
  regionRegistry.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  regionRegistry.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  HAL_MPU_ConfigRegion(&regionRegistry);
  regionNumber++;
  AttrbNumber++;

  #endif


  #ifdef ST_USE_PMEM
  #endif

}




void livetune_hook_hw_conf(void);
/* System hook init , TODO: Put me at the right place and file */

void livetune_hook_hw_conf(void)
{


}

#endif // USE_LIVETUNE_DESIGNER
