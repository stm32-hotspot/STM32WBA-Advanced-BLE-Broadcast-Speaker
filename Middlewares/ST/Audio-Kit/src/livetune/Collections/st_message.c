/**
******************************************************************************
* @file          st_message.c
* @author        MCD Application Team
* @brief         dispatch message manager
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

#include "string.h"
#include "st_message.h"
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/

#define ST_MESSAGE_MAX_POST_ELEMENT (60)
#define ST_MESSAGE_DELAY_IDLE       (500)
#ifndef ST_MESSAGE_TASK_STACK_SIZE
  #define ST_MESSAGE_TASK_STACK_SIZE (500)
#endif
#ifndef ST_MESSAGE_TASK_PRIORITY
  #define ST_MESSAGE_TASK_PRIORITY (ST_Priority_Below_Normal)
#endif
#ifndef ST_MESSAGE_TASK_NAME
  #define ST_MESSAGE_TASK_NAME "ST:Msg"
#endif

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */

/**
 * @brief look at messages for internal change state
 *
 * @param pHandle the instance handle
 * @param evt the event
 * @param pparam1 the event parameter
 * @return int32_t
 */

__weak ST_OS_Result st_message_internal(st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam)
{
  return ST_MESSAGE_OK;
}

/**
 * @brief Dispatch a message to listener
 *
 * @param pHandle the instance handle
 * @param evt the event
 * @param wparam the event param
 * @param lparam the event param
 * @return ST_OS_Result
 */

ST_OS_Result st_message_send(st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam)
{
  ST_OS_Result result = ST_MESSAGE_ERROR;
  ST_MESSAGE_ASSERT(pHandle != NULL);
  if (pHandle != NULL)
  {
    if ((evt >= ST_MESSAGE_REENTRANT_START) && (evt <= ST_MESSAGE_REENTRANT_END))
    {
      st_os_mutex_lock(&pHandle->hLockMessage);
    }

    if (pHandle->pDispatcher != 0)
    {
      result = (ST_OS_Result)pHandle->pDispatcher(pHandle, pCookie, (uint32_t)evt, wParam, lParam);
    }

    st_message_internal(pHandle, pCookie, evt, wParam, lParam);

    if ((evt >= ST_MESSAGE_REENTRANT_START) && (evt <= ST_MESSAGE_REENTRANT_END))
    {
      st_os_mutex_unlock(&pHandle->hLockMessage);
    }
  }
  return result;
}


/**
 * @brief poste a message to listeners with a delete callback
 *
 * @param pHandle the instance handle
 * @param evt the event
 * @param wparam the event param
 * @param lparam the event param
 * @param deleteCB the delete callback

 * @return ST_OS_Result
 */

ST_OS_Result st_message_post_delete_cb(st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam, void (*deleteCB)(struct st_message_post_item *pPost))
{
  ST_OS_Result result = ST_MESSAGE_ERROR;
  ST_MESSAGE_ASSERT(pHandle != NULL);
  if (pHandle != NULL)
  {
    st_message_post_item msg;
    memset(&msg, 0, sizeof(msg));
    msg.type     = ST_POST_EVENT;
    msg.wparam   = evt;
    msg.lparam1  = wParam;
    msg.lparam2  = lParam;
    msg.pCookie  = pCookie;
    msg.deleteCB = deleteCB;

    result = st_os_queue_put(&pHandle->hPostMsg, (void *)&msg, 1000UL);
    if (result != ST_MESSAGE_OK)
    {
      ST_MESSAGE_TRACE_ERROR("st_os_queue_put failed");

      // free message
      if (msg.deleteCB != NULL)
      {
        (msg.deleteCB)(&msg);
      }
    }
  }
  return result;
}


/**
 * @brief poste a message to listeners
 *
 * @param pHandle the instance handle
 * @param evt the event
 * @param wparam the event param
 * @param lparam the event param
 * @return ST_OS_Result
 */

ST_OS_Result st_message_post(st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam)
{
  ST_OS_Result result = ST_MESSAGE_ERROR;
  ST_MESSAGE_ASSERT(pHandle != NULL);
  if (pHandle != NULL)
  {
    st_message_post_item msg;
    memset(&msg, 0, sizeof(msg));
    msg.type    = ST_POST_EVENT;
    msg.wparam  = evt;
    msg.lparam1 = wParam;
    msg.lparam2 = lParam;
    msg.pCookie = pCookie;

    result = st_os_queue_put(&pHandle->hPostMsg, (void *)&msg, 1000UL);
    if (result != ST_MESSAGE_OK)
    {
      ST_MESSAGE_TRACE_ERROR("st_os_queue_put failed");
    }
  }
  return result;
}

/**
 * @brief the post callback that push in the queue the message
 *
 * @param pHandle the instance handle
 * @param timeout the timeout
 * @param cb the callback
 * @param lparam1 the param 1
 * @param lparam2 the param 2
 * @return ST_OS_Result
 */

ST_OS_Result st_message_cb_post(st_message *pHandle, uint32_t timeout, st_message_poste_cb cb, void *pCookie, st_message_param lparam1, st_message_param lparam2)
{
  ST_OS_Result result = ST_MESSAGE_ERROR;
  ST_MESSAGE_ASSERT(pHandle != NULL);
  if (pHandle != NULL)
  {
    st_message_post_item msg;
    memset(&msg, 0, sizeof(msg));

    msg.type    = ST_POST_CALLBACK;
    msg.wparam  = (st_message_param)ST_MAKE_INT64(0UL, cb); /*cstat !MISRAC2012-Rule-11.1 in this context wparam is a int64 and can be casted in function pointer*/
    msg.lparam1 = lparam1;
    msg.lparam2 = lparam2;
    msg.pCookie = pCookie;
    result      = st_os_queue_put(&pHandle->hPostMsg, (void *)&msg, timeout);
  }
  return result;
}

/**
 * @brief dispatch  a message to listener
 *
 * @param pHandle the instance handle
 * @param timeout the timeout
 * @return ST_OS_Result
 */
static ST_OS_Result st_message_post_idle(st_message *pHandle, uint32_t timeout)
{
  ST_OS_Result result;
  do
  {
    st_message_post_item msg;
    result = st_os_queue_get(&pHandle->hPostMsg, &msg, timeout);
    st_message_cycleMeasure_Start();
    if (result == ST_MESSAGE_OK)
    {
      switch (msg.type)
      {
        case ST_POST_EVENT:
        {
          st_message_send(pHandle, (void *)msg.pCookie, ST_LOW_INT64(msg.wparam), msg.lparam1, msg.lparam2);
          break;
        }
        case ST_POST_CALLBACK:
        {
          st_message_poste_cb proc_cb = (st_message_poste_cb)ST_LOW_INT64(msg.wparam); /*cstat !MISRAC2012-Rule-11.1 in this context wparam is a int64 and can be casted in function pointer*/
          if (proc_cb)
          {
            proc_cb(&msg);
          }
          break;
        }
        default:
        {
          break;
        }
      }

      if (msg.deleteCB)
      {
        msg.deleteCB(&msg);
      }
    }
    st_message_cycleMeasure_Stop();
  }
  while (result == ST_MESSAGE_OK);
  return ST_MESSAGE_OK;
}


/**
 * @brief Posting task callback
 *
 * @param pCookie user pointer
 */

static void st_message_post_task(const void *pCookie)
{
  st_message *pHandle       = (st_message *)pCookie;
  pHandle->iRunPostTaskFlag = st_task_running;
  st_message_cycleMeasure_Init();
  st_message_cycleMeasure_Reset();
  while ((pHandle->iRunPostTaskFlag & st_task_running) != 0U)
  {
    st_message_post_idle(pHandle, ST_MESSAGE_DELAY_IDLE);
  }
  st_os_task_delete(NULL);
}

__weak void st_message_cycleMeasure_Init(void)
{
}

__weak void st_message_cycleMeasure_Reset(void)
{
}

__weak void st_message_cycleMeasure_Start(void)
{
}

__weak void st_message_cycleMeasure_Stop(void)
{
}


/**
 * @brief create   a message to listener
 *
 * @param pHandle the instance handle
 * @param pDispatcher callback  dispatcher
 * @param pCookie     a cookie
 * @return ST_OS_Result
 */

ST_OS_Result st_message_create(struct st_message *pHandle, st_message_event_cb pDispatcher)
{
  ST_MESSAGE_ASSERT(pHandle != NULL);
  memset(pHandle, 0, sizeof(*pHandle));
  pHandle->pDispatcher = pDispatcher;
  ST_MESSAGE_VERIFY((st_os_mutex_create(&pHandle->hLockMessage) == ST_MESSAGE_OK));
  ST_MESSAGE_VERIFY((st_os_queue_create_named(&pHandle->hPostMsg, ST_MESSAGE_MAX_POST_ELEMENT, sizeof(st_message_post_item), NULL) == ST_MESSAGE_OK));
  pHandle->iRunPostTaskFlag = st_task_closed;
  ST_OS_Result error             = st_os_task_create(&pHandle->hPostTask, ST_MESSAGE_TASK_NAME, st_message_post_task, pHandle, ST_MESSAGE_TASK_STACK_SIZE, ST_MESSAGE_TASK_PRIORITY);
  return error;
}

/**
 * @brief delete the  message listener
 *
 * @return ST_OS_Result
 */

ST_OS_Result st_message_delete(struct st_message *pHandle)
{
  pHandle->iRunPostTaskFlag = st_task_closed;
  st_os_task_delay(ST_MESSAGE_DELAY_IDLE);
  st_os_queue_delete(&pHandle->hPostMsg);
  st_os_mutex_delete(&pHandle->hLockMessage);
  return ST_MESSAGE_OK;
}


