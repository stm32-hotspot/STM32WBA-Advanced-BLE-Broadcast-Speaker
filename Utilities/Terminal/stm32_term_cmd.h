/**
  ******************************************************************************
  * @file        stm32_term_cmd.h
  * @author      MCD Application Team
  * @brief       Manage terminal communication
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

#ifndef __STM32_TERM_CMD_H
#define __STM32_TERM_CMD_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "stm32_term.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/



/* Exported defines   --------------------------------------------------------*/

/* Exported macros ------------------------------------------------------------*/
#if defined ( __ICCARM__ ) /*!< IAR Compiler */


#pragma section = "term_cmd_entries"
#define TERM_CMD_DECLARE(name,param,help,cb) \
         _Pragma("location = \"term_cmd_entries\"") \
        __root const UTIL_TERM_cmd_str_entry_t  fn_##cb={name,param,help,cb,0}

#define TERM_CMD_DECLARE_HIDE(name,param,help,cb) \
         _Pragma("location = \"term_cmd_entries\"") \
        __root const UTIL_TERM_cmd_str_entry_t  fn_##cb={name,param,help,cb,1}


#define TERM_CMD_GET_ENTRIES(pEntry) {pEntry = (UTIL_TERM_cmd_str_entry_t*)__section_begin("term_cmd_entries");}
#elif defined ( __GNUC__   ) ||  defined ( __CC_ARM   )


typedef char term_ref_gcc;  /* allow to remove forbidden * in TERM_CMD_GET_ENTRIES_EXT macro */

#define TERM_CMD_DECLARE(name,param,help,cb) __attribute__((section("term_cmd_entries"))) const UTIL_TERM_cmd_str_entry_t fn_##cb={name,param,help,cb,0}
#define TERM_CMD_DECLARE_HIDE(name,param,help,cb) __attribute__((section("term_cmd_entries"))) const UTIL_TERM_cmd_str_entry_t fn_##cb={name,param,help,cb,1}
#define TERM_CMD_GET_ENTRIES(pEntry) extern  term_ref_gcc _term_cmd_entries;\
                                    pEntry =(UTIL_TERM_cmd_str_entry_t*) &_term_cmd_entries

#else
#error "Tool chain not supported"
#endif

#ifndef TERM_MAX
#define TERM_MAX   50
#endif

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
};
#endif


#endif  /* __STM32_TERM_CMD_H */
