/**
******************************************************************************
* @file        stm32_term_cmd.c
* @author      MCD Application Team
* @brief       Manage terminal commands
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
* All rights reserved.</center></h2>
*
* This software component is licensed by ST under Ultimate Liberty license
* SLA0044, the "License"; You may not use this file except in compliance with
* the License. You may obtain a copy of the License at:
*                             www.st.com/SLA0044
*
******************************************************************************
*/


/*
  Add and adapt the following lines in the link script to support multifile command line
  each new command must be declared using the macro TERM_CMD_DECLARE


  define symbol __ICFEDIT_size_plugin_Cmd__= 59*16;
  define block PLUGIN_CMD with alignment = 8, size = __ICFEDIT_size_plugin_Cmd__     { section st_plugin_term_cmd_entries };
  place in ROM_region       { block ROM_CONTENT,block PLUGIN_CMD };

*/



/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "string.h"
#include "stm32_term_cmd.h"




/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */

static void plugin_terminal_cmd_help(int argc, char_t *argv[]);
/**
* @brief Command definition
*
*/

const UTIL_TERM_cmd_str_entry_t *UTIL_TERM_getCustomCommands(void)
{
  void *pEntries;
  TERM_CMD_GET_ENTRIES(pEntries);
  return pEntries;
}



/**
* @brief Display terminal help
*
* @param argc  num args
* @param argv  args list
*/

static void plugin_terminal_cmd_help(int argc, char_t *argv[])
{
  uint32_t cptCmd = 0;
  const UTIL_TERM_cmd_str_entry_t *pCommands = UTIL_TERM_getCustomCommands();
  while (pCommands[cptCmd].name)
  {
    if ((pCommands[cptCmd].flags & 1U) == 0U)
    {
      UTIL_TERM_printf_cr("%-020s : %-70s : %s", pCommands[cptCmd].name, (pCommands[cptCmd].params == NULL) ? "" : pCommands[cptCmd].params, pCommands[cptCmd].comments);
    }
    cptCmd++;
  }
}

TERM_CMD_DECLARE("help", NULL, "This help", plugin_terminal_cmd_help);

