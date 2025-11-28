/**
******************************************************************************
* @file    stm32_audio_tasks_noos.c
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

#ifndef UTIL_AUDIO_RTOS_USED

/* The UTIL_AUDIO_MICPROCESS_PRIO is the priority of the task that triggers audio
processing , default is 8U because most of our examples have capture DMA IT = 7U*/
#ifndef UTIL_AUDIO_MICPROCESS_PRIO
  #define UTIL_AUDIO_MICPROCESS_PRIO             (8U)
#endif


void UTIL_AUDIO_MicProcessWrite_IRQHandler(void);
/**
* @brief  sends AudioIn Pushed message
* @param  None
* @retval None
*/
void UTIL_AUDIO_triggerTask(void)
{
  HAL_NVIC_SetPendingIRQ(UTIL_AUDIO_MicProcessWrite_IRQ);
}

/**
* @brief  AudioIn task
* @param  argument: pointer that is passed to the thread function as start argument.
* @retval None
*/
void UTIL_AUDIO_MicProcessWrite_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(UTIL_AUDIO_MicProcessWrite_IRQ);
  UTIL_AUDIO_process();
}

/**
* @brief  creates task for AudioIn capture
* @param  None
* @retval None
*/
void UTIL_AUDIO_createTask(bool const logCmsisOs)
{
  HAL_NVIC_SetPriority(UTIL_AUDIO_MicProcessWrite_IRQ, (int32_t)UTIL_AUDIO_MICPROCESS_PRIO, 0);
  HAL_NVIC_EnableIRQ(UTIL_AUDIO_MicProcessWrite_IRQ);
}

/**
* @brief  terminates task for AudioIn capture
* @param  None
* @retval None
*/
void UTIL_AUDIO_terminateTask(void)
{
  HAL_NVIC_SetPriority(UTIL_AUDIO_MicProcessWrite_IRQ, 0x0F, 0);
  HAL_NVIC_DisableIRQ(UTIL_AUDIO_MicProcessWrite_IRQ);
}
#endif /* !UTIL_AUDIO_RTOS_USED */
