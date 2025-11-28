/**
******************************************************************************
* @file    traces.c
* @author  MCD Application Team
* @brief   basic traces mechanism
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

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>
#include "traces_internals.h"
#include "log_module.h"
#include "app_conf.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static bool            G_asynchronous       = false;
static traceLvl_t      G_traceLevel         = TRACE_LVL_DEFAULT;


/* Private function prototypes -----------------------------------------------*/


/* Functions Definition ------------------------------------------------------*/

/**
* @brief  Empty weak coreLock routine to allow implementation of mutex if needed.
* @param  None
* @retval None
*/
__weak void trace_coreLock_tasks(void)
{
}


/**
* @brief  Empty weak coreLock routine to allow implementation of mutex if needed.
* @param  None
* @retval None
*/
__weak void trace_coreUnlock_tasks(void)
{
}


/**
* @brief  Set asynchronous vs synchronous mode.
* @param  new mode
* @retval previous mode
*/
bool trace_setAsynchronous(bool const asynchronousEnable)
{
  bool const prevAsynchronous = G_asynchronous;

  G_asynchronous = asynchronousEnable;

  if (prevAsynchronous)
  {
    if (!asynchronousEnable)
    {
      // if we were in asynchronous mode and if we are switching to synchronous mode; we output all buffers
      trace_flush(TRACE_OUTPUT_ALL);
    }
  }

  return prevAsynchronous;
}


/**
* @brief  Flush trace info buffer.
* @param  chosen output
* @retval None
*/
void trace_flush(traceOutput_t const output)
{

}


/**
* @brief  Add trace in buffer if asynchronous or print trace directly if synchronous.
* @param  trace output = display and/or uart
* @param  trace level
* @param  Format string and parameters
* @retval None
*/
void trace_print_args(traceOutput_t const output, traceLvl_t const level, const char *pFormat, va_list args)
{
#if CFG_LOG_SUPPORTED != 0
  if (level != TRACE_LVL_MUTE)
  {
    Log_Module_PrintWithArg(LOG_VERBOSE_INFO, LOG_REGION_APP, pFormat, args);
  }
#endif
}


/**
* @brief  similar to trace_print_args  Gives the opportunity to catch error coming from the system and react according to this, mainly used in the designer to stop the pipe in case of error
* @param  trace output = display and/or uart
* @param  trace level
* @param  Format string and parameters
* @retval None
*/
__weak bool trace_print_args_hook(traceOutput_t const output, traceLvl_t const level, const char *pFormat, va_list args)
{
  return false;
}



/**
* @brief  Add trace in buffer if asynchronous or print trace directly if synchronous.
* @param  trace output = display and/or uart
* @param  trace level
* @param  Format string and parameters
* @retval None
*/
__weak void trace_print(traceOutput_t const output, traceLvl_t const level, const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);
  if (trace_print_args_hook(output, level, pFormat, args) == false)
  {
    if (((uint8_t)level & (uint8_t)G_traceLevel) != 0U)
    {
      trace_print_args(output, level, pFormat, args);
    }
  }
  va_end(args);
}


/**
* @brief  Print an output string on the console + carriage return
*
* trace_log is used for TRACE and ASSERT, so we can pass the filename and the line number to localize an error
* @param level  Debug level
* @param pFile  File name to print
* @param line   Line number
* @param  ...   Format string and its arguments
* @return void
*/
void trace_log(const char *const pCompName, traceLvl_t const level, const char *pFile, int const line, const char *const pFormat, va_list args)
{
#if CFG_LOG_SUPPORTED != 0
  Log_Module_PrintWithArg(LOG_VERBOSE_INFO, LOG_REGION_APP, pFormat, args);
#endif
}


void trace_test(void)
{
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_VERBOSE,     "TRACE_LVL_VERBOSE\n");
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_DEBUG,       "TRACE_LVL_DEBUG\n");
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG,         "TRACE_LVL_LOG\n");
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_INFO,        "TRACE_LVL_INFO\n");
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_INFO2,       "TRACE_LVL_INFO2\n");
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_WARNING,     "TRACE_LVL_WARNING\n");
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_ERROR,       "TRACE_LVL_ERROR\n");
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_ERROR_FATAL, "TRACE_LVL_ERROR_FATAL\n"); // WARNING: this trace (fatal error) blocks the system in a while(1) loop
}


/**
* @brief  Set the trace level
* Allows to select the type of debug info
* @param level Bits combination (mask), see also TRACE_LVL_XXX
* @return The previous level
*/
traceLvl_t trace_setLevel(traceLvl_t const level)
{
  traceLvl_t prevLvl = G_traceLevel;
  G_traceLevel = level;
  return prevLvl;
}


/**
* @brief  Get the trace level
* @return The previous level
*/
traceLvl_t trace_getLevel(void)
{
  return G_traceLevel;
}



/**
* @brief  get highest level index (most important) from level mask
* @param  level mask
* @retval level index (-1 if level==0, i.e. no trace)
*/
int trace_getLevelIdx(traceLvl_t const level)
{
  uint8_t levelIdx;

  for (levelIdx = 0U; levelIdx <= (uint8_t)NB_TRACE_LVL; levelIdx++)
  {
    if (((uint8_t)level >> levelIdx) == 0U)
    {
      break;
    }
  }

  return (int)levelIdx - 1;
}

