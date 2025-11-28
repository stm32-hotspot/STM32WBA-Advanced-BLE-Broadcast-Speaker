/**
  ******************************************************************************
  * @file    traces.h
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
#ifndef __TRACES_H
#define __TRACES_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "traces_typedef.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

bool trace_setAsynchronous(bool const asynchronousEnable);  /* in synchronous mode (asynchronousEnable=0), trace is output immediately (at startup, i.e. before entering in idle loop) */
/* in asynchronous mode (asynchronousEnable=1), trace is pushed in a buffer immediately and will be output later in idle loop */
/* returns previous synchronous state */

traceLvl_t trace_setLevel(traceLvl_t const level); /* set trace level */
traceLvl_t trace_getLevel(void); /* get trace level */


/* just for test: to see available character styles and colors and chosen trace log colors depending on trace level
   WARNING: this trace blocks the system in a while(1) loop after the last trace */
void trace_test(void);


/* used for assert of debug trace (with file name and line number to help debug), '\n' is automatically added at the end */
void trace_log(const char *const pCompName, traceLvl_t const level, const char *pFile, int const line, const char *const pFormat, va_list args);

/* used to display anything like a printf (without file name nor line number), nothing is added at the end, it is the user resposability to manage end of line */
void trace_print(traceOutput_t const output, traceLvl_t const level, const char *pFormat, ...);
void trace_print_args(traceOutput_t const output, traceLvl_t const level, const char *pFormat, va_list args);
bool trace_print_args_hook(traceOutput_t const output, traceLvl_t const level, const char *pFormat, va_list args);

/* outputs trace from local buffer; if synchronous, flush all trace buffer; else output only 1 trace buffer */
void trace_outputBuffers(void);

/* outputs all trace buffers for specified output */
void trace_flush(traceOutput_t const output);

void trace_coreLock_tasks(void);
void trace_coreUnlock_tasks(void);

#ifdef __cplusplus
}
#endif

#endif /* __TRACES_H */

