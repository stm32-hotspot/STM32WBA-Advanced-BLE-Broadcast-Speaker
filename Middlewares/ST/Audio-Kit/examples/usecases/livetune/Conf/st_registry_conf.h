/**
******************************************************************************
* @file          st_registry_conf.h
* @author        MCD Application Team
* @brief         it is the private  SDK implementation ,  internal functions
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


#ifndef ST_REGISTRY_CONF_H
#define ST_REGISTRY_CONF_H
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */




#if defined(USE_LIVETUNE_DESIGNER) || defined(USE_LIVETUNE_TUNER)

  #define ST_Mem_Type_Registry         ST_Mem_Type_ANY_SLOW

  /* connect registry to st_base error mngs */
  #define ST_REGISTRY_TRACE_INFO       ST_TRACE_INFO
  #define ST_REGISTRY_TRACE_ERROR      ST_TRACE_ERROR
  #define ST_REGISTRY_TRACE_DEBUG      ST_TRACE_DEBUG
  #define ST_REGISTRY_VERIFY(arg)      ST_VERIFY(arg)
  #define ST_REGISTRY_ASSERT(arg)      ST_ASSERT(arg)

#endif /* USE_LIVETUNE_DESIGNER || USE_LIVETUNE_TUNER */

#define ST_INSTANCE_MAGIC    0x43443032UL
#define ST_MAX_STORAGE       (910 * 1024)
#define ST_STORAGE_ALIGN_PROG_PAGE  (256)



#define ST_STATE_FLG_AUTO_START           (1UL << 0UL)
#define ST_STATE_FLG_HEART                (1UL << 1UL)
#define ST_STATE_FLG_LOG_VERBOSE_INIT     (1UL << 2UL)
#define ST_STATE_FLG_LOG_VERBOSE_MALLOCS  (1UL << 3UL)
#define ST_STATE_FLG_LOG_VERBOSE_OS       (1UL << 4UL)
#define ST_STATE_FLG_LOG_VERBOSE_CYCLES   (1UL << 5UL)


ST_ALIGN_START
typedef struct st_project_storage
{
  uint32_t magic;
  uint32_t stateStorage;
  uint8_t  iChunkMemoryPool;
  uint8_t  iAlgoMemoryPool;
  uint8_t  futurExtention[100 - 2];
} st_project_storage;
ST_ALIGN_STOP



ST_ALIGN_START
typedef struct st_project_storage_save_json
{
  uint32_t magic;
  uint32_t align_prog_page[ST_STORAGE_ALIGN_PROG_PAGE];
} st_project_storage_save_json;
ST_ALIGN_STOP


/* Warning, if you modify the registry size, make sure that  ST_FLASH_REGISTRY_XXX are coherent */
typedef struct st_registry_config
{
  char_t             tFwVersion[10]; /*!< current FW version  */
  st_project_storage hStorage;
  int32_t            iAudioConfig;
  uint8_t            bRebootAction;
  char               tRebootAction[30];
  uint8_t            padding[255 - 3 - 30]; /*!< Future ext */
} st_registry_config;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#define ST_REGISTRY_CUSTOM st_registry_config
#ifdef __cplusplus
}
#endif

#endif /* ST_REGISTRY_CONF_H */



