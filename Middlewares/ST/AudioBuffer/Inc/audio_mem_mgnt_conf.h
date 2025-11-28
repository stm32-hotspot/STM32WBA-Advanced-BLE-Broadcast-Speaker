/**
******************************************************************************
* @file    audio_mem_mgnt_conf.h
* @author  MCD Application Team
* @brief   configuration file for audio_mem_mgnt.c management
******************************************************************************
* @attention
*
* Copyright (c) 2019(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_MEM_MGNT_CONF_H
#define __AUDIO_MEM_MGNT_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
//#define AUDIO_MEM_NOCACHED_ENABLED

/* Exported functions ------------------------------------------------------- */

// #define AUDIO_MEM_CONF_TRACK_MALLOC // may be defined in the project option according to the target needs.

#ifdef ST_USE_PMEM
#define AUDIO_MEM_CONF_STOS_USED // optional: if STPmem is used in the project, we use it as well for AudioMalloc
#endif

#ifdef AUDIO_MEM_CONF_TRACK_MALLOC
//#define AUDIO_MEM_CONF_TRACK_MALLOC_VERBOSE1
//#define AUDIO_MEM_CONF_TRACK_MALLOC_VERBOSE2
#define AUDIO_MEM_CONF_ASSERT_ENABLED
#endif

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_MEM_MGNT_CONF_H */
