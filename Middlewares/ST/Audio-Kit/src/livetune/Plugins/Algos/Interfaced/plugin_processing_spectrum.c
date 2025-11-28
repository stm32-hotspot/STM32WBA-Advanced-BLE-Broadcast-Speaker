/**
******************************************************************************
* @file          plugin_processing_spectrum.c
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
/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */
#include "livetune.h"
#include "livetune_helper.h"
#include "stm32_usart.h"

/* Private defines -----------------------------------------------------------*/


#define ALGO_PLUGIN_CB plugin_spectrum_event_cb
/*
{
  "PinDefOut":[{"Type":"acGraph","Name":"Grph","Description":"Must be connected to a Graph viewer"},{"Type":"acMsg","Name":"Msg","Description":"Must be connected to a Message viewer"}],
   "Params":[
      {
         "Description":"Spectrum Refresh time in ms",
         "Type":0,
         "Name":"Refresh",
         "Default":300,
         "Min":30,
         "Max":1000,
         "Control":"slider"
      },
      {
         "Description":"compute Y axis in DB rather than magnitude",
         "Type":0,
         "Name":"useDb",
         "Default":0,
         "Control":"checkbox"
      },
      {
         "Description":"Min and max will be cumulated rather than reset for each measure",
         "Type":0,
         "Name":"adaptMinMax",
         "Default":0,
         "Control":"checkbox"
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
  kPinOutGraph,
  kPinOutMsg,
};




/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

static const livetune_helper_builder_def hBuilderDef =
{
  .pAlgoName = "spectrum"
};


typedef struct st_plugin_element_ext
{
  uint32_t                    iTimeTarget;
  char_t                      sCbName[LIVETUNE_VAR_NAME_MAX];
  livetune_db_instance_cnx   *pConnectedGraph;
  livetune_db_instance_cnx   *pConnectedMsg;
  livetune_db_instance_cnx   *pConnectedIn;
  livetune_helper_builder_def hBuilderDef; /* copy because we modify the cbs */
  float_t                    *pSquareMag;
  float_t                     normalizationCoef;
  uint32_t                    szMag;
  uint32_t                    nbChannels;
  uint32_t                    mainBusFs;
  uint32_t                    fftLength;
  uint32_t                    nbBands;
  float_t                     minX, maxX;
  float_t                     minY, maxY;
  int32_t                     indexRefresh;
  int32_t                     indexDbSwitch;
  int32_t                     indexadaptMiniMaxSwitch;
  uint32_t                    nbSent;
  float_t                     uartMsByChar;
  const char_t               *pUnityY;
} st_plugin_element_ext;

/* Declarations ----------------------------------------------------------*/
static void                    plugin_processing_constructor(livetune_db_instance *pInstance);
static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd);

/* Global variables ----------------------------------------------------------*/

static char_t             tElementDescription[] = "{\"PinDefOut\":[{\"Type\":\"acGraph\",\"Name\":\"Grph\",\"Description\":\"Must be connected to a Graph viewer\"},{\"Type\":\"acMsg\",\"Name\":\"Msg\",\"Description\":\"Must be connected to a Message viewer\"}],\"Params\":[{\"Description\":\"Spectrum Refresh time in ms\",\"Type\":0,\"Name\":\"Refresh\",\"Default\":300,\"Min\":30,\"Max\":1000,\"Control\":\"slider\"},{\"Description\":\"compute Y axis in DB rather than magnitude\",\"Type\":0,\"Name\":\"useDb\",\"Default\":0,\"Control\":\"checkbox\"},{\"Description\":\"Min and max will be cumuled rather than reset for each measure\",\"Type\":0,\"Name\":\"adaptMinMax\",\"Default\":0,\"Control\":\"checkbox\"}]}";
static livetune_interface plugin_interface =  // element instance
{
  .constructor_cb      = plugin_processing_constructor,
  .pipe_builder_cb     = plugin_processing_builder
};


/* Private functions ------------------------------------------------------- */


/**
* @brief then the spectrum result as graph
* @param pCls the class instance handle
*/


static void plugin_processing_control_spectrum_idle_graph(st_class_element_instance *pCls)
{
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
  if (pClsExt->pConnectedGraph)
  {
    static char_t  tScratch[200];
    static int32_t count = 0;

    /* build extra json fields to send */
    char_t  *pUnityX = "hz";
    uint32_t szPkt   = pClsExt->nbChannels * pClsExt->nbBands * sizeof(float);
    snprintf(tScratch, sizeof(tScratch), "\"cpt\":%d,\"Len\":%d,\"Type\":\"f32\",\"Ch\":%d,\"MinX\":\"%f\",\"MaxX\":\"%f\",\"MinY\":\"%f\",\"MaxY\":\"%f\",\"UX\":\"%s\",\"UY\":\"%s\",\"To\":\"%s\"", count++, szPkt, pClsExt->nbChannels, pClsExt->minX, pClsExt->maxX, pClsExt->minY, pClsExt->maxY, pUnityX, pClsExt->pUnityY, pClsExt->pConnectedGraph->pInstance->pInstanceName);
    /* send the message using DMA*/
    pClsExt->nbSent += livetune_send_block_binary_async("acGraph", "Spectrum", tScratch, (uint8_t *)pClsExt->pSquareMag, szPkt);
  }
}

/**
* @brief then the spectrum result as row text
* @param pCls the class instance handle
*/


static void plugin_processing_control_spectrum_idle_Text(st_class_element_instance *pCls)
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
        for (uint32_t indexBand = 0; indexBand < pClsExt->nbBands; indexBand++)
        {
          pText += sprintf(pText, ";%.4f", pClsExt->pSquareMag[pos++]);

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
        strcpy(pText, "\n");
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
* @brief Compute the final spectrum result according to params
* @param pCls the class instance handle
*/


static int32_t plugin_processing_control_compute_spectrum(st_class_element_instance *pCls)
{
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);

  int32_t bToogleMagDb        = LIVETUNE_INT(pCls->hBuilder.pInstance, pClsExt->indexDbSwitch);
  int32_t badaptMiniMaxSwitch = LIVETUNE_INT(pCls->hBuilder.pInstance, pClsExt->indexadaptMiniMaxSwitch);
  float_t minX                = 0.0f;
  float_t maxX                = (float)pClsExt->mainBusFs / 2.0f;
  float_t minY;
  float_t maxY;

  pClsExt->nbBands            = (pClsExt->fftLength / 2U) + 1U;
  pClsExt->szMag              = pClsExt->nbChannels * pClsExt->nbBands;

  if (bToogleMagDb)
  {
    // convert squared magnitude in dB
    pClsExt->pUnityY = "dB";

    /* clamp min to 0.0000000001f (-100 dB) to avoid log(0) issue */
    float minSquareMag = 0.0000000001f / pClsExt->normalizationCoef;
    for (uint32_t i = 0; i < pClsExt->szMag; i++)
    {
      if (pClsExt->pSquareMag[i] < minSquareMag)
      {
        pClsExt->pSquareMag[i] = minSquareMag;
      }
    }
    /* convert squared magnitude in dB: 10 * log10(x * normalizationCoef) = 10 / ln(10) * (ln(x) + ln(normalizationCoef)) with 10 / ln(10) = 4.342944819 */
    arm_vlog_f32(pClsExt->pSquareMag, pClsExt->pSquareMag, pClsExt->szMag);
    arm_offset_f32(pClsExt->pSquareMag, logf(pClsExt->normalizationCoef), pClsExt->pSquareMag, pClsExt->szMag); /*cstat !MISRAC2012-Dir-4.11_a !MISRAC2012-Rule-22.8 the check is done before de pass the value */
    arm_scale_f32(pClsExt->pSquareMag, 4.342944819f, pClsExt->pSquareMag, pClsExt->szMag);
  }
  else
  {
    pClsExt->pUnityY = "Mag";

    /* linear case: simply divide by normalizationCoef and take square root */
    arm_scale_f32(pClsExt->pSquareMag, pClsExt->normalizationCoef, pClsExt->pSquareMag, pClsExt->szMag);
    //arm_vsqrt_f32(pCls->pSquareMag,                          pClsExt->pSquareMag, pCls->szMag);   // arm_vsqrt_f32 doesn't link => use arm_sqrt_f32 instead
    for (uint32_t i = 0; i < pClsExt->szMag; i++)
    {
      arm_sqrt_f32(pClsExt->pSquareMag[i], &pClsExt->pSquareMag[i]);
    }
  }

  minY = pClsExt->pSquareMag[0];
  maxY = pClsExt->pSquareMag[0];
  for (uint32_t i = 1; i < pClsExt->szMag; i++)
  {
    if (pClsExt->pSquareMag[i] < minY)
    {
      minY = pClsExt->pSquareMag[i];
    }
    if (pClsExt->pSquareMag[i] > maxY)
    {
      maxY = pClsExt->pSquareMag[i];
    }
  }

  if (badaptMiniMaxSwitch)
  {
    if (minX < pClsExt->minX)
    {
      pClsExt->minX = minX;
    }
    if (maxX > pClsExt->maxX)
    {
      pClsExt->maxX = maxX;
    }
    if (minY < pClsExt->minY)
    {
      pClsExt->minY = minY;
    }
    if (maxY > pClsExt->maxY)
    {
      pClsExt->maxY = maxY;
    }
  }
  else
  {
    pClsExt->minX = minX;
    pClsExt->maxX = maxX;
    pClsExt->minY = minY;
    pClsExt->maxY = maxY;
  }

  return 1;
}

/**
* @brief return true if the time is over
* @param pCls the class instance handle
*/

static uint8_t plugin_processing_check_time(st_class_element_instance *pCls)
{
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
  uint8_t                bResult = FALSE;
  if (pClsExt->iTimeTarget == 0U)
  {
    pClsExt->iTimeTarget = (uint32_t)st_os_sys_time();
  }
  uint32_t ticks = (uint32_t)st_os_sys_time();
  if (ticks > pClsExt->iTimeTarget)
  {
    pClsExt->iTimeTarget = ticks + (uint32_t)LIVETUNE_INT(pCls->hBuilder.pInstance, pClsExt->indexRefresh);
    bResult              = TRUE;
  }
  return bResult;
}


static int32_t plugin_processing_control_spectrum_cb(acAlgo hAlgo)
{
  st_class_element_instance *pCls = NULL;
  acAlgoGetCommonConfig(hAlgo, "userData", &pCls);
  ST_ASSERT(pCls != NULL);
  st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);

  uint8_t elapsed = plugin_processing_check_time(pCls);

  if ((hAlgo != NULL) && (elapsed != 0U))
  {
    pClsExt->nbSent = 0;
    LIVETUNE_DISABLE_IRQ();
    // float array with the magnitude of each band, nbBands = ((fftLength / 2) + 1)
    ST_VERIFY(acAlgoGetControl(hAlgo, "pSquareMag", &pClsExt->pSquareMag) == 0);
    ST_VERIFY(acAlgoGetControl(hAlgo, "normalizationCoef", &pClsExt->normalizationCoef) == 0); // for averaging squared magnitude
    ST_VERIFY(acAlgoGetControl(hAlgo, "nbChannels", &pClsExt->nbChannels) == 0);
    ST_VERIFY(acAlgoGetControl(hAlgo, "fs", &pClsExt->mainBusFs) == 0);
    ST_VERIFY(acAlgoGetControl(hAlgo, "fftLength", &pClsExt->fftLength) == 0);
    ST_VERIFY(acAlgoSetControl(hAlgo, "nbAccSpectrum", 0) == 0); // to acknowledge spectrum reading
    LIVETUNE_ENABLE_IRQ();

    plugin_processing_control_compute_spectrum(pCls);
    plugin_processing_control_spectrum_idle_Text(pCls);
    plugin_processing_control_spectrum_idle_graph(pCls);

    /* To get an accurate refresh rate, we need to add the time taken by the packet transmission itself */
    float    nbSendTime   = (float)pClsExt->nbSent * pClsExt->uartMsByChar;
    uint32_t extraDelayMs = (uint32_t)nbSendTime;
    pClsExt->iTimeTarget += extraDelayMs;
  }
  return 1;
}



/**
* @brief Builder overload
* @param pHandle the instance handle
* @param cmd the event
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
        livetune_ac_wrapper_acAlgoSetCommonConfig(&pCls->hBuilder, "controlCb", (void *)plugin_processing_control_spectrum_cb); /*cstat !MISRAC2012-Rule-11.1 cast a callback in pointer is mandatory for this API */
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
    pClsExt->pConnectedGraph           = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOutGraph, 0);
    pClsExt->pConnectedMsg             = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinOut, (uint32_t)kPinOutMsg, 0);
    livetune_db_instance_cnx *pCnx  = livetune_helper_cnx_get(&pCls->hBuilder.pInstance->pPinIn, (uint32_t)kPinInSrc, 0);
    if (pCnx)
    {
      pClsExt->pConnectedIn = livetune_db_cnx_name_search(&pCnx->pInstance->pPinOut, pCnx->pName, pCnx->iPinDef);
    }

    /* some params are hardcoded, we need to know the real position after json update */
    pClsExt->indexRefresh            = livetune_db_param_find_from_name(&pInstance->pElements->pParams, "Refresh");
    pClsExt->indexDbSwitch           = livetune_db_param_find_from_name(&pInstance->pElements->pParams, "useDb");
    pClsExt->indexadaptMiniMaxSwitch = livetune_db_param_find_from_name(&pInstance->pElements->pParams, "adaptMinMax");
    pClsExt->uartMsByChar            = 1000.0F / ((float)UTIL_UART_GetHdle()->Init.BaudRate / 8.0F);
  }
  else
  {
    ST_TRACE_ERROR("Memory alloc %d", sizeof(st_class_element_instance));
  }
}



/**
 * @brief nr  Event handler
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
