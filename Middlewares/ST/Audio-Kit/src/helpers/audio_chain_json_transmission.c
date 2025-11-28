/**
******************************************************************************
* @file          audio_chain_json_transmission.c
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

#include "audio_chain.h"
#include "st_json.h"
#include "st_os.h"
#include "st_os_mem.h"
#include "stdio.h"
#include "stdbool.h"
#include "audio_chain_json_transmission.h"
#include "assert.h"

#define SZ_STARTCODE     (1UL)
#define SZ_CHECKSUM      (4UL)
#define TRANSITION_START (0xF1) // 241
#define TRANSITION_STOP  (0xF2) // 242
#define TRANSITION_SYNC  (0xF3) // 243




/**
* @brief free a block used by the send block DMA
*/
static void livetune_send_free_cb(void *pBlock)
{
  if (pBlock)
  {
    /*
      we cannot free a block in an isr, because we can corrupt the malloc chain given we cannot lock a critical section.
      so, to work around this situation, we record the free in a list and the list of free will be freed during the next malloc or realloc
    */

    audio_json_transmission *pHandle = NULL;
    assert(pHandle->nbMem2free < FREE_POSTPONED_MAX);
    pHandle->pMem2free[pHandle->nbMem2free] = pBlock;
    pHandle->nbMem2free++;
  }
}


/**
* @brief Set the raw transmission function callback
*/
bool audio_json_transmission_set_cb(audio_json_transmission *pHandle, audio_trans_sync syncCB, audio_trans_async asyncCB)
{
  bool ok = false;
  if (pHandle)
  {
    pHandle->sendBlockSync  = syncCB;
    pHandle->sendBlockASync = asyncCB;

    ok = true;
  }
  return ok;
}


/**
 * @brief  Free all block postponed
 *
 * @param pHandle the instance
 */
static void audio_json_transmission_send_block_free_isr_alloc(audio_json_transmission *pHandle)
{
  while (pHandle->nbMem2free)
  {
    void *pBlk = pHandle->pMem2free[0];
    pHandle->nbMem2free--;
    memmove(&pHandle->pMem2free[0], &pHandle->pMem2free[1], pHandle->nbMem2free * sizeof(pHandle->pMem2free[0]));
    pmem_free(&pHandle->hSendBlockPool, pBlk);
  }
}


/**
 * @brief Alloc an ISR block
 *
 */
void *audio_json_transmission_send_block_alloc(audio_json_transmission *pHandle, uint32_t szAlloc)
{
  st_os_lock_tasks();
  audio_json_transmission_send_block_free_isr_alloc(pHandle);
  void *pPtr = pmem_alloc(&pHandle->hSendBlockPool, szAlloc);
  st_os_unlock_tasks();

  return pPtr;
}


/**
 * @brief ReAlloc an ISR block
 *
 * @param prevBlock  prev block
 * @param szAlloc sz to re-alloc
 * @return void*
 */
void *audio_json_transmission_send_block_realloc(audio_json_transmission *pHandle, void *prevBlock, uint32_t szAlloc)
{
  st_os_lock_tasks();
  audio_json_transmission_send_block_free_isr_alloc(pHandle);
  void *pPtr = pmem_realloc(&pHandle->hSendBlockPool, prevBlock, szAlloc);
  st_os_unlock_tasks();
  return pPtr;
}


/**
 * @brief the free could be called from ISR, so we use pmem_free_postponed
 *
 * @param pBlock the block
 */
void audio_json_transmission_send_block_free(audio_json_transmission *pHandle, void *pBlock)
{
  st_os_lock_tasks();
  audio_json_transmission_send_block_free_isr_alloc(pHandle);
  pmem_free(&pHandle->hSendBlockPool, pBlock);
  st_os_unlock_tasks();
}


/**
* @brief Send block using the livtune protocol
*        notice: the block must be a malloced block because we use realloc to minimize the peak of memory allocated twice to add headers
*/
static uint8_t audio_json_thexa_char(uint32_t val)
{
  if (val < 10U)
  {
    return (uint8_t)'0' + (uint8_t)val;
  }
  else
  {
    return (uint8_t)'A' + ((uint8_t)val - 10U);
  }
}


/**
 * @brief  Init the transmission
 *
 * @return pHandle the instance
 */
audio_json_transmission *audio_json_transmission_init(uint32_t szMallocPool)
{
  audio_json_transmission *pHandle = st_os_mem_alloc(ST_Mem_Type_ANY_FAST, sizeof(audio_json_transmission));
  if (pHandle)
  {
    memset(pHandle, 0, sizeof(*pHandle));
    pHandle->pPoolSendBlockPool = st_os_mem_alloc(ST_Mem_Type_ANY_FAST, szMallocPool);
    if (pHandle->pPoolSendBlockPool)
    {
      pmem_init(&pHandle->hSendBlockPool, pHandle->pPoolSendBlockPool, szMallocPool, 0);
    }
    else
    {
      st_os_mem_free(pHandle);
      pHandle = NULL;
    }
  }
  return pHandle;
}


/**
 * @brief  term the transmission
 * @param  pHandle the instance
 * @return true or false
*/
bool audio_json_transmission_term(audio_json_transmission *pHandle)
{
  if (pHandle->pPoolSendBlockPool)
  {
    st_os_mem_free(pHandle->pPoolSendBlockPool);
    pHandle->pPoolSendBlockPool = NULL;
  }
  if (pHandle)
  {
    st_os_mem_free(pHandle);
  }
  return true;
}


/**
* @brief Send block using the livetune protocol
         the block must be freed after send
*/
bool audio_json_transmission_send(audio_json_transmission *pHandle, void *pBlock, uint32_t szBlock, bool bAsynchronous)
{
  bool     ok       = false;
  uint32_t szPacket = szBlock + (2UL * SZ_STARTCODE) + SZ_CHECKSUM;
  uint8_t *pPacket  = audio_json_transmission_send_block_alloc(pHandle, szPacket);

  if (pPacket)
  {
    memcpy(pPacket + SZ_STARTCODE + SZ_CHECKSUM, pBlock, szBlock);
    pPacket[0] = TRANSITION_START;
    pPacket[SZ_STARTCODE + SZ_CHECKSUM + szBlock] = TRANSITION_STOP;

    /* build a check sum */
    uint8_t *pCurChk = (pPacket + SZ_STARTCODE + SZ_CHECKSUM);
    uint32_t chk     = 0UL;
    uint32_t count   = szBlock;

    while (count) /*cstat !MISRAC2012-Rule-18.3 false positif */
    {
      chk += *pCurChk++;
      count--;
    }
    chk &= 0xFFFFU;
    /* We need to pass it in ascii, because START/STOP code are not allowed in the block */

    pPacket[SZ_STARTCODE + 0UL] = audio_json_thexa_char((chk >> (3UL * 4UL)) & 0xFUL);
    pPacket[SZ_STARTCODE + 1UL] = audio_json_thexa_char((chk >> (2UL * 4UL)) & 0xFUL);
    pPacket[SZ_STARTCODE + 2UL] = audio_json_thexa_char((chk >> (1UL * 4UL)) & 0xFUL);
    pPacket[SZ_STARTCODE + 3UL] = audio_json_thexa_char((chk) & 0xFUL);
    if (bAsynchronous)
    {
      ok = true;
      if (pHandle->sendBlockASync)
      {
        ok = pHandle->sendBlockASync(pPacket, szPacket, livetune_send_free_cb, (void *)pHandle);
      }
    }
    else
    {
      ok = true;
      if (pHandle->sendBlockSync)
      {
        ok = pHandle->sendBlockSync(pPacket, szPacket);
      }
      audio_json_transmission_send_block_free(pHandle, pPacket);
    }
  }
  return ok;
}


/**
* @brief Sync the PC host with the board
*        we need to send a sync byte in order to tell to the host that the communication will start
*        this to prevent the host catches an half packet
*        the host won't  parse packet before to get the sync byte
* @param  pHandle the instance
* @return state  true or false
*/
bool audio_json_transmission_send_sync(audio_json_transmission *pHandle)
{
  bool ok = false;
  if (pHandle->sendBlockSync)
  {
    uint8_t sync = TRANSITION_SYNC;
    ok = pHandle->sendBlockSync(&sync, 1);
  }
  return ok;
}
