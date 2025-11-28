/**
******************************************************************************
* @file    stm32_audio_tasks_st_os.c
* @author  MCD Application Team
* @brief   Provide abstraction to switch from OS no OS, etc...
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
#include "stm32_audio_tasks.h"
#include "stm32_audio.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

#ifdef UTIL_AUDIO_RTOS_USED
#include "st_os.h"
#include "st_message.h"
#ifndef AUDIOIN_TASK_MESSAGE_QUEUE_DEEPNESS
  #define AUDIOIN_TASK_MESSAGE_QUEUE_DEEPNESS 1
#endif

#ifndef AUDIOIN_TASK_PROCESS_STACK_SIZE
  #define AUDIOIN_TASK_PROCESS_STACK_SIZE     500
#endif

#ifndef AUDIOIN_TASK_PROCESS_PRIO
  #define AUDIOIN_TASK_PROCESS_PRIO           (st_task_priority)ST_Priority_High
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct audio_task_t
{
  bool     bRunning;
  st_queue hAudioInQueue;
  st_task  hAudioInTask;
} audio_task_t;

/* Private variables ---------------------------------------------------------*/
static audio_task_t hAudioTask;

//static st_queue AudioInQueue;
//static st_task  AudioInTask;
/* Private function prototypes -----------------------------------------------*/

static void s_AudioInTask_Thread(const void *pCookie);


/* Functions Definition ------------------------------------------------------*/

static int32_t msg_trig;

/**
* @brief  creates task for Audio
* @param  logTaskQueueLevels: flag
* @retval None
*/
void UTIL_AUDIO_createTask(bool const logTaskQueueLevels)
{
  memset(&hAudioTask, 0, sizeof(hAudioTask));

  if (st_os_queue_create_named(&hAudioTask.hAudioInQueue,              // Message queue handler
                               AUDIOIN_TASK_MESSAGE_QUEUE_DEEPNESS,    // number of messages in the queue
                               sizeof(int32_t),                        // message size
                               "AudioIn_WakeMsg"                       // queue_name
                              ) != ST_OS_OK)
  {
    ST_MESSAGE_TRACE_ERROR("st_os_queue_create_named failed");
  }

  if (st_os_task_create(&hAudioTask.hAudioInTask,                                // thread handler
                        "AudioIn_Thread",                                        // name of thread
                        s_AudioInTask_Thread,                                    // thread routine
                        &hAudioTask,                                             // argument if any
                        AUDIOIN_TASK_PROCESS_STACK_SIZE,                         // stack size unit = 32-bit word
                        AUDIOIN_TASK_PROCESS_PRIO                                // prio level
                       ) != ST_OS_OK)
  {
    ST_MESSAGE_TRACE_ERROR("st_os_task_create failed");
  }
}


/**
* @brief  terminates task for Audio
* @param  None
* @retval None
*/
void UTIL_AUDIO_terminateTask(void)
{
  hAudioTask.bRunning = false;
  if (st_os_queue_delete(&hAudioTask.hAudioInQueue) != ST_OS_OK)
  {
    ST_MESSAGE_TRACE_ERROR("st_os_queue_delete failed");
  }
  st_os_task_delete(&hAudioTask.hAudioInTask);
}


/**
* @brief  sends Audio Pushed message
* @param  None
* @retval None
*/
void UTIL_AUDIO_triggerTask(void)
{
  if (hAudioTask.bRunning)
  {
    ST_OS_Result result = st_os_queue_put(&hAudioTask.hAudioInQueue, (void *)&msg_trig, 0UL);
    if (result != ST_OS_OK)
    {
      ST_MESSAGE_TRACE_ERROR("st_os_queue_put failed");
    }
  }
}

/**
* @brief  Audio task
* @param  argument: pointer that is passed to the thread function as start argument.
* @retval None
*/
static void s_AudioInTask_Thread(const void *pCookie)
{
  audio_task_t *pHandle = (audio_task_t *)pCookie;
  pHandle->bRunning = true;
  while (pHandle->bRunning)
  {
    ST_OS_Result result = st_os_queue_get(&pHandle->hAudioInQueue, (void *)&msg_trig, 1000L);

    if (result == ST_OS_OK)
    {
      UTIL_AUDIO_process();
    }
  }
  st_os_task_delete(NULL);
}


#endif /* UTIL_AUDIO_RTOS_USED */
