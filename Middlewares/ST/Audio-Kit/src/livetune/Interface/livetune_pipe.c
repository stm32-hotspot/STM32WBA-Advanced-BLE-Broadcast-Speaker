/**
******************************************************************************
* @file          livetune_pipe.c
* @author        MCD Application Team
* @brief         manage the audio pipe
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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "livetune_pipe.h"
#include "livetune_helper.h"



/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */



/**
* @brief push the instance in the pipe
*
* @param pHandle the  instance handle
* @param pHandle the instance handle

*/

static ST_Result livetune_pipe_build_push(livetune_pipe *pHandle, livetune_db_instance *pInstance)
{
  ST_Result result = ST_ERROR;
  if (livetune_db_list_create(&pHandle->pProcessList, pInstance))
  {
    pInstance->bResolved  = TRUE;
    pInstance->pPipe      = pHandle;
    pInstance->iUserIndex = livetune_db_list_nb(&pHandle->pProcessList);
    result                = ST_OK;
  }
  return result;
}


/**
* @brief Build pipeline in the right order of dependencies
*
* @param pHandle the instance handle
*/

ST_Result livetune_pipe_build_order(livetune_pipe *pHandle)
{
  int32_t   iMaxLoop     = 100;
  int8_t    bAllResolved = FALSE;
  ST_Result result       = ST_OK;
  /* reset the resolve flags */
  uint32_t nbInstance = livetune_db_list_nb(&pHandle->pDataBase->pInstances);


  for (uint32_t indexInst = 0; indexInst < nbInstance; indexInst++)
  {
    livetune_db_instance *pInstance = livetune_db_list_get_object(&pHandle->pDataBase->pInstances, indexInst);

    pInstance->bResolved = FALSE;
  }
  livetune_db_list_clear(&pHandle->pProcessList, FALSE);
  pHandle->pProcessList = NULL;

  /* loop until end or error */
  while ((iMaxLoop > 0) && (bAllResolved == 0))
  {
    bAllResolved = TRUE;
    /* iterate on instances */
    for (uint32_t indexInst = 0; indexInst < nbInstance; indexInst++)
    {
      livetune_db_instance *pInstance = livetune_db_list_get_object(&pHandle->pDataBase->pInstances, indexInst);
      /* if the instance is resolved , pass */
      if (pInstance->bResolved == 0)
      {
        /* if the instance is not resolved, check if all parents are resolved, in this case, the instance becomes resolved */
        bAllResolved     = FALSE;
        int8_t bResolved = TRUE;

        /* normal a pin In has only a single connection */
        for (livetune_db_instance_pins_def *pDef = pInstance->pPinIn; pDef != NULL; pDef = pDef->pNext)
        {
          for (livetune_db_instance_cnx_list *pList = pDef->pObject; pList != NULL; pList = pList->pNext)
          {
            livetune_db_instance_cnx *pPin = pList->pCnxInstance;
            if ((pPin->pInstance != NULL) && (pPin->pInstance->bResolved == 0))
            {
              /* if the pin is connected to the src or not resolved we must wait for the next turn */
              bResolved = FALSE;
            }
          }
        }
        if (bResolved)
        {
          /* if all pins are resolved, we can push the instance in the list */
          if (livetune_pipe_build_push(pHandle, pInstance) == ST_ERROR)
          {
            iMaxLoop = 0;
            break;
          }
        }
      }
    }
    iMaxLoop--;
  }
  if (iMaxLoop <= 0)
  {
    result = ST_ERROR;
  }
  return result;
}


/**
* @brief call the builder
*
* @param pHandle the pipe instance handle
* @param pInstance the  instance handle
* @param cmd the  the message
* @return  error code
*/

static livetune_builder_result livetune_pipe_notify_call(livetune_pipe *pHandle, livetune_db_instance *pInstance, uint32_t cmd)
{
  livetune_builder_result result = ST_BUILDER_DEFAULT;

  if (pInstance->pElements->hInterface.pipe_builder_cb)
  {
    result = pInstance->pElements->hInterface.pipe_builder_cb(pInstance, cmd);
  }
  if (result == ST_BUILDER_DEFAULT)
  {
    if (pInstance->pBuilderData)
    {
      result = pHandle->livetune_helper_builder_default_cb(pInstance->pBuilderData);
    }
  }
  return result;
}

/**
* @brief notify elements in the pipe ordering, if the element doesn't overload the event, the builder uses a default function
*
* @param pHandle the instance handle
* @return  error code
*/

livetune_builder_result livetune_pipe_notify(livetune_pipe *pHandle, st_pipe_builder_cmd cmd)
{
  livetune_builder_result result = ST_BUILDER_OK;
  for (livetune_db_list *pProcess = pHandle->pProcessList; pProcess != NULL; pProcess = pProcess->pNext)
  {
    livetune_db_instance *pInstance             = pProcess->pObject;
    pHandle->livetune_helper_builder_default_cb = NULL;
    if (cmd == LIVETUNE_PIPE_CONSTRUCTOR)
    {
      if ((pInstance != NULL) && (pInstance->pElements->hInterface.constructor_cb != NULL))
      {
        pInstance->pElements->hInterface.constructor_cb(pInstance);
      }
      result = ST_BUILDER_OK;
    }
    else if (cmd == LIVETUNE_PIPE_DESTRUCTOR)
    {
      if ((pInstance != NULL) && (pInstance->pElements->hInterface.destructor_cb != NULL))
      {
        pInstance->pElements->hInterface.destructor_cb(pInstance);
      }
      result = ST_BUILDER_OK;
    }
    else if (cmd == LIVETUNE_PIPE_START)
    {
      pHandle->livetune_helper_builder_default_cb = livetune_helper_builder_pipe_start_default;
      result                                         = livetune_pipe_notify_call(pHandle, pInstance, (uint32_t)cmd);
    }
    else if (cmd == LIVETUNE_PIPE_PROLOG)
    {
      pHandle->livetune_helper_builder_default_cb = livetune_helper_builder_pipe_prolog_default;
      result                                         = livetune_pipe_notify_call(pHandle, pInstance, (uint32_t)cmd);
    }
    else if (cmd == LIVETUNE_PIPE_PRE_INIT_GRAPH)
    {
      pHandle->livetune_helper_builder_default_cb = livetune_helper_builder_pre_init_graph_default;
      result                                         = livetune_pipe_notify_call(pHandle, pInstance, (uint32_t)cmd);
    }
    else if (cmd == LIVETUNE_PIPE_POST_INIT_GRAPH)
    {
      pHandle->livetune_helper_builder_default_cb = livetune_helper_builder_post_init_graph_default;
      result                                         = livetune_pipe_notify_call(pHandle, pInstance, (uint32_t)cmd);
    }

    else if (cmd == LIVETUNE_PIPE_INIT_GRAPH_ELEMENT)
    {
      pHandle->livetune_helper_builder_default_cb = livetune_helper_builder_pipe_init_graph_element_default;
      result                                         = livetune_pipe_notify_call(pHandle, pInstance, (uint32_t)cmd);
    }
    else if (cmd == LIVETUNE_PIPE_INIT_GRAPH_ATTACH_CNX)
    {
      pHandle->livetune_helper_builder_default_cb = livetune_helper_builder_pipe_init_graph_attach_cnx_default;
      result                                         = livetune_pipe_notify_call(pHandle, pInstance, (uint32_t)cmd);
    }
    else
    {
      if ((pInstance != NULL) && (pInstance->pElements->hInterface.pipe_builder_cb != NULL))
      {
        result = pInstance->pElements->hInterface.pipe_builder_cb(pInstance, (uint32_t)cmd);
      }
    }
    /* if the builder cb return default, we can call the default handler */
    if (result == ST_BUILDER_DEFAULT)
    {
      if (pHandle->livetune_helper_builder_default_cb)
      {
        if (pInstance->pBuilderData)
        {
          result = pHandle->livetune_helper_builder_default_cb(pInstance->pBuilderData);
        }
      }
      else
      {
        result = ST_BUILDER_OK;
      }
    }
    if (result == ST_BUILDER_ERROR)
    {
      ST_TRACE_ERROR("The Builder returns an error %s:%d", pInstance->pInstanceName, result);
    }
  }
  return result;
}


/**
* @brief create an audio rendering pipe
*
* @param pHandle the instance handle
* @return ST_Result
*/
ST_Result livetune_pipe_create(livetune_pipe *pHandle, livetune_db *pDataBase)
{
  ST_ASSERT(pHandle != NULL);
  memset(pHandle, 0, sizeof(*pHandle));
  pHandle->pDataBase = pDataBase;
  ST_VERIFY((st_os_mutex_create(&pHandle->hLock) == ST_OS_OK));
  return ST_OK;
}



/**
* @brief delete an audio rendering pipe
*
* @param pHandle the instance handle
* @return ST_Result
*/

ST_Result livetune_pipe_delete(livetune_pipe *pHandle)
{
  livetune_pipe_stop(pHandle);
  st_os_mutex_delete(&pHandle->hLock);
  return ST_OK;
}




/**
* @brief Push the pipe Start/stop state
*
* @param pHandle the instance handle
* @return error code
*/
ST_Result livetune_pipe_state_push(livetune_pipe *pHandle)
{
  ST_Result result = ST_ERROR;
  if (pHandle->hStateStack.iNbStack < ST_STACK_MAX)
  {
    pHandle->hStateStack.tStack[pHandle->hStateStack.iNbStack] = pHandle->bPipeState;
    pHandle->hStateStack.iNbStack++;
    result = ST_OK;
  }
  return result;
}

/**
* @brief Pop the pipe  Start/stop state
*
* @param pHandle the instance handle
* @return error code
*/

ST_Result livetune_pipe_state_pop(livetune_pipe *pHandle, livetune_db_pipe_cmd newState)
{
  ST_Result result = ST_ERROR;
  if (pHandle->hStateStack.iNbStack != 0)
  {
    pHandle->hStateStack.iNbStack--;
    ST_ASSERT(pHandle->hStateStack.iNbStack >= 0);
    if (newState != ST_PIPE_NO_CHANGE)
    {
      int8_t state = pHandle->hStateStack.tStack[pHandle->hStateStack.iNbStack];
      if (state)
      {
        result = livetune_pipe_start(pHandle);
      }
      else
      {
        result = livetune_pipe_stop(pHandle);
      }
    }
  }
  return result;
}


/**
* @brief Start the processing
*
* @param pHandle the instance handle
* @return error code
*/
ST_Result livetune_pipe_start(livetune_pipe *pHandle)
{
  ST_Result result = ST_OK;
  ST_ASSERT(pHandle != NULL);
  st_os_mutex_lock(&pHandle->hLock);
  if (pHandle->bPipeState == FALSE)
  {
    result = ST_ERROR;
    if (pHandle->bRtGraphConstruction == 0)
    {
      result = ST_OK;
    }
    else
    {
      pHandle->bBuilding = TRUE;

      if (pHandle)
      {
        if (pHandle->bPipeState == FALSE)
        {
          if (livetune_pipe_build_order(pHandle))
          {
            int32_t bError = livetune_helper_build_audio_chain(pHandle);
            if (bError == FALSE)
            {
              result = ST_OK;
            }
          }
        }
      }
      if (result == ST_OK)
      {
        livetune_db_send(pHandle->pDataBase, (uint32_t)ST_EVT_PIPE_BUILT, (st_message_param)0);

        /* the graph is ready, we can start it */
        if (livetune_ac_wrapper_acPipePlay_start(pHandle) != 0)
        {
          result = ST_ERROR;
        }
        else
        {
          pHandle->bPipeState = TRUE;
        }
      }
      pHandle->bBuilding = FALSE;
    }

    livetune_db_send(pHandle->pDataBase, (uint32_t)ST_EVT_START_PIPE, (st_message_param)ST_MAKE_INT64(0UL, result));
  }
  st_os_mutex_unlock(&pHandle->hLock);
  if (result == ST_ERROR)
  {
    livetune_db_send(pHandle->pDataBase, (uint32_t)ST_EVT_MSG, (st_message_param)ST_MAKE_INT64(0UL, "Invalid audio flow, can't initialize AudioChain pipe."));
  }
  return result ;
}

/**
* @brief Stop  the processing
*
* @param pHandle the instance handle
* @return error code
*/
ST_Result livetune_pipe_stop(livetune_pipe *pHandle)
{
  ST_Result result = ST_OK;
  ST_ASSERT(pHandle != NULL);
  uint32_t tm = 100UL;
  /* cannot stop the pipeline during a construction */
  while (pHandle->bBuilding)
  {
    st_os_task_delay(10);
    if (tm -- == 0UL)
    {
      return ST_ERROR;
    }
  }

  st_os_mutex_lock(&pHandle->hLock);
  if (pHandle->bPipeState == TRUE)
  {
    if (pHandle->bRtGraphConstruction == 0)
    {
      result = ST_OK;
    }
    else
    {
      result = ST_ERROR;
      if (pHandle->bPipeState)
      {
        result = ST_OK;
        if (livetune_ac_wrapper_acPipePlay_stop(pHandle) != 0)
        {
          result = ST_ERROR;
        }
        pHandle->bPipeState = FALSE;
      }
      livetune_db_send(pHandle->pDataBase, (uint32_t)ST_EVT_STOP_PIPE, (st_message_param)ST_MAKE_INT64(0UL, result));
    }
  }
  /* free the order list */
  livetune_pipe_notify(pHandle, LIVETUNE_PIPE_DESTRUCTOR);
  livetune_db_list_clear(&pHandle->pProcessList, FALSE);
  pHandle->pProcessList = NULL;
  st_os_mutex_unlock(&pHandle->hLock);
  return result;
}

/**
* @brief return the pipe state
*
* @param pHandle the instance handle
* @return a bool
*/
int8_t livetune_pipe_is_started(livetune_pipe *pHandle)
{
  ST_ASSERT(pHandle != NULL);
  return pHandle->bPipeState;
}

/**
* @brief Pipe printf on several logs channels
*
* @param pFormat text
*/
void livetune_pipe_log(livetune_pipe *pHandle, livetune_pipe_log_type type, char_t *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);
  if ((type == LIVETUNE_LOG_CODE) && (pHandle->bGenerateCode != 0))
  {
    char_t tBufferCode[256];
    vsnprintf(tBufferCode, sizeof(tBufferCode), pFormat, args);
    uint32_t len = strlen(tBufferCode);
    if ((pHandle->szGenerateCode + strlen(tBufferCode) + 1U) >= pHandle->szGenerateCodeMax)
    {
      pHandle->szGenerateCodeMax += ST_PIPE_GEN_CODE_BLK_SIZE;
      pHandle->pGenerateCode = st_os_mem_realloc(ST_Mem_Type_ANY_SLOW, pHandle->pGenerateCode, pHandle->szGenerateCodeMax);
    }
    if (pHandle->pGenerateCode)
    {
      strcat(pHandle->pGenerateCode, tBufferCode);
      pHandle->szGenerateCode += len;
    }
    else
    {
      ST_TRACE_ERROR("Alloc code generator");
    }

    #ifdef GENERATE_CODE_PRINT_DIRECT
    printf(tBufferCode);
    #endif
  }
  va_end(args);
}



/**
* @brief Pipe printf on several logs channels
*
* @param pFormat text
*/
void livetune_pipe_add_log(livetune_pipe *pHandle, livetune_pipe_log_type type, char_t *pText)
{
  if ((type == LIVETUNE_LOG_CODE) && (pHandle->bGenerateCode != 0))
  {
    uint32_t len = strlen(pText);
    if ((pHandle->szGenerateCode + len + 1U) >= pHandle->szGenerateCodeMax)
    {
      pHandle->szGenerateCodeMax += len + ST_PIPE_GEN_CODE_BLK_SIZE;
      pHandle->pGenerateCode = st_os_mem_realloc(ST_Mem_Type_ANY_SLOW, pHandle->pGenerateCode, pHandle->szGenerateCodeMax);
    }
    if (pHandle->pGenerateCode)
    {
      strcat(pHandle->pGenerateCode, pText);
      pHandle->szGenerateCode += len;
    }
    else
    {
      ST_TRACE_ERROR("Alloc code generator");
    }
    #ifdef GENERATE_CODE_PRINT_DIRECT
    printf(pText);
    #endif
  }
}



/**
* @brief Pipe printf on several logs channels
*
* @param pFormat text
*/
void livetune_pipe_add_printf(livetune_pipe *pHandle, livetune_pipe_log_type type, char_t *pFormat, ...)
{
  char tBuffer[128];
  va_list args;
  va_start(args, pFormat);
  vsnprintf(tBuffer, sizeof(tBuffer), pFormat, args);
  va_end(args);

  if ((type == LIVETUNE_LOG_CODE) && (pHandle->bGenerateCode != 0))
  {
    uint32_t len = strlen(tBuffer);
    if ((pHandle->szGenerateCode + len + 1U) >= pHandle->szGenerateCodeMax)
    {
      pHandle->szGenerateCodeMax += len + ST_PIPE_GEN_CODE_BLK_SIZE;
      pHandle->pGenerateCode = st_os_mem_realloc(ST_Mem_Type_ANY_SLOW, pHandle->pGenerateCode, pHandle->szGenerateCodeMax);
    }
    if (pHandle->pGenerateCode)
    {
      strcat(pHandle->pGenerateCode, tBuffer);
      pHandle->szGenerateCode += len;
    }
    else
    {
      ST_TRACE_ERROR("Alloc code generator");
    }
    #ifdef GENERATE_CODE_PRINT_DIRECT
    printf(tBuffer);
    #endif
  }
}

/**
* @brief Enable  the pipe CODE
*
* @param pHandle the instance handle
* @param state  true or false
* @return error code
*/

ST_Result livetune_pipe_enable_generate_code(livetune_pipe *pHandle, int8_t state)
{
  ST_Result result = ST_OK;
  ST_ASSERT(pHandle != NULL);
  pHandle->bGenerateCode = state;
  if (state)
  {
    if (pHandle->pGenerateCode)
    {
      st_os_mem_free(pHandle->pGenerateCode);
      pHandle->pGenerateCode = NULL;
    }
    pHandle->szGenerateCode    = 0;
    pHandle->szGenerateCodeMax = ST_PIPE_GEN_CODE_BLK_SIZE;
    if (pHandle->pGenerateCode)
    {
      st_os_mem_free(pHandle->pGenerateCode);
    }
    pHandle->pGenerateCode = st_os_mem_alloc(ST_Mem_Type_Designer, ST_PIPE_GEN_CODE_BLK_SIZE);
    if (pHandle->pGenerateCode == NULL)
    {
      result = ST_ERROR;
    }
    else
    {
      memset(pHandle->pGenerateCode, (int32_t)0, ST_PIPE_GEN_CODE_BLK_SIZE); /*cstat  !MISRAC2012-Rule-1.3_u false positive , the buffer has a size of ST_PIPE_GEN_CODE_BLK_SIZE */
    }
  }
  else
  {
    if (pHandle->pGenerateCode)
    {
      st_os_mem_free(pHandle->pGenerateCode);
    }
    pHandle->pGenerateCode  = NULL;
    pHandle->szGenerateCode = 0;
  }
  return result;
}

/**
* @brief helper for code generation defines
*
*/

const char_t *livetune_pipe_get_ivalue_string(const st_pipe_key_value **pList, uint32_t listIndex, int32_t index)
{
  return pList[listIndex][index].pKey;
}
/**
* @brief helper for code generation defines
*
*/

uint32_t livetune_pipe_get_ivalue(const st_pipe_key_value **pList, uint32_t listIndex, int32_t index)
{
  return pList[listIndex][index].value.ivalue;
}
/**
* @brief helper for code generation defines
*
*/

const char_t *livetune_pipe_find_ivalue_string(const st_pipe_key_value **pList, uint32_t ivalue)
{
  while (*pList)
  {
    const st_pipe_key_value *pKeys = *pList;
    while (pKeys->pKey)
    {
      if (pKeys->value.ivalue == ivalue)
      {
        return pKeys->pKey;
      }
      pKeys++;
    }
    pList++;
  }
  return NULL;
}


/**
* @brief set the state of the graph construction in real time
*
*/
int8_t livetune_pipe_set_rt_graph_construction(livetune_pipe *pHandle, int8_t bState)
{
  ST_ASSERT(pHandle != NULL);
  int8_t oldState               = pHandle->bRtGraphConstruction;
  pHandle->bRtGraphConstruction = bState;
  return oldState;
}


/**
* @brief return  the state of the graph construction in real time
*
*/
int8_t livetune_pipe_get_rt_graph_construction(livetune_pipe *pHandle)
{
  ST_ASSERT(pHandle != NULL);
  return pHandle->bRtGraphConstruction;
}

