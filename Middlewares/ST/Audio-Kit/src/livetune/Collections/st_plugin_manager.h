/**
******************************************************************************
* @file          st_plugin_manager.h
* @author        MCD Application Team
* @brief         Header: registry  management
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



#ifndef ST_PLUGIN_MANAGER_H
#define ST_PLUGIN_MANAGER_H
/* Includes ------------------------------------------------------------------*/
#include "st_base.h"
#include "st_message.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* Exported types ------------------------------------------------------------*/

typedef uint32_t (*st_plugin_cb)(void *PHandle, uint32_t evt, st_message_param wParam, st_message_param lParam);

/* Exported constants --------------------------------------------------------*/



typedef struct st_plugin_manager
{
  st_plugin_cb *pArray;
} st_plugin_manager;



/* Exported macros ------------------------------------------------------------*/
#if defined(__ICCARM__) /*!< IAR Compiler */
#ifndef ST_PLUGIN_SECTION_NAME_IAR
#define ST_PLUGIN_SECTION_NAME_IAR "st_plugin_entries"
#endif


#pragma section = ST_PLUGIN_SECTION_NAME_IAR
#define ST_PLUGIN_STRINGIFY(x) #x
#define ST_PLUGIN_DECLARE_EXT(fn, sec)           \
    _Pragma(ST_PLUGIN_STRINGIFY(location = sec)) \
        __root const st_plugin_cb fn_##fn = (fn)
#define ST_PLUGIN_GET_ENTRIES_EXT(sec, pEntry) pEntry = (st_plugin_cb*)__section_begin(sec)
#define ST_PLUGIN_GET_ENTRIES(pEntry)          ST_PLUGIN_GET_ENTRIES_EXT(ST_PLUGIN_SECTION_NAME_IAR, pEntry)
#ifndef __CSTAT__ /* _Pragma(ST_PLUGIN_STRINGIFY(location = sec)) generate a violation we cannot fix */
#define ST_PLUGIN_DECLARE(fn) ST_PLUGIN_DECLARE_EXT(fn, ST_PLUGIN_SECTION_NAME_IAR)
#else
#define ST_PLUGIN_DECLARE(fn)
#endif


#elif defined(__GNUC__) || defined(__CC_ARM)

#ifndef ST_PLUGIN_SECTION_NAME_GCC
#define ST_PLUGIN_SECTION_NAME_GCC st_plugin_entries
#endif


#define ST_PLUGIN_STRINGIFY(x)         #x
typedef char st_ref_gcc; /* allow to remove forbidden * in ST_PLUGIN_GET_ENTRIES_EXT macro */
#define ST_PLUGIN_DECLARE_EXT(fn, sec) __attribute__((section(ST_PLUGIN_STRINGIFY(sec)))) const st_plugin_cb fn_##fn = fn
#define ST_PLUGIN_GET_ENTRIES_EXT(sec, pEntry) \
    extern st_ref_gcc sec;                     \
    pEntry = ((void*)&sec)
#define ST_PLUGIN_GET_ENTRIES(pEntry) ST_PLUGIN_GET_ENTRIES_EXT(ST_PLUGIN_SECTION_NAME_GCC, pEntry)
#define ST_PLUGIN_DECLARE(fn)         ST_PLUGIN_DECLARE_EXT(fn, ST_PLUGIN_SECTION_NAME_GCC)

#else
#error "Tool chain nt supported"
#endif


/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

uint32_t  st_plugin_send(st_plugin_manager *pInstance, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam);
int32_t   st_plugin_manager_create(st_plugin_manager *pInstance, st_plugin_cb *pEntry);
int32_t   st_plugin_manager_delete(st_plugin_manager *pInstance);



#ifdef __cplusplus
};
#endif



#endif




