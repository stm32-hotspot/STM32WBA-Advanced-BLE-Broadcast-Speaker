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

#ifndef st_flash_storage_conf_h
#define st_flash_storage_conf_h


#ifdef __cplusplus
extern "C"
{
#endif

typedef enum st_flash_storage_type
{
  st_flash_storage_REGISTRY,
  st_flash_storage_SAVE_JSON,
  st_flash_storage_THF,
  st_flash_storage_MP3,
  st_flash_storage_MAX,
  st_flash_storage_FLASH_EXT
} st_flash_storage_type;

#if defined(STM32H735xx)

#define ST_USE_FLASHEXT
#define ST_BASE_FLASH_EXT             0x90000000UL

#define ST_FLASH_REGISTRY_BASE_ADDR        (ST_BASE_FLASH_EXT + (ST_FLASH_REGISTRY_BASE_SECTOR_SIZE * ST_FLASH_REGISTRY_BASE_SECTOR* 1024UL)) //0x90000000
#define ST_FLASH_REGISTRY_BASE_SECTOR      (0U) /* must be aligned on 4K boundary */
#define ST_FLASH_REGISTRY_BASE_SECTOR_NB   (16U) /* 64K */
#define ST_FLASH_REGISTRY_BASE_SECTOR_SIZE (4U)  /* Flash ext rease sector 4K */

#define ST_FLASH_SAVE_JSON_BASE_ADDR   (ST_BASE_FLASH_EXT + (ST_FLASH_SAVE_JSON_BASE_SECTOR_SIZE * ST_FLASH_SAVE_JSON_BASE_SECTOR * 1024UL)) //0x90140000
#define ST_FLASH_SAVE_JSON_BASE_SECTOR          32U /* must be aligned on 4K boundary */
#define ST_FLASH_SAVE_JSON_BASE_SECTOR_NB       (16U) /* 64K */
#define ST_FLASH_SAVE_JSON_BASE_SECTOR_SIZE     (4U)  /* Flash ext rease sector 4K */




#define ST_FLASH_THF_BASE_ADDR   (ST_BASE_FLASH_EXT + (ST_FLASH_THF_BASE_SECTOR_SIZE * ST_FLASH_THF_BASE_SECTOR * 1024UL)) //0x90010000
#define ST_FLASH_THF_BASE_SECTOR           (256U) /* must be aligned on 4K boundary */
#define ST_FLASH_THF_BASE_SECTOR_NB        (256U) /* 1M */
#define ST_FLASH_THF_BASE_SECTOR_SIZE     (4U)  /* Flash ext rease sector 4K */


#define ST_FLASH_MP3_BASE_ADDR   (ST_BASE_FLASH_EXT + (ST_FLASH_MP3_BASE_SECTOR_SIZE * ST_FLASH_MP3_BASE_SECTOR * 1024UL)) //0x90140000
#define ST_FLASH_MP3_BASE_SECTOR          512U /* must be aligned on 4K boundary */
#define ST_FLASH_MP3_BASE_SECTOR_NB       (256U) /* 1M */
#define ST_FLASH_MP3_BASE_SECTOR_SIZE     (4U)  /* Flash ext rease sector 4K */





#elif defined(STM32H573xx)

#define ST_USE_FLASHEXT
#define ST_BASE_FLASH_EXT             0x90000000UL


#define ST_FLASH_REGISTRY_BASE_ADDR        (ST_BASE_FLASH_EXT + (ST_FLASH_REGISTRY_BASE_SECTOR_SIZE * ST_FLASH_REGISTRY_BASE_SECTOR* 1024UL)) //0x90000000
#define ST_FLASH_REGISTRY_BASE_SECTOR      (0U) /* must be aligned on 4K boundary */
#define ST_FLASH_REGISTRY_BASE_SECTOR_NB   (16U) /* 64K */
#define ST_FLASH_REGISTRY_BASE_SECTOR_SIZE (4U)  /* Flash ext rease sector 4K */

#define ST_FLASH_SAVE_JSON_BASE_ADDR   (ST_BASE_FLASH_EXT + (ST_FLASH_SAVE_JSON_BASE_SECTOR_SIZE * ST_FLASH_SAVE_JSON_BASE_SECTOR * 1024UL)) //0x90140000
#define ST_FLASH_SAVE_JSON_BASE_SECTOR          32U /* must be aligned on 4K boundary */
#define ST_FLASH_SAVE_JSON_BASE_SECTOR_NB       (16U) /* 64K */
#define ST_FLASH_SAVE_JSON_BASE_SECTOR_SIZE     (4U)  /* Flash ext rease sector 4K */




#define ST_FLASH_THF_BASE_ADDR   (ST_BASE_FLASH_EXT + (ST_FLASH_THF_BASE_SECTOR_SIZE * ST_FLASH_THF_BASE_SECTOR * 1024UL)) //0x90010000
#define ST_FLASH_THF_BASE_SECTOR           (256U) /* must be aligned on 4K boundary */
#define ST_FLASH_THF_BASE_SECTOR_NB        (256U) /* 1M */
#define ST_FLASH_THF_BASE_SECTOR_SIZE     (4U)  /* Flash ext rease sector 4K */


#define ST_FLASH_MP3_BASE_ADDR   (ST_BASE_FLASH_EXT + (ST_FLASH_MP3_BASE_SECTOR_SIZE * ST_FLASH_MP3_BASE_SECTOR * 1024UL)) //0x90140000
#define ST_FLASH_MP3_BASE_SECTOR          512U /* must be aligned on 4K boundary */
#define ST_FLASH_MP3_BASE_SECTOR_NB       (256U) /* 1M */
#define ST_FLASH_MP3_BASE_SECTOR_SIZE     (4U)  /* Flash ext rease sector 4K */



#else
#error "Platform not supported"
#endif


#define ST_PERSISTENT_REGISTRY     st_flash_storage_REGISTRY
#ifdef __cplusplus
};
#endif

#endif


