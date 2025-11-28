/**
******************************************************************************
* @file    audio_chain_tasks_no_os.c
* @author  MCD Application Team
* @brief   audio chain os abstraction layer for bare metal implementation.
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
#ifndef AUDIO_CHAIN_TASKS_OS_USED
/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/


#ifndef AUDIO_CHAIN_TASK_DATAINOUT_PRIO
  #define AUDIO_CHAIN_TASK_DATAINOUT_PRIO 8UL
#endif

#ifndef AUDIO_CHAIN_TASK_PROCESS_PRIO
  #define AUDIO_CHAIN_TASK_PROCESS_PRIO 10UL
#endif

#ifndef AUDIO_CHAIN_TASK_PROCESS_LOWLEVEL_PRIO
  #define AUDIO_CHAIN_TASK_PROCESS_LOWLEVEL_PRIO 11UL
#endif

#ifndef AUDIO_CHAIN_TASK_CONTROL_PRIO
  #define AUDIO_CHAIN_TASK_CONTROL_PRIO 9UL
#endif

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

void AudioChain_control_IRQHandler(void);
void AudioChain_process_IRQHandler(void);
void AudioChain_process_lowlevel_IRQHandler(void);
void AudioChain_dataInOut_IRQHandler(void);


/* Functions Definition ------------------------------------------------------*/

/**
* @brief  sends AudioChain dataInOut (high prio) Pushed message
* @param  None
* @retval None
*/
void AudioChain_task_trigger_dataInOut(void)
{
  NVIC_SetPendingIRQ(AudioChain_dataInOut_IRQ);
}

/**
* @brief  AudioChain dataInOut (high prio)
* @param  None
* @retval None
*/
void AudioChain_dataInOut_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(AudioChain_dataInOut_IRQ);
  if (AudioError_isError(AudioChain_dataInOut(&AudioChainInstance)))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_dataInOut_IRQHandler error");
  }
}

/**
* @brief  creates task for AudioChain dataInOut (high prio)
* @param  None
* @retval None
*/
void AudioChain_task_create_dataInOut(bool const logCmsisOs)
{
  HAL_NVIC_SetPriority(AudioChain_dataInOut_IRQ, AUDIO_CHAIN_TASK_DATAINOUT_PRIO, 0);
  HAL_NVIC_EnableIRQ(AudioChain_dataInOut_IRQ);
}

/**
* @brief  terminates task for AudioChain dataInOut (high prio)
* @param  None
* @retval None
*/
void AudioChain_task_terminate_dataInOut(void)
{
  HAL_NVIC_SetPriority(AudioChain_dataInOut_IRQ, 0x0F, 0);
  HAL_NVIC_DisableIRQ(AudioChain_dataInOut_IRQ);
}

/**
* @brief  sends AudioChain processing task (low prio)  Pushed message
* @param  None
* @retval None
*/
void AudioChain_task_trigger_process(void)
{
  if (((uint32_t)AudioChain_getPrioLevel(&AudioChainInstance) & (uint32_t)AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW) != 0UL)
  {
    AudioChain_task_trigger_process_lowlevel();
  }
  NVIC_SetPendingIRQ(AudioChain_process_IRQ);
}

/**
* @brief  AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_process_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(AudioChain_process_IRQ);

  if (AudioError_isError(AudioChain_process(&AudioChainInstance, AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL)))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_process error");
  }
}

/**
* @brief  creates task for AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_create_process(bool const logCmsisOs)
{
  if (((uint32_t)AudioChain_getPrioLevel(&AudioChainInstance) & (uint32_t)AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW) != 0UL)
  {
    AudioChain_task_create_process_lowlevel(logCmsisOs);
  }
  HAL_NVIC_SetPriority(AudioChain_process_IRQ, AUDIO_CHAIN_TASK_PROCESS_PRIO, 0);
  HAL_NVIC_EnableIRQ(AudioChain_process_IRQ);
}

/**
* @brief  terminates task for AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_terminate_process(void)
{
  HAL_NVIC_SetPriority(AudioChain_process_IRQ, 0x0F, 0);
  HAL_NVIC_DisableIRQ(AudioChain_process_IRQ);
}


/**
* @brief  sends AudioChain processing task (low prio)  Pushed message
* @param  None
* @retval None
*/
void AudioChain_task_trigger_control(void)
{
  NVIC_SetPendingIRQ(AudioChain_control_IRQ);
}

/**
* @brief  AudioChain controlling task (low prio)
* @param  None
* @retval None
*/

void AudioChain_control_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(AudioChain_control_IRQ);
  if (AudioError_isError(AudioChain_control(&AudioChainInstance)))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_control error");
  }
}

/**
* @brief  creates task for AudioChain controlling task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_create_control(bool const logCmsisOs)
{
  HAL_NVIC_SetPriority(AudioChain_control_IRQ, AUDIO_CHAIN_TASK_CONTROL_PRIO, 0);
  HAL_NVIC_EnableIRQ(AudioChain_control_IRQ);
}

/**
* @brief  terminates task for AudioChain controlling task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_terminate_control(void)
{
  HAL_NVIC_SetPriority(AudioChain_control_IRQ, 0x0F, 0);
  HAL_NVIC_DisableIRQ(AudioChain_control_IRQ);
}

#ifdef AudioChain_process_lowlevel_IRQHandler
/**
* @brief  sends AudioChain processing task (low prio)  Pushed message
* @param  None
* @retval None
*/
void AudioChain_task_trigger_process_lowlevel(void)
{
  NVIC_SetPendingIRQ(AudioChain_process_lowlevel_IRQ);
}

/**
* @brief  AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_process_lowlevel_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(AudioChain_process_lowlevel_IRQ);

  if (AudioError_isError(AudioChain_process(&AudioChainInstance, AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW)))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_process low-level error");
  }
}

/**
* @brief  creates task for AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_create_process_lowlevel(bool const logCmsisOs)
{
  HAL_NVIC_SetPriority(AudioChain_process_lowlevel_IRQ, AUDIO_CHAIN_TASK_PROCESS_LOWLEVEL_PRIO, 0);
  HAL_NVIC_EnableIRQ(AudioChain_process_lowlevel_IRQ);
}

/**
* @brief  terminates task for AudioChain processing task (low prio)
* @param  None
* @retval None
*/
void AudioChain_task_terminate_process_lowlevel(void)
{
  HAL_NVIC_SetPriority(AudioChain_process_lowlevel_IRQ, 0x0F, 0);
  HAL_NVIC_DisableIRQ(AudioChain_process_lowlevel_IRQ);
}
#else
#error "process is now split into two priority level, please complete your audio_chain_tasks_conf.h with AudioChain_process_lowlevel_IRQHandler definition"
#endif

#endif  /*AUDIO_CHAIN_TASKS_OS_USED*/

