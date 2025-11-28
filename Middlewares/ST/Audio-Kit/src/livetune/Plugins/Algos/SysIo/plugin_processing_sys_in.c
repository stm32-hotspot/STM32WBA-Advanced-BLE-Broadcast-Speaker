/**
******************************************************************************
* @file          plugin_processing_sys_in.c
* @author        MCD Application Team
* @brief         in1 a processing
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

#include <stdio.h>
#include <ctype.h>
#include "livetune.h"
#include "livetune_pipe.h"
#include "livetune_helper.h"

/* Includes ------------------------------------------------------------------*/

#include <stdarg.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define MAX_SYS_INSTANCE_REF 4U
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

typedef enum plugin_parameters
{
  kValueCount
} plugin_parameters;

typedef enum pinDef
{
  kPinOut0,
} pinDef;



/* Declarations ----------------------------------------------------------*/
static void                           plugin_sys_in_processing_constructor(livetune_db_instance *pInstance);
static void                           plugin_sys_in_processing_destructor(livetune_db_instance *pInstance);
static livetune_builder_result     plugin_sys_in_processing_builder(livetune_db_instance *pInstance, uint32_t cmd);
static livetune_helper_builder_def tSysIn[] =
{
  {.pAlgoName = "fake@sysin1"},
  {.pAlgoName = "fake@sysin2"},
  {.pAlgoName = "fake@sysin3"},
  {.pAlgoName = "fake@sysin4"},
  {0}
};




/* Global variables ----------------------------------------------------------*/

/*

{
    "Description": "",
    "Icon": "icon-elem-src.png",
    "Name": "",
    "SysIo": 1,
    "InstanceMax":1,
    "PinDefOut": [
        {
            "ConnectionMax":1,
            "Type": "acChunk"
        }
    ]
}
}



*/
static const char_t tElementDescriptionTemplate[] = "{\"Description\":\"\",\"Icon\":\"icon-elem-src.png\",\"Name\":\"\",\"InstanceMax\":1,\"PinDefOut\":[{\"disabled\":true,\"ConnectionMax\":1,\"Type\":\"acChunk\"}]}";

static livetune_interface plugin_interface =// element instance
{
  .constructor_cb  = plugin_sys_in_processing_constructor,
  .destructor_cb   = plugin_sys_in_processing_destructor,
  .pipe_builder_cb = plugin_sys_in_processing_builder,
};


/* Private functions ------------------------------------------------------- */



/**
* @brief Create element template from a variable number of sys in instances
*/


static void plugin_sys_create_element_def(livetune_instance *pDesigner)
{
  uint32_t nbSysIo = livetune_ac_factory_get_sys_connection_nb(AC_SYS_IN);

  for (uint32_t indexSysCnx = 0; (indexSysCnx < nbSysIo) && (indexSysCnx < MAX_SYS_INSTANCE_REF); indexSysCnx++)
  {
    audio_chain_sys_connection_conf_t *pConf        = livetune_ac_factory_get_sys_connection_conf(AC_SYS_IN, (int32_t)indexSysCnx);
    livetune_db_element               *pElement     = NULL;
    char_t                            *pDescription = livetune_ac_json_sysio_create_def(pConf, tElementDescriptionTemplate);
    ST_ASSERT(pDescription != NULL);
    tSysIn[indexSysCnx].pCookie = pConf;
    switch (livetune_db_element_json_register(&pDesigner->hDesignerDB, pDescription, &tSysIn[indexSysCnx], plugin_interface, &pElement))
    {
      case ST_OK:
      case ST_NOT_IMPL:
        // ignore case of algo not linked
        break;
      default:
        ST_TRACE_ERROR("cannot register algo SysIn%d", indexSysCnx);
        break;
    }
  }
}

/**
* @brief Manage the pipe
*/

static livetune_builder_result plugin_sys_in_processing_builder(livetune_db_instance *pInstance, uint32_t cmd)
{
  livetune_builder_result result = ST_BUILDER_OK;
  st_class_element_instance *pCls   = (st_class_element_instance *)pInstance->pUserData;
  ST_ASSERT(pCls != NULL);
  const audio_chain_sys_connection_conf_t *pConf = ((livetune_helper_builder_def *)pInstance->pElements->pRefBuilder)->pCookie;
  ST_ASSERT(pConf != NULL);

  switch (cmd)
  {
    case LIVETUNE_PIPE_START:
    {
      /* assume the algo not valid*/
      memset(&pInstance->hAc, 0, sizeof(pInstance->hAc));
      livetune_helper_set_default_attrb(pInstance);
      pInstance->hAc.bValid             = FALSE;
      pInstance->hAc.bSysIO             = TRUE;
      livetune_db_instance_cnx *pCnx = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOut0, 0);
      if (pCnx)
      {
        const char_t *pVarName = livetune_ac_factory_get_sys_connection_name(pConf);
        snprintf(pCnx->hAc.tVarName, sizeof(pCnx->hAc.tVarName), "h%c%s_%s", toupper((int32_t)*pVarName), &pVarName[1], pCnx->pName);
      }
      break;
    }


    case LIVETUNE_PIPE_PROLOG:
    {
      livetune_db_instance_cnx *pCnx  = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOut0, 0);
      if (pCnx)
      {
        pCnx->hAc.bValid      = TRUE;
        pInstance->hAc.bSysIO = FALSE;
        livetune_ac_factory_clone_sys_connection_chunk(pConf, &pCnx->hAc.hConf);
        livetune_generate_chunk_handles(pCls->hBuilder.pPipe, &pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOut0);
        pInstance->hAc.bSysIO = 1U;
      }
      result = ST_BUILDER_OK;
      break;
    }



    case LIVETUNE_PIPE_POST_INIT_GRAPH:
    {
      livetune_db_instance_cnx *pCnx = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOut0, 0);
      if (pCnx)
      {
        const char_t *pVarName = livetune_ac_factory_get_sys_connection_name(pConf);
        livetune_ac_factory_init_sysio(pVarName, pCnx);
        livetune_generate_create_chunks(&pCls->hBuilder, &pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOut0);
        livetune_ac_wrapper_acChunkCreate(&pCls->hBuilder, &pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOut0);
      }
      result = ST_BUILDER_OK;
      break;
    }


    case LIVETUNE_PIPE_INIT_GRAPH_ELEMENT:
    {
      /* the in is not an algo */
      result = ST_BUILDER_OK;
      break;
    }
    case LIVETUNE_PIPE_INIT_GRAPH_ATTACH_CNX:
    {
      result = ST_BUILDER_OK;
      break;
    }
    default:
    {
      result = ST_BUILDER_NOT_IMPLEMENTED;
      break;
    }
  }
  return result;
}


/**
* @brief Class construction, allocate local memory, prepare variables  & parameter for the transform
*       This function is called before the pipeline start
* * @param pHandle the instance handle
*/

static void plugin_sys_in_processing_constructor(livetune_db_instance *pInstance)
{
  pInstance->pUserData = livetune_ac_factory_builder_create(pInstance, 0);
  ST_ASSERT(pInstance->pUserData != NULL);
}


/**
* @brief Class destruction , free all resources allocated in the construction
*       This function is called before the pipeline stop
*       The destruct MUST cleanup the pipe
* * @param pHandle the instance handle
*/

static void plugin_sys_in_processing_destructor(livetune_db_instance *pInstance)
{
  if (pInstance->pUserData)
  {
    st_class_element_instance *pCls = (st_class_element_instance *)pInstance->pUserData;
    ST_VERIFY(livetune_helper_builder_delete(pCls) == ST_OK);
  }
}




/**
 * @brief sys_in1  Event handler
 *
 * @param handle the instance handle
 * @param evt  the event
 * @param wparam  the opaque param
 * @param lparam  the opaque param
 * @return ST_Result Error code
 */
static uint32_t plugin_sys_in_event_cb(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param lParam)
{
  if (evt == ST_EVT_REGISTER_ELEMENTS)
  {
    /* register elements */
    plugin_sys_create_element_def(hInst);
  }

  return 0;
}

/* plugin declaration, notify that this element must be add as a client */

ST_PLUGIN_DECLARE(plugin_sys_in_event_cb);


#endif
