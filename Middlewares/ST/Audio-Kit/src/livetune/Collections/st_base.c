/**
******************************************************************************
* @file          st_base.c
* @author        MCD Application Team
* @brief         implements basic logs & debug function
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


/* Includes ------------------------------------------------------------------*/

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "st_base.h"
#include "st_os.h"

/* Private defines -----------------------------------------------------------*/
#define ST_NB_TRACE_LEVEL_CB 2

#define TRACE_COLOR_RESET "\033[0m" /* Reset */
#ifndef TRACE_COLOR_ERROR
  #define TRACE_COLOR_ERROR "\033[31;1m" /* RED*/
#endif
#ifndef TRACE_COLOR_WARNING
  #define TRACE_COLOR_WARNING "\033[32;1m" /* GREEN*/
#endif
#ifndef TRACE_COLOR_INFO
  #define TRACE_COLOR_INFO "\033[33;1m" /* YELLOW*/
#endif
#ifndef TRACE_COLOR_DEBUG
  #define TRACE_COLOR_DEBUG "\033[36;1m" /* CYNAN */
#endif
#ifndef TRACE_COLOR_VERBOSE
  #define TRACE_COLOR_VERBOSE "\033[37;1m" /* bright white */
#endif
#ifndef TRACE_COLOR_JSON
  #define TRACE_COLOR_JSON "\033[33;1m" /* YELLOW*/
#endif

#ifndef TRACE_COLOR_USER1
  #define TRACE_COLOR_USER1 0
#endif

#ifndef TRACE_COLOR_USER2
  #define TRACE_COLOR_USER2 0
#endif
#ifndef TRACE_COLOR_USER3
  #define TRACE_COLOR_USER3 0
#endif
#ifndef TRACE_COLOR_USER4
  #define TRACE_COLOR_USER4 0
#endif
#ifndef TRACE_COLOR_USER5
  #define TRACE_COLOR_USER5 0
#endif
#ifndef TRACE_COLOR_USER6
  #define TRACE_COLOR_USER6 0
#endif
#ifndef TRACE_COLOR_USER7
  #define TRACE_COLOR_USER7 0
#endif




/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct st_base_trace_level_cb
{
  st_base_debug_level_cb get_debug_level;
  st_base_debug_level_cb set_debug_level;
} st_base_trace_level_cb;


/* Global variables ----------------------------------------------------------*/

static uint32_t               gdebugLevel = ST_TRACE_LVL_DEFAULT; /* All message by default */
static char_t                 tPrintBuffer[500];
static st_base_trace_level_cb tTraceLevelCB[ST_NB_TRACE_LEVEL_CB];


#define MAX_COLOR_SLOT (sizeof(tColors) / sizeof(tColors[0]))
static char_t *tColors[13] =
{
  TRACE_COLOR_ERROR,
  TRACE_COLOR_WARNING,
  TRACE_COLOR_INFO,
  TRACE_COLOR_DEBUG,
  TRACE_COLOR_VERBOSE,
  TRACE_COLOR_JSON,
  TRACE_COLOR_USER1,
  TRACE_COLOR_USER2,
  TRACE_COLOR_USER3,
  TRACE_COLOR_USER4,
  TRACE_COLOR_USER5,
  TRACE_COLOR_USER6,
  TRACE_COLOR_USER7
};

/* Private functions ------------------------------------------------------- */



__weak uint32_t st_base_set_led_state(st_base_leds numLed, uint32_t state)
{
  return (uint32_t) -1;
}

/**
 * @brief create an indentation
 *
 * @param index nb space
 */
static void st_base_insert_tabs(uint32_t index)
{
  for (uint32_t a = 0; a < index; a++)
  {
    st_base_puts(ST_TRACE_LVL_ALL, "  ");
  }
}

/**
 * @brief prints a json string
 *
 * @param pJson the json
 */
static void st_base_trace_json(const char *pJson)
{
  uint32_t level      = 0;
  char_t   theChar[2] = {0, 0};

  while (*pJson)
  {
    theChar[0] = *pJson;
    pJson++;

    if (theChar[0] == '"')
    {
      st_base_puts(ST_TRACE_LVL_ALL, theChar);
      while ((*pJson != '\0') && (*pJson != '"'))
      {
        theChar[0] = *pJson++;
        st_base_puts(ST_TRACE_LVL_ALL, theChar);
      }

      theChar[0] = *pJson++;
      st_base_puts(ST_TRACE_LVL_ALL, theChar);
      continue;
    }
    if (theChar[0] == '{')
    {
      st_base_puts(ST_TRACE_LVL_ALL, "\n");
      st_base_insert_tabs(level);
      st_base_puts(ST_TRACE_LVL_ALL, theChar);
      st_base_puts(ST_TRACE_LVL_ALL, "\n");
      level++;
      st_base_insert_tabs(level);
    }
    else if (theChar[0] == '}')
    {
      level--;
      st_base_puts(ST_TRACE_LVL_ALL, "\n");
      st_base_insert_tabs(level);
      st_base_puts(ST_TRACE_LVL_ALL, theChar);
      st_base_puts(ST_TRACE_LVL_ALL, "\n");
      st_base_insert_tabs(level);
    }
    else if (theChar[0] == ',')
    {
      st_base_puts(ST_TRACE_LVL_ALL, theChar);
    }
    else
    {
      st_base_puts(ST_TRACE_LVL_ALL, theChar);
    }
  }
  st_base_puts(ST_TRACE_LVL_ALL, "\n");
}

/**
 * @brief Set a terminal color if the option is enabled
 *
 */
static void st_base_set_trace_color(const char_t *pColor)
{
  if ((gdebugLevel & ST_TRACE_LVL_COLORIZE))
  {
    st_base_puts(ST_TRACE_LVL_ALL, pColor);
  }
}


/**
 * @brief generate a delay by loop
 *
 * @param delay
 */
void st_base_generate_long_delay(uint64_t delay)
{
  for (volatile uint64_t a = 0; a < delay; a++)
  {
  }
}

/**
 * @brief  string according to the flag level
 *
 * @param level the flag level
 * @return const char_t*
 */
static const char_t *st_base_get_level_string(uint32_t level)
{
  const char_t *pStr   = NULL;
  const char_t *pColor = NULL;
  uint32_t      shift  = level;
  uint32_t      index  = 0;

  if (level == ST_TRACE_LVL_ALL)
  {
    st_base_set_trace_color(TRACE_COLOR_RESET);
    return NULL;
  }

  while (index < (MAX_COLOR_SLOT + 1U))
  {
    if (shift & 1U)
    {
      break;
    }
    shift >>= 1;
    index++;
  }
  if (index < MAX_COLOR_SLOT)
  {
    pColor = tColors[index];
  }
  if (pColor)
  {
    st_base_set_trace_color(pColor);
  }
  switch (level)
  {
    case ST_TRACE_LVL_ERROR:
    {
      pStr = "Error";
      break;
    }

    case ST_TRACE_LVL_JSON:
    {
      st_base_set_trace_color(TRACE_COLOR_JSON);
      pStr = "JSon";
      break;
    }
    case ST_TRACE_LVL_WARNING:
    {
      pStr = "Warning";
      break;
    }


    case ST_TRACE_LVL_DEBUG:
    {
      pStr = "Debug";
      break;
    }
    case ST_TRACE_LVL_VERBOSE:
    {
      pStr = "Verbose";
      break;
    }

    default:
    {
      break;
    }
  }
  return pStr;
}



/**
 * @brief  basic put string
 *
 */
__weak void st_base_puts(uint32_t level, const char_t *pString)
{
  if (level & st_base_get_debug_level())
  {
    while (*pString)
    {
      putchar((int32_t)*pString++);
    }
    if (level & ST_TRACE_LVL_ADD_CR)
    {
      putchar((int32_t)'\n');
    }
  }
}




/*!
@brief Print an output string on the console
This function doesn't format the string, and don't have string size limitation
Warning, if the flag ST_TRACE_LVL_JSON_FORMATTED is set, the function will try
to format and indent the string with a JSON format
@param level   Debug level
@param pString String to print
@return void
*/

void st_base_trace_puts(uint32_t level, const char *pString)
{
  if (((level & ST_TRACE_LVL_JSON) != 0U) && (level != ST_TRACE_LVL_ALL))
  {
    st_base_get_level_string(level);
    if ((gdebugLevel & (uint32_t)ST_TRACE_LVL_JSON_FORMATTED))
    {
      st_base_trace_json(pString);
    }
    else
    {
      st_base_puts(level, pString);
    }
  }
  else
  {
    st_base_get_level_string(level);
    st_base_puts(level, pString);
  }

  st_base_set_trace_color(TRACE_COLOR_RESET);
}


void st_base_string(uint32_t level, const char_t *pString)
{
  if (level & gdebugLevel)
  {
    st_base_puts(level, pString);
  }
}



/**
 * @brief Low level produce a trace and clamp the string if out of buffer
 *
 * @param flag the flags
 * @param lpszFormat the format
 * @param args list args
 */

void st_base_printf_args(uint32_t level, const char *lpszFormat, va_list args)
{
  int32_t szTxt = (int32_t)sizeof(tPrintBuffer);
  char_t *pTxt  = (char_t *)tPrintBuffer;
  ;
  int32_t nChars = vsnprintf(pTxt, (size_t)szTxt, lpszFormat, args);
  if (nChars > (szTxt - 4))
  {
    strcpy(&pTxt[szTxt - 4], "...");
  }
  st_base_trace_puts(level, pTxt);
}

/*!
@brief  Dumps a memory block on the console

This function is mainly used for debug purpose, It allow to dump a memory block on the serial console
the dump display the offset, bytes, and ascii.
@param level Debug level
@param pTitle The string that will be displayed in the dump header
@param pData  Pointer on the memory block
@param size Block size
@return void
*/

void st_base_dump(uint32_t level, const char_t *pTitle, void *pData, uint32_t size)
{
  uint32_t segment = 8;
  level &= ~ST_TRACE_LVL_ADD_CR;
  uint8_t *pData8 = (uint8_t *)pData;
  if (level & gdebugLevel)
  {
    if (pTitle)
    {
      st_base_printf(level, "%s 0x%p:0x%03X \n", pTitle, pData, size);
    }
    uint32_t index = 0;
    while (index < size)
    {
      uint8_t *p8 = &(pData8[index]);
      st_base_printf(level, "%03x: ", index);

      for (uint32_t b = 0; b < segment; b++)
      {
        if ((b + index) < size)
        {
          st_base_printf(level, "%02x ", p8[b]);
        }
        else
        {
          st_base_printf(level, "   ");
        }
      }
      st_base_printf(level, " ");

      for (uint32_t b = 0; b < segment; b++)
      {
        char_t ascii;
        if ((b + index) < size)
        {
          if ((p8[b] < (uint8_t)' ') || (p8[b] > (uint8_t)128))
          {
            ascii = '.';
          }
          else
          {
            ascii = (char_t)p8[b];
          }
        }
        else
        {
          ascii = ' ';
        }
        st_base_printf(level, "%c", ascii);
      }
      index += (uint32_t)8;
      st_base_printf(level, "\n");
    }
    st_base_printf(level, "\n");
  }
}


/*!
@brief  Dumps a memory block on the console as an C array

This function is mainly used for debug purpose, It allow to dump a memory block on the serial console
the dump display the offset, bytes, and ascii.
@param level Debug level
@param pTitle The string that will be displayed in the dump header
@param pData  Pointer on the memory block
@param size Block size
@return void
*/

void st_base_dump_array(uint32_t level, const char_t *pTitle, void *pData, uint32_t size)
{
  level &= ~ST_TRACE_LVL_ADD_CR;
  uint8_t *pData8 = (uint8_t *)pData;
  if (pTitle)
  {
    st_base_printf(level, "uint8_t %s[]={\n", pTitle, pData, size);
  }
  for (uint32_t a = 0; a < size; a++)
  {
    if (a != 0U)
    {
      st_base_printf(level, ",");
    }
    st_base_printf(level, "0x%02x", pData8[a]);
  }
  st_base_printf(level, "};\n");
}

/*!
@brief  Print an output string on the console + carriage return

This function is similar to st_base_printf, but it is limited in size, the final string should be less than 400 bytes.
If the result of the string is higher, the end of string after 400 will be clamped with "..."
ST_Trace is used for TRACE and ASSERT, so we can pass the filename and the line number to localize an error
@param level  Debug level
@param pFile  File name to print
@param line   Line number
@param  ...   Format string and parameters
@return void
*/
void st_base_trace(uint32_t level, const char_t *pFile, uint32_t line, ...)
{
  if (((level & gdebugLevel) != 0U))
  {
    va_list args;
    va_start(args, line);

    char_t       *pFormat = va_arg(args, char *);
    const char_t *pPrefix = st_base_get_level_string(level);
    if (pPrefix)
    {
      st_base_printf(level & ~ST_TRACE_LVL_ADD_CR, "%s : ", pPrefix);
    }
    if (pFile)
    {
      /* Remove the full path */
      char_t *pf = strchr(pFile, 0);
      while ((pf != NULL) && (pf != pFile) && (*pf != '\\') && (*pf != '/'))
      {
        pf--;
      }
      if (pFile != pf)
      {
        pf++;
      }
      pFile = pf;
      st_base_printf(level & ~ST_TRACE_LVL_ADD_CR, "%s:%d : ", pFile, line);
    }
    st_base_printf_args(level, pFormat, args);
    va_end(args);
  }
}

/*!
@brief  Print an output string on the console

This function is similar ST_Trace but doesn't have filename, line number parameter
The function doesn't add a carriage return at the end of the string

@param level Debug level
@param pFormat Format string, similar to the stdlib printf
@param  ...   Format string and parameters
@return void

*/
void st_base_printf(uint32_t level, const char_t *pFormat, ...)
{
  if ((level & gdebugLevel) != 0U)
  {
    va_list args;
    va_start(args, pFormat);
    st_base_printf_args(level, pFormat, args);
    va_end(args);
  }
}

/*!
@brief  Set the  trace debug level
Allows to select the type of debug info
@param level Bits combination, see also ST_TRACE_LVL_XXX
@return The previous level
*/

uint32_t st_base_set_debug_level(uint32_t level)
{
  uint32_t prevLvl = gdebugLevel;
  gdebugLevel      = level;
  for (int32_t a = 0; a < ST_NB_TRACE_LEVEL_CB; a++)
  {
    if (tTraceLevelCB[a].set_debug_level)
    {
      tTraceLevelCB[a].set_debug_level(&level);
    }
  }
  return prevLvl;
}

/*!
@brief  Get the  trace debug level
@return The level
*/

uint32_t st_base_get_debug_level()
{
  uint32_t prevLvl = gdebugLevel;
  for (int32_t a = 0; a < ST_NB_TRACE_LEVEL_CB; a++)
  {
    if (tTraceLevelCB[a].get_debug_level)
    {
      tTraceLevelCB[a].get_debug_level(&prevLvl);
    }
  }

  return prevLvl;
}


/*!
@brief  add a  callback for  ST_Get_Debug_Level and ST_Set_Debug_Level to overload these functions
@return ST_Result
*/

ST_Result st_base_set_debug_level_cb(st_base_debug_level_cb set, st_base_debug_level_cb get)
{
  for (int32_t a = 0; a < ST_NB_TRACE_LEVEL_CB; a++)
  {
    if ((tTraceLevelCB[a].get_debug_level == 0U) && (tTraceLevelCB[a].set_debug_level == 0U))
    {
      tTraceLevelCB[a].set_debug_level = set;
      tTraceLevelCB[a].get_debug_level = get;
      return ST_OK;
    }
  }
  return ST_ERROR;
}



/*!
@brief  Check the condition  and produces a Assert message

This function is called by Macros such as ST_TRACE_XXX

@param eval Evaluation condition, TRUE or FALSE
@param peval Condition string
@param line  Line where occurs the assert
@param file  File where occurs the assert
@return void

*/
void st_base_assert(uint32_t eval, const char *peval, uint32_t line, const char *file)
{
  if (eval == 0U)
  {
    if (file)
    {
      /* Remove the full path */
      char_t *pFile = strchr(file, 0);
      while ((pFile != NULL) && (pFile != file) && (*pFile != '\\') && (*pFile != '/'))
      {
        pFile--;
      }
      if (file != pFile)
      {
        pFile++;
      }
      file = pFile;
    }
    st_base_set_trace_color(TRACE_COLOR_ERROR);
    st_base_printf((uint32_t)ST_TRACE_LVL_ERROR, "Assert : %s %s:%d\n", peval, file, line);
    st_base_signal_exception(ST_SIGNAL_EXCEPTION_ASSERT);
  }
}


/*!
@brief Signal an exception
The function use the level parameter to produce a LED lighting using the level parameter
This function is blocking and must be used for panic issues
@param level Lighting type
@return void

*/

void st_base_signal_exception(st_base_exception level)
{
  uint32_t flag   = (uint32_t)level & 3U;
  uint64_t llevel = (uint64_t)level >> 2;
  uint64_t delay  = (llevel + 1ULL) * (uint64_t)10000ULL * (uint64_t)1000ULL;
  uint32_t bRun   = 1;
  while (bRun)
  {
    st_base_generate_long_delay(delay);
    if ((flag & 1U) != 0U)
    {
      st_base_set_led_state(SYS_LED_RED, 1);
    }
    if ((flag & 2U) != 0U)
    {
      st_base_set_led_state(SYS_LED_GREEN, 1);
    }
    st_base_generate_long_delay(delay);

    if ((flag & 1U) != 0U)
    {
      st_base_set_led_state(SYS_LED_RED, 0);
    }
    if ((flag & 2U) != 0U)
    {
      st_base_set_led_state(SYS_LED_GREEN, 0);
    }
  }
}



