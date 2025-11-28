/**
  ******************************************************************************
  * @file    st_os_hl.h
  * @author  MCD Application Team
  * @brief   Header for configuration of st_os_hl.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/*******************************************************************************
    st_os_hl documentation:
    ==============================

    common API for CMSISOS v1 & v2

    st_os_hl_start() routine:
      - starts OS and launches already created tasks
      - waits that all already created task have been launched before starting to manage
        messages from queues (in the mean time, only the last message is kept)
      - a timeout has been added in case user uses st_os_hl_task_create() API to create
        a task but doesn't use st_os_hl_task_thread() to manage its thread
        (because in such case, st_os_hl can't know that this task has been launched
        and thus would be blocked if there was no timeout)

    st_os_hl_task_create() routine:
      - may be called before or after st_os_hl_start() call; it returns the created task thread handle
      - when it is called after st_os_hl_start(), it is launched immediately since OS is already running
      - tasks may be declared as back-ground task or not: only 1 task may be back-ground task
        => you can have 0 or 1 back-ground task
      - tasks may have a message queue or not (user must set queue_deepness to 0 for tasks without queue)
      - if task has a queue and if user sets queue_timeout to osWaitForever, message receive is blocking,
        else when queue_timeout expires, task_func() is called with 0xFFFF param (typically,
        back-ground task is created with a queue_timeout equal to the max time interval it must be run)
      - tasks may have a task_func or not:
        - task_func(param) is called inside st_os_hl_task_thread() when task receives
          a ST_OS_HL_TASK_PROCESS message (message's param is the param given to task_func)
        - if a task thread uses st_os_hl_task_thread() API but has no message queue, task_func is called
          with 0 as parameter; if such case, task_func mustn't be blocking, i.e. user must manage himself
          inside task_func a way to give back control to other tasks (using osDelay for instance)
        - if a task thread doesn't use st_os_hl_task_thread() API, task_func is never called
          and thus is useless (it may be set to NULL)
        - if a task thread doesn't use st_os_hl_task_thread() API, it's message is not managed
          by st_os_hl and thus must be managed by user's task thread routine

    st_os_hl_task_terminate() routine:
      - terminates task for tasks using st_os_hl_task_thread() API to manage its thread
      - for tasks not using st_os_hl_task_thread() API, user must call st_os_hl_task_terminated() API
        inside its own thread routine to terminate it at the right moment to avoid FW crash
        (when user's routine isn't running)
      - timeout should never be useful but it has been added in case a task is blocked
        (if so, FW will perhaps crash when st_os_hl will force its terminate)

    st_os_hl_task_terminated() routine:
      - useful only for tasks not using st_os_hl_task_thread() API
        (see st_os_hl_task_terminate routine description)
      - checks if task must be terminated and terminates it if so
      - if task is terminated, it returns 1, else it returns 0

    st_os_hl_task_trigger() routine:
      - sends a message to task; 3 message commands are available:
        - ST_OS_HL_TASK_CONTINUE: do nothing, simply permits to st_os_hl_task_thread to run for
                                 a loop (for instance to check if it must be terminated),
                                 message's param is ignored
        - ST_OS_HL_TASK_WAIT:     do nothing except waiting for a delay given by message's param
                                 and then loop inside st_os_hl_task_thread
        - ST_OS_HL_TASK_PROCESS:  runs task_func with message's param and then loop inside
                                 st_os_hl_task_thread

    st_os_hl_task_thread() routine:
      - argument parameter comes from FreeRTOS thread API but is currently not used
        inside st_os_hl
      - waits that all tasks are launched before entering the message receive loop
        (see st_os_hl_start() routine description regarding its timeout management)
      - wait for messages (blocking or non-blocking : see queue_timeout into
        st_os_hl_task_create() routine description) and execute the message command
        (see st_os_hl_task_trigger() routine)
      - when task_func has been run (when ST_OS_HL_TASK_PROCESS command was received),
        sends a ST_OS_HL_TASK_PROCESS command message to back-ground task if it exists
        so that back-ground task may do its periodical stuff
      - st_os_hl_task_thread is never ending routine (infinite loop) except when task
        is terminated (see st_os_hl_task_terminate routine description)
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ST_OS_HL_H
#define __ST_OS_HL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
/*cstat -MISRAC2012-Rule-10.4_a issue in FreeRTOS*/
#include "st_os.h"
/*cstat +MISRAC2012-Rule-10.4_a*/

/* Exported macros -----------------------------------------------------------*/
#ifndef   AUDIO_BUILD_RELEASE
#define   ST_OS_HL_FAULT()  {__disable_irq();while (1);}
#else
#define   ST_OS_HL_FAULT()
#endif
/* Exported types ------------------------------------------------------------*/
#define ARGUMENT_TYPE const void *
typedef enum
{
  ST_OS_HL_TASK_CONTINUE,
  ST_OS_HL_TASK_WAIT,
  ST_OS_HL_TASK_PROCESS
} st_os_hl_task_msg_t;

typedef union
{
  struct
  {
    uint16_t cmd;     // st_os_hl_task_msg_t
    uint16_t param;   // param given to task_func when cmd is CMSISOS_TASK_PROCESS
  };
  uint32_t u32;
} st_os_hl_msg_t;

typedef void (*st_os_hl_task_thread_cb)(ARGUMENT_TYPE argument);
typedef void (*st_os_hl_task_func_cb)(uint16_t const param, ARGUMENT_TYPE argument);

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void st_os_hl_init(void);

void st_os_hl_start(uint32_t                  const timeout);                                       /* timeout is ms (max wait time for all already created tasks to be launched) */

void st_os_hl_task_create(char const            *const thread_name,                                 /* thread name */
                          char const            *const queue_name,                                  /* queue name: not used in CMSISOS v1 */
                          uint32_t               const stack_size,                                  /* stack size in bytes */
                          st_task_priority const priority,                                          /* FreeRTOS priority */
                          uint32_t               const queue_deepness,                              /* max number of messages in queue, 0 if no queue */
                          uint32_t               const queue_timeout,                               /* message receive timeout in ms */
                          st_os_hl_task_thread_cb      thread_func,                                 /* thread func to be called by FreeRTOS */
                          st_os_hl_task_func_cb        task_func,                                   /* task func to execute in case of msg receive (param of func will be set to param of msg if no queue_timeout, else to 0xFFFF) */
                          ARGUMENT_TYPE          const argument,                                    /* pointer to CMSISOS's argument */
                          bool                   const is_back_ground_task,                         /* flag saying if this task is a back-ground task or not */
                          bool                   const activate_back_ground_task,                   /* flag saying if this task must activate back-ground task or not */
                          bool                   const logTaskQueueLevels,                          /* log CMSIS OS stack/queue level */
                          void                 **const ppHdle);                                     /* pointer to CMSISOS task handle pointer (returned value) */

bool st_os_hl_task_terminate(void           **const ppHdle,                                         /* pointer to CMSISOS task handle pointer (returned value) */
                             uint32_t         const timeout);                                       /* terminate timeout in ms */

bool st_os_hl_task_terminated(void          **const ppHdle);                                        /* pointer to CMSISOS task handle pointer (returned value) */

bool st_os_hl_task_trigger(void              *const pHdle,                                          /* CMSISOS task handle pointer */
                           st_os_hl_msg_t const msg);                                               /* queue message */

void st_os_hl_task_thread(void               *const pHdle,                                          /* CMSISOS task handle pointer */
                          ARGUMENT_TYPE             argument);                                      /* argument given in CMSISOS task creation (internal CMSISOS param) */

bool st_os_hl_is_task_triggered(void *const pHdle);


/* Prototypes of weak or extern functions used by st_os_hl------------------- */
void StartIdleMonitor(void);
void EndIdleMonitor(void);

#ifdef __cplusplus
}
#endif

#endif /* __ST_OS_HL_H */
