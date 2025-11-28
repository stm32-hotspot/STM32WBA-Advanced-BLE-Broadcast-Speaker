/**
******************************************************************************
* @file          st_plugin_manager.c
* @author        MCD Application Team
* @brief         plugin management
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
#include "st_plugin_manager.h"
#include "string.h"

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */


int32_t st_plugin_manager_create(st_plugin_manager *pInstance, st_plugin_cb *pEntry)
{
  ST_ASSERT(pInstance != NULL);
  ST_ASSERT(pEntry != NULL);
  memset(pInstance, 0, sizeof(*pInstance));
  pInstance->pArray = pEntry;
  return 1;
}

int32_t st_plugin_manager_delete(st_plugin_manager *pInstance)
{
  return 1;
}




/**
* @brief  dispatch event cb to all plugins
*
* @param hHandle  instance handle
* @param evt      ST_Result event
* @param wParam   ST_Result param
* @param lParam   ST_Result param
* @return TRUE = handled
*/

uint32_t st_plugin_send(st_plugin_manager *pInstance, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam)
{
  st_plugin_cb *pEntries = pInstance->pArray;

  while ((pEntries != NULL) && (*pEntries != NULL))
  {
    st_plugin_cb fn  = *pEntries++;
    uint32_t     ret = fn(pCookie, evt, wParam, lParam);
    if (ret != 0U)
    {
      return ret;
    }
  }
  return 0;
}


