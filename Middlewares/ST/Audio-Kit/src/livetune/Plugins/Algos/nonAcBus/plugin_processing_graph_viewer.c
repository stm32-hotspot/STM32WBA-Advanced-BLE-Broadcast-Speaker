/**
******************************************************************************
* @file          plugin_processing_graph_viewer.c
* @author        MCD Application Team
* @brief         dummy a processing
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

#include "livetune.h"
#include "livetune_helper.h"
#include "string.h"

/* Includes ------------------------------------------------------------------*/

#include <stdarg.h>

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

typedef struct st_plugin_element_viewer
{
  uint32_t dummy;

} st_plugin_element_viewer;

enum plugin_parameters
{
  kMyParamater,
  kValueCount
};


/* Declarations ----------------------------------------------------------*/
static void                       plugin_test_control_processing_constructor(livetune_db_instance *pInstance);
static void                       plugin_test_control_processing_destructor(livetune_db_instance *pInstance);
static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd);


/* Global variables ----------------------------------------------------------*/
static livetune_instance   *gpInstance;
static livetune_db_element *pElement;


/*
{
   "Description":"Show Graphics coming from acGraph pins",
   "Name":"Graph-Viewer",
   "Icon":"icon-elem-Tool.png",
   "PinDefIn":[
      {
         "Type":"acGraph",
         "Name":"Grph",
         "ConnectionMax":1,
         "Description": "Connect a graph connection"
       }
       ],
   "Params":[
      {
         "Description":"Graph Window",
         "Type":0,
         "Default":"",
         "Name":"graphic",
         "Control":"graph"
      },
      {
         "Description":"Method used to display numerical data",
         "Type":0,
         "Default":0,
         "Name":"Scale",
         "Control":"droplist"
         "ControlParams":"[{'Name':'Linear','Value':'0'},{'Name':'Logarithmic','Value':'1'}]",

      }
   ]
}
*/
static char_t                tElementDescription[] = "{\"Description\":\"Show Graphics coming from acGraph pins\",\"Name\":\"Graph-Viewer\",\"Icon\":\"icon-elem-Tool.png\",\"PinDefIn\":[{\"Type\":\"acGraph\",\"Name\":\"Grph\",\"ConnectionMax\":1,\"Description\":\"Connect a graph connection\"}],\"Params\":[{\"Description\":\"Graph Window\",\"Type\":0,\"Default\":\"\",\"Name\":\"graphic\",\"Control\":\"graph\"}]}";
static livetune_interface interface_test_control =// element instance
{
  .constructor_cb  = plugin_test_control_processing_constructor,
  .destructor_cb   = plugin_test_control_processing_destructor,
  .pipe_builder_cb = plugin_processing_builder,
};




/* Private functions ------------------------------------------------------- */


/**
* @brief instantiate or generate code
* * @param pHandle the instance handle
*/

static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd)
{
  switch (cmd)
  {
    case LIVETUNE_PIPE_START:
    {
      /* assume the algo not valid*/
      memset(&pInstance->hAc, 0, sizeof(pInstance->hAc));
      livetune_helper_set_default_attrb(pInstance);
      pInstance->hAc.bValid = FALSE;
      break;
    }
    default:
    {
      break;
    }
  }

  return ST_BUILDER_OK;
}


static void plugin_test_control_processing_constructor(livetune_db_instance *pInstance)
{
  pInstance->pUserData = st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(st_plugin_element_viewer));
  if (pInstance->pUserData)
  {
    memset(pInstance->pUserData, 0, sizeof(st_plugin_element_viewer)); /*cstat !MISRAC2012-Rule-1.3_u false positif the malloc size  is sizeof(st_plugin_element_viewer) */
  }
  else
  {
    ST_TRACE_ERROR("Memory alloc %d", sizeof(st_plugin_element_viewer));
  }
}

static void plugin_test_control_processing_destructor(livetune_db_instance *pInstance)
{
  if (pInstance->pUserData)
  {
    st_os_mem_free(pInstance->pUserData);
    pInstance->pUserData = NULL;
  }
}


/**
 * @brief Module init
 *
 */
static ST_Result plugin_test_control_create(ST_Handle hInst)
{
  /* alloc ring buffer and dma*/
  gpInstance = hInst;
  return ST_OK;
}

/**
 * @brief Module terminate
 *
 */


static ST_Result plugin_test_control_delete(void)
{
  return ST_OK;
}



/**
 * @brief test_control  Event handler
 *
 * @param handle the instance handle
 * @param evt  the event
 * @param wparam  the opaque param
 * @param lparam  the opaque param
 * @return ST_Result Error code
 */
static uint32_t plugin_graph_viewer_event_cb(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param lParam)
{
  if (evt == ST_EVT_SYSTEM_STARTED)
  {
    ST_VERIFY(plugin_test_control_create(hInst) == ST_OK);
  }

  if (evt == ST_EVT_SYSTEM_SHUTDOWN)
  {
    ST_VERIFY(plugin_test_control_delete() == ST_OK);
  }

  if (evt == ST_EVT_REGISTER_ELEMENTS)
  {
    switch (livetune_db_element_json_register(&gpInstance->hDesignerDB, tElementDescription, NULL, interface_test_control, &pElement))
    {
      case ST_OK:
      case ST_NOT_IMPL:
        // ignore case of algo not linked
        break;
      default:
        ST_TRACE_ERROR("cannot register algo %s", "Graph-Viewer");
        break;
    }
  }

  return 0;
}


ST_PLUGIN_DECLARE(plugin_graph_viewer_event_cb);


#endif