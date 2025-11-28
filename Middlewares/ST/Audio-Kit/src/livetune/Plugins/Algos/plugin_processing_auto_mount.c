/**
******************************************************************************
* @file          plugin_processing_auto_mount.c
* @author        MCD Application Team
* @brief         register all standard algo, a standard algo is an algo that doesn't need specific implementation for livetune
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
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

static livetune_helper_builder_def *pAutoMountAlgo;

/* Declarations ----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */

/**
* @brief Event handler
*
* @param handle the instance handle
* @param evt  the event
* @param pparam  the opaque param
* @return ST_Result Error code
*/

static uint32_t plugin_common_event_cb(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param lParam)
{
  if (evt == ST_EVT_SYSTEM_SHUTDOWN)
  {
    if (pAutoMountAlgo)
    {
      st_os_mem_free(pAutoMountAlgo);
      pAutoMountAlgo = NULL;
    }

  }

  if (evt == ST_EVT_REGISTER_ELEMENTS)
  {
    livetune_instance                 *pDesigner = (livetune_instance *)hInst;
    uint32_t                           iMaxAlgo  = 0UL;
    uint32_t                           countAlgo = 0UL;
    static livetune_interface plugin_interface; // element instance
    const audio_factory_entry_t *pFactoryIterator;
    ALGO_FACTORY_GET_ENTRIES(pFactoryIterator);
    if (pFactoryIterator)
    {
      /* First record all standard algos */
      while (pFactoryIterator->pFactory)
      {
        ST_ASSERT(pFactoryIterator->pFactory->pCapabilities)
        if ((pFactoryIterator->pFactory->pCapabilities->misc.flags & AUDIO_ALGO_FLAGS_MISC_DISABLE_AUTO_MOUNT) == 0U)
        {
          if ((countAlgo <= iMaxAlgo) || (pAutoMountAlgo == NULL))
          {
            iMaxAlgo += 10UL;
            pAutoMountAlgo = st_os_mem_realloc(ST_Mem_Type_ANY_SLOW, pAutoMountAlgo, iMaxAlgo * sizeof(livetune_helper_builder_def));
            ST_ASSERT(pAutoMountAlgo);
          }
          memset(&pAutoMountAlgo[countAlgo], 0, sizeof(pAutoMountAlgo[0]));
          pAutoMountAlgo[countAlgo].pAlgoName = pFactoryIterator->pFactory->pCapabilities->pName;
          countAlgo++;
        }
        pFactoryIterator++;
      }
      /* Due to the realloc that create builder instances, we must register algos after the creation list*/

      for (uint32_t count = 0UL; count < countAlgo; count++)
      {
        livetune_helper_builder_def *pBuilderDef = &pAutoMountAlgo[count];
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
      }
    }
  }
  return 0;
}
/* plugin event declaration*/
ST_PLUGIN_DECLARE(plugin_common_event_cb);


#endif