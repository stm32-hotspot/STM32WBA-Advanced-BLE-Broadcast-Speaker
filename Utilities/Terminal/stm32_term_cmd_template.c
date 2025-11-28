/**
******************************************************************************
* @file    stm32_term_cmd_template.c
* @author  MCD Application Team
* @brief   Custom definition of commands for terminal
******************************************************************************
* @attention
*
* Copyright (c) 2022(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "stm32_term.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#define MAX_TUNING_CFG_FILE_SIZE   (5*1024)

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void s_cmd_tune_aec(int argc, char *argv[]);
static void s_get_algo_list(int argc, char *argv[]);
static void s_get_dyn_param_list(int argc, char *argv[]);
static void s_get_dyn_param_val(int argc, char *argv[]);
static void s_set_dyn_param_val(int argc, char *argv[]);
static void s_get_static_param_list(int argc, char *argv[]);
static void s_get_static_param_val(int argc, char *argv[]);
static void s_set_static_param_val(int argc, char *argv[]);

/* Private typedef -----------------------------------------------------------*/



/**
* @brief Command definition
*
*/
const UTIL_TERM_cmd_str_entry_t gUTIL_TERM_customCmds[] =
{
  {"help",              NULL,                             "This help",                          UTIL_TERM_cmd_help,      0},
  {"reboot",            NULL,                             "Reboot the board",                   UTIL_TERM_cmd_reboot,    0},
  {"aec",               NULL,                             "AEC tuning",                         s_cmd_tune_aec,          0},
  {"get_algo_list",     NULL,                             "display algo list in current graph", s_get_algo_list,         0},
  {"get_dyn_par_list",  "algo id",                        "display algo's dynamic param list",  s_get_dyn_param_list,    0},
  {"get_dyn_par_val",   "algo id, param id",              "display algo's dynamic param value", s_get_dyn_param_val,     0},
  {"set_dyn_par_val",   "algo id, param id, param value", "set algo's dynamic param value",     s_set_dyn_param_val,     0},
  {"get_stat_par_list", "algo id",                        "display algo's static param list",   s_get_static_param_list, 0},
  {"get_stat_par_val",  "algo id, param id",              "display algo's static param value",  s_get_static_param_val,  0},
  {"set_stat_par_val",  "algo id, param id, param value", "set algo's static param value",      s_set_static_param_val,  0},

  {NULL,                NULL,                             NULL,                                 NULL,                    0}
};

/* Functions Definition ------------------------------------------------------*/
/* Static Functions Definition -----------------------------------------------*/

/**
* @brief Must return pointer on custom commands
*
* @param argc  num args
* @param argv  args list
*/
UTIL_TERM_cmd_str_entry_t const *UTIL_TERM_getCustomCommands(void)
{
  return (UTIL_TERM_cmd_str_entry_t const *) gUTIL_TERM_customCmds;
}



/**
* @brief AWS board registration
*
* @param argc  num args
* @param argv  args list
*/
static void s_cmd_tune_aec(int argc, char *argv[])
{
  /*User defined codec*/
}


static void s_get_algo_list(int argc, char *argv[])
{
  /*User defined codec*/
}


static void s_get_dyn_param_list(int argc, char *argv[])
{
  /*User defined codec*/
}


static void s_get_dyn_param_val(int argc, char *argv[])
{
  /*User defined codec*/
}


static void s_set_dyn_param_val(int argc, char *argv[])
{
  /*User defined codec*/
}


static void s_get_static_param_list(int argc, char *argv[])
{
  /*User defined codec*/
}


static void s_get_static_param_val(int argc, char *argv[])
{
  /*User defined codec*/
}


static void s_set_static_param_val(int argc, char *argv[])
{
  /*User defined codec*/
}

