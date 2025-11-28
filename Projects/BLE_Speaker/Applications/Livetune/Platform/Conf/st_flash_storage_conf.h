/**
******************************************************************************
* @file          st_flash_storage_conf.h
* @author        MCD Application Team
* @brief         Board HW init
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

#ifndef ST_FLASH_STORAGE_CONF_H
#define ST_FLASH_STORAGE_CONF_H


#ifdef __cplusplus
extern "C"
{
#endif


#ifdef USE_LIVETUNE_DESIGNER
#include "st_base.h"
/* connect storage to st_base error mngs */

#define ST_STORAGE_TRACE_INFO       ST_TRACE_INFO
#define ST_STORAGE_TRACE_ERROR      ST_TRACE_ERROR
#define ST_STORAGE_VERIFY(arg)      ST_VERIFY(arg)
#define ST_STORAGE_ASSERT(arg)      ST_ASSERT(arg)

#endif /* USE_LIVETUNE_DESIGNER */

typedef enum st_flash_storage_type
{
  st_flash_storage_REGISTRY,
  st_flash_storage_SAVE_JSON1,
  st_flash_storage_SAVE_JSON2,
  st_flash_storage_SAVE_JSON3,
  st_flash_storage_MAX,
  st_flash_storage_FLASH_EXT
} st_flash_storage_type;



#if defined(STM32H573xx)

#define ST_BASE_FLASH_INT                       (0x081F0000UL) /* have 64K available from here */


#define ST_FLASH_REGISTRY_BASE_ADDR             (ST_BASE_FLASH_INT + (ST_FLASH_REGISTRY_BASE_SECTOR_SIZE * ST_FLASH_REGISTRY_BASE_SECTOR* 1024UL)) //0x081F0000
#define ST_FLASH_REGISTRY_BASE_SECTOR           (0U)
#define ST_FLASH_REGISTRY_BASE_SECTOR_NB        (2U)  /* 16K */
#define ST_FLASH_REGISTRY_BASE_SECTOR_SIZE      (8U)  /* Flash ext rease sector 4K */

#define ST_FLASH_SAVE_JSON1_BASE_ADDR           (ST_BASE_FLASH_INT + (ST_FLASH_SAVE_JSON1_BASE_SECTOR_SIZE * ST_FLASH_SAVE_JSON1_BASE_SECTOR * 1024UL)) //0x081F4000
#define ST_FLASH_SAVE_JSON1_BASE_SECTOR         (2U)
#define ST_FLASH_SAVE_JSON1_BASE_SECTOR_NB      (2U ) /* 16K */
#define ST_FLASH_SAVE_JSON1_BASE_SECTOR_SIZE    (8U)  /* Flash ext rease sector 4K */

#define ST_FLASH_SAVE_JSON2_BASE_ADDR           (ST_BASE_FLASH_INT + (ST_FLASH_SAVE_JSON2_BASE_SECTOR_SIZE * ST_FLASH_SAVE_JSON2_BASE_SECTOR * 1024UL)) //0x081F8000
#define ST_FLASH_SAVE_JSON2_BASE_SECTOR         (4U)
#define ST_FLASH_SAVE_JSON2_BASE_SECTOR_NB      (2U)  /* 16K */
#define ST_FLASH_SAVE_JSON2_BASE_SECTOR_SIZE    (8U)  /* Flash ext rease sector 4K */

#define ST_FLASH_SAVE_JSON3_BASE_ADDR           (ST_BASE_FLASH_INT + (ST_FLASH_SAVE_JSON3_BASE_SECTOR_SIZE * ST_FLASH_SAVE_JSON3_BASE_SECTOR * 1024UL)) //0x081FC000
#define ST_FLASH_SAVE_JSON3_BASE_SECTOR         (6U)
#define ST_FLASH_SAVE_JSON3_BASE_SECTOR_NB      (2U)  /* 16K */
#define ST_FLASH_SAVE_JSON3_BASE_SECTOR_SIZE    (8U)  /* Flash ext rease sector 4K */

#else
#error "Platform not supported"
#endif


#define ST_PERSISTENT_REGISTRY     st_flash_storage_REGISTRY
#ifdef __cplusplus
};
#endif

#endif


