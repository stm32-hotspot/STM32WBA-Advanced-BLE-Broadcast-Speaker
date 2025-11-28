/**
******************************************************************************
* @file    audio_chunk_list.h
* @author  MCD Application Team
* @brief   chained list of chunks.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_CHUNK_LIST_H
#define __AUDIO_CHUNK_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "audio_chunk.h"
#include "audio_mem_mgnt.h"

/* Exported types ------------------------------------------------------------*/
typedef struct audio_chunk_list
{
  audio_chunk_t           *pChunk;
  struct audio_chunk_list *next;
  struct audio_chunk_list *prev;
} audio_chunk_list_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t AudioChunkList_getNbElements(audio_chunk_list_t  *const pChunkList);
uint8_t AudioChunkList_getTotalNbChan(audio_chunk_list_t *const pChunkList);


#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_CHUNK_LIST_H */


