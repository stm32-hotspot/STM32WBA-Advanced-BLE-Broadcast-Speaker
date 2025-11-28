/**
  ******************************************************************************
  * @file    traces_internals.h
  * @author  MCD Application Team
  * @brief   Header for traces.c module
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
#ifndef __TRACES_INTERNALS_H
#define __TRACES_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "traces.h"
#include "traces_conf.h"

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#ifdef TRACE_ASSERT_DEFINED
#include <assert.h>
#endif

/* Private macros -----------------------------------------------------------*/
#ifdef TRACE_ASSERT_DEFINED
#define TRACE_ASSERT(cond) assert(cond)                                   // cond is executed and assert is issued if cond is false
#else
// 2 options: only one definition must be enabled
//#define TRACE_ASSERT(cond)                                              // cond is not executed
#define TRACE_ASSERT(cond) do { int a = (int)(cond); (void)a; } while(0)  // cond is executed but no assert is issued
#endif

#if !defined(TRACE_USE_UART) && !defined(TRACE_USE_DISPLAY)
//#error "Error: trace.h; no output is defined " // it is not an error, remove trace especially for the release mode
#endif

#ifndef TRACE_MAX_LOG_LINE_SIZE
#define TRACE_MAX_LOG_LINE_SIZE     150U
#endif

#ifndef DISPLAY_LINE_WRAPPING_SIZE
#define DISPLAY_LINE_WRAPPING_SIZE  100UL
#endif

#ifndef TRACE_MAX_SCROLL_LINE
#define TRACE_MAX_SCROLL_LINE       10UL
#endif

#ifndef TRACE_LVL_DEFAULT
#define TRACE_LVL_DEFAULT           (TRACE_LVL_ERROR_FATAL | TRACE_LVL_ERROR | TRACE_LVL_WARNING | TRACE_LVL_INFO) /*!< Default traces configuration */
#endif


/* Private types ------------------------------------------------------------*/
typedef struct traceLogInfo
{
  char                 buffer[TRACE_MAX_LOG_LINE_SIZE + 1U];  // +1 for end of string
  traceLvl_t           level;
  traceOutput_t        output;
  struct traceLogInfo *pPrev;
  struct traceLogInfo *pNext;
} traceLogInfo_t;

/* Private constants --------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */

#ifdef TRACE_USE_UART
void trace2uart(char *const pString);
#endif

#ifdef TRACE_USE_DISPLAY
void trace2display(traceLogInfo_t const *const pTraceLogInfo);
#endif

int trace_getLevelIdx(traceLvl_t const level);

#ifdef __cplusplus
}
#endif

#endif /* __TRACES_INTERNALS_H */


