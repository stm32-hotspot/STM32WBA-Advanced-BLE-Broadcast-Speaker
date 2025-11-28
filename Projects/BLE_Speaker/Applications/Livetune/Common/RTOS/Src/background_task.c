/**
******************************************************************************
* @file    background_task.c
* @author  MCD Application Team
* @brief   background task
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
#include "background_task.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#ifndef ST_OS_HL_TASK_BACKGROUND_STACK_SIZE
  #define ST_OS_HL_TASK_BACKGROUND_STACK_SIZE 512UL
#endif

#ifndef ST_OS_HL_TASK_BACKGROUND_PRIO
  #define ST_OS_HL_TASK_BACKGROUND_PRIO       (st_task_priority)ST_Priority_Idle
#endif

#ifndef ST_OS_HL_TASK_BACKGROUND_DEEPNESS
  #define ST_OS_HL_TASK_BACKGROUND_DEEPNESS   4UL
#endif

#ifndef ST_OS_HL_TASK_BACKGROUND_TIMEOUT
  #define ST_OS_HL_TASK_BACKGROUND_TIMEOUT    10UL
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static void *s_pBackgroundThreadHandler = NULL;

/* Private function prototypes -----------------------------------------------*/
static void s_backgroundThread(ARGUMENT_TYPE argument);

/* Functions Definition ------------------------------------------------------*/

/**
* @brief  Implements background processing as regular OS thread
*         This should be the default implementation,
*         We should not use vApplicationIdleHook for user processing, because
*         it is not possible to control the heap size
* @param  routine to be called in thread
* @retval TRUE if OK, else FALSE
*/
bool st_os_hl_task_background_create(st_os_hl_task_func_cb task_func)
{
  st_os_hl_task_create("Background_Thread",                      // thread_name
                       "Background_Queue",                       // queue_name
                       ST_OS_HL_TASK_BACKGROUND_STACK_SIZE,      // stack_size
                       ST_OS_HL_TASK_BACKGROUND_PRIO,            // priority
                       ST_OS_HL_TASK_BACKGROUND_DEEPNESS,        // queue_deepness
                       ST_OS_HL_TASK_BACKGROUND_TIMEOUT,         // queue_timeout
                       s_backgroundThread,                       // thread_func
                       task_func,                                // task_func
                       NULL,                                     // cmsisos void *argument
                       true,                                     // it is the background task
                       false,                                    // it is the background task, so do not activate itself
                       false,                                    // activate logTaskQueueLevels
                       &s_pBackgroundThreadHandler);             // returned handler

  return (s_pBackgroundThreadHandler != NULL);
}


/* ---------------------------------------------------------------------------*/
/* Static Functions Definition -----------------------------------------------*/
/* ---------------------------------------------------------------------------*/

static void s_backgroundThread(ARGUMENT_TYPE argument)
{
  st_os_hl_task_thread(s_pBackgroundThreadHandler, argument);
}
