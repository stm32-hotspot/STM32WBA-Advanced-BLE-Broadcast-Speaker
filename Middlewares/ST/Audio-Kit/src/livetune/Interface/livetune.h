/**
******************************************************************************
* @file          livetune.h
* @author        MCD Application Team
* @brief         Header: implement the designer support
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



#ifndef LIVETUNE_H
#define LIVETUNE_H

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include "st_base.h"
#include "st_os.h"
#include "st_plugin_manager.h"
#include "st_message.h"
#include "st_registry.h"
#include "st_pmem.h"
#include "livetune_pipe.h"
#include "livetune_db_json.h"
#include "livetuneConf.h"

/* Exported constants --------------------------------------------------------*/
#define ST_INSTANCE_SIGNATURE 0x43494f56 /* VOIC */
#define ST_AC_DEFAULT_OUT_MAX 1
#define ST_AC_DEFAULT_IN_MAX  1

#define TRANSITION_START (0xF1) // 241
#define TRANSITION_STOP  (0xF2) // 242
#define TRANSITION_SYNC  (0xF3) // 243

#define UTIL_UART_STREAM_SIZE 10 /* nb  dma buffer */

#define FREE_POSTPONED_MAX 10U

#define ST_STATE_DEFAULT ST_STATE_FLG_AUTO_START || ST_STATE_FLG_HEART

/* Exported types ------------------------------------------------------------*/

typedef struct livetune_instance /* main instance , gather all collection instances in a single struct */
{
  uint32_t          iSignature;
  st_plugin_manager hPlugins;
  st_message        hMessage;
  st_registry       hRegistry;
  livetune_db       hDesignerDB;
  livetune_pipe     hAudioPipe;
  st_mutex          hSendLock;
  st_event          hSendEvt;
  void             *pPoolSendBlock;
  pmem_pool_t       hSendBlockPool;
  uint32_t          iStateFlag;
  uint8_t           iChunkPool;
  uint8_t           iAlgoPool;
  uint8_t          *pStartComBlock;
  uint8_t           bSystemReady;
  volatile uint8_t  nbMem2free;
  void             *pMem2free[FREE_POSTPONED_MAX];

} livetune_instance;


typedef void (*livetune_free_send_block_cb)(void *pBlock, void *pCookie);


/* Exported macros ------------------------------------------------------------*/

#ifndef BUTTON_WAKEUP
#define BUTTON_WAKEUP BUTTON_USER
#endif
#if defined(STM32H573xx)
#define LIVETUNE_SEND_BLOCK_SIZE (70 * 1024)
#else
#define LIVETUNE_SEND_BLOCK_SIZE (150 * 1024)
#endif

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

uint32_t              livetune_send(uint32_t evt, st_message_param wParam, st_message_param lParam);
uint32_t              livetune_post(uint32_t evt, st_message_param wParam, st_message_param lParam);
uint32_t              livetune_post_delete_cb(uint32_t evt, st_message_param wParam, st_message_param lParam, void (*deleteCB)(struct st_message_post_item *pPost));
livetune_instance *livetune_get_instance(void);
ST_Result             livetune_term(void);
ST_Result             livetune_create(void);
ST_Result             livetune_delete(void);
void                  livetune_reset(void);
void                  livetune_update_string(char_t **pString, const char_t *pNewString);
uint32_t              livetune_send_and_wait_consumed(uint8_t *pBuffer, uint32_t szBuffer, uint32_t bAddStartCode);
uint32_t              livetune_send_block_binary_async(const char_t *pSignature, char_t *pTitle, char_t *pExtra, uint8_t *pBin, uint32_t szBin);
uint32_t              livetune_send_block_text_async(uint8_t bForce, const char_t *pSignature, char_t *pTitle, char_t *pExtra, char_t *pText);
int8_t                livetune_send_block_async(const char_t *pBlock, uint32_t szBlock, livetune_free_send_block_cb cbFreeBlock);
uint8_t               livetune_send_sync_host(void);
void                 *livetune_send_block_alloc(uint32_t szAlloc);
void                 *livetune_send_block_realloc(void *prevBlock, uint32_t szAlloc);
void                  livetune_send_block_free(void *prevBlock);
uint32_t              livetune_base64_encode(uint8_t *pSrc, uint32_t szSrc, char_t *pDst, uint32_t szDst);
uint32_t              livetune_state_get_flags(void);
uint32_t              livetune_state_set_flags(uint32_t flags);
uint32_t              livetune_state_get_chunk_pool(void);
uint32_t              livetune_state_set_chunk_pool(uint8_t mem_pool);
uint32_t              livetune_state_get_algo_pool(void);
uint32_t              livetune_state_set_algo_pool(uint8_t mem_pool);
int32_t               livetune_wait_ready(void);

/* system hooks by weak */
void                  AudioChainInstance_initGraph(void);
void                  SystemHook_Init(void);
void                  main_hooks_systemReset(void);


#ifdef __cplusplus
};
#endif
#endif

