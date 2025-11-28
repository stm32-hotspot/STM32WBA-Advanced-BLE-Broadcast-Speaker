/**
******************************************************************************
* @file          plugin_processing_$(ALGO_GROUP).c
* @author        MCD Application Team
* @brief         instantiate a packet of algos
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
#include "livetune.h"
#include "livetune_helper.h"

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* All following algos are simple and doesn't require Controls and specific parameter managements, we can register them by packet */

static const livetune_helper_builder_def tCommonAlgo[] =
{
  $(ALGO_NAME_LIST)
  {0}
};
/* Declarations ----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/

/* interface use only default behaviour */

static livetune_interface plugin_interface =
{
  .constructor_cb = livetune_helper_processing_constructor_default,
  .destructor_cb  = livetune_helper_processing_destructor_default,
  .parameter_change_cb = livetune_helper_builder_parameter_change_cb_default,
  .pipe_builder_cb     = NULL,

};

/* Private functions ------------------------------------------------------- */

/**
* @brief Event handler
*
* @param handle the instance handle
* @param evt  the event
* @param wparam  the opaque param
* @param lparam  the opaque param
* @return ST_Result Error code
*/

static uint32_t plugin_$(ALGO_GROUP)_event_cb(ST_Handle hInst, uint32_t evt, st_message_param wParam, st_message_param lParam)
{

  if (evt == ST_EVT_REGISTER_ELEMENTS)
  {
    /* register the elements*/
    livetune_instance  *pDesigner = (livetune_instance *)hInst;
    const livetune_helper_builder_def *pBuilderDef = tCommonAlgo;
    while (pBuilderDef->pAlgoName)
    {
      /* register the element, the element json will be generated on fly using the pBuilderDef factory */
      livetune_db_element *pElement = NULL;
      switch (livetune_db_element_json_register(&pDesigner->hDesignerDB, NULL, pBuilderDef, plugin_interface, &pElement))
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

/* plugin event declaration*/

ST_PLUGIN_DECLARE(plugin_$(ALGO_GROUP)_event_cb);

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
