/**
******************************************************************************
* @file          audio_chain_json_transmission.h
* @author        MCD Application Team
* @brief         manage the transmission to livetune
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



#ifndef __AUDIOCHAIN_JSON_TRANSMISSION
#define __AUDIOCHAIN_JSON_TRANSMISSION

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "st_pmem.h"
#include "st_os_mem.h"

/* Exported constants --------------------------------------------------------*/
/* Exported macros ------------------------------------------------------------*/
#define FREE_POSTPONED_MAX      4U

/* Exported types ------------------------------------------------------------*/



typedef bool (* audio_trans_sync)(const void *pBlock, uint32_t szBlock);
typedef bool (* audio_trans_async)(const void *pBlock, uint32_t szBlock, void (*cbFreeBlock)(void *pFreeBlock), void *pMem2Free);


typedef struct audio_json_transmission
{

  pmem_pool_t       hSendBlockPool;
  void              *pPoolSendBlockPool;

  volatile uint8_t  nbMem2free;
  void             *pMem2free[FREE_POSTPONED_MAX];

  audio_trans_sync   sendBlockSync;
  audio_trans_async  sendBlockASync;

} audio_json_transmission;

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

bool                     audio_json_transmission_send(audio_json_transmission *pHandle, void *pBlock, uint32_t szBlock, bool bAsynchronous);
audio_json_transmission *audio_json_transmission_init(uint32_t szMallocPool);
bool                     audio_json_transmission_term(audio_json_transmission *pHandle);
void                    *audio_json_transmission_send_block_alloc(audio_json_transmission *pHandle, uint32_t szAlloc);
void                    *audio_json_transmission_send_block_realloc(audio_json_transmission *pHandle, void *prevBlock, uint32_t szAlloc);
void                     audio_json_transmission_send_block_free(audio_json_transmission *pHandle, void *pBlock);
bool                     audio_json_transmission_send_sync(audio_json_transmission *pHandle);
bool                     audio_json_transmission_set_cb(audio_json_transmission *pHandle, audio_trans_sync syncCB, audio_trans_async asyncCB);

#ifdef __cplusplus
};
#endif


#endif  // __AUDIOCHAIN_JSON_TRANSMISSION

