/**
******************************************************************************
* @file          livetune.c
* @author        MCD Application Team
* @brief         Implement the designer support
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
#include "platform_setup_conf.h"
#include "livetune.h"
#include "st_flash_storage.h"
#include "livetune_ac_wrapper.h"
#include "stm32_usart.h"
#include "st_flash_storage.h"
#include "string.h"


/* Private defines -----------------------------------------------------------*/
#define TEXT_ATTRB               ",\"Text\":\""
#define SZ_PROLOG_CHECK_SUM_SIZE 4U
#define SZ_PROLOG_EPILOG         (1U + SZ_PROLOG_CHECK_SUM_SIZE + 1U + 1U)

/* Private macros ------------------------------------------------------------*/
#ifndef ST_STATE_DEFAULT
  #define ST_STATE_DEFAULT (ST_STATE_FLG_HEART)
#endif

/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
static livetune_instance *gpInstance;
/* Private functions ------------------------------------------------------- */

static uint8_t livetune_build_hexa_char(uint32_t val)
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
static char_t *livetune_add_block_epilog(char_t *pCurBuffer)
{
  /* build a check sum */
  uint8_t *pCurChk = (uint8_t *)(gpInstance->pStartComBlock + SZ_PROLOG_CHECK_SUM_SIZE);
  uint8_t *pStream = (uint8_t *)pCurBuffer;
  uint32_t chk     = 0;
  while (pCurChk < pStream) /*cstat !MISRAC2012-Rule-18.3 false positif */
  {
    chk += *pCurChk++;
  }
  chk &= 0xFFFFU;
  /* We need to pass it in ascii, because START/STOP code are not allowed in the block */

  gpInstance->pStartComBlock[0] = livetune_build_hexa_char((chk >> (3UL * 4UL)) & 0xFUL);
  gpInstance->pStartComBlock[1] = livetune_build_hexa_char((chk >> (2UL * 4UL)) & 0xFUL);
  gpInstance->pStartComBlock[2] = livetune_build_hexa_char((chk >> (1UL * 4UL)) & 0xFUL);
  gpInstance->pStartComBlock[3] = livetune_build_hexa_char((chk) & 0xFUL);
  /* add stop code terminator */
  *pCurBuffer++ = (char_t)TRANSITION_STOP;
  /* add string terminator */
  *pCurBuffer++ = '\0';
  return pCurBuffer;
}

static char_t *livetune_add_block_prolog(char_t *pCurBuffer)
{
  /* add start code */
  *pCurBuffer++              = (char_t)TRANSITION_START;
  gpInstance->pStartComBlock = (uint8_t *)pCurBuffer;
  *pCurBuffer++              = 'X';
  *pCurBuffer++              = 'X';
  *pCurBuffer++              = 'X';
  *pCurBuffer++              = 'X';
  return pCurBuffer;
}



/**
* @brief Message dispatcher
*
*/

static uint32_t livetune_message_cb(struct st_message *pMsgHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam)
{
  ST_ASSERT(pCookie != NULL);
  return st_plugin_send(&livetune_get_instance()->hPlugins, pCookie, evt, wParam, lParam);
}


/**
* @brief audio message  dispatcher
*
*/

static uint32_t livetune_db_cb(livetune_db *pHandle, uint32_t evt, st_message_param lParam)
{
  livetune_instance *pInstance = livetune_get_instance();
  ST_ASSERT(pInstance != NULL);
  ST_UNUSED(pHandle != NULL);
  return (uint32_t)st_message_send(&pInstance->hMessage, pInstance, (uint32_t)evt, (st_message_param)0U, lParam);
}


/**
 * @brief create   a Designer instance
 *
 * @return ST_Result
 */

static void *livetune_json_realloc(void *pMemory, size_t size)
{
  return st_os_mem_realloc(ST_Mem_Type_ANY_FAST, pMemory, size);
}

ST_Result livetune_create(void)
{
  livetune_instance *pHandle = (livetune_instance *)st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(livetune_instance));
  ST_ASSERT(pHandle != NULL);
  gpInstance = pHandle;
  memset(pHandle, 0, sizeof(*pHandle));
  /* anti-corruption */
  pHandle->iSignature = ST_INSTANCE_SIGNATURE;


  /* Save the persistent callback */

  //ANALYZER_DBG_INIT_DV();


  ST_VERIFY((st_os_mutex_create(&pHandle->hSendLock) == ST_OS_OK));
  ST_VERIFY((st_os_event_create(&pHandle->hSendEvt) == ST_OS_OK));


  /*
  * notice,  sending block using dma require to be able to free memory from the ISR, it is not possible when alloc functions have mutex
  * so, we create a pool only used by send block function , this make sure we can  free a block without crashes
  */

  pHandle->pPoolSendBlock = st_os_mem_alloc(ST_Mem_Type_Designer, LIVETUNE_SEND_BLOCK_SIZE);
  ST_ASSERT(pHandle->pPoolSendBlock != NULL);
  pmem_init(&pHandle->hSendBlockPool, pHandle->pPoolSendBlock, LIVETUNE_SEND_BLOCK_SIZE, 0);


  /* st json takes advantage of the full memory partitions */

  //json_set_alloc_funcs(st_os_mem_generic_realloc_slow, st_os_mem_generic_free);
  json_set_alloc_funcs(livetune_json_realloc, st_os_mem_generic_free);

  void *pPlgEntry;
  ST_PLUGIN_GET_ENTRIES(pPlgEntry);

  ST_VERIFY(st_plugin_manager_create(&pHandle->hPlugins, pPlgEntry) == (int32_t)ST_OK);
  /* instantiate the message management (Post and Send) */
  ST_VERIFY(st_message_create(&pHandle->hMessage, livetune_message_cb) == ST_OS_OK);
  /* create the persistent storage in flash */
  ST_VERIFY(st_registry_create(&pHandle->hRegistry) != (int32_t)ST_ERROR);



  /* the base init is done, we can mount the registry, we need to "send" in direct to make sure that the registry is enable when the call returns */
  st_flash_storage_init();

  livetune_state_set_flags(ST_STATE_FLG_AUTO_START);

  if (st_registry_is_valid(&pHandle->hRegistry) == (int32_t)ST_ERROR)
  {
    ST_TRACE_DEBUG("The registry is corrupted, re-init the registry");
    st_registry_erase(&pHandle->hRegistry);
  }



  /* restore project setting flags*/
  st_persist_sys *pSystem = st_registry_lock_sys(&pHandle->hRegistry);
  if (pSystem)
  {
    pHandle->iStateFlag = pSystem->hUser.hStorage.stateStorage;
    pHandle->iChunkPool = pSystem->hUser.hStorage.iChunkMemoryPool;
    pHandle->iAlgoPool  = pSystem->hUser.hStorage.iAlgoMemoryPool;
    st_base_set_debug_level(pSystem->iLogLevel);
    st_registry_unlock_sys(&pHandle->hRegistry, FALSE);
  }

  ST_VERIFY(livetune_db_create(&pHandle->hDesignerDB) != ST_ERROR);
  livetune_db_msg_cb_set(&pHandle->hDesignerDB, livetune_db_cb);
  livetune_db_set_register_update_cb(&pHandle->hDesignerDB, livetune_ac_json_update_element_description);

  ST_VERIFY(livetune_pipe_create(&pHandle->hAudioPipe, &pHandle->hDesignerDB) != ST_ERROR);
  livetune_pipe_set_rt_graph_construction(&pHandle->hAudioPipe, TRUE);

  /* Starts the UART asynchronous  mode using DMA to prevent ac callback lock during transmission  */
  UTIL_UART_EnableLogAsync(TRUE, UTIL_UART_STREAM_SIZE);
  /* the system is ready, we can send the message to plugins for their initialization */
  st_message_send(&pHandle->hMessage, pHandle, (uint32_t)ST_EVT_SYSTEM_STARTED, 0U, 0U);
  st_message_send(&pHandle->hMessage, pHandle, (uint32_t)ST_EVT_REGISTER_ELEMENTS, 0U, 0U);


  livetune_instance *pInstance = livetune_get_instance();
  /* the system is now started & free rtos we can load the project */
  /* load the graph from the registry if it exists */
  livetune_db_json_load(&pInstance->hDesignerDB);

  /* signal the system is ready */
  st_message_send(&pHandle->hMessage, pHandle, (uint32_t) ST_EVT_MSG, 0L, (st_message_param)ST_MAKE_INT64(0UL, "Livetune Ready"));
  st_message_send(&pHandle->hMessage, pHandle, (uint32_t)ST_EVT_SYSTEM_READY, 0U, 0U);

  pHandle->bSystemReady = TRUE;

  return ST_OK;
}

/**
 * @brief destroy   a message to listener
 *
 * @param pHandle the instance handle
 * @param pDispatcher callback  dispatcher
 * @param pCookie     a cookie
 * @return ST_Result
 */
ST_Result livetune_delete(void)
{
  livetune_instance *pHandle = livetune_get_instance();
  ST_VERIFY(livetune_db_delete(&pHandle->hDesignerDB) != ST_ERROR);
  ST_VERIFY(st_plugin_manager_delete(&pHandle->hPlugins) != (int32_t)ST_ERROR);
  /* instantiate the message management (Post and Send) */
  ST_VERIFY(st_message_delete(&pHandle->hMessage) == ST_OS_OK);
  /* create the persistent storage in flash */
  ST_VERIFY(st_registry_delete(&pHandle->hRegistry) != (int32_t)ST_ERROR);
  st_os_mutex_delete(&pHandle->hSendLock);
  st_os_event_delete(&pHandle->hSendEvt);
  if (pHandle->pPoolSendBlock)
  {
    st_os_mem_free(pHandle->pPoolSendBlock);
  }
  return ST_OK;
}
/**
 * @brief return the designer instance
 * @return the instance
 */
livetune_instance *livetune_get_instance(void)
{
  return gpInstance;
}


/**
* @brief send a message to all
*
*/
uint32_t livetune_send(uint32_t evt, st_message_param wParam, st_message_param lParam)
{
  livetune_instance *pHandle = livetune_get_instance();
  ST_ASSERT(pHandle != NULL);
  st_message_send(&pHandle->hMessage, pHandle, evt, wParam, lParam);
  return 0;
}

/**
* @brief post a message to all
*
*/
uint32_t livetune_post(uint32_t evt, st_message_param wParam, st_message_param lParam)
{
  livetune_instance *pHandle = livetune_get_instance();
  ST_ASSERT(pHandle != NULL);
  return (uint32_t)st_message_post(&pHandle->hMessage, pHandle, evt, wParam, lParam);
}




/**
* @brief post a message to all
*
*/
uint32_t livetune_post_delete_cb(uint32_t evt, st_message_param wParam, st_message_param lParam, void (*deleteCB)(struct st_message_post_item *pPost))
{
  livetune_instance *pHandle = livetune_get_instance();
  ST_ASSERT(pHandle != NULL);
  return (uint32_t)st_message_post_delete_cb(&pHandle->hMessage, pHandle, evt, wParam, lParam, deleteCB);
}


/**
* @brief reset the registry to its default value
*
*/
ST_Registry_Result st_registry_reset_default(st_registry *pHandle, st_persist_sys *pRegistry)
{
  pRegistry->iLogLevel                   = ST_TRACE_LVL_DEFAULT;
  pRegistry->hUser.hStorage.stateStorage = ST_STATE_DEFAULT;
  strncpy(pRegistry->hUser.tFwVersion, livetune_ac_factory_get_ac_version_string(), sizeof(pRegistry->hUser.tFwVersion) - 1U);
  return ST_REGISTRY_OK;
}




/**
* @brief print overload coming from st_base.c, connect the print to audio change
*
*/

void st_base_puts(uint32_t level, const char_t *pString)
{
  if (level & st_base_get_debug_level())
  {
    if (level & ST_TRACE_LVL_ADD_CR)
    {
      UTIL_UART_fputs((char *)pString);
      UTIL_UART_fputs("\n");
    }
    else
    {
      UTIL_UART_fputs((char *)pString);
    }
  }
}


/**
 * @brief  Reset cleanly the platform
 *
 */

void livetune_reset(void)
{
  livetune_instance *pInstance = livetune_get_instance();
  ST_ASSERT(pInstance != NULL);
  /* prevent access to the registry during the reboot */
  st_os_mutex_lock(&pInstance->hRegistry.systemLock);
  st_os_task_delay(500);
  NVIC_SystemReset();
}



/**
 * @brief update  malloced   string
 *
 * @param pString the string
 * @param pNewString the new string
 */
void livetune_update_string(char_t **pString, const char_t *pNewString)
{
  if (pNewString)
  {
    char_t *pOld = *pString;
    (*pString)   = st_os_mem_realloc(ST_Mem_Type_ANY_FAST, pOld, strlen(pNewString) + 1U); /*cstat !MISRAC2012-Dir-4.13_e !MISRAC2012-Rule-1.3_p in this context, the memory blick will be freed by the next call or when the module instance will be deleted */
    ST_ASSERT((*pString) != NULL);
    strcpy(*pString, pNewString);
  }
  else
  {
    if ((*pString) != NULL)
    {
      st_os_mem_free((*pString));
      (*pString) = NULL;
    }
  }
}

/* overload weak function  managing the pipe */
void livetune_db_cmd(livetune_db_pipe_cmd cmd, livetune_db_pipe_reason reason)
{
  livetune_instance *pHandle = livetune_get_instance();
  ST_ASSERT(pHandle != NULL);
  if ((cmd == ST_PIPE_PUSH_STOP) && (reason != ST_PIPE_REASON_UPDATE))
  {
    livetune_pipe_stop(&pHandle->hAudioPipe);
  }
}

/* Internal event management capture, allow to load and run a flashed project  */
ST_OS_Result st_message_internal(st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam)
{
  switch (evt)
  {
    case ST_EVT_SYSTEM_READY:
    {
      livetune_instance *pInstance = livetune_get_instance();
      if (livetune_state_get_flags() & ST_STATE_FLG_AUTO_START)
      {
        livetune_pipe_start(&pInstance->hAudioPipe);
      }

      break;
    }
  }
  return ST_MESSAGE_OK;
}



/**
* @brief Compute the size after json encoding
* @param pText  raw text
* @return the size
*/
static uint32_t livetune_compute_encoded_json_size(const char_t *pText)
{
  uint32_t szEncoded = 0;
  while (*pText)
  {
    const char_t *pEncoded = json_encode_char(*pText++);
    szEncoded += strlen(pEncoded);
  }
  return szEncoded;
}

/**
* @brief Compute the json encoding
* @param pText  raw text
* @param pBufEncoded  json text
* @return the size encoded
*/
static uint32_t livetune_compute_encoded_json(const char_t *pText, char_t *pBufEncoded)
{
  uint32_t szEncoded = 0;
  uint32_t szChar;
  while (*pText)
  {
    const char_t *pEncoded = json_encode_char(*pText++);
    strcat(pBufEncoded, pEncoded);
    szChar = strlen(pEncoded);
    pBufEncoded += szChar;
    szEncoded += szChar;
  }
  return szEncoded;
}


/**
* @brief add a raw json
* @param pText  raw text
* @param pBufEncoded  json text
* @return the size encoded
*/
static uint32_t livetune_add_json(const char_t *pText, char_t *pBufEncoded)
{
  strcat(pBufEncoded, pText);
  return strlen(pText);
}



/**
* @brief Build a text file to the designer
* @param pSignature  type of message
* @param pTitle      title
* @param pText      the text body
*/
static char_t *livetune_build_block_text(livetune_instance *pHandle, const char_t *pSignature, char_t *pTitle, char_t *pExtra, char_t *pText)
{
  char_t   header[100];
  uint32_t extraSize = 0;
  uint32_t textSize  = 0;

  /* build the header ; ie signature etc... */
  snprintf(header, sizeof(header), "{\"Signature\":\"%s\",\"Title\":\"%s\"", pSignature, pTitle);

  uint32_t stringSize = strlen(header);
  /* a packet may have extra json info, take it in account  */
  uint32_t textString = strlen(TEXT_ATTRB);
  if (pExtra)
  {
    extraSize = strlen(pExtra);
  }
  /* the we have the real payload, the file may have ascii encoding, so compute new size  */
  if (pText)
  {
    textSize = livetune_compute_encoded_json_size(pText);
  }

  /* we can now compute et the packet finalize size */

  uint32_t totalEncodedSize = textString + stringSize + textSize + extraSize + SZ_PROLOG_EPILOG + 10U /* include "," "\"}"and \0 and startcodes + extra*/;
  /* to prevent corruption, alloc it in a special pool that will be used only by the dispatched  */
  char_t *pBuffer = livetune_send_block_alloc(totalEncodedSize);
  if (pBuffer)
  {
    char_t *pCurBuffer = pBuffer;
    /* start code */
    pCurBuffer = livetune_add_block_prolog(pCurBuffer);
    /* add string terminator */
    *pCurBuffer = '\0';
    pCurBuffer += livetune_add_json(header, pCurBuffer);
    if (pExtra)
    {
      pCurBuffer += livetune_add_json(",", pCurBuffer);
      pCurBuffer += livetune_add_json(pExtra, pCurBuffer);
    }

    pCurBuffer += livetune_add_json(TEXT_ATTRB, pCurBuffer);
    if (pText)
    {
      pCurBuffer += livetune_compute_encoded_json(pText, pCurBuffer);
    }
    /* json terminator */
    pCurBuffer += livetune_add_json("\"}", pCurBuffer);
    pCurBuffer = livetune_add_block_epilog(pCurBuffer);
    ST_ASSERT(strlen(pBuffer) < totalEncodedSize);
  }
  return pBuffer;
}

/**
 * @brief  Record a  block that will be freed latter on
 *
 * @param pHandle the instance
 * @param pBlk the block
 */

static void livetune_send_block_record_isr_free(livetune_instance *pHandle, void *pBlk)
{
  ST_ASSERT(pHandle->nbMem2free < FREE_POSTPONED_MAX);
  LIVETUNE_DISABLE_IRQ();
  pHandle->pMem2free[pHandle->nbMem2free] = pBlk;
  pHandle->nbMem2free++;
  LIVETUNE_ENABLE_IRQ();
}




/**
* @brief free a block used by the send block DMA
*/

static void livetune_send_free_cb(void *pBlock, void *pCookie)
{
  if (pBlock)
  {
    /*
    we cannot free a block in an isr, because we can corrupt the malloc chain given we cannot lock a critical section.
    so, to work around this situation, we record the free in a list and the list of free will be freed during the next malloc or realloc

    */

    livetune_send_block_record_isr_free(livetune_get_instance(), pBlock);
  }
}


/**
* @brief free a block used by the send block DMA
*/

static void livetune_send_free_signal_cb(void *pBlock, void *pCookie)
{
  livetune_instance *pHandle = livetune_get_instance();
  /* send the event end of Transmission */

  st_os_event_set(&pHandle->hSendEvt);

  if (pBlock)
  {
    /* free the block previously allowed*/
    livetune_send_block_free(pBlock);
  }
}



/**
* @brief Send a text file to the designer
* @param pSignature  type of message
* @param pTitle      title
* @param pText      the text body
*/
uint32_t livetune_send_block_binary_async(const char_t *pSignature, char_t *pTitle, char_t *pExtra, uint8_t *pBin, uint32_t szBin)
{
  uint32_t nbSnd = 0;
  char     string[200];
  uint32_t extraSize     = (pExtra == NULL) ? 0U : strlen(pExtra);
  uint32_t textSize      = strlen(TEXT_ATTRB);
  uint32_t base64BinSize = 4U * ((szBin + 2U) / 3U);
  uint32_t stringSize, szBuffer;
  stringSize = (uint32_t)snprintf(string, sizeof(string), "{\"Signature\":\"%s\",\"Title\":\"%s\"", pSignature, pTitle);
  szBuffer   = stringSize + extraSize + textSize + base64BinSize + 2U + SZ_PROLOG_EPILOG; // "}
  if (pExtra)
  {
    szBuffer++; // for ','
  }

  char_t *pBuffer = livetune_send_block_alloc(szBuffer);
  if (pBuffer)
  {
    char_t *pCurBuffer = pBuffer;

    /* add the json prolog */

    pCurBuffer = livetune_add_block_prolog(pCurBuffer);

    memcpy(pCurBuffer, string, stringSize);
    pCurBuffer += stringSize;
    if (pExtra)
    {
      *pCurBuffer++ = ',';
      memcpy(pCurBuffer, pExtra, extraSize);
      pCurBuffer += extraSize;
    }
    memcpy(pCurBuffer, TEXT_ATTRB, textSize);
    pCurBuffer += textSize;
    uint32_t sz = livetune_base64_encode(pBin, szBin, pCurBuffer, szBuffer - (pCurBuffer - pBuffer)); /*cstat  !MISRAC2012-Rule-18.2 false positif */
    ST_ASSERT(sz == base64BinSize);
    pCurBuffer += base64BinSize;
    /* add the json epilog */
    *pCurBuffer++    = '"';
    *pCurBuffer++    = '}';
    pCurBuffer       = livetune_add_block_epilog(pCurBuffer);
    uint32_t nb2Send = strlen(pBuffer);
    ST_ASSERT(nb2Send < szBuffer);

    /* make sure no trace garbages */
    if (livetune_send_block_async(pBuffer, szBuffer, livetune_send_free_cb) == TRUE)
    {
      nbSnd = szBuffer - 1U;
    }
    else
    {
      /* the async is muted or the Q is full */
      livetune_send_block_free(pBuffer);
    }
  }
  return nbSnd;
}


/**
* @brief Send a json string to Livetune (asynchronous transmission)
* @param pSignature  type of message
* @param pTitle      title or id
* @param pExtra      title extra fields as text to concate to the json string, pExtra could be null
* @param pText       the text body in the field "Text:", pText could be null
*/
uint32_t livetune_send_block_text_async(uint8_t bForce, const char_t *pSignature, char_t *pTitle, char_t *pExtra, char_t *pText)
{
  uint32_t nbSent = 0;
  /* the URL encoding takes ~10% of extra memory */
  livetune_instance *pHandle = livetune_get_instance();
  char_t *pPkt = livetune_build_block_text(pHandle, pSignature, pTitle, pExtra, pText);
  if (pPkt)
  {
    /* make sure no trace garbages */
    nbSent = strlen(pPkt);
    if (livetune_send_block_async(pPkt, nbSent, livetune_send_free_cb) == FALSE)
    {
      nbSent = 0;
      livetune_send_block_free(pPkt);
    }
  }
  return nbSent;
}


/**
* @brief Send a text file to the designer
* @param pSignature  type of message
* @param pTitle      title
* @param pText      the text body
*/
uint32_t livetune_send_and_wait_consumed(uint8_t *pBuffer, uint32_t szBuffer, uint32_t bAddStartCode)
{
  livetune_instance *pHandle = livetune_get_instance();
  if (bAddStartCode)
  {
    char_t *pPkt = livetune_send_block_alloc(szBuffer + SZ_PROLOG_EPILOG + 1U); /* 2 for the start code and end code */
    if (pPkt)
    {
      char_t *pCurBuffer = pPkt;

      pCurBuffer = livetune_add_block_prolog(pCurBuffer);

      memcpy((void *)pCurBuffer, (void *)pBuffer, szBuffer);
      pCurBuffer += szBuffer;
      pCurBuffer = livetune_add_block_epilog(pCurBuffer);
      /* make sure no trace garbages */
      st_os_mutex_lock(&pHandle->hSendLock);
      if (livetune_send_block_async((void *)pPkt, strlen(pPkt), livetune_send_free_signal_cb) == FALSE)
      {
        /* The block is not sent due to an alloc memory, so free it */
        livetune_send_block_free(pPkt);
        szBuffer = 0;
      }
      else
      {
        st_os_event_wait(&pHandle->hSendEvt, 1000);
      }
      st_os_mutex_unlock(&pHandle->hSendLock);
    }
  }
  else
  {
    /* make sure no trace garbages */
    st_os_mutex_lock(&pHandle->hSendLock);
    if (livetune_send_block_async((void *)pBuffer, szBuffer, NULL) == FALSE)
    {
      /* The block is not sent due to an alloc memory, so free it */
      szBuffer = 0;
    }
    else
    {
      st_os_event_wait(&pHandle->hSendEvt, 1000);
    }
    st_os_mutex_unlock(&pHandle->hSendLock);
  }
  return szBuffer;
}

/**
* @brief Sync the PC host with the board
*        we need to send a sync byte in order to tell to the host that the communication will start
*        this to prevent the host catches an half packet
*        the host won't  parse packet before to get the sync byte
* @param state  true or false
*/

uint8_t livetune_send_sync_host(void)
{
  UTIL_UART_FlushAsyncChars();
  st_os_task_delay(5);
  UTIL_UART_fputc_force(TRANSITION_SYNC);
  UTIL_UART_FlushAsyncChars();
  st_os_task_delay(5);
  return TRUE;
}




/**
* @brief Send a block using the DMA
* notice,  sending block using dma require to be able to free memory from the ISR, it is not possible when alloc functions have mutex
* so, we create a pool only used by send block function, this make sure we can free a block without crashes
* @param pName  flag name
* @param state  true or false
*/
extern int32_t usb_tx_hook(UART_HandleTypeDef *ptr, uint8_t *pBuffer, uint32_t pszBuffer);
int8_t livetune_send_block_async(const char_t *pBlock, uint32_t szBlock, livetune_free_send_block_cb cbFreeBlock)
{
  int8_t                error   = TRUE;
  livetune_instance *pHandle = livetune_get_instance();
  st_os_mutex_lock(&pHandle->hSendLock);
#ifndef USE_UART_OVER_USB
  error = (UTIL_UART_SendBlockAsync(pBlock, szBlock, cbFreeBlock, (void *)pBlock, pHandle) == 0) ? TRUE : FALSE;
#else
  error = usb_tx_hook(NULL, (uint8_t*)pBlock, szBlock);

  cbFreeBlock((void*)pBlock, (void*)pHandle);
#endif /* USE_UART_OVER_USB */
  st_os_mutex_unlock(&pHandle->hSendLock);
  return error;
}



/**
 * @brief  Free all block postponed
 *
 * @param pHandle the instance
 */


static void livetune_send_block_free_isr_alloc(livetune_instance *pHandle)
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
void *livetune_send_block_alloc(uint32_t szAlloc)
{
  st_os_lock_tasks();
  livetune_instance *pHandle = livetune_get_instance();
  livetune_send_block_free_isr_alloc(pHandle);
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
void *livetune_send_block_realloc(void *prevBlock, uint32_t szAlloc)
{
  st_os_lock_tasks();
  livetune_instance *pHandle = livetune_get_instance();
  livetune_send_block_free_isr_alloc(pHandle);

  void *pPtr = pmem_realloc(&pHandle->hSendBlockPool, prevBlock, szAlloc);
  st_os_unlock_tasks();
  return pPtr;
}



/**
 * @brief the free could be called from ISR, so we use pmem_free_postponed
 *
 * @param pBlock the block
 */
void livetune_send_block_free(void *pBlock)
{
  st_os_lock_tasks();
  livetune_instance *pHandle = livetune_get_instance();
  livetune_send_block_free_isr_alloc(pHandle);
  pmem_free(&pHandle->hSendBlockPool, pBlock);
  st_os_unlock_tasks();
}




/**
* @brief Encode the binary in base 64
*/

uint32_t livetune_base64_encode(uint8_t *pSrc, uint32_t szSrc, char_t *pDst, uint32_t szDst)
{
  static const char_t tEncBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  uint32_t            i, n;
  char_t             *p;
  union
  {
    uint8_t  u8[3];
    uint32_t u32;
  } base64_encode;

  if (szSrc == 0U)
  {
    return (0);
  }
  n = 4U * ((szSrc + 2U) / 3U);
  if ((szDst < (n + 1U)) || (NULL == pDst))
  {
    return (n + 1U);
  }
  n                 = szSrc / 3U;
  base64_encode.u32 = 0U;
  for (i = 0, p = pDst; i < n; i++)
  {
    base64_encode.u8[2] = *pSrc++;
    base64_encode.u8[1] = *pSrc++;
    base64_encode.u8[0] = *pSrc++;
    *p++                = tEncBase64[(base64_encode.u32 >> 18U)]; // mask with 0x3F is useless
    *p++                = tEncBase64[(base64_encode.u32 >> 12U) & 0x3FU];
    *p++                = tEncBase64[(base64_encode.u32 >> 6U) & 0x3FU];
    *p++                = tEncBase64[(base64_encode.u32) & 0x3FU];
  }
  n = szSrc - (3U * n);
  if (n > 0U)
  {
    base64_encode.u8[2] = *pSrc++;
    *p++                = tEncBase64[(base64_encode.u32 >> 18)];
    if (n == 1U)
    {
      *p++ = tEncBase64[(base64_encode.u32 >> 12U) & 0x30U];
      *p++ = '=';
    }
    else
    {
      base64_encode.u8[1] = *pSrc;
      *p++                = tEncBase64[(base64_encode.u32 >> 12U) & 0x3FU];
      *p++                = tEncBase64[(base64_encode.u32 >> 6U) & 0x3CU];
    }
    *p++ = '=';
  }
  *p = '\0';

  return (((uint8_t *)p) - ((uint8_t *)pDst)); /*cstat !MISRAC2012-Rule-10.3 false positive, pointer are properly casted */
}


/**
* @brief return the designer state flags
*/

uint32_t livetune_state_get_flags(void)
{
  uint32_t           iState    = 0;
  livetune_instance *pInstance = livetune_get_instance();
  if (pInstance)
  {
    iState = pInstance->iStateFlag;
  }
  return iState;
}


/**
* @brief set the designer state flags
*/

uint32_t livetune_state_set_flags(uint32_t flags)
{
  uint32_t           iState    = 0;
  livetune_instance *pInstance = livetune_get_instance();

  if (pInstance)
  {
    iState                = pInstance->iStateFlag;
    pInstance->iStateFlag = flags;
    /* record the change */
    st_persist_sys *pSystem = st_registry_lock_sys(&pInstance->hRegistry);
    if (pSystem)
    {
      pSystem->hUser.hStorage.stateStorage = flags;
      st_registry_unlock_sys(&pInstance->hRegistry, TRUE);
    }
  }
  return iState;
}


/**
* @brief return the designer chunk memory pool
*/

uint32_t livetune_state_get_chunk_pool(void)
{
  uint32_t           iChunkPool = 0;
  livetune_instance *pInstance  = livetune_get_instance();
  if (pInstance)
  {
    iChunkPool = pInstance->iChunkPool;
  }
  return iChunkPool;
}


/**
* @brief set the designer chunk memory pool
*/

uint32_t livetune_state_set_chunk_pool(uint8_t mem_pool)
{
  livetune_instance *pInstance = livetune_get_instance();
  if (pInstance)
  {
    pInstance->iChunkPool = mem_pool;
    /* record the change */
    st_persist_sys *pSystem = st_registry_lock_sys(&pInstance->hRegistry);
    if (pSystem)
    {
      pSystem->hUser.hStorage.iChunkMemoryPool = mem_pool;
      st_registry_unlock_sys(&pInstance->hRegistry, TRUE);
    }
  }
  return mem_pool;
}


/**
* @brief return the designer algo memory pool
*/

uint32_t livetune_state_get_algo_pool(void)
{
  uint32_t           iAlgoPool = 0;
  livetune_instance *pInstance = livetune_get_instance();
  if (pInstance)
  {
    iAlgoPool = pInstance->iAlgoPool;
  }
  return iAlgoPool;
}


/**
* @brief set the designer algo memory pool
*/

uint32_t livetune_state_set_algo_pool(uint8_t mem_pool)
{
  livetune_instance *pInstance = livetune_get_instance();
  if (pInstance)
  {
    pInstance->iAlgoPool = mem_pool;
    /* record the change */
    st_persist_sys *pSystem = st_registry_lock_sys(&pInstance->hRegistry);
    if (pSystem)
    {
      pSystem->hUser.hStorage.iAlgoMemoryPool = mem_pool;
      st_registry_unlock_sys(&pInstance->hRegistry, TRUE);
    }
  }
  return mem_pool;
}


/**
* @brief return true if the system is initialized
*/

int32_t livetune_wait_ready(void)
{
  int32_t            timeout   = 5 * 1000 / 10; // 5 secs
  livetune_instance *pInstance = livetune_get_instance();
  while ((pInstance->bSystemReady == 0U) && (timeout != 0))
  {
    st_os_task_delay(10);
    timeout--;
  }

  return (timeout == 0) ? FALSE : TRUE;
}

