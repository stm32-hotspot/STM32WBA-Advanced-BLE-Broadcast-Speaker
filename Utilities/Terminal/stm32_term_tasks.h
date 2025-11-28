/**
  ******************************************************************************
  * @file    stm32_term_tasks.h
  * @author  MCD Application Team
  * @brief   Header for stm32_term.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_TERM_TASKS_H
#define __STM32_TERM_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_term.h"

/* Exported functions ------------------------------------------------------- */
int32_t  UTIL_TERM_TASK_create(void);
void     UTIL_TERM_TASK_terminate(void);
int32_t  UTIL_TERM_TASK_wait_data(uint8_t                *const pData, size_t const size, uint32_t const timeout, size_t *const pRemainingBytes);
int32_t  UTIL_TERM_TASK_send_data_from_isr(uint8_t const *const pData, size_t const size);
int32_t  UTIL_TERM_TASK_send_data(uint8_t          const *const pData, size_t const size);

/* cycles count functions */
void UTIL_TERM_cycleMeasure_Init(void);
void UTIL_TERM_cycleMeasure_Reset(void);
void UTIL_TERM_cycleMeasure_Start(void);
void UTIL_TERM_cycleMeasure_Stop(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_TERM_TASKS_H */

