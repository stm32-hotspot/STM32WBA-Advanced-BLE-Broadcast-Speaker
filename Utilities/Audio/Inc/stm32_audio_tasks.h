/**
  ******************************************************************************
  * @file    stm32_audio_tasks.h
  * @author  MCD Application Team
  * @brief   Header for stm32_audio_tasks.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_AUDIO_TASKS_H
#define __STM32_AUDIO_TASKS_H


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_audio.h"

/* Exported functions ------------------------------------------------------- */
void UTIL_AUDIO_createTask(bool const logTaskQueueLevels);
void UTIL_AUDIO_triggerTask(void);
void UTIL_AUDIO_terminateTask(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_AUDIO_TASKS_H */

