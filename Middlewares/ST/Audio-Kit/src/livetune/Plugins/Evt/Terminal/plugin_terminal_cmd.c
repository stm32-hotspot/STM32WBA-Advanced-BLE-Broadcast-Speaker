/**
******************************************************************************
* @file          plugin_terminal_cmd.c
* @author        MCD Application Team
* @brief         Manage terminal communication
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
#include "st_base.h"
#include "st_registry.h"
#include "livetune.h"
#include "plugin_terminal_cmd.h"
#include "livetune_db_json.h"
#include "string.h"


/* Private defines -----------------------------------------------------------*/
#define MAX_EVENT_DELEGATION    3

/* Global variables ----------------------------------------------------------*/

static int32_t      nbEventDelegation;
static st_plugin_cb tEventDelegation[MAX_EVENT_DELEGATION];


/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */




/**
* @brief Command definition
*
*/
int32_t  plugin_terminal_cmd_add_event_delegation(st_plugin_cb cb)
{
  int32_t bResult = false;
  if (nbEventDelegation < MAX_EVENT_DELEGATION)
  {
    tEventDelegation[nbEventDelegation++] = cb;
    bResult = true;
  }
  return bResult;
}


/**
* @brief Command definition
*
*/
static void plugin_terminal_cmd_disaptch_event_delegation(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param pParam)
{
  for (int32_t index = 0 ; index < nbEventDelegation; index++)
  {
    tEventDelegation[index](hInst, evt, wParam, pParam);
  }
}





/**
* @brief Command definition
*
*/

static livetune_instance *gInstance;

livetune_instance *UTIL_TERM_get_instance(void)
{
  return gInstance;
}

__weak uint32_t plugin_terminal_cmd_parse_valid_event(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param pParam)
{
  return 0U;
}


__weak uint32_t plugin_terminal_cmd_parse_designer_event(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param pParam)
{
  return 0U;
}
__weak uint32_t plugin_terminal_cmd_parse_common_event(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param pParam)
{
  return 0U;
}


/**
 * @brief Terminal  Event handler
 *
 * @param handle the instance handle
 * @param evt  the event
 * @param wparam  the opaque param
 * @param lparam  the opaque param
 * @return ST_Result Error code
 */
static uint32_t plugin_terminal_event_cb(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param pParam)
{
  if (evt == ST_EVT_SYSTEM_STARTED)
  {
    gInstance = hInst;
    if (UTIL_TERM_set_uart_rx(UTIL_UART_GetHdle()) != UTIL_ERROR_NONE)
    {
      ST_TRACE_ERROR("UTIL_TERM_set_uart_rx");
    }
    UTIL_TERM_set_alloc_funcs(st_os_mem_generic_realloc_slow, st_os_mem_generic_free);
    if (UTIL_TERM_create() != UTIL_ERROR_NONE)
    {
      ST_TRACE_ERROR("UTIL_TERM_create");
    }

    plugin_terminal_cmd_add_event_delegation(plugin_terminal_cmd_parse_designer_event);
    plugin_terminal_cmd_add_event_delegation(plugin_terminal_cmd_parse_common_event);
    plugin_terminal_cmd_add_event_delegation(plugin_terminal_cmd_parse_valid_event);


  }

  if (evt == ST_EVT_SYSTEM_SHUTDOWN)
  {
    ST_VERIFY(UTIL_TERM_delete());
  }

  plugin_terminal_cmd_disaptch_event_delegation(hInst, evt, wParam, pParam);

  return 0;
}

ST_PLUGIN_DECLARE(plugin_terminal_event_cb);

#endif