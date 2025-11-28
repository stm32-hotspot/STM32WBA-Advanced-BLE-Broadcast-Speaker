/**
******************************************************************************
* @file    st_os_hl.c
* @author  MCD Application Team
* @brief   Manage cmsisos tasks start stop etc...
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "st_os_hl.h"
#include "traces.h"

#ifdef ST_OS_DEBUG
  #define ST_OS_DISPLAY_ALL_TASKS_DEBUG
  //#define ST_OS_TRG_TASKS_MSG_DEBUG   /* debug msg queues (put/get) */
  #include "cycles.h"
#endif

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#ifdef ST_OS_USE_CMSIS_V2
  #define st_os_hl_tick            osKernelGetTickCount
  #define st_os_hl_timeout         osErrorTimeout
#else
  #define st_os_hl_tick            osKernelSysTick
  #define st_os_hl_timeout         osEventTimeout
#endif

#ifndef ST_OS_TRG_TASKS_NB_MAX
  #define ST_OS_TRG_TASKS_NB_MAX   32U
#endif

#ifdef ST_OS_DEBUG
  #define ST_OS_HL_MSG_TRACE_DEPTH 1000
  #define ST_OS_TRG_TASKS_MSG_DEBUG_WRAP_IDX  /* wrap (if defined) or not wrap (if undefined) ContextDebug.taskMsgTrace buffer */
#endif

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  ST_OS_HL_AVAILABLE,
  ST_OS_HL_BUSY
} st_os_hl_dispo_t;

typedef enum
{
  ST_OS_HL_IDLE,
  ST_OS_HL_READY,
  ST_OS_HL_STARTED
} st_os_hl_state_t;

typedef enum
{
  ST_OS_HL_TASK_NONE,
  ST_OS_HL_TASK_IDLE,
  ST_OS_HL_TASK_RUNNING,
  ST_OS_HL_TASK_TERMINATE_REQUEST
} st_os_hl_task_state_t;

typedef enum
{
  ST_OS_MSG_GET,
  ST_OS_MSG_PUT
} st_os_msg_dir_t;

#ifdef ST_OS_TRG_TASKS_MSG_DEBUG
static struct
{
  context_triggeredTask_t *pTaskHdle;
  st_os_hl_state_t         cmsios_state;
  st_os_msg_dir_t          msgDir;
  st_os_hl_msg_t           msg;
  uint32_t                 cycles;
  ST_OS_Result             Status;
} context_debug_trace_t;

static struct
{
  volatile int         idx_msg_trace;
  volatile uint32_t    start_cycles ;
  context_debug_trace_t taskMsgTrace[ST_OS_HL_MSG_TRACE_DEPTH];
} context_debug_t;

#endif

typedef struct
{
  #ifdef ST_OS_DEBUG
  uint32_t                           deepness;
  uint32_t                           msgLastMax;
  #endif
  st_queue                           stos;
  uint32_t                           timeout;
  uint32_t                           msg;
  uint32_t                           msgMax;
} context_msgQueue_t;

typedef struct
{
  char                        const *name;
  #ifdef ST_OS_DEBUG
  uint32_t                           last_trace_cycles;
  #ifdef INCLUDE_uxTaskGetStackHighWaterMark
  uint32_t                           stack_size;
  uint32_t                           stack_level;
  uint32_t                           last_stack_level;
  #endif
  #endif
  st_task                            stos;
  st_os_hl_task_state_t volatile     state;
  st_task_priority                   priority;
  bool                               activateBackGroundTask;
  bool                               logQueueLevels;
  uint8_t                   volatile table_id;
  bool                               firstTraceFifoFull;
  uint32_t                           lastTickTraceFifoFull;
  ARGUMENT_TYPE                      argument;
  st_os_hl_task_func_cb              process_cb;
} context_task_t;

typedef struct
{
  context_task_t     task;
  context_msgQueue_t queue;
} context_triggeredTask_t;


typedef struct
{
  context_triggeredTask_t       tasksHdleTable[ST_OS_TRG_TASKS_NB_MAX];
  volatile int                  nb_tasks;
  volatile uint32_t             all_tasks_mask;
  volatile uint32_t             started_tasks_mask;
  volatile uint32_t             started_tasks_timeout;
  volatile st_os_hl_state_t     state;
  volatile uint32_t             start_tick;
  context_triggeredTask_t      *pBgTaskHdle;
} context_t;


/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef ST_OS_TRG_TASKS_MSG_DEBUG
  static context_debug_t ContextDebug;
#endif

static context_t Context;


/* Private function prototypes -----------------------------------------------*/

static ST_OS_Result s_task_add(context_triggeredTask_t *const pTaskHdle,
                               char              const *const thread_name,
                               uint32_t                 const stack_size,
                               st_task_priority const         priority,
                               st_os_hl_task_thread_cb        thread_func,
                               st_os_hl_task_func_cb          task_func,
                               ARGUMENT_TYPE                  argument,
                               bool                     const is_back_ground_task,
                               bool                     const activate_back_ground_task,
                               bool                     const logTaskQueueLevels,
                               uint8_t                        idx);

static ST_OS_Result s_task_create(context_triggeredTask_t *const pTaskHdle,
                                  char              const *const thread_name,
                                  uint32_t                 const stack_size,
                                  st_task_priority         const priority,
                                  st_os_hl_task_thread_cb        thread_func,
                                  st_os_hl_task_func_cb          task_func,
                                  ARGUMENT_TYPE                  argument,
                                  bool                     const activate_back_ground_task,
                                  bool                     const logTaskQueueLevels);

static ST_OS_Result s_queue_create(context_msgQueue_t *const pQueue,
                                   char         const *const name,
                                   uint32_t            const deepness,
                                   uint32_t            const timeout
                                  );

static bool                  s_delete_handler(context_triggeredTask_t       **const ppTaskHdle);
static ST_OS_Result          s_put_msg(context_triggeredTask_t               *const pTaskHdle, st_os_hl_msg_t const *const pMsg);
static ST_OS_Result          s_get_msg(context_triggeredTask_t               *const pTaskHdle, uint32_t const queue_timeout, st_os_hl_msg_t *const pMsg);
static void                  s_run_back_ground_task(context_triggeredTask_t  *const pTaskHdle);
static void                  s_traceFifoFull(context_triggeredTask_t         *const pTaskHdle);

/* Static functions for debug; empty if ST_OS_DEBUG is not set */
static void s_debug_init(void);
static void s_debug_displayIncreaseStackQueueLevel(context_triggeredTask_t            *const pTaskHdle);
static void s_debug_displayStackQueueLevel(char const *header, context_triggeredTask_t *const pTaskHdle);
static void s_debug_addMsg(context_triggeredTask_t                                     *const pTaskHdle, st_os_msg_dir_t const msgDir, st_os_hl_msg_t const msg, ST_OS_Result const Status);

#define ST_OS_DEBUG_MSG(msgDir) s_debug_addMsg(pTaskHdle, msgDir, msg, Status);


/* Functions Definition ------------------------------------------------------*/

/**
* @brief Init kernel
* @retval None
*/
void st_os_hl_init(void)
{
  /* Init scheduler */
  st_os_init();
}


/**
* @brief start kernel
* @retval None
*/
void st_os_hl_start(uint32_t const timeout)
{
  s_debug_init();

  Context.start_tick            = st_os_hl_tick();
  Context.state                 = ST_OS_HL_READY;
  Context.started_tasks_mask    = 0UL;
  Context.started_tasks_timeout = timeout;

  /* Starts scheduler */
  st_os_start();
}


/**
* @brief creates task
* @param thread_name
* @param queue_name
* @param stack_size
* @param priority
* @param queue_deepness
* @param queue_timeout
* @param thread_func
* @param task_func
* @param is_back_ground_task
* @param activate_back_ground_task
* @param logTaskQueueLevels
* @param ppHdle: pointer to CMSISOS task handle pointer
* @retval anonymous OS Thread handler pointer
*/
void st_os_hl_task_create(char            const *const thread_name,
                          char            const *const queue_name,
                          uint32_t               const stack_size,
                          st_task_priority       const priority,
                          uint32_t               const queue_deepness,
                          uint32_t               const queue_timeout,
                          st_os_hl_task_thread_cb      thread_func,
                          st_os_hl_task_func_cb        task_func,
                          ARGUMENT_TYPE                argument,
                          bool                   const is_back_ground_task,
                          bool                   const activate_back_ground_task,
                          bool                   const logTaskQueueLevels,
                          void                 **const ppHdle)
{
  ST_OS_Result                    error      = ST_OS_OK;
  context_triggeredTask_t **const ppTaskHdle = (context_triggeredTask_t **)ppHdle;
  context_triggeredTask_t        *pTaskHdle  = NULL;
  uint8_t                         idx;

  /* Search for an empty slot */
  for (idx = 0U; idx < ST_OS_TRG_TASKS_NB_MAX; idx++)
  {
    if (Context.tasksHdleTable[idx].task.state  == ST_OS_HL_TASK_NONE)
    {
      pTaskHdle = &Context.tasksHdleTable[idx];
      memset(pTaskHdle, 0, sizeof(context_triggeredTask_t));
      break;
    }
  }
  *ppTaskHdle = pTaskHdle;
  if (pTaskHdle == NULL)
  {
    /* no empty slot */
    error = ST_OS_ERROR;
  }

  if ((error == ST_OS_OK) && (queue_deepness != 0UL))
  {
    error = s_queue_create(&pTaskHdle->queue, queue_name, queue_deepness, queue_timeout);
  }

  if (error == ST_OS_OK)
  {
    error =  s_task_add(pTaskHdle,
                        thread_name,
                        stack_size,
                        priority,
                        thread_func,
                        task_func,
                        argument,
                        is_back_ground_task,
                        activate_back_ground_task,
                        logTaskQueueLevels,
                        idx);
  }

  if (error != ST_OS_OK)
  {
    /* error */
    s_delete_handler(ppTaskHdle);
  }
}


/**
* @brief  terminates task
* @param  ppHdle: anonymous pointer to OS Thread handler pointer
* @param  timeout: timeout in ms
* @retval true if terminated, else false
*/
bool st_os_hl_task_terminate(void **const ppHdle, uint32_t const timeout)
{
  context_triggeredTask_t **const ppTaskHdle = (context_triggeredTask_t **)ppHdle;
  context_triggeredTask_t  *const pTaskHdle  = *ppTaskHdle;
  bool                            terminated = false;

  if ((pTaskHdle != NULL) && (pTaskHdle->task.stos.hTask != NULL))
  {
    /* request thread by setting its state to TERMINATE REQUEST */
    pTaskHdle->task.state = ST_OS_HL_TASK_TERMINATE_REQUEST;

    /* if this thread has a queue, send CONTINUE message to speed up terminate */
    if (st_os_hl_is_task_triggered(pTaskHdle))
    {
      static st_os_hl_msg_t const msg_continue = {.cmd = ST_OS_HL_TASK_CONTINUE, .param = 0U};
      st_os_hl_task_trigger(pTaskHdle, msg_continue);
    }

    /* wait until its state has changed to IDLE (acknowledge of TERMINATE_REQUEST) */
    for (uint32_t time = 0UL; time < timeout; time++)
    {
      if (pTaskHdle->task.state == ST_OS_HL_TASK_IDLE)
      {
        terminated = s_delete_handler(ppTaskHdle);
        break;
      }
      /* wait for task to go back in idle state */
      osDelay(1UL);
    }
  }

  return terminated;
}


/**
* @brief  if task terminate requested, terminates it and return true, else return false
* @param  ppHdle: anonymous pointer to OS Thread handler pointer
* @retval true if task terminated, else false
*/
bool st_os_hl_task_terminated(void **const ppHdle)
{
  context_triggeredTask_t **const ppTaskHdle = (context_triggeredTask_t **)ppHdle;
  context_triggeredTask_t  *const pTaskHdle  = *ppTaskHdle;
  bool                            terminated = false;

  if ((pTaskHdle != NULL) && (pTaskHdle->task.stos.hTask != NULL))
  {
    if (pTaskHdle->task.state == ST_OS_HL_TASK_TERMINATE_REQUEST)
    {
      terminated = s_delete_handler(ppTaskHdle);
    }
  }

  return terminated;
}


/**
* @brief  sends task Pushed message
* @param  pHdle: anonymous OS Thread handler pointer
* @retval true if task is triggered by message, else false
*/
bool st_os_hl_is_task_triggered(void *const pHdle)
{
  context_triggeredTask_t *const pTaskHdle = (context_triggeredTask_t *)pHdle;
  return (pTaskHdle->queue.stos.hQueue != NULL);
}


/**
* @brief  sends task Pushed message
* @param  pHdle: anonymous OS Thread handler pointer
* @param  msg:   message for thread
* @retval true if OK, else false
*/
bool st_os_hl_task_trigger(void *const pHdle, st_os_hl_msg_t const msg)
{
  context_triggeredTask_t *const pTaskHdle = (context_triggeredTask_t *)pHdle;
  ST_OS_Result                   Status;
  st_os_hl_msg_t                 dummy_msg;
  bool                           ok = true;  /* by default, it is OK */

  if (pTaskHdle == NULL)
  {
    /* no handler */
    ok = false;
  }
  if (ok)
  {
    if (pTaskHdle->task.stos.hTask == NULL)
    {
      /* no thread */
      ok = false;
    }
  }
  if (ok)
  {
    if (!st_os_hl_is_task_triggered(pHdle))
    {
      /* no queue */
      ok = false;
    }
  }
  if (ok)
  {
    if (Context.state == ST_OS_HL_IDLE)
    {
      /* CMSISOS_IDLE state => ignore msg */
    }
    else
    {
      if ((Context.state == ST_OS_HL_READY) && (pTaskHdle->queue.msg > 0UL))
      {
        /* CMSISOS_READY state => dequeue last message and ignore it */
        Status = s_get_msg(pTaskHdle, 0UL, &dummy_msg);    // no timeout
        ST_OS_DEBUG_MSG(ST_OS_MSG_GET)
        if (Status == osOK)
        {
          ST_OS_DISABLE_IRQ();
          pTaskHdle->queue.msg--;
          ST_OS_ENABLE_IRQ();
        }
        else
        {
          /* issue in dequeueing => ok = false */
          ok = false;
        }
      }

      if (ok)
      {
        /* send message */
        Status = s_put_msg(pTaskHdle, &msg);
        ok     = (Status == osOK);
        ST_OS_DEBUG_MSG(ST_OS_MSG_PUT)
        if (Status == osErrorResource)
        {
          s_traceFifoFull(pTaskHdle);
        }
      }
      if (ok)
      {
        ST_OS_DISABLE_IRQ();
        pTaskHdle->queue.msg++;
        pTaskHdle->queue.msgMax = (pTaskHdle->queue.msg > pTaskHdle->queue.msgMax) ? pTaskHdle->queue.msg : pTaskHdle->queue.msgMax;
        ST_OS_ENABLE_IRQ();
      }
      else if (Context.state == ST_OS_HL_STARTED)
      {
        uint32_t time_from_start = st_os_hl_tick() - Context.start_tick;

        if (time_from_start < 100UL)
        {
          /* signal an issue only in CMSISOS_STARTED state and after a timeout of 100 ms => ignore issue => ok = true */
          ok = true;
        }
        else
        {
          /* more than 1000 ms after CMSISOS_STARTED state => keep issue => do nothing */
        }
      }
      else
      {
        /* ignore issue in CMSISOS_READY state => ok = true */
        ok = true;
      }
    }
  }

  return ok;
}


/**
* @brief  task
* @param  pHdle: anonymous OS Thread handler pointer
* @param  argument: dummy argument
* @retval None
*/
void st_os_hl_task_thread(void *const pHdle, ARGUMENT_TYPE argument)
{
  context_triggeredTask_t *const pTaskHdle = (context_triggeredTask_t *)pHdle;
  ST_OS_Result                   Status;
  st_os_hl_msg_t                 msg;
  bool                           ok;

  if (pTaskHdle != argument)
  {
    /* wrong handler */
    ST_OS_HL_FAULT();
  }

  if (pTaskHdle != NULL)
  {
    uint32_t maskTmp;

    /* wait for pTaskHdle completely initialized (last pTaskHdle field set is table_id) */
    while (pTaskHdle->task.table_id == 0xFFU)
    {
      osDelay(1UL);
    }

    /* mark current task as started */
    ST_OS_DISABLE_IRQ();
    maskTmp                    = Context.started_tasks_mask;
    maskTmp                   |= (1UL << pTaskHdle->task.table_id);
    Context.started_tasks_mask = maskTmp;
    ST_OS_ENABLE_IRQ();

    /* wait that all tasks are launched */
    while (maskTmp != Context.all_tasks_mask)
    {
      uint32_t time_from_start = st_os_hl_tick() - Context.start_tick;

      if (time_from_start > Context.started_tasks_timeout)
      {
        /* Context.started_tasks_timeout ms timeout => consider all tasks are started (useful for tasks not using st_os_hl_task_thread) */
        break;
      }
      osDelay(1UL);
      maskTmp = Context.started_tasks_mask;
    }

    #ifdef ST_OS_DEBUG
    pTaskHdle->task.last_trace_cycles  = cycleMeasure_currentCycles();
    pTaskHdle->queue.msgLastMax        = 0UL;
    #ifdef INCLUDE_uxTaskGetStackHighWaterMark
    pTaskHdle->task.stack_level        = 0UL;
    pTaskHdle->task.last_stack_level   = 0UL;
    #endif
    #endif

    /* set state to CMSISOS_STARTED when all tasks are launched */
    if (Context.state != ST_OS_HL_STARTED)
    {
      Context.start_tick = st_os_hl_tick();
      Context.state      = ST_OS_HL_STARTED;
    }

    /* current task is now running */
    pTaskHdle->task.state = ST_OS_HL_TASK_RUNNING;

    /* infinite loop until task is terminated */
    while (pTaskHdle->task.state != ST_OS_HL_TASK_IDLE)
    {
      if (st_os_hl_is_task_triggered(pHdle))
      {
        /* current task is a task with message queue => get message */
        Status = s_get_msg(pTaskHdle, pTaskHdle->queue.timeout, &msg);
        ok     = (Status == osOK);
        ST_OS_DEBUG_MSG(ST_OS_MSG_GET)
        if (ok)
        {
          ST_OS_DISABLE_IRQ();
          pTaskHdle->queue.msg--;
          ST_OS_ENABLE_IRQ();
          switch (msg.cmd)
          {
            case ST_OS_HL_TASK_CONTINUE:
              /* do nothing */
              break;
            case ST_OS_HL_TASK_WAIT:
              osDelay(msg.param);
              break;
            case ST_OS_HL_TASK_PROCESS:
              if (pTaskHdle->task.process_cb != NULL)
              {
                pTaskHdle->task.process_cb(msg.param, pTaskHdle->task.argument);
                s_run_back_ground_task(pTaskHdle);
              }
              break;
            default:
            {
              /* unknown task state */
              ST_OS_HL_FAULT();
              break;
            }
          }
        }
        else if (Status == st_os_hl_timeout)
        {
          if (pTaskHdle->task.process_cb != NULL)
          {
            pTaskHdle->task.process_cb(0xFFFFU, pTaskHdle->task.argument);  /* queue_timeout => param = 0xFFFFU */
            s_run_back_ground_task(pTaskHdle);
          }
        }
        else
        {
          /* dequeueing error other than st_os_hl_timeout */
          ST_OS_HL_FAULT();
        }
      }
      else
      {
        /* current task is a task without message queue */
        if (pTaskHdle->task.process_cb != NULL)
        {
          pTaskHdle->task.process_cb(0U, pTaskHdle->task.argument);  /* no message queue => no param */
          s_run_back_ground_task(pTaskHdle);
        }
      }

      /* if terminate is requested, go back in CMSISOS_TASK_IDLE state (then exit from loop, then exit from routine and thus terminate) */
      if (pTaskHdle->task.state == ST_OS_HL_TASK_TERMINATE_REQUEST)
      {
        pTaskHdle->task.state = ST_OS_HL_TASK_IDLE;
      }
    }
  }

  /* wait for task to be killed without doing anything */
  while (1)
  {
    osDelay(1000UL);
  }
}


/* ---------------------------------------------------------------------------*/
/* Static Functions Definition -----------------------------------------------*/
/* ---------------------------------------------------------------------------*/

/**
* @brief creates task
* @param thread_name
* @param stack_size
* @param priority
* @param thread_func
* @param task_func
* @param is_back_ground_task
* @param activate_back_ground_task
* @param logTaskQueueLevels
* @param ppHdle: pointer to CMSISOS task handle pointer
* @retval error
*/
ST_OS_Result s_task_add(context_triggeredTask_t *const pTaskHdle,
                        char              const *const thread_name,
                        uint32_t                 const stack_size,
                        st_task_priority         const priority,
                        st_os_hl_task_thread_cb        thread_func,
                        st_os_hl_task_func_cb          task_func,
                        ARGUMENT_TYPE                  argument,
                        bool                     const is_back_ground_task,
                        bool                     const activate_back_ground_task,
                        bool                     const logTaskQueueLevels,
                        uint8_t                        idx)
{
  ST_OS_Result ret = ST_OS_OK;

  if (idx < ST_OS_TRG_TASKS_NB_MAX)
  {
    if (pTaskHdle != NULL)
    {
      if (is_back_ground_task == true)
      {
        Context.pBgTaskHdle = &Context.tasksHdleTable[idx];
      }

      pTaskHdle->task.argument = argument;
      ret = s_task_create(pTaskHdle,
                          thread_name,
                          stack_size,
                          priority,
                          thread_func,
                          task_func,
                          pTaskHdle,
                          activate_back_ground_task,
                          logTaskQueueLevels);

      if (ret == ST_OS_OK)
      {
        Context.nb_tasks++;
        Context.all_tasks_mask |= (1UL << idx);
        pTaskHdle->task.table_id = idx; // set table_id at the last moment after all other pTaskHdle fields are set => means pTaskHdle is completely initialized
      }
    }
  }
  else
  {
    ret = ST_OS_ERROR;
    #ifdef ST_OS_DEBUG
    trace_print(TRACE_OUTPUT_UART, TRACE_LVL_DEBUG, "%d: CMSISOS too many tasks\n", __FILE__);
    #endif
    ST_OS_HL_FAULT();
  }

  return ret;
}


static ST_OS_Result s_task_create(context_triggeredTask_t *const pTaskHdle,
                                  char              const *const thread_name,
                                  uint32_t                 const stack_size,
                                  st_task_priority         const priority,
                                  st_os_hl_task_thread_cb        thread_func,
                                  st_os_hl_task_func_cb          task_func,
                                  ARGUMENT_TYPE                  argument,
                                  bool                     const activate_back_ground_task,
                                  bool                     const logTaskQueueLevels)
{
  pTaskHdle->task.state                   = ST_OS_HL_TASK_IDLE;
  pTaskHdle->task.table_id                = 0xFFU; // means pTaskHdle not completely initialized
  pTaskHdle->task.firstTraceFifoFull      = true;
  pTaskHdle->task.name                    = thread_name;
  #ifdef ST_OS_DEBUG
  #ifdef INCLUDE_uxTaskGetStackHighWaterMark
  pTaskHdle->task.stack_size              = stack_size;
  #endif
  #endif
  pTaskHdle->task.state                   = ST_OS_HL_TASK_IDLE;
  pTaskHdle->task.process_cb              = task_func;
  pTaskHdle->task.priority                = priority;
  pTaskHdle->task.activateBackGroundTask  = activate_back_ground_task;
  pTaskHdle->task.logQueueLevels          = logTaskQueueLevels;

  return st_os_task_create(&pTaskHdle->task.stos, // thread handler
                           thread_name,           // name of thread
                           thread_func,           // thread routine
                           pTaskHdle,             // argument if any
                           stack_size / 4UL,      // stack size unit = 32-bit word
                           priority               // prio level
                          );
}


static ST_OS_Result s_queue_create(context_msgQueue_t *const pQueue,
                                   char         const *const name,
                                   uint32_t            const deepness,
                                   uint32_t            const timeout
                                  )
{
  #ifdef ST_OS_DEBUG
  pQueue->deepness   = deepness;
  pQueue->msgLastMax = 0UL;
  #endif
  pQueue->timeout    = timeout;
  pQueue->msg        = 0UL;
  pQueue->msgMax     = 0UL;

  return st_os_queue_create_named(&pQueue->stos, deepness, sizeof(int32_t), name);
}


static bool s_delete_handler(context_triggeredTask_t **const ppTaskHdle)
{
  context_triggeredTask_t *const pTaskHdle = *ppTaskHdle;
  uint32_t                 const maskTmp   = ~(1UL << pTaskHdle->task.table_id);
  bool                           ok        = true;

  if (pTaskHdle->task.stos.hTask != NULL)
  {
    /* task has been launched => terminate it */
    st_os_task_delete(&pTaskHdle->task.stos);
  }
  if (st_os_hl_is_task_triggered(pTaskHdle))
  {
    /* task with a queue => delete queue */
    if (st_os_queue_delete(&pTaskHdle->queue.stos) != ST_OS_OK)
    {
      ok = false;
    }
  }

  /* clear task entry into Context.tasksHdleTable */
  ST_OS_DISABLE_IRQ();
  if (pTaskHdle == Context.pBgTaskHdle)
  {
    /* if task to delete is back-ground task, reset its handler */
    Context.pBgTaskHdle = NULL;
  }
  Context.tasksHdleTable[pTaskHdle->task.table_id].task.state = ST_OS_HL_TASK_NONE;
  Context.started_tasks_mask                                 &= maskTmp;
  Context.all_tasks_mask                                     &= maskTmp;
  *ppTaskHdle                                                 = NULL;
  Context.nb_tasks--;
  ST_OS_ENABLE_IRQ();

  return ok;
}


static ST_OS_Result s_put_msg(context_triggeredTask_t *const pTaskHdle, st_os_hl_msg_t const *const pMsg)
{
  return st_os_queue_put(&pTaskHdle->queue.stos, (void *)&pMsg->u32, 0UL);
}


static ST_OS_Result s_get_msg(context_triggeredTask_t *const pTaskHdle, uint32_t const queue_timeout, st_os_hl_msg_t *const pMsg)
{
  return st_os_queue_get(&pTaskHdle->queue.stos, &pMsg->u32, queue_timeout);
}


static void s_run_back_ground_task(context_triggeredTask_t *const pTaskHdle)
{
  if ((pTaskHdle != Context.pBgTaskHdle) && pTaskHdle->task.activateBackGroundTask &&
      (Context.pBgTaskHdle != NULL) && (Context.pBgTaskHdle->queue.msg == 0UL))
  {
    /* when a task different from back-ground task has run,
    send a CMSISOS_TASK_PROCESS to back-ground task
    (if back-ground task exists and if it has not already any message in its queue)
    to make back-ground task run */
    static st_os_hl_msg_t const msg_process = {.cmd = ST_OS_HL_TASK_PROCESS, .param = 0U};
    st_os_hl_task_trigger(Context.pBgTaskHdle, msg_process);
  }

  s_debug_displayIncreaseStackQueueLevel(pTaskHdle);
}


static void s_traceFifoFull(context_triggeredTask_t *const pTaskHdle)
{
  bool traceOk = false;

  if (pTaskHdle->task.firstTraceFifoFull)
  {
    pTaskHdle->task.firstTraceFifoFull    = false;
    pTaskHdle->task.lastTickTraceFifoFull = HAL_GetTick();
    traceOk                               = true;
  }
  else
  {
    uint32_t const curTick = HAL_GetTick();

    if ((curTick - pTaskHdle->task.lastTickTraceFifoFull) > 1000UL)
    {
      pTaskHdle->task.lastTickTraceFifoFull = curTick;
      traceOk                               = true;
    }
  }
  if (traceOk)
  {
    trace_print(TRACE_OUTPUT_UART, TRACE_LVL_DEBUG, "%s FIFO full\n", pTaskHdle->task.name);
  }
}


void s_debug_init(void)
{
  #ifdef ST_OS_DEBUG
  cycleMeasure_Init();
  #ifdef ST_OS_TRG_TASKS_MSG_DEBUG
  memset(ContextDebug.taskMsgTrace, 0, sizeof(ContextDebug.taskMsgTrace));
  ContextDebug.idx_msg_trace         = 0;
  ContextDebug.start_cycles          = cycleMeasure_currentCycles();
  #endif
  #endif
}


static void s_debug_displayIncreaseStackQueueLevel(context_triggeredTask_t *const pTaskHdle)
{
  #ifdef ST_OS_DEBUG
  uint32_t const  cycles         = cycleMeasure_currentCycles();
  uint32_t const  elapsed_cycles = cycles - pTaskHdle->task.last_trace_cycles;
  #ifdef INCLUDE_uxTaskGetStackHighWaterMark
  uint32_t const  free_stack     = st_os_task_get_stack_high_water_mark(&pTaskHdle->task.stos) * sizeof(StackType_t);

  pTaskHdle->task.stack_level = pTaskHdle->task.stack_size - free_stack;
  if ((pTaskHdle->task.stack_level > pTaskHdle->task.last_stack_level) ||
      ((pTaskHdle->queue.deepness > 0UL) && (pTaskHdle->queue.msgMax > pTaskHdle->queue.msgLastMax)))
  #else
  if ((pTaskHdle->queue.deepness > 0UL) && (pTaskHdle->queue.msgMax > pTaskHdle->queue.msgLastMax))
  #endif
  {
    if (elapsed_cycles > cycleMeasure_getSystemCoreClock())
    {
      pTaskHdle->task.last_trace_cycles  = cycles;
      pTaskHdle->queue.msgLastMax = pTaskHdle->queue.msgMax;
      #ifdef INCLUDE_uxTaskGetStackHighWaterMark
      pTaskHdle->task.last_stack_level   = pTaskHdle->task.stack_level;
      #endif
      if (pTaskHdle->task.logQueueLevels)
      {
        #ifdef ST_OS_DISPLAY_ALL_TASKS_DEBUG
        trace_print(TRACE_OUTPUT_UART, TRACE_LVL_DEBUG, "************************************* CMSISOS stack / queue stats ***********************************\n");
        for (uint8_t i = 0U; i < ST_OS_TRG_TASKS_NB_MAX; i++)
        {
          if (((Context.started_tasks_mask & (1UL << i)) != 0UL) && Context.tasksHdleTable[i].task.logQueueLevels)
          {
            s_debug_displayStackQueueLevel("", &Context.tasksHdleTable[i]);
          }
        }
        #else
        s_debug_displayStackQueueLevel("task ",  pTaskHdle);
        #endif
      }
    }
  }
  #else /* !ST_OS_DEBUG => avoid warning */
  s_debug_displayStackQueueLevel(NULL, NULL); // set arguments to NULL: they are not used when !ST_OS_DEBUG
  #endif
}


static void s_debug_displayStackQueueLevel(char const *header, context_triggeredTask_t *const pTaskHdle)
{
  #ifdef ST_OS_DEBUG
  char     const *thread_name    = pTaskHdle->task.name;
  uint32_t const  queue_deepness = pTaskHdle->queue.deepness;
  #ifdef INCLUDE_uxTaskGetStackHighWaterMark
  uint32_t const  stack_size     = pTaskHdle->task.stack_size;
  uint32_t const  stack_level    = pTaskHdle->task.stack_level;
  uint32_t const  stack_pcent    = (stack_size == 0UL) ? 100UL : ((100UL * stack_level) / stack_size);
  #endif

  if (queue_deepness > 0UL)
  {
    uint32_t const msg_queue_max = pTaskHdle->queue.msgMax;
    uint32_t const queue_pcent   = (100UL * msg_queue_max) / queue_deepness;

    trace_print(TRACE_OUTPUT_UART,
                TRACE_LVL_DEBUG,
                #ifdef INCLUDE_uxTaskGetStackHighWaterMark
                "%s%-20s: stack level = %4d / %4d (%2d %%) - queue level = %d / %d (%2d %%) - prio level = %2d\n",
                header,
                thread_name,
                stack_level,
                stack_size,
                stack_pcent,
                #else
                "%s%-20s: queue level = %d / %d (%2d %%) - prio level = %2d\n",
                header,
                thread_name,
                #endif
                msg_queue_max,
                queue_deepness,
                queue_pcent,
                pTaskHdle->task.priority);
  }
  #ifdef INCLUDE_uxTaskGetStackHighWaterMark
  else
  {
    trace_print(TRACE_OUTPUT_UART,
                TRACE_LVL_DEBUG,
                "%s%-20s: stack level = %4d / %4d (%2d %%) - queue level = 0 / 0        - prio level = %2d\n",
                header,
                thread_name,
                stack_level,
                stack_size,
                stack_pcent,
                pTaskHdle->task.priority);
  }
  #endif
  #endif
}


static void s_debug_addMsg(context_triggeredTask_t *const pTaskHdle, st_os_msg_dir_t const msgDir, st_os_hl_msg_t const msg, ST_OS_Result const Status)
{
  #ifdef ST_OS_TRG_TASKS_MSG_DEBUG
  ST_OS_DISABLE_IRQ();
  #ifndef ST_OS_TRG_TASKS_MSG_DEBUG_WRAP_IDX
  if (ContextDebug.idx_msg_trace < ST_OS_HL_MSG_TRACE_DEPTH)
  #endif
  {
    ContextDebug.taskMsgTrace[ContextDebug.idx_msg_trace].pTaskHdle = pTaskHdle;
    ContextDebug.taskMsgTrace[ContextDebug.idx_msg_trace].cmsios_state           = Context.state;
    ContextDebug.taskMsgTrace[ContextDebug.idx_msg_trace].msgDir                 = msgDir;
    ContextDebug.taskMsgTrace[ContextDebug.idx_msg_trace].msg                    = msg;
    ContextDebug.taskMsgTrace[ContextDebug.idx_msg_trace].cycles                 = cycleMeasure_currentCycles() - ContextDebug.start_cycles;
    ContextDebug.taskMsgTrace[ContextDebug.idx_msg_trace].Status                 = Status;
    ContextDebug.idx_msg_trace++;
    #ifdef ST_OS_TRG_TASKS_MSG_DEBUG_WRAP_IDX
    if (ContextDebug.idx_msg_trace == ST_OS_HL_MSG_TRACE_DEPTH)
    {
      ContextDebug.idx_msg_trace = 0;
    }
    #endif
  }
  ST_OS_ENABLE_IRQ();
  #endif
}
