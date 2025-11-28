/**
******************************************************************************
* @file    audio_chain_tasks.h
* @author  MCD Application Team
* @brief   Header for audio_chain_tasks_xxx.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __AUDIO_CHAIN_TASKS_H
#define __AUDIO_CHAIN_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain_tasks_conf.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void AudioChain_task_create_dataInOut(bool const logTaskQueueLevels);
void AudioChain_task_trigger_dataInOut(void);
void AudioChain_task_terminate_dataInOut(void);

void AudioChain_task_create_process(bool const logTaskQueueLevels);
void AudioChain_task_trigger_process(void);
void AudioChain_task_terminate_process(void);

void AudioChain_task_create_process_lowlevel(bool const logTaskQueueLevels);
void AudioChain_task_trigger_process_lowlevel(void);
void AudioChain_task_terminate_process_lowlevel(void);

void AudioChain_task_create_control(bool const logTaskQueueLevels);
void AudioChain_task_trigger_control(void);
void AudioChain_task_terminate_control(void);

#ifdef __cplusplus
}
#endif

#endif /*__AUDIO_CHAIN_TASKS_H*/



