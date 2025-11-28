/**
******************************************************************************
* @file          st_os_cmsis_v2.c
* @author        MCD Application Team
* @brief         implement operating system abstraction for CMSIS  V2
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

//#include "platform_setup_conf.h"
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
/*cstat -MISRAC2012-Rule-10.4_a  A third-party produces a violation message we cannot fix*/
#include "cmsis_os.h"
/*cstat +MISRAC2012-Rule-10.4_a*/
#include "st_os.h"

#ifdef ST_OS_USE_CMSIS_V2

#include "st_os_mem.h"
#include <stdio.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// WARNING : workaround for ThreadX osThreadTerminate wrapper issue which doesn't delete task //////
////////           to be removed when wrapper issue will be updated                                 //////
#ifdef USE_THREADX
  #include "tx_api.h"
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_THREADX
  // osKernelLock and osKernelUnlock are not defined in ThreadX version of cmsis_os2.c => disable/enable irq instead
  #include "irq_utils.h"
  #define osKernelLock   disable_irq_with_cnt
  #define osKernelUnlock enable_irq_with_cnt
#endif

#define MAX_STACK_SLOT (30U)


typedef struct st_task_impl
{
  osThreadAttr_t       attrb;
  osThreadId_t         id;
  st_task             *pTaskInstance;
  uint32_t             szStack;
  struct st_task_impl *pNext;
} st_task_impl;

static st_task_impl *pFirstThread = NULL;

typedef struct st_mutex_impl
{
  osMutexAttr_t attrb;
  osMutexId_t   id;
} st_mutex_impl;


typedef struct st_queue_impl
{
  osMessageQueueAttr_t attrb;
  osMessageQueueId_t   id;
} st_queue_impl;


typedef struct st_event_impl
{
  osSemaphoreAttr_t attrb;
  osSemaphoreId_t   id;
} st_event_impl;

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

#define LOCK_ALLOC()   st_os_lock_tasks()
#define UNLOCK_ALLOC() st_os_unlock_tasks()

/* Private typedef -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

void st_os_task_create_prolog(st_task *pTask);
void st_os_task_create_epilog(st_task *pTask);
void st_os_task_delete_prolog(st_task *pTask);
void st_os_task_delete_epilog(st_task *pTask);


/**
 * @brief delete a task link
 *
 * @param pTask the instance handle
 * @return None
 */
static void st_os_task_link_delete(st_task *pTask)
{
  st_os_lock_tasks();
  if (pTask == pFirstThread->pTaskInstance)
  {
    pFirstThread = pFirstThread->pNext;
  }
  else
  {
    /* look for the previous task */
    st_task_impl *pNext = NULL;
    for (st_task_impl *pCur = pFirstThread; pCur != NULL; pCur = pNext)
    {
      pNext = pCur->pNext;
      if ((pNext != NULL) && (pTask == pNext->pTaskInstance))
      {
        /* remove the link */
        pCur->pNext  = pNext->pNext;
        pNext->pNext = NULL;
        break;
      }
    }
  }
  st_os_unlock_tasks();
}


/**
 * @brief return the a task link from the task id
 *
 * @param id     id
 * @return task link
 */
static st_task *st_os_task_link_get(osThreadId_t id)
{
  st_task *pTaskInstance = NULL;
  st_os_lock_tasks();
  for (st_task_impl *pCurTask = pFirstThread; pCurTask != NULL; pCurTask = pCurTask->pNext)
  {
    if (pCurTask->id == id)
    {
      pTaskInstance = pCurTask->pTaskInstance;
      break;
    }
  }
  st_os_unlock_tasks();
  return pTaskInstance;
}


/**
 * @brief add a task link
 *
 * @param pTask the instance handle
 */
static void st_os_task_link_add(st_task *pTask)
{
  ST_ASSERT(pTask);
  st_os_lock_tasks();
  st_task_impl *pCurTask;
  if (pFirstThread)
  {
    /* move to the last */
    for (pCurTask = pFirstThread; pCurTask->pNext != NULL; pCurTask = pCurTask->pNext);
    pCurTask->pNext = pTask->hTask;
    pCurTask        = pCurTask->pNext;
  }
  else
  {
    pFirstThread = pTask->hTask;
    pCurTask     = pFirstThread;
  }
  pCurTask->pTaskInstance = pTask;
  pCurTask->pNext         = NULL;
  st_os_unlock_tasks();
}


static bool st_os_kernel_started(void)
{
  return (osKernelGetState() == osKernelRunning) ? true : false;
}


/**
 * @brief creates an event object
 *
 * @param event the instance handle
 * @param pName the name
 * @return ST_OS_Result
 */
ST_OS_Result st_os_event_create_named(st_event *event, const char *pName)
{
  ST_OS_Result   result = ST_OS_ERROR;
  st_event_impl *pEvent = st_os_mem_alloc(ST_Mem_Type_HEAP, sizeof(st_event_impl));
  if (pEvent)
  {
    memset(pEvent, 0, sizeof(*pEvent));
    event->hEvent      = pEvent;
    pEvent->attrb.name = pName;
    pEvent->id         = osSemaphoreNew(1, 0, &pEvent->attrb);
    result             = (ST_OS_Result)((pEvent->id != NULL) ? ST_OS_OK : ST_OS_ERROR);
  }
  return result;
}


/**
 * @brief deletes  an event
 *
 * @param event the instance handle
 * @return ST_OS_Result
 */
ST_OS_Result st_os_event_delete(st_event *event)
{
  ST_OS_Result ret = ST_OS_OK;
  ST_OS_ASSERT(event != NULL);
  st_event_impl *pEvent = event->hEvent;
  ST_OS_ASSERT(pEvent != NULL);
  ret = osSemaphoreDelete(pEvent->id);
  st_os_mem_free(pEvent);
  event->hEvent = 0;
  return ret;
}


/**
 * @brief return the event count
 *
 * @param event the instance handle
 */

uint32_t st_os_event_get_count(st_event *event)
{
  ST_OS_ASSERT(event != NULL);
  st_event_impl *pEvent = event->hEvent;
  ST_OS_ASSERT(pEvent != NULL);
  return osSemaphoreGetCount(pEvent->id);
}


/**
 * @brief Signals the event
 *
 * @param event the instance handle
 * @return ST_OS_Result
 */
ST_OS_Result st_os_event_set(st_event *event)
{
  ST_OS_Result ret = ST_OS_ERROR;
  ST_OS_ASSERT(event != NULL);
  st_event_impl *pEvent = event->hEvent;
  ST_OS_ASSERT(pEvent != NULL);
  if (osKernelGetState() != osKernelInactive)
  {
    ret = osSemaphoreRelease(pEvent->id);
  }
  return ret;
}


/**
 * @brief Waits for an event
 *
 * @param event the instance handle
 * @param timeout the timeout
 * @return ST_OS_Result
 */
ST_OS_Result st_os_event_wait(st_event *event, uint32_t timeout)
{
  ST_OS_Result result = ST_OS_ERROR;
  ST_OS_ASSERT(event != NULL);
  st_event_impl *pEvent = event->hEvent;
  ST_OS_ASSERT(pEvent != NULL);
  if (osKernelGetState() != osKernelInactive)
  {
    result = osSemaphoreAcquire(pEvent->id, timeout); /*cstat !MISRAC2012-Rule-10.3 this error comes from a third party, the macro is not MISRA compatible */
  }
  return result;
}


/**
 * @brief Signals an event form the isr
 *
 * @param event the instance handle
 * @return ST_OS_Result
 */

ST_OS_Result st_os_event_set_isr(st_event *event)
{
  /* TODO: check with will why not status from event_set is returned */
  st_os_event_set(event);
  return ST_OS_OK;
}


/**
 * @brief creates a queue object
 *
 * @param pQueue the instance handle
 * @param nbElement number of element in the queue
 * @param itemSize  the element size
 * @param pName  the name
 * @return ST_OS_Result
 */

ST_OS_Result st_os_queue_create_named(st_queue *pQueue, uint32_t nbElement, uint32_t itemSize, const char *pName)
{
  ST_OS_Result result = ST_OS_ERROR;
  if (pQueue != NULL)
  {
    st_queue_impl *pQ = st_os_mem_alloc(ST_Mem_Type_HEAP, (uint32_t)sizeof(st_queue_impl)); /*cstat !MISRAC2012-Rule-22.1_a False Positive, the pointer is passed to a struct handle that will be freed later one in st_os_queue_delete */
    if (pQ != NULL)
    {
      memset(pQ, 0, sizeof(*pQ));
      pQueue->hQueue = pQ;
      pQ->attrb.name = pName;
      pQ->id         = osMessageQueueNew(nbElement, itemSize, &pQ->attrb);
      ST_OS_ASSERT(pQ->id);
      result = (ST_OS_Result)((pQ->id != NULL) ? ST_OS_OK : ST_OS_ERROR);
    }
  }
  return result;
}


/**
 * @brief Deletes a Queue
 *
 * @param pQueue the instance handle
 * @return uint32_t
 */

ST_OS_Result st_os_queue_delete(st_queue *pQueue)
{
  ST_OS_Result ret = ST_OS_OK;
  ST_OS_ASSERT(pQueue != NULL);
  st_queue_impl *pQ = pQueue->hQueue;
  ST_OS_ASSERT(pQ != NULL);
  ST_OS_ASSERT(pQ->id != NULL);
  ret = osMessageQueueDelete(pQ->id);
  st_os_mem_free(pQ);
  pQueue->hQueue = 0;
  return ret;
}


/**
 * @brief puts an element in the queue
 *
 * @param pQueue the instance handle
 * @param pElemen the pointer on the element
 * @param timeout the timeout
 * @return ST_OS_Result
 */

ST_OS_Result st_os_queue_put(st_queue *pQueue, void *pElemen, uint32_t timeout)
{
  ST_OS_ASSERT(pQueue != NULL);
  st_queue_impl *pQ = pQueue->hQueue;
  ST_OS_ASSERT(pQ != NULL);
  return osMessageQueuePut(pQ->id, pElemen, 0, timeout);
}


/**
 * @brief gets a element from the queue
 *
 * @param pQueue the instance handle
 * @param pElemen the pointer on the element
 * @param timeout the timeout
 * @return ST_OS_Result
 */

ST_OS_Result st_os_queue_get(st_queue *pQueue, void *pElemen, uint32_t timeout)
{
  ST_OS_ASSERT(pQueue != NULL);
  st_queue_impl *pQ = pQueue->hQueue;
  ST_OS_ASSERT(pQ != NULL);
  /*
  We don't use because the API doesn't allow to embed statically the object
  */
  return osMessageQueueGet(pQ->id, pElemen, NULL, timeout);
}


/**
 * @brief creates a mutex
 *
 * @param mutex the instance handle
 * @param pName the name
 * @return int8_t
 */

ST_OS_Result st_os_mutex_create_named(st_mutex *mutex, const char *pName)
{
  ST_OS_Result         result = ST_OS_ERROR;
  st_mutex_impl *pMutex = st_os_mem_alloc(ST_Mem_Type_HEAP, sizeof(st_mutex_impl));
  if (pMutex)
  {
    memset(pMutex, 0, sizeof(*pMutex));
    pMutex->attrb.name      = pName;
    pMutex->attrb.attr_bits = osMutexRecursive;
    pMutex->id              = osMutexNew(&pMutex->attrb);
    ST_OS_ASSERT(pMutex->id);
    mutex->hMutex = pMutex;
    result        = (ST_OS_Result)((pMutex->id != NULL) ? ST_OS_OK : ST_OS_ERROR);
  }
  return result;
}


/**
 * @brief deletes the mutex
 *
 * @param mutex the instance handle
 */

void st_os_mutex_delete(st_mutex *mutex)
{
  ST_OS_ASSERT(mutex != NULL);
  if (mutex->hMutex)
  {
    st_mutex_impl *pMutex = mutex->hMutex;
    osMutexDelete(pMutex->id);
    st_os_mem_free(pMutex);
    mutex->hMutex = 0;
  }
}


/**
 * @brief locks the mutex
 *
 * @param mutex the instance handle
 */

void st_os_mutex_lock(st_mutex *mutex)
{
  /* it is preferable to ignore mutext lock/unlock if the kernel is not started to prevent crash if used before kernel start */
  if (st_os_kernel_started() && (mutex != NULL) && (mutex->hMutex != NULL))
  {
    st_mutex_impl *pMutex = mutex->hMutex;
    ST_OS_ASSERT(pMutex->id != NULL);
    osMutexAcquire(pMutex->id, osWaitForever);
  }
}


/**
 * @brief try Lock the mutex
 *
 * @param mutex the instance
 * @return ST_OS_Result
 */

ST_OS_Result st_os_mutex_trylock(st_mutex *mutex)
{
  /* it is preferable to ignore mutext lock/unlock if the kernel is not started to prevent crash if used before kernel start */
  if (st_os_kernel_started() && (mutex != NULL) && (mutex->hMutex != NULL))
  {
    st_mutex_impl *pMutex = mutex->hMutex;
    if ((int32_t)osSemaphoreGetCount(pMutex->id) <= 0)
    {
      return ST_OS_BUSY;
    }
    osMutexAcquire(pMutex->id, osWaitForever);
  }
  return ST_OS_OK;
}


/**
 * @brief Unlock the mutex
 *
 * @param mutex the instance handle
 */

void st_os_mutex_unlock(st_mutex *mutex)
{
  /* it is preferable to ignore mutext lock/unlock if the kernel is not started to prevent crash if used before kernel start */
  if (st_os_kernel_started() && (mutex != NULL) && (mutex->hMutex != NULL))
  {
    st_mutex_impl *pMutex = mutex->hMutex;
    ST_OS_ASSERT(pMutex->id != NULL);
    osMutexRelease(pMutex->id);
  }
}


/**
 * @brief create a task
 *
 * @param pTask  the instance handle
 * @return __weak st_os_task_create_prolog
 */
__WEAK void st_os_task_create_prolog(st_task *pTask)
{
}


/**
 * @brief delete  a task
 *
 * @param pTask  the instance handle
 * @return __weak st_os_task_create_prolog
 */
__WEAK  void st_os_task_create_epilog(st_task *pTask)
{
}


/**
 * @brief create  a task
 *
 * @param pTask  the instance handle
 * @return __weak st_os_task_create_prolog
 */
__WEAK  void st_os_task_delete_prolog(st_task *pTask)
{
}


/**
 * @brief delete  a task
 *
 * @param pTask  the instance handle
 * @return __weak st_os_task_create_prolog
 */

__WEAK  void st_os_task_delete_epilog(st_task *pTask)
{
}


/**
 * @brief Task exit and auto kill the thread
 *
 * @param task task handle
 */
void st_os_task_exit(st_task *task)
{
  if (task == NULL)
  {
    task = (st_task *)st_os_task_link_get(osThreadGetId());
  }
  if (task)
  {
    st_os_task_delete_prolog(task);
    st_task_impl *pTask = task->hTask;
    ST_OS_ASSERT(pTask != NULL);
    st_os_task_link_delete(task);
    st_os_mem_free(pTask);
    task->hTask = NULL;

  }
  osThreadExit();
}


static osPriority_t st_os_convert_priority(st_task_priority priority)
{
  osPriority_t   ospriority = osPriorityError;
  static struct
  {
    osPriority_t     osPrio;
    st_task_priority stPrio;
  } tTaskPriority[] =
  {
    {osPriorityIdle, ST_Priority_Idle},
    {osPriorityLow,  ST_Priority_Low},
    {osPriorityLow1, ST_Priority_Low1},
    {osPriorityLow2, ST_Priority_Low2},
    {osPriorityLow3, ST_Priority_Low3},
    {osPriorityLow4, ST_Priority_Low4},
    {osPriorityLow5, ST_Priority_Low5},
    {osPriorityLow6, ST_Priority_Low6},
    {osPriorityLow7, ST_Priority_Low7},

    {osPriorityBelowNormal,  ST_Priority_Below_Normal},
    {osPriorityBelowNormal1, ST_Priority_Below_Normal1},
    {osPriorityBelowNormal2, ST_Priority_Below_Normal2},
    {osPriorityBelowNormal3, ST_Priority_Below_Normal3},
    {osPriorityBelowNormal4, ST_Priority_Below_Normal4},
    {osPriorityBelowNormal5, ST_Priority_Below_Normal5},
    {osPriorityBelowNormal6, ST_Priority_Below_Normal6},
    {osPriorityBelowNormal7, ST_Priority_Below_Normal7},

    {osPriorityNormal,  ST_Priority_Normal},
    {osPriorityNormal1, ST_Priority_Normal1},
    {osPriorityNormal2, ST_Priority_Normal2},
    {osPriorityNormal3, ST_Priority_Normal3},
    {osPriorityNormal4, ST_Priority_Normal4},
    {osPriorityNormal5, ST_Priority_Normal5},
    {osPriorityNormal6, ST_Priority_Normal6},
    {osPriorityNormal7, ST_Priority_Normal7},

    {osPriorityAboveNormal,  ST_Priority_Above_Normal},
    {osPriorityAboveNormal1, ST_Priority_Above_Normal1},
    {osPriorityAboveNormal2, ST_Priority_Above_Normal2},
    {osPriorityAboveNormal3, ST_Priority_Above_Normal3},
    {osPriorityAboveNormal4, ST_Priority_Above_Normal4},
    {osPriorityAboveNormal5, ST_Priority_Above_Normal5},
    {osPriorityAboveNormal6, ST_Priority_Above_Normal6},
    {osPriorityAboveNormal7, ST_Priority_Above_Normal7},

    {osPriorityHigh,  ST_Priority_High},
    {osPriorityHigh1, ST_Priority_High1},
    {osPriorityHigh2, ST_Priority_High2},
    {osPriorityHigh3, ST_Priority_High3},
    {osPriorityHigh4, ST_Priority_High4},
    {osPriorityHigh5, ST_Priority_High5},
    {osPriorityHigh6, ST_Priority_High6},
    {osPriorityHigh7, ST_Priority_High7},

    {osPriorityRealtime,  ST_Priority_Realtime},
    {osPriorityRealtime1, ST_Priority_Realtime1},
    {osPriorityRealtime2, ST_Priority_Realtime2},
    {osPriorityRealtime3, ST_Priority_Realtime3},
    {osPriorityRealtime4, ST_Priority_Realtime4},
    {osPriorityRealtime5, ST_Priority_Realtime5},
    {osPriorityRealtime6, ST_Priority_Realtime6},
    {osPriorityRealtime7, ST_Priority_Realtime7},

    {osPriorityError, ST_Priority_Max} /* Max*/
  };

  uint32_t nbPriority = sizeof(tTaskPriority) / sizeof(tTaskPriority[0]);
  for (uint32_t index = 0UL; index < nbPriority; index++)
  {
    if (tTaskPriority[index].stPrio == priority)
    {
      ospriority = tTaskPriority[index].osPrio;
      break;
    }
  }
  return ospriority;
}


/**
 * @brief Change the task priority
 *
 * @param task task handle
 * @param priority  the priority
 * @return error code
 */
ST_OS_Result st_os_task_priority_set(st_task *task, st_task_priority priority)
{
  ST_OS_Result ret = ST_OS_ERROR;
  osPriority_t cmsisPrio  = st_os_convert_priority(priority);
  ST_OS_ASSERT(cmsisPrio != osPriorityError);
  st_task_impl *pTask = task->hTask;
  if ((pTask != NULL) && (pTask->id != NULL))
  {
    ret = osThreadSetPriority(pTask->id, cmsisPrio);
  }
  return ret;
}



/**
 * @brief Creates a task
 *
 * @param task task handle
 * @param pName the name
 * @param pCb   the task callback
 * @param pCookie the user pointer
 * @param stackSize  the stack size
 * @param priority  the priority
 * @return true or false
 */
ST_OS_Result st_os_task_create(st_task *task, const char *pName, st_task_cb pCb, void *pCookie, uint32_t stackSize, st_task_priority priority)
{
  ST_OS_Result ret = ST_OS_ERROR;
  osPriority_t cmsisPrio  = st_os_convert_priority(priority);
  ST_OS_ASSERT(cmsisPrio != osPriorityError);
  st_task_impl *pTask = st_os_mem_alloc(ST_Mem_Type_Os, sizeof(st_task_impl));
  if (pTask)
  {
    task->hTask = pTask;
    memset(pTask, 0, sizeof(st_task_impl));

    pTask->attrb.name       = pName;
    pTask->attrb.priority   = cmsisPrio;
    pTask->attrb.stack_size = stackSize * sizeof(uint32_t); /* CMCIS V2 stack is in bytes, but stos is int uint32_t */
    pTask->szStack          = stackSize * sizeof(uint32_t);
    /* Specific processing before thread creation*/
    st_os_task_create_prolog(task);
    ret = ST_OS_OK;
    st_os_lock_tasks();/* make sure the task will not finish before to quit this call */
    pTask->id = osThreadNew((osThreadFunc_t)(pCb), pCookie, &pTask->attrb); /*cstat !MISRAC2012-Rule-11.1 false positive, the pCb is well casted but Cstat seems to not see it */
    if (pTask->id)
    {
      st_os_task_link_add(task);
      /* Specific processing after thread creation*/
      st_os_task_create_epilog(task);
    }
    if (pTask->id == 0)
    {
      st_os_mem_free(pTask);
      task->hTask = 0;
      ret = ST_OS_ERROR;
    }
    st_os_unlock_tasks();
  }
  return ret;
}


unsigned long st_os_task_get_stack_high_water_mark(st_task *task)
{
  #ifdef INCLUDE_uxTaskGetStackHighWaterMark
  if ((task != NULL) && (task->hTask != NULL))
  {
    st_task_impl *pTask = task->hTask;
    return uxTaskGetStackHighWaterMark(pTask->id);
  }
  #endif
  return 0UL;
}


/**
 * @brief Wait the task dies
 *
 * @param task the instance handle
 */

ST_OS_Result st_os_task_join(st_task *task, uint32_t timeout)
{
  ST_OS_Result ret = ST_OS_ERROR;

  if ((task != NULL) && (task->hTask != NULL))
  {
    #ifdef USE_THREADX
    st_task_impl *pTask = task->hTask;
    ret = osThreadJoin(pTask->id);
    #else
    uint32_t tmo = timeout;
    while (tmo)
    {
      if (task->hTask == NULL) { break; }
      tmo--;
      st_os_task_delay(1);
    }
    ret = (tmo > 0UL) ? ST_OS_OK : ST_OS_ERROR;
    #endif
  }
  return ret;
}



/**
 * @brief Deletes a task
 *
 * @param task the instance handle
 */

void st_os_task_delete(st_task *task)
{
  osThreadId_t id = 0;
  if (task == NULL)
  {
    task = st_os_task_link_get(osThreadGetId());
  }
  if (task)
  {
    st_os_lock_tasks();/* make sure the task will not finish before to quit this call */
    st_os_task_delete_prolog(task);
    st_task_impl *pTask = task->hTask;
    ST_OS_ASSERT(pTask != NULL);
    id = pTask->id;
    st_os_task_link_delete(task);
    st_os_mem_free(pTask);
    task->hTask = 0;
    st_os_unlock_tasks();

  }
  osThreadTerminate(id);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////// WARNING : workaround for ThreadX osThreadTerminate wrapper issue which doesn't delete task //////
  ////////           to be removed when wrapper issue will be updated                                 //////
  #ifdef USE_THREADX
  tx_thread_delete((TX_THREAD *)id);
  #endif
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/**
 * @brief A task can't return to the user
          So, let's wait for a vTaskDelete
 *
 */
void st_os_task_end(void)
{
  while (1)
  {
    st_os_task_delay(1000);
  }
}

/**
 * @brief sleep for n ms
 *
 * @param ms the time to sleep
 */
void st_os_task_delay(uint32_t ms)
{
  uint32_t  nbTickBySec = osKernelGetTickFreq();
  uint32_t  nbTicks     = (ms * nbTickBySec) / 1000UL;
  st_os_task_tdelay(nbTicks);
}

/**
 * @brief sleep for micro sec
 *
 * @param micro the time to sleep
 */
void st_os_task_udelay(uint32_t micro)
{
  uint32_t  nbTickBySec = osKernelGetTickFreq();
  uint64_t  nbTicks     = (((uint64_t)micro) * nbTickBySec) / 1000000U;
  st_os_task_tdelay((uint32_t)nbTicks);
}


/**
 * @brief sleep for nbTicks ticks
 *
 * @param nbTicks the time to sleep
 */
void st_os_task_tdelay(uint32_t nbTicks)
{
  if (nbTicks == 0UL)
  {
    osThreadYield();
  }
  else
  {
    osDelay(nbTicks);
  }
}


/**
 * @brief return true if in an isr
 *
 * @return int32_t
 */
int8_t st_os_is_irq(void)
{
  int32_t isr = __get_IPSR(); /*cstat !MISRAC2012-Rule-10.3 false positif, This error is caused by a build in asm */
  return (isr != 0L) ? 1 : 0;
}

/**
 * @brief Stops all tasks
 *
 */
void st_os_lock_tasks(void)
{
  if (st_os_is_irq() == ST_OS_TRUE)
  {
    return;
  }
  if (osKernelGetState() == osKernelInactive)
  {
    return;
  }
  osKernelLock();
}

/**
 * @brief Restart all tasks
 *
 */
void st_os_unlock_tasks(void)
{
  if (st_os_is_irq() == ST_OS_TRUE)
  {
    return;
  }
  if (osKernelGetState() == osKernelInactive)
  {
    return;
  }

  osKernelUnlock();
}


/**
 * @brief return the current systick in ms
 *
 * @return uint32_t
 */
uint32_t st_os_sys_tick(void)
{
  return (osKernelGetTickCount() * 1000U) / osKernelGetTickFreq();
}

/**
 * @brief return the kernel sys ticks freq
 *
 * @return uint32_t
 */

uint32_t st_os_sys_tick_freq(void)
{
  return osKernelGetTickFreq();
}



/**
 * @brief return the kernel time in ms
 *
 * @return uint32_t
 */

uint64_t st_os_sys_time(void)
{

  uint32_t  nbTickBySec = osKernelGetTickFreq();
  uint64_t  ms          = (1000ULL * osKernelGetTickCount()) / nbTickBySec; /*cstat !MISRAC2012-Rule-1.3_h osKernelGetTickFreq() can't be equal to 0*/
  return ms;
}



/**
 * @brief return the kernel time in micros sec
 *
 * @return uint32_t
 */

uint64_t st_os_sys_utime(void)
{

  uint32_t  nbTickBySec = osKernelGetTickFreq();
  uint64_t  ms          = (1000000ULL * osKernelGetTickCount()) / nbTickBySec;  /*cstat !MISRAC2012-Rule-1.3_h osKernelGetTickFreq() can't be equal to 0*/
  return ms;
}





/**
 * @brief Enter critical section
 *
 * @return error code
 */
ST_OS_Result st_os_enter_critical_section(void)
{
  ST_OS_DISABLE_IRQ();
  return ST_OS_OK;
}
/**
 * @brief Leave critical section
 *
 * @return error code
 */

ST_OS_Result st_os_exit_critical_section(void)
{
  ST_OS_ENABLE_IRQ();
  return ST_OS_OK;
}


void st_os_switch_isr_required(int8_t xSwitchRequired)
{
#define portNVIC_INT_CTRL_REG   ( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT    ( 1UL << 28UL )
  if (xSwitchRequired)
  {
    portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
  }
}


#ifdef USE_FREERTOS

uint32_t osThreadGetStackSize(osThreadId_t id)
{
  uint32_t      szStkSpace           = 0;
  /* osThreadGetStackSize is not implemented in freertos */
  st_task *task = st_os_task_link_get(id);
  if (task)
  {
    st_task_impl *pTask = task->hTask;
    szStkSpace = pTask->szStack;
  }
  return szStkSpace;
}

#endif // USE_FREERTOS



/**
 * @brief callback  to sort a list
 *
 * @param p1 element 1
 * @param p2  element 2
 * @return int32_t
 */
static int st_os_task_info_print_sort(const void *p1, const void *p2)
{
  int32_t priority1 = (int32_t)osThreadGetPriority(*(osThreadId_t *)p1);
  int32_t priority2 = (int32_t)osThreadGetPriority(*(osThreadId_t *)p2);
  return (int32_t)priority1 - (int32_t)priority2;
}

static const char *st_os_task_state_text(osThreadState_t state)
{
  const char *pState = "None";
  switch (state)
  {
    case osThreadInactive: {pState = "Inactive"; break;}
    case  osThreadReady: {pState = "Ready"; break;}
    case  osThreadRunning: {pState = "Running"; break;}
    case  osThreadBlocked: {pState = "Blocked"; break;}
    case  osThreadTerminated: {pState = "Terminated"; break;}
    case  osThreadError: {pState = "Error"; break;}
    default:  {pState = "Error"; break;}

  }
  return pState;
}





/**
 * @brief prints  task info, notice we use printf because this function could be called from IT
 *
 * @return ST_OS_Result
 */
ST_OS_Result st_os_task_info_print(void)
{
  osThreadId_t    pTaskI[MAX_STACK_SLOT];
  uint32_t        nbTask = osThreadEnumerate(pTaskI, MAX_STACK_SLOT);

  // uint32_t curTime = (uint32_t)st_os_sys_time();

  qsort(pTaskI, nbTask, sizeof(osThreadId_t), st_os_task_info_print_sort); /*cstat !MISRAC2012-Rule-21.9 This function is a debug function and not used in production, and we need to sort object for a better presentation */

  ST_OS_PRINT("---------------------------\n");
  ST_OS_PRINT("-  st_os_task_info_print  -\n");
  ST_OS_PRINT("---------------------------\n");

  for (uint32_t a = 0; a < nbTask; a++)
  {
    osThreadId_t  id          = pTaskI[a];
    const char   *pThreadName = osThreadGetName(id);
    uint32_t      szStkSpace  = osThreadGetStackSize(id);
    uint32_t      szStkUsed   = szStkSpace - osThreadGetStackSpace(id);
    uint32_t      priority    = (uint32_t)osThreadGetPriority(id);
    const char   *pState      = st_os_task_state_text(osThreadGetState(id));

    if (pThreadName == NULL)
    {
      // workaround in case pThreadName==NULL (which shouldn't happen) => get name from attr
      // it can only happen if we are under interrupt (impossible for st_os_task_info_print) or if irq are disabled (which shouldn't happen)
      st_task      *task  = st_os_task_link_get(id);
      st_task_impl *pTask = (task == NULL) ? NULL : (st_task_impl *)task->hTask;

      pThreadName = (pTask == NULL) ? "" : pTask->attrb.name;
    }

    if (szStkSpace > 0UL)
    {
      ST_OS_PRINT("%02ld:\"%-25s\": %15s : priority %2ld  : stack %4ld/%4ld \n", a, pThreadName, pState, priority, szStkUsed, szStkSpace);
    }
    else
    {
      ST_OS_PRINT("%02ld:\"%-25s\": %15s : priority %2ld  : stack used %4ld\n", a, pThreadName, pState, priority, szStkUsed);
    }
  }

  return ST_OS_OK;
}

#endif // ST_OS_USE_CMSIS_V2

