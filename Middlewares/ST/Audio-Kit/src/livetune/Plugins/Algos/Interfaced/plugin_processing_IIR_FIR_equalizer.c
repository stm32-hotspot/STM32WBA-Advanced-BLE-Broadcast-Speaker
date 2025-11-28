/**
******************************************************************************
* @file          plugin_processing_IIR_FIR_Equalizer.c
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

/*
    ATTENTION: This designer wrapper assumes that FIR and IIR algo
    exposes the same control interface, hence this wrapper can be
    common for both. otherwise, we have to duplicate this code,
    and manage the control callback according to their specific interface

*/

#ifdef USE_LIVETUNE_DESIGNER


/* Includes ------------------------------------------------------------------*/
#include "livetune.h"
#include "livetune_helper.h"
#include "stm32_usart.h"
#include "limits.h"
#include "string.h"

/* Private defines -----------------------------------------------------------*/


#define ALGO_PLUGIN_CB plugin_IIR_FIR_Equalizer_event_cb


/*
Add a Pidef acGraph
{
    "PinDefOut": [
        {
            "Type": "acGraph",
            "Name": "Grph",
            "Description": "Must be connected to a Graph Viewer"
        }
    ],
}
*/



enum plugin_pin_out
{
  kPinOutGraph
};


/* Private typedef -----------------------------------------------------------*/

static const livetune_helper_builder_def tEqualizerDef[] =
{
  {.pAlgoName = "IIR-equalizer"},
  {.pAlgoName = "FIR-equalizer"},
  {.pAlgoName = "FIR-graphic-equalizer"},
  //{.pAlgoName = "IIR-design"},
  {0}
};




typedef struct st_plugin_element_ext
{
  /* Data supporting acGraph */
  livetune_db_instance_cnx *pConnectedGraph;
  float_t                      minY, maxX;
  float_t                      minX, maxY;
  const char_t                *pUnityY;
  const char_t                *pUnityX;
  uint32_t                     szSamples;
  uint32_t                     nbChannels;
  /* data extracted from the control */
  uint32_t nbBands;
  float    samplingRate;
  float   *pResponse_dB;
  float    modMin_dB;
  float    modMax_dB;
} st_plugin_element_ext;




/* Declarations ----------------------------------------------------------*/
static void                    plugin_processing_constructor(livetune_db_instance *pInstance);
static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd);

/* Global variables ----------------------------------------------------------*/

/* json compatected */
static char_t tElementDescription[] = "{\"PinDefOut\":[{\"Type\":\"acGraph\",\"Name\":\"Grph\",\"Description\":\"Must be connected to a Graph Viewer\"}]}";

/* element construct definition overload */

static livetune_interface plugin_interface = // element instance
{
  .constructor_cb      = plugin_processing_constructor,
  .pipe_builder_cb     = plugin_processing_builder

};

/* Private functions ------------------------------------------------------- */

/**
* @brief send the packet to the tool
* @param pCls the class instance handle
*/


static void plugin_processing_control_Equalizer_idle_graph(st_class_element_instance *pCls)
{
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
  if (pClsExt->pConnectedGraph)
  {
    static char_t  tScratch[200];
    static int32_t count = 0;
    /* build extra json fields to send */
    snprintf(tScratch, sizeof(tScratch), "\"mode\":1,\"cpt\":%d,\"Len\":%d,\"Type\":\"f32\",\"Ch\":%d,\"MinX\":\"%f\",\"MaxX\":\"%f\",\"MinY\":\"%f\",\"MaxY\":\"%f\",\"UX\":\"%s\",\"UY\":\"%s\",\"To\":\"%s\"", count++, pClsExt->szSamples, pClsExt->nbChannels, pClsExt->minX, pClsExt->maxX, pClsExt->minY, pClsExt->maxY, pClsExt->pUnityX, pClsExt->pUnityY, pClsExt->pConnectedGraph->pInstance->pInstanceName);
    /* send the message using DMA*/
    livetune_send_block_binary_async("acGraph", "SamplesView", tScratch, (uint8_t *)pClsExt->pResponse_dB, pClsExt->szSamples);
  }
}


/**
* @brief Implement the control callback
* @param hAlfo the instance
*/


static int32_t plugin_processing_control_IIR_FIR_Equalizer_cb(acAlgo hAlgo)
{
  st_class_element_instance *pCls = NULL;
  acAlgoGetCommonConfig(hAlgo, "userData", &pCls);
  ST_ASSERT(pCls != NULL);
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
  if (hAlgo)
  {
    /* retrieve exposed data */
    ST_VERIFY(acAlgoGetControl(hAlgo, "nbBands",      &pClsExt->nbBands)      == 0);
    ST_VERIFY(acAlgoGetControl(hAlgo, "samplingRate", &pClsExt->samplingRate) == 0);
    ST_VERIFY(acAlgoGetControl(hAlgo, "pResponse_dB", &pClsExt->pResponse_dB) == 0);
    ST_VERIFY(acAlgoGetControl(hAlgo, "modMin_dB",    &pClsExt->modMin_dB)    == 0);
    ST_VERIFY(acAlgoGetControl(hAlgo, "modMax_dB",    &pClsExt->modMax_dB)    == 0);

    /* Init units & limits */
    pClsExt->pUnityY = "dB";
    pClsExt->pUnityX = "Hz";
    pClsExt->minX    = 0.0f;
    pClsExt->maxX    = (pClsExt->samplingRate / 2.0f) * (1.0f - (1.0f / (float)pClsExt->nbBands));
    pClsExt->minY    = pClsExt->modMin_dB;
    pClsExt->maxY    = pClsExt->modMax_dB;

    /* some other info also mandatory */
    pClsExt->szSamples  = pClsExt->nbBands * sizeof(float);
    pClsExt->nbChannels = 1;
    /* send the packet */
    plugin_processing_control_Equalizer_idle_graph(pCls);
  }
  return 1;
}


/**
* @brief Callback builder overload
* @param pHandle the instance handle
* @param cmd event
*/

static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd)
{
  livetune_builder_result result = ST_BUILDER_DEFAULT;
  st_class_element_instance *pCls   = (st_class_element_instance *)pInstance->pUserData;
  ST_UNUSED(pCls);
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
        livetune_ac_wrapper_acAlgoSetCommonConfig(&pCls->hBuilder, "controlCb", (void *)plugin_processing_control_IIR_FIR_Equalizer_cb); /*cstat !MISRAC2012-Rule-11.1 cast a callback in pointer is mandatory for this API */
        livetune_ac_wrapper_acAlgoSetCommonConfig(&pCls->hBuilder, "userData", pCls);
      }
      result = ST_BUILDER_OK;
      break;
    }
  }
  return result;
}

/**
* @brief overload the constructor to add extra class
*       This function is called before the pipeline start
* * @param pHandle the instance handle
*/


static void plugin_processing_constructor(livetune_db_instance *pInstance)
{
  /* create a constructor with extension class */
  pInstance->pUserData = livetune_ac_factory_builder_create(pInstance, sizeof(st_plugin_element_ext));
  if (pInstance->pUserData)
  {
    /* fill the extension class */
    st_class_element_instance *pCls    = pInstance->pUserData;
    st_plugin_element_ext     *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
    pClsExt->pConnectedGraph           = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOutGraph, 0);
  }
  else
  {
    ST_TRACE_ERROR("Memory alloc %d", sizeof(st_class_element_instance));
  }
}


/**
 * @brief algo  Event handler
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
    /* register elements */
    livetune_instance                 *pDesigner   = (livetune_instance *)hInst;
    const livetune_helper_builder_def *pBuilderDef = tEqualizerDef;

    while (pBuilderDef->pAlgoName)
    {
      /* register the element, the element json will be updated on fly using the pBuilderDef factory */
      livetune_db_element *pElement = NULL;
      switch (livetune_db_element_json_register(&pDesigner->hDesignerDB, tElementDescription, pBuilderDef, plugin_interface, &pElement))
      {
        case ST_OK:
        case ST_NOT_IMPL:
          // ignore case of algo not linked
          break;
        default:
          ST_TRACE_ERROR("cannot register algo %s", pBuilderDef->pAlgoName);
          break;
      }

      pBuilderDef++;
    }
  }
  return 0;
}

/* Plugin declaration, notify that this element must be added to the plugin list*/
ST_PLUGIN_DECLARE(ALGO_PLUGIN_CB);


#endif
