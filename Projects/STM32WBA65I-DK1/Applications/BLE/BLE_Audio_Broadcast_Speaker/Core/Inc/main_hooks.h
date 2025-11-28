/**
  ******************************************************************************
  * @file    main_hooks.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_HOOKS_H
#define __MAIN_HOOKS_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/

#include "main_hooks_conf.h"
#include "audio_buffer.h"


/* Exported types ------------------------------------------------------------*/
typedef void(*IDLE_PROCESS_TASK)(void);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


/* list of hooks for common services init*/
void main_hooks_pre_init(void);
void main_hooks_post_init(void);

void main_hooks_rtos_init(void);    /* OS init */
void main_hooks_task_init(void);    /* To allow some general task creation*/
void main_hooks_coreInit(void);     /* Common clock, MPU Cache settings */
void main_hooks_boardPreInit(void); /* HW that needs to be ready earlier (led, button, etc.. */
void main_hooks_boardInit(void);    /* HW used in all audio demos */

void main_hooks_error_handler(void);
void main_hooks_print(void);

void main_hooks_run(void);
void main_hooks_idle(void);
void main_hooks_user_idle(void);

void main_hooks_systemReset(void);
void main_hooks_footprintDump(void);
void main_hooks_cacheMonitor(void);


/* routines for traces mgnt */
void main_hooks_logTrace(void);
void main_hooks_activateTrace(void);

/* routines for an audio stream */
void main_hooks_audioInit(void);
void main_hooks_audioDeInit(void);
void main_hooks_audioIdle(void);
void main_hooks_audioFeed(audio_buffer_t const *const pBuff1,
                          audio_buffer_t const *const pBuff2);
void main_hooks_usbRecFeed(audio_buffer_t const *const pBuff1,
                           audio_buffer_t const *const pBuff2);

/* cycles count functions */
void main_hooks_cyclesMeasureInit(void);
void main_hooks_cyclesMeasureReset(void);
void main_hooks_cyclesMeasureStart(void);
void main_hooks_cyclesMeasureStop(void);


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_HOOKS_H */
