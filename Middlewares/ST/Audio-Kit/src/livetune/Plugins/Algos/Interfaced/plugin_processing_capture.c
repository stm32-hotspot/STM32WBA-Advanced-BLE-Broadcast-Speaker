/**
******************************************************************************
* @file          plugin_processing_capture.c
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
#include "stm32_usart.h"
#include "limits.h"


/* Private defines -----------------------------------------------------------*/


#define ALGO_PLUGIN_CB plugin_capture_event_cb


/*
{
  "PinDefOut":[{"Type":"acGraph","Name":"Grph","Description":"Must be connected to a Graph viewer"},{"Type":"acMsg","Name":"Msg","Description":"Must be connected to a Message viewer"}],
   "Params":[
      {
         "Description":"samples Refresh delay in ms",
         "Type":0,
         "Name":"Refresh",
         "Default":30,
         "Min":30,
         "Max":1000,
         "Control":"slider"
      }
    ]
}

*/

enum plugin_pin_in
{
  kPinInSrc,
};
enum plugin_pin_out
{
  kPinOutGraph = 0U,
  kPinOutMsg,
};


/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

static const livetune_helper_builder_def hBuilderDef =
{
  .pAlgoName = "capture"
};


typedef struct st_plugin_element_ext
{
  uint32_t                       iTimeTarget;
  char_t                         sCbName[LIVETUNE_VAR_NAME_MAX];
  livetune_db_instance_cnx   *pConnectedGraph;
  livetune_db_instance_cnx   *pConnectedMsg;
  livetune_db_instance_cnx   *pConnectedIn;
  livetune_helper_builder_def hBuilderDef; /* copy because we modify the cbs */
  float_t                        minY, maxX;
  float_t                        minX, maxY;
  uint32_t                       indexRefresh;
  uint32_t                       nbSent;
  float_t                        uartMsByChar;
  const char_t                  *pUnityY;
  const char_t                  *pUnityX;
  int16_t                       *pSamples;
  int16_t                       *pCtrlSamples;
  uint32_t                       szSamples;
  uint32_t                       nbSamples;
  uint32_t                       nbChannels;
  uint32_t                       nbSamplesByChannels;

} st_plugin_element_ext;

/* Declarations ----------------------------------------------------------*/

static void                       plugin_processing_constructor(livetune_db_instance *pInstance);
static void                       plugin_processing_destructor(livetune_db_instance *pInstance);
static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd);

/* Global variables ----------------------------------------------------------*/

static char_t tElementDescription[] = "{\"PinDefOut\":[{\"Type\":\"acGraph\",\"Name\":\"Grph\",\"Description\":\"Must be connected to a Graph viewer\"},{\"Type\":\"acMsg\",\"Name\":\"Msg\",\"Description\":\"Must be connected to a Message viewer\"}],\"Params\":[{\"Description\":\"samples Refresh delay in ms\",\"Type\":0,\"Name\":\"Refresh\",\"Default\":30,\"Min\":30,\"Max\":1000,\"Control\":\"slider\"}]}";

/* interface overload */
static livetune_interface plugin_interface = // element instance
{
  .constructor_cb      = plugin_processing_constructor,
  .destructor_cb       = plugin_processing_destructor,
  .pipe_builder_cb     = plugin_processing_builder
};


/* Private functions ------------------------------------------------------- */


/**
* @brief capture result as graph
* @param pCls the class instance handle
*/


static void plugin_processing_control_capture_idle_graph(st_class_element_instance *pCls)
{
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
  if (pClsExt->pConnectedGraph)
  {
    static char_t  tScratch[200];
    static int32_t count = 0;

    /* build extra json fields to send */
    snprintf(tScratch, sizeof(tScratch), "\"cpt\":%d,\"Len\":%d,\"Type\":\"i16\",\"Ch\":%d,\"MinX\":\"%f\",\"MaxX\":\"%f\",\"MinY\":\"%f\",\"MaxY\":\"%f\",\"UX\":\"%s\",\"UY\":\"%s\",\"To\":\"%s\"", count++, pClsExt->szSamples, pClsExt->nbChannels, pClsExt->minX, pClsExt->maxX, pClsExt->minY, pClsExt->maxY, pClsExt->pUnityX, pClsExt->pUnityY, pClsExt->pConnectedGraph->pInstance->pInstanceName);
    /* send the message using DMA*/
    pClsExt->nbSent += livetune_send_block_binary_async("acGraph", "SamplesView", tScratch, (uint8_t *)pClsExt->pSamples, pClsExt->szSamples);
  }
}

/**
* @brief capture result as row text
* @param pCls the class instance handle
*/


static void plugin_processing_control_capture_idle_Text(st_class_element_instance *pCls)
{
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
  if (pClsExt->pConnectedMsg)
  {
    uint32_t szScratchBody = 5 * 1024;
    char_t  *pTextBuffer   = st_os_mem_alloc(ST_Mem_Type_Designer, szScratchBody);
    if (pTextBuffer)
    {
      uint32_t pos   = 0;
      char_t  *pText = pTextBuffer;
      pText += sprintf(pText, "Text as CSV format;\n");
      for (uint32_t indexCh = 0; indexCh < pClsExt->nbChannels; indexCh++)
      {
        pText += sprintf(pText, "CH%02d", indexCh);
        for (uint32_t indexSample = 0; indexSample < pClsExt->nbSamplesByChannels; indexSample++)
        {
          pText += sprintf(pText, ";%d", pClsExt->pSamples[pos++]);

          /* updated the buffer size if > 80%*/
          uint32_t txtSize = (uint32_t)(pText - pTextBuffer);
          if (txtSize > ((szScratchBody * 80U) / 100U))
          {
            szScratchBody += 5U * 1024U;
            pTextBuffer = st_os_mem_realloc(ST_Mem_Type_Designer, pTextBuffer, szScratchBody);
            ST_ASSERT(pTextBuffer != NULL);
            pText = &pTextBuffer[txtSize];
          }
        }
        pText += sprintf(pText, "\n");
      }
      char_t tScratch[100];
      snprintf(tScratch, sizeof(tScratch), "\"Ch\":%d,\"From\":\"%s\",\"To\":\"%s\"", pClsExt->nbChannels, pCls->hBuilder.pInstance->pInstanceName, pClsExt->pConnectedMsg->pInstance->pInstanceName);
      /* send the message using DMA*/
      pClsExt->nbSent += livetune_send_block_text_async(FALSE, "acMsg", "Text", tScratch, pTextBuffer);
      st_os_mem_free(pTextBuffer);
    }
  }
}

/**
* @brief Compute the final capture result according to params
* @param pCls the class instance handle
*/


static int32_t plugin_processing_control_compute_capture(st_class_element_instance *pCls)
{
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
  /* init default param */

  pClsExt->pUnityY = "Mag";
  pClsExt->pUnityX = "Smp";
  pClsExt->minX    = 0.0F;
  pClsExt->maxX    = (float)pClsExt->nbSamplesByChannels;
  pClsExt->minY    = SHRT_MIN;
  pClsExt->maxY    = SHRT_MAX;
  uint32_t szAlloc = pClsExt->nbSamplesByChannels * pClsExt->nbChannels * sizeof(int16_t);
  if (pClsExt->pSamples == NULL)
  {
    pClsExt->pSamples = st_os_mem_alloc(ST_Mem_Type_Designer, szAlloc);
  }

  if (pClsExt->pSamples)
  {
    memcpy(pClsExt->pSamples, pClsExt->pCtrlSamples, szAlloc);
    pClsExt->nbSamples = szAlloc / sizeof(int16_t);
  }
  pClsExt->szSamples = szAlloc;
  return 1;
}

/**
* @brief return true if the time is over
* @param pCls the class instance handle
*/

static int8_t plugin_processing_check_time(st_class_element_instance *pCls)
{
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
  int8_t                 bResult = FALSE;
  if (pClsExt->iTimeTarget == 0U)
  {
    pClsExt->iTimeTarget = (uint32_t)st_os_sys_time();
  }
  uint32_t ticks = (uint32_t)st_os_sys_time();
  if (ticks > pClsExt->iTimeTarget)
  {
    int32_t delay        = LIVETUNE_INT(pCls->hBuilder.pInstance, pClsExt->indexRefresh);
    pClsExt->iTimeTarget = (ticks + (uint32_t)delay);
    bResult              = TRUE;
  }

  return bResult;
}

/* AC control callback */

static int32_t plugin_processing_control_capture_cb(acAlgo hAlgo)
{
  st_class_element_instance *pCls = NULL;
  acAlgoGetCommonConfig(hAlgo, "userData", &pCls);
  ST_ASSERT(pCls != NULL);
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
  int8_t                 ret     = plugin_processing_check_time(pCls);
  if ((hAlgo != NULL) && (ret != FALSE))
  {
    ST_VERIFY(acAlgoGetControl(hAlgo, "nbChannels", &pClsExt->nbChannels) == 0);
    ST_VERIFY(acAlgoGetControl(hAlgo, "nbSamplesByChannels", &pClsExt->nbSamplesByChannels) == 0);
    ST_VERIFY(acAlgoGetControl(hAlgo, "pSamples", &pClsExt->pCtrlSamples) == 0);

    pClsExt->nbSent = 0;
    plugin_processing_control_compute_capture(pCls);
    plugin_processing_control_capture_idle_Text(pCls);
    plugin_processing_control_capture_idle_graph(pCls);
    /* To get an accurate refresh rate, we need to add the time taken by the packet transmission itself */
    float    result       = (float)pClsExt->nbSent * pClsExt->uartMsByChar;
    uint32_t extraDelayMs = (uint32_t)result;
    pClsExt->iTimeTarget += extraDelayMs;
  }
  return 1;
}




/**
* @brief Builder overload
* @param pHandle the instance handle
* @param cmd   event
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
        livetune_ac_wrapper_acAlgoSetCommonConfig(&pCls->hBuilder, "controlCb", (void *)plugin_processing_control_capture_cb); /*cstat !MISRAC2012-Rule-11.1 cast a callback in pointer is mandatory for this API */
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
    st_class_element_instance *pCls    = pInstance->pUserData;
    st_plugin_element_ext     *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);

    pClsExt->pConnectedGraph          = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOutGraph, 0U);
    pClsExt->pConnectedMsg            = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOutMsg, 0U);
    livetune_db_instance_cnx *pCnx = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinIn, (uint32_t)kPinInSrc, 0U);
    if (pCnx)
    {
      pClsExt->pConnectedIn = livetune_db_cnx_name_search(&pCnx->pInstance->pPinOut, pCnx->pName, pCnx->iPinDef);
    }

    /* some params are hardcoded, we need to know the real position after json update */
    pClsExt->indexRefresh = (uint32_t)livetune_db_param_find_from_name(&pInstance->pElements->pParams, "Refresh");
    pClsExt->uartMsByChar = 1000.0F / (((float)UTIL_UART_GetHdle()->Init.BaudRate) / 8.0F);
  }
  else
  {
    ST_TRACE_ERROR("Memory alloc %d", sizeof(st_class_element_instance));
  }
}


/**
* @brief Overload the destruction because we have to free a buffer
*       This function is called before the pipeline start
* * @param pHandle the instance handle
*/

static void plugin_processing_destructor(livetune_db_instance *pInstance)
{
  if (pInstance->pUserData)
  {
    st_class_element_instance *pCls    = (st_class_element_instance *)pInstance->pUserData;
    st_plugin_element_ext     *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
    if (pClsExt->pSamples)
    {
      st_os_mem_free(pClsExt->pSamples);
    }
    ST_VERIFY(livetune_helper_builder_delete(pCls) == ST_OK);
  }
  pInstance->pUserData = NULL;
}


/**
 * @brief nr  Event handler
 *
 * @param handle the instance handle
 * @param evt  the event
 * @param lparam  the opaque param
 * @param wParam  the opaque param
 * @return ST_Result Error code
 */

static uint32_t ALGO_PLUGIN_CB(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param lParam)
{
  if (evt == ST_EVT_REGISTER_ELEMENTS)
  {
    /* register the element */
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

/* Plugin declaration, notify that this element must be added to the plugin list*/
ST_PLUGIN_DECLARE(ALGO_PLUGIN_CB);

#endif
