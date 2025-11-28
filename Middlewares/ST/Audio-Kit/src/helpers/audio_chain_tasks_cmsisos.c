/**
******************************************************************************
* @file    audio_chain_tasks_cmsisos.c
* @author  MCD Application Team
* @brief   audio chain os abstraction layer for freeRTOS
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
#include "audio_chain_instance.h"
#include "audio_chain_tasks.h"

#ifdef AUDIO_CHAIN_TASKS_OS_USED

#include "st_os_hl.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#ifndef AUDIO_CHAIN_TASKS_MESSAGE_QUEUE_DEEPNESS
  #define AUDIO_CHAIN_TASKS_MESSAGE_QUEUE_DEEPNESS      7UL
#endif

#ifndef AUDIO_CHAIN_TASKS_LL_MESSAGE_QUEUE_DEEPNESS
  #define AUDIO_CHAIN_TASKS_LL_MESSAGE_QUEUE_DEEPNESS   7UL
#endif

#ifndef AUDIO_CHAIN_DATAINOUT_TASK_STACK_SIZE
  #define AUDIO_CHAIN_DATAINOUT_TASK_STACK_SIZE         1536UL
#endif

#ifndef AUDIO_CHAIN_PROCESS_TASK_STACK_SIZE
  #define AUDIO_CHAIN_PROCESS_TASK_STACK_SIZE           4096UL
#endif

#ifndef AUDIO_CHAIN_PROCESS_LOW_LEVEL_TASK_STACK_SIZE
  #define AUDIO_CHAIN_PROCESS_LOW_LEVEL_TASK_STACK_SIZE 1024UL
#endif

#ifndef AUDIO_CHAIN_CONTROL_TASK_STACK_SIZE
  #define AUDIO_CHAIN_CONTROL_TASK_STACK_SIZE           1024UL
#endif

/* WARNING: if tasks cycles measure is used: audioCapture task and audioChain tasks must all have different priorities with FreeRTOS (tasks of same priority mustn't interrupt each other) */
/* moreover, they must all have a higher priority than ST_OS_HL_TASK_BACKGROUND_PRIO */

#ifndef AUDIO_CHAIN_TASK_DATAINOUT_PRIO
  #define AUDIO_CHAIN_TASK_DATAINOUT_PRIO               (st_task_priority)ST_Priority_Above_Normal
#endif

#ifndef AUDIO_CHAIN_TASK_PROCESS_PRIO
  #define AUDIO_CHAIN_TASK_PROCESS_PRIO                 (st_task_priority)ST_Priority_Normal
#endif

#ifndef AUDIO_CHAIN_TASK_PROCESS_LOW_LEVEL_PRIO
  #define AUDIO_CHAIN_TASK_PROCESS_LOW_LEVEL_PRIO       (st_task_priority)ST_Priority_Below_Normal
#endif

#ifndef AUDIO_CHAIN_TASK_CONTROL_PRIO
  #define AUDIO_CHAIN_TASK_CONTROL_PRIO                 (st_task_priority)ST_Priority_Normal
#endif


/* Private function prototypes -----------------------------------------------*/
static void s_audioChainDataInOut_Thread(ARGUMENT_TYPE       argument);
static void s_audioChainProcess_Thread(ARGUMENT_TYPE         argument);
static void s_audioChainProcessLowLevel_Thread(ARGUMENT_TYPE argument);
static void s_audioChainControl_Thread(ARGUMENT_TYPE         argument);

static void s_audioChain_dataInOut(uint16_t            const param, ARGUMENT_TYPE argument);
static void s_audioChain_process(uint16_t              const param, ARGUMENT_TYPE argument);
static void s_audioChain_processLowLevel(uint16_t      const param, ARGUMENT_TYPE argument);
static void s_audioChain_control(uint16_t              const param, ARGUMENT_TYPE argument);

/* Private variables ---------------------------------------------------------*/
static void *AudioChainDataInOut_Thread_handler       = NULL;
static void *AudioChainProcess_Thread_handler         = NULL;
static void *AudioChainProcessLowLevel_Thread_handler = NULL;
static void *AudioChainControl_Thread_handler         = NULL;

/* Functions Definition ------------------------------------------------------*/

static st_os_hl_msg_t const msg_process =
{
  .cmd   = ST_OS_HL_TASK_PROCESS,
  .param = 0U                 // not used for audio_chain_task
};

/* ---------------------------------------------------------------------------*/
/* DataInOut API -------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

/**
* @brief  creates task for AudioChain dataInOut (high prio)
* @param  None
* @retval None
*/
void AudioChain_task_create_dataInOut(bool const logTaskQueueLevels)
{
  if (AudioChainDataInOut_Thread_handler == NULL)
  {
    st_os_hl_task_create("AC_DataIO_Thread",                        // thread_name
                         "AC_DataIO_WakeMsg",                       // queue_name
                         AUDIO_CHAIN_DATAINOUT_TASK_STACK_SIZE,     // stack_size
                         AUDIO_CHAIN_TASK_DATAINOUT_PRIO,           // priority
                         AUDIO_CHAIN_TASKS_MESSAGE_QUEUE_DEEPNESS,  // queue_deepness
                         osWaitForever,                             // queue_timeout
                         s_audioChainDataInOut_Thread,              // thread_func
                         s_audioChain_dataInOut,                    // task_func
                         NULL,                                      // cmsisos void *argument
                         false,                                     // not a back-ground task
                         true,                                      // activate back-ground task
                         logTaskQueueLevels,                        // log CMSIS OS stack/queue level
                         &AudioChainDataInOut_Thread_handler);      // returned handler
    if (AudioChainDataInOut_Thread_handler == NULL)
    {
      AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "st_os_hl_task_create AC_DataIO_Thread error");
    }
  }
}


/**
* @brief  terminates task for AudioChain dataInOut (high prio)
* @param  None
* @retval None
*/
void AudioChain_task_terminate_dataInOut(void)
{
  if (st_os_hl_task_terminate(&AudioChainDataInOut_Thread_handler, 1000UL))
  {
    AudioChainDataInOut_Thread_handler = NULL;
  }
  else
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "st_os_hl_task_terminate error");
  }
}


/**
* @brief  sends AudioChain dataInOut (high prio) Pushed message
* @param  None
* @retval None
*/
void AudioChain_task_trigger_dataInOut(void)
{
  st_os_hl_task_trigger(AudioChainDataInOut_Thread_handler, msg_process);
}


/**
* @brief  AudioChain dataInOut (high prio)
* @param  argument argument
* @retval None
*/
static void s_audioChainDataInOut_Thread(ARGUMENT_TYPE argument)
{
  st_os_hl_task_thread(AudioChainDataInOut_Thread_handler, argument);
}

/* ---------------------------------------------------------------------------*/
/* Process API ---------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

/**
* @brief  creates task for AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_create_process(bool const logTaskQueueLevels)
{
  if (AudioChainProcess_Thread_handler == NULL)
  {
    st_os_hl_task_create("AC_Proc_Thread",                          // thread_name
                         "AC_Proc_WakeMsg",                         // queue_name
                         AUDIO_CHAIN_PROCESS_TASK_STACK_SIZE,       // stack_size
                         AUDIO_CHAIN_TASK_PROCESS_PRIO,             // priority
                         AUDIO_CHAIN_TASKS_MESSAGE_QUEUE_DEEPNESS,  // queue_deepness
                         osWaitForever,                             // queue_timeout
                         s_audioChainProcess_Thread,                // thread_func
                         s_audioChain_process,                      // task_func
                         NULL,                                      // cmsisos void *argument
                         false,                                     // not a back-ground task
                         true,                                      // activate back-ground task
                         logTaskQueueLevels,                        // log CMSIS OS stack/queue level
                         &AudioChainProcess_Thread_handler);        // returned handler
    if (AudioChainProcess_Thread_handler == NULL)
    {
      AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "st_os_hl_task_create AC_Proc_Thread error");
    }
  }

  /* create low-level process thread if any low-level processing required */
  if (((uint32_t)AudioChain_getPrioLevel(&AudioChainInstance) & (uint32_t)AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW) != 0UL)
  {
    AudioChain_task_create_process_lowlevel(logTaskQueueLevels);
  }
}


/**
* @brief  terminates task for AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_terminate_process(void)
{
  if (st_os_hl_task_terminate(&AudioChainProcess_Thread_handler, 1000UL))
  {
    AudioChainProcess_Thread_handler = NULL;
  }
  else
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "st_os_hl_task_terminate error");
  }

  /* terminate low-level process thread if any low-level processing required */
  if (((uint32_t)AudioChain_getPrioLevel(&AudioChainInstance) & (uint32_t)AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW) != 0UL)
  {
    AudioChain_task_terminate_process_lowlevel();
  }
}


/**
* @brief  sends AudioChain processing task (low prio)  Pushed message
* @param  None
* @retval None
*/
void AudioChain_task_trigger_process(void)
{
  st_os_hl_task_trigger(AudioChainProcess_Thread_handler, msg_process);
}


/**
* @brief  AudioChain processing task (low prio)
* @param  argument argument
* @retval None
*/
static void s_audioChainProcess_Thread(ARGUMENT_TYPE argument)
{
  st_os_hl_task_thread(AudioChainProcess_Thread_handler, argument);
}


/* ---------------------------------------------------------------------------*/
/* Process Low API -----------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

/**
* @brief  creates task for AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_create_process_lowlevel(bool const logTaskQueueLevels)
{
  if (AudioChainProcessLowLevel_Thread_handler == NULL)
  {
    st_os_hl_task_create("AC_ProcLL_Thread",                            // thread_name
                         "AC_ProcLL_WakeMsg",                           // queue_name
                         AUDIO_CHAIN_PROCESS_LOW_LEVEL_TASK_STACK_SIZE, // stack_size
                         AUDIO_CHAIN_TASK_PROCESS_LOW_LEVEL_PRIO,       // priority
                         AUDIO_CHAIN_TASKS_LL_MESSAGE_QUEUE_DEEPNESS,   // queue_deepness
                         osWaitForever,                                 // queue_timeout
                         s_audioChainProcessLowLevel_Thread,            // thread_func
                         s_audioChain_processLowLevel,                  // task_func
                         NULL,                                          // cmsisos void *argument
                         false,                                         // not a back-ground task
                         true,                                          // activate back-ground task
                         logTaskQueueLevels,                            // log CMSIS OS stack/queue level
                         &AudioChainProcessLowLevel_Thread_handler);    // returned handler
    if (AudioChainProcessLowLevel_Thread_handler == NULL)
    {
      AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "st_os_hl_task_create AC_ProcLL_Thread error");
    }
  }
}


/**
* @brief  terminates task for AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_terminate_process_lowlevel(void)
{
  if (st_os_hl_task_terminate(&AudioChainProcessLowLevel_Thread_handler, 1000UL))
  {
    AudioChainProcessLowLevel_Thread_handler = NULL;
  }
  else
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "st_os_hl_task_terminate error");
  }
}


/**
* @brief  sends AudioChain processing task (low prio) Pushed message
* @param  None
* @retval None
*/
void AudioChain_task_trigger_process_lowlevel(void)
{
  st_os_hl_task_trigger(AudioChainProcessLowLevel_Thread_handler, msg_process);
}


/**
* @brief  AudioChain processing task (low prio)
* @param  argument argument
* @retval None
*/
static void s_audioChainProcessLowLevel_Thread(ARGUMENT_TYPE argument)
{
  st_os_hl_task_thread(AudioChainProcessLowLevel_Thread_handler, argument);
}


/* ---------------------------------------------------------------------------*/
/* Control API ---------------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

/**
* @brief  creates task for AudioChain control (high prio)
* @param  None
* @retval None
*/
void AudioChain_task_create_control(bool const logTaskQueueLevels)
{
  if (AudioChainControl_Thread_handler == NULL)
  {
    st_os_hl_task_create("AC_Control_Thread",                       // thread_name
                         "AC_Control_WakeMsg",                      // queue_name
                         AUDIO_CHAIN_CONTROL_TASK_STACK_SIZE,       // stack_size
                         AUDIO_CHAIN_TASK_CONTROL_PRIO,             // priority
                         AUDIO_CHAIN_TASKS_MESSAGE_QUEUE_DEEPNESS,  // queue_deepness
                         osWaitForever,                             // queue_timeout
                         s_audioChainControl_Thread,                // thread_func
                         s_audioChain_control,                      // task_func
                         NULL,                                      // cmsisos void *argument
                         false,                                     // not a back-ground task
                         true,                                      // activate back-ground task
                         logTaskQueueLevels,                        // log CMSIS OS stack/queue level
                         &AudioChainControl_Thread_handler);        // returned handler
    if (AudioChainControl_Thread_handler == NULL)
    {
      AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "st_os_hl_task_create AC_Control_Thread error");
    }
  }
}


/**
* @brief  terminates task for AudioChain control (high prio)
* @param  None
* @retval None
*/
void AudioChain_task_terminate_control(void)
{
  if (st_os_hl_task_terminate(&AudioChainControl_Thread_handler, 1000UL))
  {
    AudioChainControl_Thread_handler = NULL;
  }
  else
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "st_os_hl_task_terminate error");
  }
}


/**
* @brief  sends AudioChain control (high prio) Pushed message
* @param  None
* @retval None
*/
void AudioChain_task_trigger_control(void)
{
  st_os_hl_task_trigger(AudioChainControl_Thread_handler, msg_process);
}


/**
* @brief  AudioChain control (high prio)
* @param  argument argument
* @retval None
*/
static void s_audioChainControl_Thread(ARGUMENT_TYPE argument)
{
  st_os_hl_task_thread(AudioChainControl_Thread_handler, argument);
}


/* Private Functions Definition ------------------------------------------------------*/

static void s_audioChain_dataInOut(uint16_t const param, ARGUMENT_TYPE argument)
{
  int32_t error = AudioChain_dataInOut(&AudioChainInstance);

  if (AudioError_isError(error))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_dataInOut error");
  }
}


static void s_audioChain_process(uint16_t const param, ARGUMENT_TYPE argument)
{
  int32_t error;

  /* trigger low-level process task if any low-level processing required */
  if (((uint32_t)AudioChain_getPrioLevel(&AudioChainInstance) & (uint32_t)AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW) != 0UL)
  {
    AudioChain_task_trigger_process_lowlevel();
  }

  error = AudioChain_process(&AudioChainInstance, AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL);
  if (AudioError_isError(error))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_process error");
  }
}


static void s_audioChain_processLowLevel(uint16_t const param, ARGUMENT_TYPE argument)
{
  int32_t error = AudioChain_process(&AudioChainInstance, AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW);

  if (AudioError_isError(error))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_process low-level error");
  }
}


static void s_audioChain_control(uint16_t const param, ARGUMENT_TYPE argument)
{
  int32_t error = AudioChain_control(&AudioChainInstance);

  if (AudioError_isError(error))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_control error");
  }
}

#endif // #ifdef AUDIO_CHAIN_TASKS_OS_USED
