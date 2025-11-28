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


/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
//#define TRACE_NO_VSNPRINTF_UNDER_INTERRUPT
//#define CHECK_TRACE_CONSISTENCY
//#define TRACE_DEPTH_DEBUG
#define NB_TRACE_LOG_BUFFERS 50

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef TRACE_USE_UART
static const char *trace_levelString[] =
{
  [TRACE_LVL_DEBUG_BIT]       = "Debug      ",
  [TRACE_LVL_LOG_BIT]         = "Log        ",
  [TRACE_LVL_INFO_BIT]        = "Info       ",
  [TRACE_LVL_INFO2_BIT]       = "Info2      ",
  [TRACE_LVL_WARNING_BIT]     = "Warning    ",
  [TRACE_LVL_ERROR_BIT]       = "Error      ",
  [TRACE_LVL_ERROR_FATAL_BIT] = "Fatal Error",
  [NB_TRACE_LVL]              = NULL
};
#endif

static traceLogInfo_t  G_traceLogBuffers[NB_TRACE_LOG_BUFFERS];
static traceLogInfo_t *G_pFreeTraceBuffers  = NULL;
static traceLogInfo_t *G_pFirstTraceBuffer  = NULL;
static traceLogInfo_t *G_pLastTraceBuffer   = NULL;
#ifdef TRACE_DEPTH_DEBUG
  static uint8_t       G_debugTraceDepth    = 0U;
  static uint8_t       G_debugMaxTraceDepth = 0U;
#endif
static bool            G_asynchronous       = false;
static traceLvl_t      G_traceLevel         = TRACE_LVL_DEFAULT;


/* Private function prototypes -----------------------------------------------*/
static void s_trace_outputBuffersOnDisplay(bool const onlyOneBuffer);
static void s_trace_outputBuffersOnUart(bool const onlyOneBuffer);
#if defined(TRACE_USE_DISPLAY) || defined(TRACE_USE_UART)
static void s_updateTraceBufferList(void);
#endif

#ifdef CHECK_TRACE_CONSISTENCY
  static void s_checkTraceBufferListsConsistency(void);
  static void s_traceError(int const line);
#endif

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
  if (((uint8_t)output & (uint8_t)TRACE_OUTPUT_DISPLAY) != 0U)
  {
    // flush all buffers (onlyOneBuffer=0)
    s_trace_outputBuffersOnDisplay(false);
  }
  if (((uint8_t)output & (uint8_t)TRACE_OUTPUT_UART) != 0U)
  {
    // flush all buffers (onlyOneBuffer=0)
    s_trace_outputBuffersOnUart(false);
  }
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
  if (level != TRACE_LVL_MUTE)
  {
    static uint8_t firstTime      = true;
    uint8_t        modifiedOutput = (uint8_t)output;

    #ifndef TRACE_USE_UART
    modifiedOutput &= ~(uint8_t)TRACE_OUTPUT_UART;
    #endif
    #ifndef TRACE_USE_DISPLAY
    modifiedOutput &= ~(uint8_t)TRACE_OUTPUT_DISPLAY;
    #endif

    if (!G_asynchronous || (level >= TRACE_LVL_ERROR))
    {
      trace_flush((traceOutput_t)modifiedOutput);
    }

    if (modifiedOutput != 0U)
    {
      char           *pMalloc      = NULL;
      const char     *pTmpBuff     = NULL;
      traceLogInfo_t *pTraceBuffer = NULL;
      size_t          size;

      if (strchr(pFormat, (int)'%') == NULL)
      {
        // there is no '%' in format thus format is simply a string
        size     = strlen(pFormat);
        pTmpBuff = pFormat;
      }
      else
      {
        // format line to be printed
        #ifdef TRACE_NO_VSNPRINTF_UNDER_INTERRUPT
        if (__get_IPSR() != 0)
        {
          // we can't use vsnprintf in an interrupt => format won't be applied, simply display format string
          size     = strlen(pFormat);
          pTmpBuff = pFormat;
        }
        else
        #endif
        {
          // when there is a format, we assume 2 * TRACE_MAX_LOG_LINE_SIZE is enough
          pMalloc = malloc((2U * TRACE_MAX_LOG_LINE_SIZE) + 1U);
          if (pMalloc != NULL)
          {
            size     = (size_t)vsnprintf(pMalloc, (2U * TRACE_MAX_LOG_LINE_SIZE) + 1U, pFormat, args);
            pTmpBuff = (const char *)pMalloc;
          }
          else
          {
            // no enough memory => format won't be applied, simply display format string
            size     = strlen(pFormat);
            pTmpBuff = pFormat;
          }
        }
      }

      while (size > 0UL)
      {
        size_t tmpSize = (size > TRACE_MAX_LOG_LINE_SIZE) ? TRACE_MAX_LOG_LINE_SIZE : size;

        // disable IRQ to insert pTraceBuffer in the chained list of buffers to be logged and to remove it from the list of free buffers
        TRACE_DISABLE_IRQ();

        if (firstTime)
        {
          // create list of free buffers
          G_pFirstTraceBuffer        = NULL;
          G_pLastTraceBuffer         = NULL;
          G_pFreeTraceBuffers        = &G_traceLogBuffers[0];
          G_traceLogBuffers[0].pPrev = NULL;
          for (int i = 1; i < NB_TRACE_LOG_BUFFERS; i++)
          {
            G_traceLogBuffers[i - 1].pNext = &G_traceLogBuffers[i];
            G_traceLogBuffers[i].pPrev     = &G_traceLogBuffers[i - 1];
          }
          G_traceLogBuffers[NB_TRACE_LOG_BUFFERS - 1].pNext = NULL;
          firstTime = false;
        }

        // if list of free buffers is not empty, take 1 buffer in this list, remove it from this list and insert it in the list of log buffers to trace
        if (G_pFreeTraceBuffers != NULL)
        {
          // take 1 buffer in the list of free buffers
          pTraceBuffer        = G_pFreeTraceBuffers;
          G_pFreeTraceBuffers = G_pFreeTraceBuffers->pNext;
          if (G_pFreeTraceBuffers != NULL)
          {
            G_pFreeTraceBuffers->pPrev = NULL;
          }
          else
          {
            // if max trace depth is reached (i.e. if there is no more free trace buffer), replace log with a warning message saying to increase trace depth
            static const char string[] = "*** trace depth too small => increase NB_TRACE_LOG_BUFFERS ***\n";

            pTmpBuff = string;
            size     = strlen(string);
            size     = (size > TRACE_MAX_LOG_LINE_SIZE) ? TRACE_MAX_LOG_LINE_SIZE : size;
            tmpSize  = size;
          }

          // insert pTraceBuffer into the chained list of trace buffers to be logged
          if (G_pFirstTraceBuffer == NULL)
          {
            pTraceBuffer->pPrev       = NULL;
            G_pFirstTraceBuffer       = pTraceBuffer;
          }
          else
          {
            pTraceBuffer->pPrev       = G_pLastTraceBuffer;
            G_pLastTraceBuffer->pNext = pTraceBuffer;
          }
          pTraceBuffer->pNext         = NULL;
          G_pLastTraceBuffer          = pTraceBuffer;

          #ifdef TRACE_DEPTH_DEBUG
          // trace depth debug
          G_debugTraceDepth++;
          G_debugMaxTraceDepth = (G_debugTraceDepth > G_debugMaxTraceDepth) ? G_debugTraceDepth : G_debugMaxTraceDepth;
          #endif
        }

        #ifdef CHECK_TRACE_CONSISTENCY
        s_checkTraceBufferListsConsistency();
        #endif

        // re-enable IRQ
        TRACE_ENABLE_IRQ();

        if (pTraceBuffer != NULL)
        {
          // fill this trace buffer with data to log
          memcpy(pTraceBuffer->buffer, pTmpBuff, tmpSize);
          pTraceBuffer->buffer[tmpSize] = '\0';  // end of string
          pTraceBuffer->level           = level;
          pTraceBuffer->output          = (traceOutput_t)modifiedOutput;
          size                         -= tmpSize;
          pTmpBuff                     += tmpSize;

          // if log must be traced immediately, do it now
          if (!G_asynchronous)
          {
            trace_outputBuffers();
          }
          if (level >= TRACE_LVL_ERROR)
          {
            trace_flush((traceOutput_t)modifiedOutput);
            if (level >= TRACE_LVL_ERROR_FATAL)
            {
              //TRACE_DISABLE_IRQ();
              while (1);
            }
          }
        }

        if ((G_pFreeTraceBuffers == NULL) && (size > 0UL))
        {
          // trace buffer full: can't trace remaining data
          break;
        }
      }

      if (pMalloc != NULL)
      {
        free(pMalloc);  /*cstat !MISRAC2012-Dir-4.13_g pMalloc is freed only if it was allocated (else it would be NULL: see above)*/
      }
    }
  }
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
* @brief  outputs trace from local buffer; if synchronous, flush all trace buffer; else output only 1 trace buffer
* @param  None
* @retval None
*/
void trace_outputBuffers(void)
{
  // in asynchronous mode, log only 1 trace, if there is more than 1 trace in the list, they will be logged later (in next trace_outputBuffers calls)
  // in synchronous mode, log all buffered traces
  s_trace_outputBuffersOnDisplay(G_asynchronous);
  s_trace_outputBuffersOnUart(G_asynchronous);
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
#define LOG_LINE_MAX_SIZE (2U * TRACE_MAX_LOG_LINE_SIZE)
void trace_log(const char *const pCompName, traceLvl_t const level, const char *pFile, int const line, const char *const pFormat, va_list args)
{
  #ifdef TRACE_USE_UART
  if (((uint8_t)level & (uint8_t)G_traceLevel) != 0U)
  {
    char        logLine[LOG_LINE_MAX_SIZE];
    int const   levelIdx = trace_getLevelIdx(level); /* can't be <0 because level!=0 */
    const char *pPrefix  = trace_levelString[levelIdx];
    uint32_t    nChars   = 0UL;

    if (pFile != NULL)
    {
      /* Remove the full path */
      const char *p;

      for (p = &pFile[strlen(pFile)]; p > pFile; p--)
      {
        if ((*p == '/') || (*p == '\\'))
        {
          p++;
          break;
        }
      }

      nChars = (uint32_t)snprintf(logLine, LOG_LINE_MAX_SIZE, "%s:%d: ", p, line);
    }

    if (nChars < (LOG_LINE_MAX_SIZE - 1U))
    {
      if ((pPrefix != NULL) && (pCompName != NULL))
      {
        nChars += (uint32_t)snprintf(&logLine[nChars], LOG_LINE_MAX_SIZE - nChars, "%s %s: ", pCompName, pPrefix);
      }
      else if (pPrefix != NULL)
      {
        nChars += (uint32_t)snprintf(&logLine[nChars], LOG_LINE_MAX_SIZE - nChars, "%s ", pPrefix);
      }
      else if (pCompName != NULL)
      {
        nChars += (uint32_t)snprintf(&logLine[nChars], LOG_LINE_MAX_SIZE - nChars, "%s: ", pCompName);
      }
      else
      {
        // do nothing: MISRA
      }
    }

    if (nChars < (LOG_LINE_MAX_SIZE - 1U))
    {
      nChars += (uint32_t)vsnprintf(&logLine[nChars], LOG_LINE_MAX_SIZE - nChars, (const char *)pFormat, args);
    }

    if (nChars < (LOG_LINE_MAX_SIZE - 2U))
    {
      strncpy(&logLine[nChars], "\n", LOG_LINE_MAX_SIZE - nChars);
      nChars++;
    }

    if (nChars < (LOG_LINE_MAX_SIZE - 1U))
    {
      // replace any % with %% because trace_print will be called with logLine as pFormat and thus will consider % character as format character if it is not doubled
      for (uint32_t i = 0UL; i < nChars; i++)
      {
        if (logLine[i] == '%')
        {
          nChars++;
          if (nChars < (LOG_LINE_MAX_SIZE - 1U))
          {
            memmove(&logLine[i + 1UL], &logLine[i], nChars - i);
            i++;
          }
          else
          {
            break;
          }
        }
      }
    }

    if (nChars >= (LOG_LINE_MAX_SIZE - 1U))
    {
      strncpy(logLine, "Error with Trace: increase LOG_LINE_MAX_SIZE\n", LOG_LINE_MAX_SIZE);
    }

    trace_print(TRACE_OUTPUT_UART, level, logLine);
  }
  #endif
}


void trace_test(void)
{
#ifdef TRACE_USE_UART
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_WHITE TRACE_UART_BOLD TRACE_UART_UNDERLINED "\nAvailable character styles and colors:\n" TRACE_UART_RESET);
  trace2uart(TRACE_UART_RESET                    "TRACE_UART_RESET\n"                    TRACE_UART_RESET);
  trace2uart(TRACE_UART_BOLD                     "TRACE_UART_BOLD\n"                     TRACE_UART_RESET);
  trace2uart(TRACE_UART_UNDERLINED               "TRACE_UART_UNDERLINED\n"               TRACE_UART_RESET);
  trace2uart(TRACE_UART_REVERSED                 "TRACE_UART_REVERSED\n"                 TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BLACK          "TRACE_UART_CHARACTER_BLACK\n"          TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_RED            "TRACE_UART_CHARACTER_RED\n"            TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_GREEN          "TRACE_UART_CHARACTER_GREEN\n"          TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_YELLOW         "TRACE_UART_CHARACTER_YELLOW\n"         TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BLUE           "TRACE_UART_CHARACTER_BLUE\n"           TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_MAGENTA        "TRACE_UART_CHARACTER_MAGENTA\n"        TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_CYAN           "TRACE_UART_CHARACTER_CYAN\n"           TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_WHITE          "TRACE_UART_CHARACTER_WHITE\n"          TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_LIGHT_BLACK    "TRACE_UART_CHARACTER_LIGHT_BLACK\n"    TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_LIGHT_RED      "TRACE_UART_CHARACTER_LIGHT_RED\n"      TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_LIGHT_GREEN    "TRACE_UART_CHARACTER_LIGHT_GREEN\n"    TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_LIGHT_YELLOW   "TRACE_UART_CHARACTER_LIGHT_YELLOW\n"   TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_LIGHT_BLUE     "TRACE_UART_CHARACTER_LIGHT_BLUE\n"     TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_LIGHT_MAGENTA  "TRACE_UART_CHARACTER_LIGHT_MAGENTA\n"  TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_LIGHT_CYAN     "TRACE_UART_CHARACTER_LIGHT_CYAN\n"     TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_LIGHT_WHITE    "TRACE_UART_CHARACTER_LIGHT_WHITE\n"    TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_BLACK   "TRACE_UART_CHARACTER_BRIGHT_BLACK\n"   TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_RED     "TRACE_UART_CHARACTER_BRIGHT_RED\n"     TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_GREEN   "TRACE_UART_CHARACTER_BRIGHT_GREEN\n"   TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_YELLOW  "TRACE_UART_CHARACTER_BRIGHT_YELLOW\n"  TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_BLUE    "TRACE_UART_CHARACTER_BRIGHT_BLUE\n"    TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_MAGENTA "TRACE_UART_CHARACTER_BRIGHT_MAGENTA\n" TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_CYAN    "TRACE_UART_CHARACTER_BRIGHT_CYAN\n"    TRACE_UART_RESET);
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_WHITE   "TRACE_UART_CHARACTER_BRIGHT_WHITE\n"   TRACE_UART_RESET);
  trace_setLevel(TRACE_LVL_ALL);
  trace2uart(TRACE_UART_CHARACTER_BRIGHT_WHITE TRACE_UART_BOLD TRACE_UART_UNDERLINED "\nChosen colors for trace levels:\n" TRACE_UART_RESET);
#endif /*  TRACE_USE_UART */
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


/**
* @brief  print buffer on display
* @param  onlyOneBuffer: if 1 output only 1 buffer, else output all bufferized data
* @retval None
*/
static void s_trace_outputBuffersOnDisplay(bool const onlyOneBuffer)
{
  #ifdef TRACE_USE_DISPLAY
  for (traceLogInfo_t *pTraceBuffer = G_pFirstTraceBuffer; pTraceBuffer != NULL; pTraceBuffer = pTraceBuffer->pNext)
  {
    uint8_t output = (uint8_t)pTraceBuffer->output;

    if ((output & (uint8_t)TRACE_OUTPUT_DISPLAY) != 0U)
    {
      trace2display(pTraceBuffer);
      output              &= ~(uint8_t)TRACE_OUTPUT_DISPLAY;
      pTraceBuffer->output = (traceOutput_t)output;
      if (onlyOneBuffer)
      {
        // log only 1 buffer
        break;
      }
    }
  }

  s_updateTraceBufferList();
  #endif  // TRACE_USE_DISPLAY
}


/**
* @brief  print buffer on uart
* @param  onlyOneBuffer: if 1 output only 1 buffer, else output all bufferized data
* @retval None
*/
static void s_trace_outputBuffersOnUart(bool const onlyOneBuffer)
{
  #ifdef TRACE_USE_UART
  for (traceLogInfo_t *pTraceBuffer = G_pFirstTraceBuffer; pTraceBuffer != NULL; pTraceBuffer = pTraceBuffer->pNext)
  {
    uint8_t output = (uint8_t)pTraceBuffer->output;

    if ((output & (uint8_t)TRACE_OUTPUT_UART) != 0U)
    {
      static const char *const traceColorTable[NB_TRACE_LVL] =
      {
        [TRACE_LVL_VERBOSE_BIT]     = TRACE_UART_CHARACTER_BRIGHT_WHITE,
        [TRACE_LVL_DEBUG_BIT]       = TRACE_UART_CHARACTER_BRIGHT_GREEN,
        [TRACE_LVL_LOG_BIT]         = TRACE_UART_CHARACTER_BRIGHT_CYAN,
        [TRACE_LVL_INFO_BIT]        = TRACE_UART_CHARACTER_BRIGHT_BLUE,
        [TRACE_LVL_INFO2_BIT]       = TRACE_UART_CHARACTER_LIGHT_BLUE,
        [TRACE_LVL_WARNING_BIT]     = TRACE_UART_CHARACTER_BRIGHT_YELLOW,
        [TRACE_LVL_ERROR_BIT]       = TRACE_UART_CHARACTER_BRIGHT_RED,
        [TRACE_LVL_ERROR_FATAL_BIT] = TRACE_UART_CHARACTER_BRIGHT_RED TRACE_UART_REVERSED
      };
      char *traceColor = NULL;

      // get highest trace level color
      for (uint8_t traceLvlBit = 0U; traceLvlBit < (uint8_t)NB_TRACE_LVL; traceLvlBit++)
      {
        if (((pTraceBuffer->level >> traceLvlBit) & 1U) != 0U)
        {
          traceColor = (char *)traceColorTable[traceLvlBit];
        }
      }

      if (traceColor != NULL)
      {
        trace2uart(traceColor);
        trace2uart(pTraceBuffer->buffer);
        trace2uart(TRACE_UART_RESET);
      }
      else
      {
        trace2uart(pTraceBuffer->buffer);
      }
      output              &= ~(uint8_t)TRACE_OUTPUT_UART;
      pTraceBuffer->output = (traceOutput_t)output;
      if (onlyOneBuffer)
      {
        // log only 1 buffer
        break;
      }
    }
  }

  s_updateTraceBufferList();
  #endif  // TRACE_USE_UART
}


/**
* @brief  free trace buffer entry
* @param  trace buffer pointer
* @retval None
*/
#if defined(TRACE_USE_DISPLAY) || defined(TRACE_USE_UART)
static void s_updateTraceBufferList(void)
{
  traceLogInfo_t *pPrevTraceBuffer = NULL;
  traceLogInfo_t *pNextTraceBuffer = NULL;

  // disable IRQ to remove pTraceBuffer from the chained list of trace buffers to be logged and to put it in the list of free buffers
  TRACE_DISABLE_IRQ();

  for (traceLogInfo_t *pTraceBuffer = G_pFirstTraceBuffer; pTraceBuffer != NULL; pTraceBuffer = pNextTraceBuffer)
  {
    pPrevTraceBuffer = pTraceBuffer->pPrev;
    pNextTraceBuffer = pTraceBuffer->pNext;

    if ((uint8_t)pTraceBuffer->output == 0U)
    {
      // buffer has been printed on all outputs => it may be freed
      // remove pTraceBuffer from the chained list of trace buffers to be logged
      if (pPrevTraceBuffer == NULL)
      {
        if (pNextTraceBuffer == NULL)
        {
          // pPrevTraceBuffer and pNextTraceBuffer are both NULL => pTraceBuffer was the only buffer to print => there is no more buffer to print (all buffers are free)
          G_pFirstTraceBuffer = NULL;
          G_pLastTraceBuffer  = NULL;
        }
        else
        {
          // pPrevTraceBuffer only is NULL => pTraceBuffer was the first buffer to print => the first buffer to print is now pNextTraceBuffer
          G_pFirstTraceBuffer     = pNextTraceBuffer;
          pNextTraceBuffer->pPrev = NULL;
        }
      }
      else
      {
        if (pNextTraceBuffer == NULL)
        {
          // pPrevTraceBuffer only is NULL => pTraceBuffer was the last buffer to print => the last buffer to print is now pPrevTraceBuffer
          G_pLastTraceBuffer      = pPrevTraceBuffer;
          pPrevTraceBuffer->pNext = NULL;
        }
        else
        {
          // pPrevTraceBuffer and pNextTraceBuffer are both not NULL => update the chained list to remove pTraceBuffer from the list
          pPrevTraceBuffer->pNext = pNextTraceBuffer;
          pNextTraceBuffer->pPrev = pPrevTraceBuffer;
        }
      }

      // put pTraceBuffer on the top of list of free buffers
      if (G_pFreeTraceBuffers != NULL)
      {
        G_pFreeTraceBuffers->pPrev = pTraceBuffer;
      }
      pTraceBuffer->pNext = G_pFreeTraceBuffers;
      pTraceBuffer->pPrev = NULL;
      G_pFreeTraceBuffers = pTraceBuffer;

      #ifdef CHECK_TRACE_CONSISTENCY
      s_checkTraceBufferListsConsistency();
      #endif

      #ifdef TRACE_DEPTH_DEBUG
      // trace depth debug
      G_debugTraceDepth--;
      #endif
    }
  }

  // re-enable IRQ
  TRACE_ENABLE_IRQ();
}
#endif

#ifdef CHECK_TRACE_CONSISTENCY
static void s_traceError(int const line)
{
  while (1)
  {
  }
}


static void s_checkTraceBufferListsConsistency(void)
{
  traceLogInfo_t *pTraceBuffer, *pLastTraceBuffer;
  int             nbBuffers = 0;

  if (G_pFreeTraceBuffers == NULL)
  {
    if (G_pFirstTraceBuffer == NULL)
    {
      s_traceError(__LINE__);
    }
    if (G_pLastTraceBuffer == NULL)
    {
      s_traceError(__LINE__);
    }
  }

  if ((G_pFirstTraceBuffer == NULL))
  {
    if (G_pFreeTraceBuffers == NULL)
    {
      s_traceError(__LINE__);
    }
    if (G_pLastTraceBuffer != NULL)
    {
      s_traceError(__LINE__);
    }
  }

  if (G_pLastTraceBuffer == NULL)
  {
    if (G_pFreeTraceBuffers == NULL)
    {
      s_traceError(__LINE__);
    }
    if (G_pFirstTraceBuffer != NULL)
    {
      s_traceError(__LINE__);
    }
  }

  if ((G_pFreeTraceBuffers != NULL) && (G_pFreeTraceBuffers->pPrev != NULL))
  {
    s_traceError(__LINE__);
  }

  if ((G_pFirstTraceBuffer != NULL) && (G_pFirstTraceBuffer->pPrev != NULL))
  {
    s_traceError(__LINE__);
  }

  if ((G_pLastTraceBuffer != NULL) && (G_pLastTraceBuffer->pNext != NULL))
  {
    s_traceError(__LINE__);
  }

  for (pTraceBuffer = G_pFreeTraceBuffers; pTraceBuffer != NULL; pTraceBuffer = pTraceBuffer->pNext)
  {
    nbBuffers++;
    if (nbBuffers > NB_TRACE_LOG_BUFFERS)
    {
      s_traceError(__LINE__);
    }
    if ((pTraceBuffer->pPrev != NULL) && (pTraceBuffer->pPrev->pNext != pTraceBuffer))
    {
      s_traceError(__LINE__);
    }
    if ((pTraceBuffer->pNext != NULL) && (pTraceBuffer->pNext->pPrev != pTraceBuffer))
    {
      s_traceError(__LINE__);
    }
  }

  pLastTraceBuffer = NULL;

  for (pTraceBuffer = G_pFirstTraceBuffer; pTraceBuffer != NULL; pTraceBuffer = pTraceBuffer->pNext)
  {
    pLastTraceBuffer = pTraceBuffer;
    nbBuffers++;
    if (nbBuffers > NB_TRACE_LOG_BUFFERS)
    {
      s_traceError(__LINE__);
    }
    if ((pTraceBuffer->pPrev != NULL) && (pTraceBuffer->pPrev->pNext != pTraceBuffer))
    {
      s_traceError(__LINE__);
    }
    if ((pTraceBuffer->pNext != NULL) && (pTraceBuffer->pNext->pPrev != pTraceBuffer))
    {
      s_traceError(__LINE__);
    }
  }

  if (pLastTraceBuffer != G_pLastTraceBuffer)
  {
    s_traceError(__LINE__);
  }

  if (nbBuffers != NB_TRACE_LOG_BUFFERS)
  {
    s_traceError(__LINE__);
  }
}
#endif
