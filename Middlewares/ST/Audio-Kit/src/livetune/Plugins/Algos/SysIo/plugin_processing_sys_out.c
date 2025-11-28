/**
******************************************************************************
* @file          plugin_processing_sys_out.c
* @author        MCD Application Team
* @brief         sink  processing
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
#include "ctype.h"
#include "livetune.h"
#include "livetune_pipe.h"
#include "livetune_helper.h"

/* Includes ------------------------------------------------------------------*/

#include <stdarg.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define MAX_SYS_INSTANCE_REF 3U

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/


/* this template are not const because we going to update it */
static livetune_helper_builder_def tSysOut[] =
{
  {.pAlgoName = "fake@sysout1"},
  {.pAlgoName = "fake@sysout2"},
  {.pAlgoName = "fake@sysout3"},
  {0}
};



typedef struct st_plugin_sys_out
{
  livetune_helper_builder hBuilder;
} st_plugin_sys_out;

typedef enum plugin_parameters
{
  kValueCount
} plugin_parameters;

typedef enum pinDef
{
  kPinIn0,
} pinDef;

/*
typedef struct livetune_sys_ref
{
  livetune_interface                      hInterface;
  char_t                                 *pDescription;
  livetune_db_element                    *pElement;
  struct audio_chain_sys_connection_conf *pConf;
} livetune_sys_ref;
*/

/* Declarations ----------------------------------------------------------*/
static void                       plugin_sys_out_processing_constructor(livetune_db_instance *pInstance);
static void                       plugin_sys_out_processing_destructor(livetune_db_instance *pInstance);
static livetune_builder_result plugin_sys_out_processing_builder(livetune_db_instance *pInstance, uint32_t cmd);


/* Global variables ----------------------------------------------------------*/
/*
{
   "Description":"Sink Connection point",
   "Icon":"icon-elem-sink.png",
   "InstanceMax":1,
   "Name":"SysOutChunk1",
   "PinDefIn":[
      {
         "Name":"sink",
         "Type":"acChunk"}]

}
*/
static char_t tElementDescriptionTemplate[] = "{\"Description\":\"Sink Connection point\",\"Icon\":\"icon-elem-sink.png\",\"Name\":\"SysOutChunk1\",\"InstanceMax\":1,\"PinDefIn\":[{\"Name\":\"sink\",\"Type\":\"acChunk\"}]}";

static livetune_interface plugin_interface =// element instance
{
  .constructor_cb  = plugin_sys_out_processing_constructor,
  .destructor_cb   = plugin_sys_out_processing_destructor,
  .pipe_builder_cb = plugin_sys_out_processing_builder,
};

/* Private functions ------------------------------------------------------- */

static void plugin_sys_in_create_element_def(livetune_instance *pDesigner)
{
  uint32_t nbSysIo = livetune_ac_factory_get_sys_connection_nb(AC_SYS_OUT);

  for (uint32_t indexSysCnx = 0; (indexSysCnx < nbSysIo) && (indexSysCnx < MAX_SYS_INSTANCE_REF); indexSysCnx++)
  {
    audio_chain_sys_connection_conf_t *pConf        = livetune_ac_factory_get_sys_connection_conf(AC_SYS_OUT, (int32_t)indexSysCnx);
    livetune_db_element               *pElement     = NULL;
    char_t                            *pDescription = livetune_ac_json_sysio_create_def(pConf, tElementDescriptionTemplate);
    ST_ASSERT(pDescription != NULL);
    tSysOut[indexSysCnx].pCookie = pConf;
    switch (livetune_db_element_json_register(&pDesigner->hDesignerDB, pDescription, &tSysOut[indexSysCnx], plugin_interface, &pElement))
    {
      case ST_OK:
      case ST_NOT_IMPL:
        // ignore case of algo not linked
        break;
      default:
        ST_TRACE_ERROR("cannot register algo SysOut%d", indexSysCnx);
        break;
    }
  }
}




static livetune_builder_result plugin_sys_out_processing_builder(livetune_db_instance *pInstance, uint32_t cmd)
{
  livetune_builder_result result = ST_BUILDER_OK;
  st_plugin_sys_out         *pCls   = (st_plugin_sys_out *)pInstance->pUserData;
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
      pInstance->hAc.bSysIO             = 1U;
      livetune_db_instance_cnx *pCnx = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinIn, (uint32_t)kPinIn0, 0);
      if (pCnx)
      {
        const char_t *pVarName = livetune_ac_factory_get_sys_connection_name(pConf);
        snprintf(pCnx->hAc.tVarName, sizeof(pCnx->hAc.tVarName), "h%c%s_%s", toupper((int32_t)*pVarName), pVarName + 1, pCnx->pName);
      }

      break;
    }


    case LIVETUNE_PIPE_PROLOG:
    {
      /* the out is not an algo */


      livetune_db_instance_cnx *pCnx = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinIn, (uint32_t)kPinIn0, 0);
      if (pCnx)
      {
        pInstance->hAc.bSysIO = FALSE;
        pInstance->hAc.bSysIO = TRUE;
        livetune_ac_factory_clone_sys_connection_chunk(pConf, &pCnx->hAc.hConf);
        livetune_generate_chunk_handles(pCls->hBuilder.pPipe, &pCls->hBuilder.pInstance->pPinIn, (uint32_t)kPinIn0);
      }

      result = ST_BUILDER_OK;
      break;
    }




    case LIVETUNE_PIPE_POST_INIT_GRAPH:
    {
      livetune_db_instance_cnx *pCnx = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinIn, (uint32_t)kPinIn0, 0);
      if (pCnx)
      {
        const char_t *pVarName = livetune_ac_factory_get_sys_connection_name(pConf);
        livetune_ac_factory_init_sysio(pVarName, pCnx);
        livetune_generate_create_chunks(&pCls->hBuilder, &pCls->hBuilder.pInstance->pPinIn, (uint32_t)kPinIn0);
        livetune_ac_wrapper_acChunkCreate(&pCls->hBuilder, &pCls->hBuilder.pInstance->pPinIn, (uint32_t)kPinIn0);
      }
      result = ST_BUILDER_OK;
      break;
    }




    case LIVETUNE_PIPE_INIT_GRAPH_ELEMENT:
    {
      /* the sink is not an algo */
      result = ST_BUILDER_OK;

      break;
    }
    case LIVETUNE_PIPE_INIT_GRAPH_ATTACH_CNX:
    {
      livetune_db_instance_cnx *pCnxIn = livetune_helper_cnx_get(&pInstance->pPinIn, (uint32_t)kPinIn0, 0);
      /* special case for a non algo element,we cannot use helpers, we need to link the chunk to the algo ref, but it is not symmetric  */

      if (pCnxIn)
      {
        if (strlen(pCnxIn->pInstance->hAc.tVarName) != 0U)
        {
          /* add the connection */
          if ((pCnxIn->pInstance->hAc.bValid != 0U) && (pCnxIn->hAc.bValid != 0U))
          {
            /* Warning big Hack , TODO: Fix me */
            pCls->hBuilder.pInstance = pCnxIn->pInstance;
            livetune_ac_wrapper_acPipeConnectPinOut(&pCls->hBuilder, 0, pCnxIn->hAc.hChunk);
          }
          /* Generate the code */
          livetune_generate_connect_pins(pInstance->pPipe, pCnxIn->pInstance->hAc.tVarName, 0, pCnxIn->hAc.tVarName, ST_GENERATOR_PIN_TYPE_OUT);
        }
      }
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

static void plugin_sys_out_processing_constructor(livetune_db_instance *pInstance)
{
  pInstance->pUserData = livetune_ac_factory_builder_create(pInstance, 0);
  if (pInstance->pUserData == NULL)
  {
    ST_TRACE_ERROR("Memory alloc %d", sizeof(st_plugin_sys_out));
  }
}


/**
* @brief Class destruction , free all resources allocated in the construction
*       This function is called before the pipeline stop
*       The destruct MUST cleanup the pipe
* * @param pHandle the instance handle
*/

static void plugin_sys_out_processing_destructor(livetune_db_instance *pInstance)
{
  if (pInstance->pUserData)
  {
    st_class_element_instance *pCls = (st_class_element_instance *)pInstance->pUserData;
    ST_VERIFY(livetune_helper_builder_delete(pCls) == ST_OK);
  }
}




/**
 * @brief sys_out  Event handler
 *
 * @param handle the instance handle
 * @param evt  the event
 * @param wparam  the opaque param
 * @param lparam  the opaque param
 * @return ST_Result Error code
 */
static uint32_t plugin_sys_out_event_cb(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param lParam)
{
  if (evt == ST_EVT_REGISTER_ELEMENTS)
  {
    /* register elements */
    plugin_sys_in_create_element_def(hInst);
  }

  return 0;
}

/* plugin declaration, notify that this element must be add as a client */

ST_PLUGIN_DECLARE(plugin_sys_out_event_cb);


#endif
