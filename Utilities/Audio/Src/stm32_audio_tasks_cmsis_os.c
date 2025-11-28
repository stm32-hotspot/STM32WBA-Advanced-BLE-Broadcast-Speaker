/**
******************************************************************************
* @file    stm32_audio_tasks_cmsisos.c
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

#ifdef USE_FREERTOS
  /*cstat -MISRAC2012-Rule-10.4_a  A third-party produces a violation message we cannot fix*/
  #include "FreeRTOS.h"
  /*cstat +MISRAC2012-Rule-10.4_a */
  #define RTOS_TICK_RATE_HZ configTICK_RATE_HZ
#endif
#ifdef USE_THREADX
  #include "tx_user.h"
  #define RTOS_TICK_RATE_HZ TX_TIMER_TICKS_PER_SECOND
#endif

#include "st_os_hl.h"

#ifndef max
  #define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef AUDIOIN_TASK_MESSAGE_QUEUE_DEEPNESS
  #define AUDIOIN_TASK_MESSAGE_QUEUE_DEEPNESS 8UL
#endif

#ifndef AUDIOIN_TASK_PROCESS_STACK_SIZE
  #define AUDIOIN_TASK_PROCESS_STACK_SIZE     2048UL
#endif

#ifndef AUDIOIN_TASK_PROCESS_PRIO
  #define AUDIOIN_TASK_PROCESS_PRIO           (st_task_priority)ST_Priority_High
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static void *AudioInTask_Thread_handler = NULL;

/* Private function prototypes -----------------------------------------------*/
static void s_AudioInTask_Thread(ARGUMENT_TYPE argument);
static void s_process(uint16_t const param, ARGUMENT_TYPE argument);


/* Functions Definition ------------------------------------------------------*/

static st_os_hl_msg_t const msg_process =
{
  .cmd   = ST_OS_HL_TASK_PROCESS,
  .param = 0U                 // not used for audio_task
};

/**
* @brief  creates task for Audio
* @param  logTaskQueueLevels: flag
* @retval None
*/
void UTIL_AUDIO_createTask(bool const logTaskQueueLevels)
{
  if (RTOS_TICK_RATE_HZ != max(1000UL, (1000UL * UTIL_AUDIO_N_MS_DIV) / UTIL_AUDIO_N_MS_PER_INTERRUPT))
  {
    // RTOS_TICK_RATE_HZ must be equal to max(1000UL, (1000UL * UTIL_AUDIO_N_MS_DIV) / UTIL_AUDIO_N_MS_PER_INTERRUPT)
    UTIL_AUDIO_error();
  }

  st_os_hl_task_create("AudioIn_Thread",                      // thread_name
                       "AudioIn_WakeMsg",                     // queue_name
                       AUDIOIN_TASK_PROCESS_STACK_SIZE,       // stack_size
                       AUDIOIN_TASK_PROCESS_PRIO,             // priority
                       AUDIOIN_TASK_MESSAGE_QUEUE_DEEPNESS,   // queue_deepness
                       osWaitForever,                         // queue_timeout
                       s_AudioInTask_Thread,                  // thread_func
                       s_process,                             // task_func
                       NULL,                                  // cmsisos void *argument
                       false,                                 // not a back-ground task
                       false,                                 // don't activate back-ground task
                       logTaskQueueLevels,                    // log CMSIS OS stack/queue level
                       &AudioInTask_Thread_handler);          // returned handler
  if (AudioInTask_Thread_handler == NULL)
  {
    UTIL_AUDIO_error();
  }
}


/**
* @brief  terminates task for Audio
* @param  None
* @retval None
*/
void UTIL_AUDIO_terminateTask(void)
{
  if (st_os_hl_task_terminate(&AudioInTask_Thread_handler, 1000UL))
  {
    AudioInTask_Thread_handler = NULL;
  }
  else
  {
    UTIL_AUDIO_error();
  }
}


/**
* @brief  sends Audio Pushed message
* @param  None
* @retval None
*/
void UTIL_AUDIO_triggerTask(void)
{
  st_os_hl_task_trigger(AudioInTask_Thread_handler, msg_process);
}


/**
* @brief  Audio task
* @param  argument: pointer that is passed to the thread function as start argument.
* @retval None
*/
static void s_AudioInTask_Thread(ARGUMENT_TYPE argument)
{
  st_os_hl_task_thread(AudioInTask_Thread_handler, argument);
}


static void s_process(uint16_t const param, ARGUMENT_TYPE argument)
{
  UTIL_AUDIO_process();
}


#endif /* UTIL_AUDIO_RTOS_USED */
