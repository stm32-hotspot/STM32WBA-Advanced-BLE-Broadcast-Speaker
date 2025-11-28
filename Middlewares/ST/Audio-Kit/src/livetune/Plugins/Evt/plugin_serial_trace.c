/**
******************************************************************************
* @file          plugin_serial_trace.c
* @author        MCD Application Team
* @brief         Trace event instrumentation
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

#ifdef USE_LIVETUNE_DESIGNER

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>
#include "livetune.h"
#include "plugin_serial_trace.h"
#include "time.h"

/* Private defines -----------------------------------------------------------*/
#define TEXT_DEFINE(a) #a, a
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/



/**
 * @brief Trace Format record entry
 *
 */
typedef struct service_serial_entry
{
  char_t  *pText;
  uint32_t evt_code;
  void    *wParam;
  void (*cb)(const struct service_serial_entry *pEntry, st_message_param wParam, st_message_param lParam, char_t *pTime);
} service_serial_entry;


static const service_serial_entry tTracetList[];

/**
 * @brief Key value converter
 *
 */
typedef struct t_trace_key_value
{
  uint32_t    key;
  const char *pValue;
} trace_key_value_t;

/* Private prototypes -----------------------------------------------------------*/
void plugin_serial_update_param(const struct service_serial_entry *pEntry, st_message_param wParam, st_message_param lParam, char_t *pTime);
/* Private functions ------------------------------------------------------- */



/**
 * @brief parse the test report message
 *
 * @param pEntry Formatter entry instance
 * @param pparam opaque param
 * @param pTime  buffer string
 */

static void plugin_serial_test_report(const struct service_serial_entry *pEntry, st_message_param wParam, st_message_param lParam, char_t *pTime)
{
  char_t   tScratch[400];
  char_t  *pType      = (char_t *)ST_LOW_INT64(wParam);
  char_t  *pOptString = (char_t *)ST_HIGH_INT64(wParam);

  char_t  *pDesc  = (char_t *)ST_LOW_INT64(lParam);
  uint32_t result = ST_HIGH_INT64(lParam);
  if (pOptString  == NULL)
  {
    pOptString = "";
  }
  if (st_base_get_debug_level() & ST_TRACE_LVL_COLORIZE)
  {
    char_t tResult[50];
    if (result)
    {
      snprintf(tResult, sizeof(tResult), "%sOK%s", "\033[32;1m", "\033[0m");
    }
    else
    {
      snprintf(tResult, sizeof(tResult), "%sKO%s", "\033[41m", "\033[0m");
    }
    snprintf(tScratch, sizeof(tScratch), "%s : %s  : (result=%s) : %s", pType, pDesc, tResult, pOptString);
  }
  else
  {
    snprintf(tScratch, sizeof(tScratch), "%s : %s  : (result=%s) : %s", pType, pDesc, (result != 0U) ? "OK" : "FAILS", pOptString);
  }
  ST_TRACE_INFO("%10s :  %-40s : %02d:%s(0x%X:0x%X)", pTime, tScratch, pEntry->evt_code, pEntry->pText, wParam, lParam);
}



/**
 * @brief Print the pipe state
 *
 * @param pEntry Formatter entry instance
 * @param pparam opaque param
 * @param pTime  buffer string
 */

static void plugin_serial_start_pipe(const struct service_serial_entry *pEntry, st_message_param wParam, st_message_param lParam, char_t *pTime)
{
  char_t *pText;
  if (lParam == (st_message_param)ST_MAKE_INT64(0UL, ST_OK))
  {
    pText = "Audio chain started";
  }
  else
  {
    pText = "Audio chain error, check the log";
  }

  ST_TRACE_INFO("%10s :  %-40s : %02d:%s(0x%X:0x%X)", pTime, pText, pEntry->evt_code, pEntry->pText, wParam, lParam);
}


/**
 * @brief Print the pipe state
 *
 * @param pEntry Formatter entry instance
 * @param pparam opaque param
 * @param pTime  buffer string
 */

static void plugin_serial_stop_pipe(const struct service_serial_entry *pEntry, st_message_param wParam, st_message_param lParam, char_t *pTime)
{
  char_t *pText;
  if (lParam == (st_message_param)ST_MAKE_INT64(0UL, ST_OK))
  {
    pText = "Audio chain stopped";
  }
  else
  {
    pText = "Audio chain stopped with errors, check the log";
  }

  ST_TRACE_INFO("%10s :  %-40s : %02d:%s(0x%X:0x%X)", pTime, pText, pEntry->evt_code, pEntry->pText, wParam, lParam);
}



/**
 * @brief Print the pipe state
 *
 * @param pEntry Formatter entry instance
 * @param pparam opaque param
 * @param pTime  buffer string
 */

static void plugin_serial_print_string(const struct service_serial_entry *pEntry, st_message_param wParam, st_message_param lParam, char_t *pTime)
{
  char_t tScratch[50] = "";
  snprintf(tScratch, 40, "%s", (lParam == 0ULL) ? "" : (char_t *)(uint32_t)lParam);
  ST_TRACE_INFO("%10s :  %-40s : %02d:%s(0x%X:0x%X)", pTime, tScratch, pEntry->evt_code, pEntry->pText, wParam, lParam);
}




/**
 * @brief Print a registration text
 *
 * @param pEntry Formatter entry instance
 * @param pparam opaque param
 * @param pTime  buffer string
 */

#ifndef BUILD_REDUCED_LOG_MESSAGE
static void plugin_serial_registered(const struct service_serial_entry *pEntry, st_message_param wParam, st_message_param lParam, char_t *pTime)
{
  static uint32_t         count    = 0;
  livetune_db_element *pElement = (lParam  != 0U) ? (livetune_db_element *)(uint32_t)lParam : NULL;
  ST_UNUSED(pElement);
  ST_UNUSED(lParam);
  char_t tmpBuff[160];
  snprintf(tmpBuff, sizeof(tmpBuff), "%02d:Registered: %s", count++, pElement->pName);
  ST_TRACE_INFO("%10s :  %-40s : %02d:%s(0x%X:0x%X)", pTime, tmpBuff, pEntry->evt_code, pEntry->pText, wParam, lParam);
}
#endif


/**
 * @brief Find an entry in the list
 *
 * @param pList the list instance
 * @param code event code
 * @return int32_t the index
 */

static int32_t findTextEntry(const struct service_serial_entry *pList, uint32_t code)
{
  for (int32_t a = 0; pList[a].pText != NULL; a++)
  {
    if (pList[a].evt_code == code)
    {
      return a;
    }
  }
  return -1;
}


static void plugin_serial_trace_format_time(char_t *pBuffer, uint32_t szBuffer, uint64_t elapsed)
{
  uint64_t   d, h, m, s, ms;
  uint64_t   over;
  d = (elapsed / (3600000U * 24U));
  elapsed -= (d * (3600000U * 24U));
  h    = (elapsed / 3600000U);
  over = (elapsed % 3600000U);
  m    = (over / 60000U);
  over = (over % 60000U);
  s    = (over / 1000U);
  ms   = (over % 1000U);
  snprintf(pBuffer, szBuffer, "%02d:%02d:%02d:%02d:%03d", (uint32_t)d, (uint32_t)h, (uint32_t)m, (uint32_t)s, (uint32_t)ms);
}

/**
 * @brief Trace console Event handler
 *
 * @param handle the instance handle
 * @param evt  the event
 * @param pparam  the opaque param
 * @return ST_Result Error code
 */

static uint32_t plugin_serial_trace_cb(ST_Handle handle, uint32_t evt, st_message_param wParam, st_message_param lParam)
{
  static char_t strTime[20];
  plugin_serial_trace_format_time(strTime, sizeof(strTime), clock());
  if (evt == ST_EVT_MSG)
  {
    ST_TRACE_INFO("%10s :  %s ", strTime, (char_t *)(uint32_t)lParam);
    return 0;
  }
  int32_t index = findTextEntry(tTracetList, evt);
  if (index != -1)
  {
    if (tTracetList[index].wParam)
    {
      if (tTracetList[index].cb)
      {
        tTracetList[index].cb(&tTracetList[index], wParam, lParam, strTime);
      }
      else
      {
        ST_TRACE_INFO("%10s :  %-40s : %02d:%s(0x%X:0x%X)", strTime, tTracetList[index].wParam, evt, tTracetList[index].pText, wParam, lParam);
      }
    }
    else
    {
      ST_TRACE_INFO("%10s : %-40s  : %02d:%s(0x%X:0x%X)", strTime, "", evt, tTracetList[index].pText, wParam, lParam);
    }
  }
  return 0;
}


/**
 * @brief print a timed formatted message on the console
 *
 * @param pFormat
 * @param ...
 */
void plugin_serial_msg(ST_Handle hInstance, const char_t *pFormat, ...)
{
  if (hInstance)
  {
    va_list args;
    va_start(args, pFormat);
    char_t msg[100];
    vsnprintf(msg, sizeof(msg), pFormat, args);
    livetune_send(ST_EVT_MSG, 0, (st_message_param)ST_MAKE_INT64(0UL, msg));
    va_end(args);
  }
}

/* Variable ------------------------------------------------------- */

/**
 * @brief trace converter to a formatted string
 *
 */
static const service_serial_entry tTracetList[] =
{

  {TEXT_DEFINE(ST_EVT_SYSTEM_STARTED), "", plugin_serial_print_string},
  {TEXT_DEFINE(ST_EVT_SYSTEM_SHUTDOWN), NULL, NULL},
  {TEXT_DEFINE(ST_EVT_REGISTER_ELEMENTS), NULL, NULL},
  {TEXT_DEFINE(ST_EVT_LOAD), NULL, NULL},
  {TEXT_DEFINE(ST_EVT_SAVE), NULL, NULL},
  {TEXT_DEFINE(ST_EVT_NEW), NULL, NULL},
  /*  {TEXT_DEFINE(ST_EVT_START_ISSUE_MSG), "", plugin_serial_print_string},*/
  {TEXT_DEFINE(ST_EVT_TEST_REPORT), "", plugin_serial_test_report},
  {TEXT_DEFINE(ST_EVT_START_PIPE), "", plugin_serial_start_pipe},
  {TEXT_DEFINE(ST_EVT_STOP_PIPE), "", plugin_serial_stop_pipe},
  #ifndef BUILD_REDUCED_LOG_MESSAGE
  {TEXT_DEFINE(ST_EVT_REGISTERED_ELEMENTS), "", plugin_serial_registered},
  #endif
  {TEXT_DEFINE(ST_EVT_PIPE_STOP_ERROR), "Error", NULL},
  {NULL, 0, NULL, NULL}
};




ST_PLUGIN_DECLARE(plugin_serial_trace_cb);


#endif
