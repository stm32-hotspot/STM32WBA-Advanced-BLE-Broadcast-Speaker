/**
  ******************************************************************************
  * @file    st_os_conf.h
  * @author  MCD Application Team
  * @brief   Header for configuration of cycles.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ST_OS_CONF_H
#define __ST_OS_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "cmsis_compiler.h"
#include "stm32xxx_voice_bsp.h"
#include "irq_utils.h"
#include "st_base.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#if defined(USE_LIVETUNE_DESIGNER) || defined(USE_LIVETUNE_TUNER)

#define ST_Mem_Type_Os         ST_Mem_Type_ANY_FAST

#define ST_OS_ASSERT(arg)      ST_ASSERT(arg)
#define ST_OS_VERIFY(arg)      ST_VERIFY(arg)
#define ST_OS_TRACE_ERROR      ST_TRACE_ERROR

#endif /* USE_LIVETUNE_DESIGNER || USE_LIVETUNE_TUNER */

#ifndef ST_OS_USE_CMSIS_V1
#define ST_OS_USE_CMSIS_V2
#endif



#define st_infinit_delay osWaitForever

#define ST_OS_HL_TASKS_DEBUG
#define ST_OS_HL_TASK_BACKGROUND_STACK_SIZE 2048
#define ST_OS_HL_TASK_BACKGROUND_PRIO       (st_task_priority)ST_Priority_Low

#ifndef ST_OS_Result
#ifdef ST_OS_USE_CMSIS_V2
#define ST_OS_Result osStatus_t
#else
#define ST_OS_Result osStatus
#endif
#endif


#ifndef ST_OS_OK
#define ST_OS_OK osOK
#endif

#ifndef ST_OS_ERROR
#define ST_OS_ERROR osError
#endif

#ifndef ST_OS_BUSY
#define ST_OS_BUSY osErrorResource
#endif

#ifndef ST_OS_ERROR_TIMEOUT
#define ST_OS_ERROR_TIMEOUT osErrorTimeout
#endif

#define ST_OS_DISABLE_IRQ disable_irq_with_cnt
#define ST_OS_ENABLE_IRQ  enable_irq_with_cnt

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */



#ifdef __cplusplus
}
#endif

#endif /* __ST_OS_CONF_H */
