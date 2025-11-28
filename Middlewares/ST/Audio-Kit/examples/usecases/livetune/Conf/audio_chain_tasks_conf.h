/**
  ******************************************************************************
  * @file    audio_chain_tasks_conf.h
  * @author  MCD Application Team
  * @brief   Header for audio_chain_tasks_*.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2021) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __AUDIO_CHAIN_TASKS_CONF_H_
#define __AUDIO_CHAIN_TASKS_CONF_H_


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32xxx_voice_bsp.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define AUDIO_CHAIN_DATAINOUT_TASK_STACK_SIZE         2560UL
#define AUDIO_CHAIN_PROCESS_LOW_LEVEL_TASK_STACK_SIZE 2048UL
#define AUDIO_CHAIN_CONTROL_TASK_STACK_SIZE           2048UL

#ifdef USE_THREADS

#define AUDIO_CHAIN_TASKS_OS_USED
#define AUDIO_CHAIN_TASKS_LL_MESSAGE_QUEUE_DEEPNESS 20UL
#define AUDIO_CHAIN_TASK_DATAINOUT_PRIO             (st_task_priority)ST_Priority_Above_Normal
#define AUDIO_CHAIN_TASK_PROCESS_PRIO               (st_task_priority)ST_Priority_Normal1
#define AUDIO_CHAIN_TASK_PROCESS_LOW_LEVEL_PRIO     (st_task_priority)ST_Priority_Below_Normal1
#define AUDIO_CHAIN_TASK_CONTROL_PRIO               (st_task_priority)ST_Priority_Normal

#else // USE_THREADS

#define AudioChain_dataInOut_IRQHandler             I2C2_EV_IRQHandler
#define AudioChain_dataInOut_IRQ                    I2C2_EV_IRQn

#define AudioChain_process_IRQHandler               I2C2_ER_IRQHandler
#define AudioChain_process_IRQ                      I2C2_ER_IRQn

#define AudioChain_process_lowlevel_IRQHandler      I2C3_EV_IRQHandler
#define AudioChain_process_lowlevel_IRQ             I2C3_EV_IRQn

#define AudioChain_control_IRQHandler               I2C3_ER_IRQHandler
#define AudioChain_control_IRQ                      I2C3_ER_IRQn


#define AUDIO_CHAIN_TASK_DATAINOUT_PRIO 14UL

#define AUDIO_CHAIN_TASK_PROCESS_PRIO 14UL

#define AUDIO_CHAIN_TASK_PROCESS_LOWLEVEL_PRIO 14UL

#define AUDIO_CHAIN_TASK_CONTROL_PRIO 14UL

#endif // USE_THREADS

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif // __AUDIO_CHAIN_TASKS_CONF_H_
