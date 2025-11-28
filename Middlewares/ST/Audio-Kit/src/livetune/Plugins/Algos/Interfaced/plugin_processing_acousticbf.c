/**
******************************************************************************
* @file          plugin_processing_acousticBF.c
* @author        MCD Application Team
* @brief         element processing
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
#include "livetune.h"
#include "livetune_helper.h"


/* Private defines -----------------------------------------------------------*/
#define ALGO_PLUGIN_CB plugin_acousticBF_event_cb

enum plugin_pin_out
{
  kPinOutMsg,
};



/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
static livetune_helper_builder_def hBuilderDef =
{
  .pAlgoName = "acousticBF"
};


typedef struct st_plugin_element_ext
{
  livetune_db_instance_cnx *pConnectedMsg;
  float    energy_mic_db;
} st_plugin_element_ext;

/* Declarations ----------------------------------------------------------*/
static void                    plugin_processing_constructor(livetune_db_instance *pInstance);
static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd);

/* Global variables ----------------------------------------------------------*/

static char_t tElementDescription[] = "{\"PinDefOut\":[{\"Type\":\"acMsg\",\"Name\":\"Msg\",\"Description\":\"Must be connected to an acMsg viewer\"}]}";

static livetune_interface plugin_interface =  // element instance
{
  .constructor_cb      = plugin_processing_constructor,
  .pipe_builder_cb     = plugin_processing_builder
};


/* Private functions ------------------------------------------------------- */

#define LOCAL_SCRATCH_MSG 256U

/* AC control callback */

static int32_t plugin_processing_control_acousticBF(acAlgo hAlgo)
{
  if (hAlgo)
  {
    st_class_element_instance *pCls = NULL;
    acAlgoGetCommonConfig(hAlgo, "userData", &pCls);
    ST_ASSERT(pCls != NULL);
    st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
    /* if not connected, no events */
    if (pClsExt->pConnectedMsg)
    {
      ST_VERIFY(acAlgoGetControl(hAlgo, "energy_mic_db", &pClsExt->energy_mic_db) == 0);
      /* alloc the memory for the the msg body */

      char_t   tlocalScratch[LOCAL_SCRATCH_MSG];
      char_t   tlocalVar[LOCAL_SCRATCH_MSG] = "";
      uint32_t szScratchBody = LOCAL_SCRATCH_MSG ;
      char_t  *pScratchBody  = st_os_mem_alloc(ST_Mem_Type_Designer, szScratchBody);
      if (pScratchBody)
      {
        memset(pScratchBody, 0, szScratchBody);
        //        livetune_helper_format_clear(tlocalScratch);
        //        /* build message to be sent */
        //        snprintf(tlocalScratch, sizeof(tlocalScratch), "Energy of mic signal in Db = %6.2f", pClsExt->energy_mic_db);
        //        strcat(pScratchBody, tlocalScratch);
        /* Enable a variable watcher */
        char *pVars = strchr(tlocalVar, 0);
        snprintf(pVars, sizeof(tlocalVar), "ch%d:%2.1fdB", 0, (double)pClsExt->energy_mic_db); /* the syntax is Key:Value*/

        /* build mandatory json fields to identify the MsgViewer instance to send to */
        livetune_helper_format_clear(tlocalScratch);
        snprintf(tlocalScratch, sizeof(tlocalScratch), "\"From\":\"%s\",\"To\":\"%s\",\"Vars\":\"%s\"", pCls->hBuilder.pInstance->pInstanceName, pClsExt->pConnectedMsg->pInstance->pInstanceName, tlocalVar);


        /* send the message */
        livetune_send_block_text_async(FALSE, "acMsg", "Text", tlocalScratch, pScratchBody);

        st_os_mem_free(pScratchBody);
      }
    }
  }
  return 1;
}


/**
* @brief instantiate or generate code
* * @param pHandle the instance handle
*/

static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd)
{
  livetune_builder_result result = ST_BUILDER_DEFAULT;
  st_class_element_instance *pCls   = (st_class_element_instance *)pInstance->pUserData;

  switch (cmd)
  {
    case LIVETUNE_PIPE_INIT_GRAPH_ELEMENT:
    {
      /* Process Init Graph default behaviour */
      ST_VERIFY(livetune_helper_builder_pipe_init_graph_element_default(&pCls->hBuilder) == ST_BUILDER_OK);
      /* overload the default with a set control callback */

      if (pInstance->hAc.bValid)
      {
        /* Add the send Graph support control */
        livetune_ac_wrapper_acAlgoSetCommonConfig(&pCls->hBuilder, "controlCb", (void *)plugin_processing_control_acousticBF); /*cstat !MISRAC2012-Rule-11.1 cast a callback in pointer is mandatory for this API */
        livetune_ac_wrapper_acAlgoSetCommonConfig(&pCls->hBuilder, "userData", pCls);
      }


      result = ST_BUILDER_OK;
      break;
    }
  }
  return result;
}

/**
* @brief Overload the construction because we derive the builder with an extension
*       This function is called before the pipeline start
* * @param pHandle the instance handle
*/


static void plugin_processing_constructor(livetune_db_instance *pInstance)
{
  pInstance->pUserData = livetune_ac_factory_builder_create(pInstance, sizeof(st_plugin_element_ext));
  if (pInstance->pUserData)
  {
    st_class_element_instance *pCls    = (st_class_element_instance *)pInstance->pUserData;
    st_plugin_element_ext     *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
    pClsExt->pConnectedMsg             = livetune_helper_cnx_get(&pInstance->pPinOut, (uint32_t)kPinOutMsg, 0);
  }
  else
  {
    ST_TRACE_ERROR("Memory alloc %d", sizeof(st_class_element_instance));
  }
}


/**
* @brief  Event handler
*
* @param handle the instance handle
* @param evt  the event
* @param wparam  the opaque param
* @param lparam  the opaque param
* @return ST_Result Error code
*/

static uint32_t ALGO_PLUGIN_CB(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param lParam)
{
  if (evt == ST_EVT_REGISTER_ELEMENTS)
  {
    livetune_instance *pDesigner = (livetune_instance *)hInst;
    livetune_db_element *pElement = NULL;
    switch (livetune_db_element_json_register(&pDesigner->hDesignerDB, tElementDescription, &hBuilderDef, plugin_interface, &pElement))
    {
      case ST_OK:
      case ST_NOT_IMPL:
        // ignore case of algo not linked
        break;
      default:
        ST_TRACE_ERROR("cannot register algo %s", hBuilderDef.pAlgoName);
        break;
    }
  }
  return 0;
}

/* plugin declaration, notify that this element must be add as a client */

ST_PLUGIN_DECLARE(ALGO_PLUGIN_CB);

#endif
