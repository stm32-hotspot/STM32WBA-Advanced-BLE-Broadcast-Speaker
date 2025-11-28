/**
******************************************************************************
* @file          livetune_helper.c
* @author        MCD Application Team
* @brief         some common utilities
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
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "livetune_helper.h"


/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/




/**
* @brief Create an element instance  ie: Overload creation callbacks
*
* @param pElement the element handle
*/

void livetune_helper_create_element_instance(livetune_db_element *pElement)
{
  /* if a construction element is null, replace it by a default implementation */

  if (pElement->hInterface.constructor_cb == NULL)
  {
    pElement->hInterface.constructor_cb = livetune_helper_processing_constructor_default;
  }

  if (pElement->hInterface.destructor_cb == NULL)
  {
    pElement->hInterface.destructor_cb = livetune_helper_processing_destructor_default;
  }

  if (pElement->hInterface.parameter_change_cb == NULL)
  {
    pElement->hInterface.parameter_change_cb = livetune_helper_builder_parameter_change_cb_default;
  }
}



/**
* @brief Build the graph
*
* @param pHandle the instance handle
*/

int32_t livetune_helper_build_audio_chain(livetune_pipe *pHandle)
{
  /* before to start the graph, we need to build it and call all construction step */

  livetune_ac_wrapper_acPipeCreate(pHandle);

  /* all elements allocs its resources */
  int8_t bError = FALSE;
  if (livetune_pipe_notify(pHandle, LIVETUNE_PIPE_CONSTRUCTOR) != ST_BUILDER_OK)
  {
    bError = TRUE;
  }

  /* all elements init is internal Audio chain resources */

  livetune_builder_result ret = livetune_pipe_notify(pHandle, LIVETUNE_PIPE_START);
  if ((bError != FALSE) && (ret != ST_BUILDER_OK))
  {
    bError = TRUE;
  }

  /* all elements computes and create its config and chunk */
  ret = livetune_pipe_notify(pHandle, LIVETUNE_PIPE_PROLOG);

  if ((bError == FALSE) && (ret != ST_BUILDER_OK))
  {
    bError = TRUE;
  }

  /* optimization:  this phase could be omitted in the real construction,it allows to generate control function */
  //            if (!bError  && livetune_pipe_notify(pHandle, LIVETUNE_PIPE_PRE_INIT_GRAPH) != ST_BUILDER_OK)
  //            {
  //
  //              bError = TRUE;
  //            }

  ret = livetune_pipe_notify(pHandle, LIVETUNE_PIPE_POST_INIT_GRAPH);
  if ((bError == FALSE) && (ret != ST_BUILDER_OK))
  {
    bError = TRUE;
  }


  /* all elements  init and configure its element*/
  ret = livetune_pipe_notify(pHandle, LIVETUNE_PIPE_INIT_GRAPH_ELEMENT);
  if ((bError == FALSE) && (ret != ST_BUILDER_OK))
  {
    bError = TRUE;
  }

  /* all elements  attach its connections*/
  ret = livetune_pipe_notify(pHandle, LIVETUNE_PIPE_INIT_GRAPH_ATTACH_CNX);
  if ((bError == FALSE) && (ret != ST_BUILDER_OK))
  {
    bError = TRUE;
  }
  return bError;
}




/**
* @brief Convert a json encoded to true json string
*
* @param pRawStr json encoded pointer
* @param pStrJson json decoded pointer
* @param szStrJson max decoded  buffer
* @return TRUE or FALSE
*/

int32_t livetune_helper_string2json(const char_t *pRawStr, char_t *pStrJson, int32_t szStrJson)
{
  szStrJson--;
  while (((*pRawStr) != '\0') && ((szStrJson - 1) != 0))
  {
    if (*pRawStr == '\'')
    {
      *pStrJson = '\"';
    }
    else
    {
      *pStrJson = *pRawStr;
    }
    pStrJson++;
    pRawStr++;
    szStrJson--;
  }
  *pStrJson = '\0';
  return (szStrJson != 0) ? TRUE : FALSE;
}

/**
* @brief format a string in a scratch buffer
*
*/

char_t *livetune_helper_format(char_t *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);
  static char_t   tScratch[6][200];
  static uint32_t cptBuffer = 0;
  char_t         *pScratch  = tScratch[cptBuffer++ % 6U];
  vsnprintf(pScratch, sizeof(tScratch[1]), pFormat, args);
  va_end(args);
  return pScratch;
}


/**
* @brief format a string in a scratch buffer
*
*/

char_t *livetune_helper_format_float(double value)
{
  static char_t tScratch[50];
  snprintf(tScratch, sizeof(tScratch), "%f", value);
  char_t *pPeriod = strchr(tScratch, (int32_t)'.');
  char_t *pEnd    = strchr(tScratch, 0);
  if ((pPeriod != NULL) && (pEnd != NULL))
  {
    while (pEnd[-1] != '.')
    {
      if (pEnd[-1] != '\0')
      {
        break;
      }
      pEnd--;
    }
    if ((pEnd[-1] == '.') && (*pEnd == '\0'))
    {
      pEnd--;
    }
    *pEnd = '\0';
  }
  return tScratch;
}

/**
* @brief return the nb cnx on a pin def
*
* @param pHandle the instance handle
*/

uint32_t livetune_helper_cnx_nb(livetune_db_instance_pins_def **pDefList, uint32_t pinDef)
{
  uint32_t nbCnx = 0;
  ST_ASSERT(pDefList != NULL);
  livetune_db_list *pDef = livetune_db_instance_pin_def_get(pDefList, pinDef);
  if (pDef)
  {
    livetune_db_instance_cnx_list *pList = pDef->pObject;
    nbCnx                                   = (uint32_t)livetune_db_cnx_nb(&pList);
  }
  return nbCnx;
}


/**
* @brief Get a Connection instance
*
* @param pHandle the instance handle
* @param pinDef  pin def num
* @param indexCnx  Index connection
* @return NULL or the Instance
*/

livetune_db_instance_cnx *livetune_helper_cnx_get(livetune_db_instance_pins_def **pDefList, uint32_t pinDef, uint32_t indexCnx)
{
  livetune_db_instance_cnx      *pCnx = NULL;
  livetune_db_instance_pins_def *pDef = livetune_db_instance_pin_def_get(pDefList, pinDef);
  if (pDef)
  {
    livetune_db_instance_cnx_list *pList = pDef->pObject;
    pCnx                                    = livetune_db_cnx_get(&pList, indexCnx);
  }
  return pCnx;
}


/**
* @brief return the cnx index from its name
*
* @param pHandle the instance handles
* @param pin     pin
* @param pName   cnx name
*/

int32_t livetune_helper_cnx_find_index(livetune_db_instance_pins_def **pDef, uint32_t pin, char_t *pName)
{
  int32_t                           iCnx  = -1;
  livetune_db_instance_pins_def *pPin  = livetune_db_instance_pin_def_get(pDef, pin);
  livetune_db_instance_cnx_list *pList = pPin->pObject;
  uint32_t                          nbCnx = (uint32_t)livetune_db_cnx_nb(&pList);
  for (uint32_t indexCnx = 0; indexCnx < nbCnx; indexCnx++)
  {
    livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, indexCnx);
    if (pCnx)
    {
      if (strcmp(pCnx->pName, pName) == 0)
      {
        iCnx = (int32_t)indexCnx;
        break;
      }
    }
  }
  return iCnx;
}

/**
* @brief Copy Cnx in to Cnx out
*
* @param pHandle the instance handle
* @param pCnxInName  cnx name in
* @param pinDefIn  index pin in
* @param pCnxOutName  cnx name out
* @param pinDefOut  index pin out
* @return error code
*/
ST_Result livetune_helper_chunk_copy(livetune_db_instance *pInstance, uint32_t pinDefIn, char_t *pCnxInName, uint32_t pinDefOut, char_t *pCnxOutName, char_t *pChunkPrefix, int32_t typeOverWrite)
{
  ST_Result result = ST_ERROR;

  livetune_db_instance_cnx *pCnxIn = livetune_db_cnx_name_search(&pInstance->pPinIn, pCnxInName, pinDefIn);
  if (pCnxIn)
  {
    livetune_db_instance_cnx *pCnxOut = livetune_db_cnx_name_search(&pInstance->pPinOut, pCnxOutName, pinDefOut);
    if (pCnxOut)
    {
      /* the pin in is connected with an instance pin out and must have the same cnx name*/
      livetune_db_instance *pInstanceInDst = pCnxIn->pInstance;
      ST_ASSERT(pInstanceInDst != NULL); // must exist else corruption
      uint32_t pinDst = pCnxIn->iPinDef;
      /* pCnxOutDst is the connection reciprocal  */
      livetune_db_instance_cnx *pCnxOutDst = livetune_db_cnx_name_search(&pInstanceInDst->pPinOut, pCnxIn->pName, pinDst);
      if (pCnxOutDst) // must exist else corruption
      {
        livetune_ac_factory_copy_chunk(&pCnxOutDst->hAc.hConf, &pCnxOut->hAc.hConf, typeOverWrite);
        result = ST_OK;
      }
      else
      {
        ST_TRACE_ERROR("Topology corruption, %s id not found in the instance %s", pCnxOut->pName, pCnxIn->pInstance);
      }
    }
  }
  return result;
}

/**
* @brief By default, the chunk configuration copied on the out chunk, a mechanism allows to overwrite some parameters
*
* @param pHandle the instance handle
* @param pinIn  index pin in
* @param pinIn  index pin out
*/
void livetune_helper_chunk_propagation(livetune_db_instance *pInstance, uint32_t pinIn, uint32_t pinOut, char_t *pChunkPrefix, int32_t typeOverWrite)
{
  livetune_db_instance_pins_def *pPinIn = livetune_db_instance_pin_def_get(&pInstance->pPinIn, pinIn);
  if (pPinIn)
  {
    livetune_db_instance_cnx_list *pList = pPinIn->pObject;
    /* We must have only a pin in  to be able to propagate( limited by ConnectMax )*/
    uint32_t nbCnxIn = livetune_db_cnx_nb(&pList);
    if (nbCnxIn)
    {
      /* Get the single connection in */
      livetune_db_instance_cnx *pCnxIn = livetune_db_cnx_get(&pList, pinIn);
      ST_ASSERT(pCnxIn != NULL); /* must exist else corruption */
      /* get the pindef out connected to the algo in */
      livetune_db_instance_pins_def *pPinDefDstOut = livetune_db_instance_pin_def_get(&pCnxIn->pInstance->pPinOut, pCnxIn->iPinDef);
      ST_ASSERT(pPinDefDstOut != NULL); /* must exist else corruption*/
      /* we have now the connection out connected with our algo pin in */
      livetune_db_instance_cnx *pParentCnx = livetune_db_cnx_name_search(&pCnxIn->pInstance->pPinOut, pCnxIn->pName, pCnxIn->iPinDef);
      if (pParentCnx) /* must exist */
      {
        /* we have now the connection to propagate */
        livetune_db_instance_pins_def *pDef     = livetune_db_instance_pin_def_get(&pInstance->pPinOut, pinOut);
        livetune_db_instance_cnx_list *pList    = pDef->pObject;
        uint32_t                          nbCnxOut = (uint32_t)livetune_db_cnx_nb(&pList);

        /* copy the pin in on each cnx out */
        for (uint32_t indexCnxOut = 0; indexCnxOut < nbCnxOut; indexCnxOut++)
        {
          livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, indexCnxOut);
          livetune_ac_factory_copy_chunk(&pParentCnx->hAc.hConf, &pCnx->hAc.hConf, typeOverWrite);
        }
      }
      else
      {
        ST_TRACE_ERROR("Topology corruption, %s id not found in the instance %s", pCnxIn->pName, pCnxIn->pInstance);
      }
    }
    else
    {
      /*
      the pin in is not connected, this will cause an error during the graph construction
      but to make happy everybody we propagate and default chunk
      */
      livetune_db_instance_pins_def *pDef  = livetune_db_instance_pin_def_get(&pInstance->pPinOut, pinOut);
      livetune_db_instance_cnx_list *pList = pDef->pObject;

      uint32_t nbCnxOut = (uint32_t)livetune_db_cnx_nb(&pList);
      for (uint32_t indexCnxOut = 0; indexCnxOut < nbCnxOut; indexCnxOut++)
      {
        livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, indexCnxOut);
        if (pCnx->pInstance->hAc.bSysIO == 0U)
        {
          livetune_ac_factory_init_default(pInstance, &pCnx->hAc.hConf);
        }
      }
    }
  }
  else
  {
    /* if there is no pin in (ie something like a generator,we create an default chunk */
    livetune_db_instance_pins_def *pDef     = livetune_db_instance_pin_def_get(&pInstance->pPinOut, pinOut);
    livetune_db_instance_cnx_list *pList    = pDef->pObject;
    uint32_t                          nbCnxOut = (uint32_t)livetune_db_cnx_nb(&pList);
    for (uint32_t indexCnxOut = 0; indexCnxOut < nbCnxOut; indexCnxOut++)
    {
      livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, indexCnxOut);
      if (pCnx->pInstance->hAc.bSysIO == 0U)
      {
        livetune_ac_factory_init_default(pInstance, &pCnx->hAc.hConf);
      }
    }
  }
  livetune_ac_factory_apply_changes(pInstance, pinOut);
}


void livetune_helper_format_clear(char_t *pScratch)
{
  pScratch[0] = '\0';
}

void livetune_helper_format_tab(char_t *pScratch, int32_t size)
{
  char_t tTab[50];
  memset(tTab, (int32_t)' ', sizeof(tTab));
  tTab[size] = '\0';
  strcat(pScratch, tTab);
}


void livetune_helper_format_add(char_t *pScratch, uint32_t size, char_t *pFormat, ...)
{
  char_t  tTabResult[128];
  va_list args;
  va_start(args, pFormat);
  vsnprintf(tTabResult, sizeof(tTabResult), pFormat, args);
  va_end(args);
  char_t  *pTxt = strchr(tTabResult, 0);
  uint32_t sz   = strlen(tTabResult);
  while (sz < size)
  {
    *pTxt++ = ' ';
    sz++;
  }
  *pTxt = '\0';
  strcat(pScratch, tTabResult);
}
/**
* @brief Delete a connection chunk on each Cnx
*
* @param pHandle the instance handle
*/

void livetune_helper_make_c_name(char_t *pName)
{
  while (*pName)
  {
    if (isalnum((int32_t)*pName) == 0) /*cstat !MISRAC2012-Dir-4.11_h false positive, caused by the stdlib isalnum, we cannot fix it */
    {
      *pName = '_';
    }
    pName++;
  }
}

/**
* @brief return true if the instance has a control
*
* @param pHandle the pipe instance handles
* @return true or false
*/

int8_t livetune_helper_instance_has_control(livetune_db_instance *pInstance)
{
  int8_t                      bControl = FALSE;
  const audio_algo_factory_t *pFactory = livetune_ac_factory_get_factory_from_element(pInstance->pElements);
  ST_ASSERT(pFactory != NULL);
  if (pFactory->pControlTemplate)
  {
    bControl = TRUE;
  }
  return bControl;
}

/**
* @brief Append a pin in and generate its code
*
* @param pHandle the pipe instance handles
* @param pInstance the db instance
* @param pindef  pin in
*/
void livetune_helper_set_pin_in(livetune_helper_builder *pBuilder, uint32_t pinID, livetune_db_instance_cnx *pCnxIn)
{
  livetune_db_instance     *pInstanceDst = pCnxIn->pInstance;
  uint32_t                     indexPinDst  = pCnxIn->iPinDef;
  livetune_db_instance_cnx *pCnxOutDst   = livetune_db_cnx_name_search(&pInstanceDst->pPinOut, pCnxIn->pName, indexPinDst);
  ST_ASSERT(pCnxOutDst != NULL); // must exist else corrupted

  if ((strlen(pCnxOutDst->hAc.tVarName) != 0U) && (strlen(pBuilder->pInstance->hAc.tVarName) != 0U))
  {
    /* add the connection */
    if ((pBuilder->pInstance->hAc.bValid != 0U) && (pCnxOutDst->hAc.bValid != 0U))
    {
      livetune_ac_wrapper_acPipeConnectPinIn(pBuilder, pinID, pCnxOutDst->hAc.hChunk);
    }
    /* Generate the code */
    livetune_generate_connect_pins(pBuilder->pPipe, pBuilder->pInstance->hAc.tVarName, pinID, pCnxOutDst->hAc.tVarName, ST_GENERATOR_PIN_TYPE_IN);
  }
}

/**
* @brief Append a pin ou and generate its code
*
* @param pHandle the pipe instance handles
* @param pInstance the db instance
* @param pCnxOut   the cnx out
*/

void livetune_helper_set_pin_out(livetune_helper_builder *pBuilder, uint32_t pinID, livetune_db_instance_cnx *pCnxOut)
{
  if (strlen(pBuilder->pInstance->hAc.tVarName) != 0U)
  {
    /* add the connection */
    if ((pBuilder->pInstance->hAc.bValid != 0U) && (pCnxOut->hAc.bValid != 0U))
    {
      /* if the target element is a sysio, the chunk is already created and hard coded by AudioChainSysIOs_initOutxx , so by pass*/
      if (pCnxOut->pInstance->hAc.bSysIO == 0U)
      {
        livetune_ac_wrapper_acPipeConnectPinOut(pBuilder, pinID, pCnxOut->hAc.hChunk);
        /* Generate the code */
        livetune_generate_connect_pins(pBuilder->pPipe, pBuilder->pInstance->hAc.tVarName, pinID, pCnxOut->hAc.tVarName, ST_GENERATOR_PIN_TYPE_OUT);
      }
    }
  }
}



/**
*  @brief Init the algo config
* * @param pBuilder the pBuilder instance handle
*/

void livetune_helper_builder_create_algo(livetune_helper_builder *pBuilder)
{
  livetune_ac_wrapper_acAlgoCreate(pBuilder);
}


/**
*   @brief Init function state
* * @param pBuilder builder instance
*/

livetune_builder_result livetune_helper_builder_pre_init_graph_default(livetune_helper_builder *pBuilder)
{
  if (livetune_helper_instance_has_control(pBuilder->pInstance))
  {
    snprintf(pBuilder->sCbCtrlName, sizeof(pBuilder->sCbCtrlName), "cb_control_%s", pBuilder->pInstance->hAc.tVarName);
    livetune_helper_make_c_name(pBuilder->sCbCtrlName);
    livetune_generate_control_callback(pBuilder, pBuilder->sCbCtrlName, NULL);
  }
  return ST_BUILDER_OK;
}

/**
*   @brief Init function state
* * @param pBuilder builder instance
*/

livetune_builder_result livetune_helper_builder_post_init_graph_default(livetune_helper_builder *pBuilder)
{
  return ST_BUILDER_OK;
}


/* We must create the chunk instance, ie Create the handle and apply the propagation */

void livetune_helper_builder_create_chunk_handles(livetune_helper_builder *pBuilder)
{
  uint32_t nbPinOut = livetune_db_instance_pin_def_nb(&pBuilder->pInstance->pPinOut);
  for (uint32_t indexPinOut = 0; indexPinOut < nbPinOut; indexPinOut++)
  {
    uint32_t nbCnx = livetune_helper_cnx_nb(&pBuilder->pInstance->pPinOut, indexPinOut);
    if (nbCnx)
    {
      /* Generate code chunk handle out as statics on all out pins */
      if (livetune_ac_factory_is_acBus(pBuilder->pInstance, TRUE, indexPinOut))
      {
        livetune_generate_chunk_handles(pBuilder->pPipe, &pBuilder->pInstance->pPinOut, indexPinOut);
      }
    }
    for (uint32_t indexCnx = 0; indexCnx < nbCnx; indexCnx++)
    {
      livetune_db_instance_cnx *pCnx = livetune_helper_cnx_get(&pBuilder->pInstance->pPinOut, indexPinOut, indexCnx);
      ST_ASSERT(pCnx != NULL);
      if (pCnx->pInstance->hAc.bValid != 0U)
      {
        /* Propagate Pin in to out, notice this call works only of there a single in and out */
        livetune_helper_chunk_propagation(pBuilder->pInstance, pBuilder->pBuilderDef->pinPropagation, indexPinOut, pBuilder->pInstance->pInstanceName, -2);
      }
    }
  }
}



/**
*   @brief Init the global state
* * @param pBuilder builder instance
*/

livetune_builder_result livetune_helper_builder_pipe_prolog_default(livetune_helper_builder *pBuilder)
{
  if (pBuilder->pInstance->hAc.bValid)
  {
    /* Generate the global */
    livetune_generate_global(pBuilder->pPipe, "static acAlgo", pBuilder->pInstance->hAc.tVarName, livetune_helper_format("// %s handle ", pBuilder->pInstance->pDescription));
  }
  livetune_helper_builder_create_chunk_handles(pBuilder);
  return ST_BUILDER_OK;
}


/**
*   @brief Init the Pipe state
* * @param pBuilder builder instance
*/

void livetune_helper_builder_pipe_init(livetune_pipe *pPipe)
{
  if (pPipe->bGenerateCode)
  {
    // Create the pipe
    livetune_generate_fn_body(pPipe, "// Create the pipe", NULL);
    livetune_generate_fn_body(pPipe, "error = acPipeCreate(&hPipe);", NULL);
    livetune_generate_fn_body(pPipe, "assert(error == 0);", NULL);
  }
  else
  {
    livetune_ac_wrapper_acPipeCreate(pPipe);
  }
}




/**
*   @brief generate the element state
* * @param pBuilder builder instance
*/

livetune_builder_result livetune_helper_builder_pipe_init_graph_element_generate_default(livetune_helper_builder *pBuilder)
{
  /* generate config variable name */
  if (pBuilder->pPipe->bGenerateCode)
  {
    /* Instantiate the configuration */
    livetune_generate_fn_body(pBuilder->pPipe, "", NULL);
    livetune_generate_algo_create(pBuilder, NULL);
    livetune_generate_algo_config(pBuilder);
    if (livetune_helper_instance_has_control(pBuilder->pInstance))
    {
      livetune_generate_fn_body(pBuilder->pPipe, "", NULL);
      livetune_generate_algo_set_common_config_value(pBuilder, "userData", "\"MyCustomData\"");
      livetune_generate_algo_set_common_config_value(pBuilder, "controlCb", livetune_helper_format("(void *)&%s", pBuilder->sCbCtrlName));
    }
  }
  return ST_BUILDER_OK;
}



/**
*   @brief apply the element state
* * @param pBuilder builder instance
*/

livetune_builder_result livetune_helper_builder_pipe_init_graph_element_apply_default(livetune_helper_builder *pBuilder)
{
  /* Real algo Configuration  */
  /* Here is the real algo configuration, other aspects are most of time similar*/
  /* this piece of code configures the algo, mostly change the pAlgoCbs   and create statics and dynamics params. You must refer to the algo documentation. */

  if (pBuilder->pInstance->hAc.bValid)
  {
    livetune_helper_builder_create_algo(pBuilder);
    livetune_ac_factory_apply_algo_config_params(pBuilder);
  }

  return ST_BUILDER_OK;
}


/**
*   @brief apply the element state
* * @param pBuilder builder instance
*/

livetune_builder_result livetune_helper_builder_pipe_init_graph_element_chuck_default(livetune_helper_builder *pBuilder)
{
  /* Create real the instance & code generation, always the same calls */
  uint32_t nbPinOut = livetune_db_instance_pin_def_nb(&pBuilder->pInstance->pPinOut);
  for (uint32_t indexOut = 0; indexOut < nbPinOut; indexOut++)
  {
    if (livetune_ac_factory_is_acBus(pBuilder->pInstance, TRUE, indexOut))
    {
      livetune_ac_wrapper_acChunkCreate(pBuilder, &pBuilder->pInstance->pPinOut, indexOut);
      livetune_generate_create_chunks(pBuilder, &pBuilder->pInstance->pPinOut, indexOut);
    }
  }

  return ST_BUILDER_OK;
}




/**
*   @brief Init the element state
* * @param pBuilder builder instance
*/

livetune_builder_result livetune_helper_builder_pipe_init_graph_element_default(livetune_helper_builder *pBuilder)
{
  livetune_helper_builder_pipe_init_graph_element_chuck_default(pBuilder);
  livetune_helper_builder_pipe_init_graph_element_apply_default(pBuilder);
  livetune_helper_builder_pipe_init_graph_element_generate_default(pBuilder);

  return ST_BUILDER_OK;
}


/**
*   @brief Init the cnx state
* * @param pBuilder builder instance
*/

livetune_builder_result livetune_helper_builder_pipe_init_graph_attach_cnx_default(livetune_helper_builder *pBuilder)
{
  /* Most of the time, the attachment is always the same and supposes the same code */
  uint32_t iPinCountIn  = livetune_db_instance_pin_def_nb(&pBuilder->pInstance->pPinIn);
  uint32_t iPinCountOut = livetune_db_instance_pin_def_nb(&pBuilder->pInstance->pPinOut);
  for (uint32_t indexPinID = 0; indexPinID < iPinCountOut; indexPinID++)
  {
    if (livetune_ac_factory_is_acBus(pBuilder->pInstance, TRUE, indexPinID))
    {
      uint32_t nbCnxOut = (uint32_t)livetune_helper_cnx_nb(&pBuilder->pInstance->pPinOut, indexPinID);
      for (uint32_t IndexCnxID = 0; IndexCnxID < nbCnxOut; IndexCnxID++)
      {
        livetune_db_instance_cnx *pCnxOut = livetune_helper_cnx_get(&pBuilder->pInstance->pPinOut, indexPinID, IndexCnxID);
        ST_ASSERT(pCnxOut != NULL); /* must exist */
        livetune_helper_set_pin_out(pBuilder, indexPinID, pCnxOut);
      }
    }
  }


  for (uint32_t indexInID = 0; indexInID < iPinCountIn; indexInID++)
  {
    if (livetune_ac_factory_is_acBus(pBuilder->pInstance, FALSE, indexInID))
    {
      uint32_t nbCnxIn = livetune_helper_cnx_nb(&pBuilder->pInstance->pPinIn, indexInID);
      for (uint32_t IndexCnxID = 0; IndexCnxID < nbCnxIn; IndexCnxID++)
      {
        livetune_db_instance_cnx *pCnxIn = livetune_helper_cnx_get(&pBuilder->pInstance->pPinIn, indexInID, IndexCnxID);
        ST_ASSERT(pCnxIn != NULL); /* must exist */
        livetune_helper_set_pin_in(pBuilder, indexInID, pCnxIn);
      }
    }
  }
  return ST_BUILDER_OK;
}

/**
*   @brief Assume all Cnx is  valid and has a name
* * @param pInstance instance handle
*/

livetune_builder_result livetune_helper_set_default_attrb(livetune_db_instance *pInstance)
{
  /* Most of the time, the attachment is always the same and supposes the same code */
  uint32_t iPinCountIn  = livetune_db_instance_pin_def_nb(&pInstance->pPinIn);
  uint32_t iPinCountOut = livetune_db_instance_pin_def_nb(&pInstance->pPinOut);
  /* Names variable the algo */
  snprintf(pInstance->hAc.tVarName, sizeof(pInstance->hAc.tVarName), "h%c%s", toupper((int32_t)*pInstance->pInstanceName), &pInstance->pInstanceName[1]);
  livetune_helper_make_c_name(pInstance->hAc.tVarName);

  for (uint32_t indexIn = 0; indexIn < iPinCountIn; indexIn++)
  {
    livetune_db_instance_pins_def *pPin = livetune_db_instance_pin_def_get(&pInstance->pPinIn, indexIn);
    if (pPin)
    {
      livetune_db_instance_cnx_list *pList         = pPin->pObject;
      uint8_t                           bDefaultValid = FALSE;
      livetune_db_element_pin       *pElempPin     = livetune_db_list_get_object(&pInstance->pElements->pPinIn, indexIn);
      if (strcmp(pElempPin->pType, LIVETUNE_AC_BUS) == 0)
      {
        bDefaultValid = TRUE;
      }
      uint32_t nbCnxIn = livetune_db_cnx_nb(&pList);
      for (uint32_t indexCnx = 0; indexCnx < nbCnxIn; indexCnx++)
      {
        livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, indexCnx);
        if (pCnx)
        {
          pCnx->hAc.bValid = bDefaultValid;
          snprintf(pCnx->hAc.tVarName, sizeof(pCnx->hAc.tVarName), "hChunk_%s", pCnx->pName);
          livetune_helper_make_c_name(pCnx->hAc.tVarName);
        }
      }
    }
  }
  for (uint32_t indexOut = 0; indexOut < iPinCountOut; indexOut++)
  {
    livetune_db_instance_pins_def *pPin = livetune_db_instance_pin_def_get(&pInstance->pPinOut, indexOut);
    if (pPin)
    {
      uint8_t                     bDefaultValid = FALSE;
      livetune_db_element_pin *pElempPin     = livetune_db_list_get_object(&pInstance->pElements->pPinOut, indexOut);
      if (strcmp(pElempPin->pType, LIVETUNE_AC_BUS) == 0)
      {
        bDefaultValid = TRUE;
      }
      livetune_db_instance_cnx_list *pList   = pPin->pObject;
      uint32_t                          nbCnxIn = livetune_db_cnx_nb(&pList);
      for (uint32_t indexCnx = 0; indexCnx < nbCnxIn; indexCnx++)
      {
        livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, indexCnx);
        if (pCnx)
        {
          pCnx->hAc.bValid = bDefaultValid;
          snprintf(pCnx->hAc.tVarName, sizeof(pCnx->hAc.tVarName), "hChunk_%s", pCnx->pName);
          livetune_helper_make_c_name(pCnx->hAc.tVarName);
        }
      }
    }
  }
  return ST_BUILDER_OK;
}




/**
* @brief Class change , change a parameter when the pipe is alive
* @param pHandle the instance handle
* @param iIndex parameter index
* @param iType parameter type element,pinout ....
*/

void livetune_helper_builder_parameter_change_cb_default(livetune_db_instance *pInstance, livetune_info_param typeParam)
{
  /* Check if the pipe is active */
  if (livetune_pipe_is_started(&livetune_get_instance()->hAudioPipe))
  {
    if ((pInstance != NULL) && (pInstance->hAc.bValid != 0U))
    {
      if (typeParam.byField.infoType == ST_INFO_TYPE_ELEMENT)
      {
        const char_t *pStrValue = LIVETUNE_STRING(pInstance, typeParam.byField.indexParam);
        const char_t *pStrName  = LIVETUNE_NAME(pInstance, typeParam.byField.indexParam);
        /* we  need to translate the contents for control using key values */
        const audio_descriptor_param_t *pParam = livetune_ac_factory_get_algo_config_param_descriptor(pInstance->pBuilderData, pStrName);
        if (pParam)
        {
          char_t *pTranslated = AudioChainJson_factory_translate_key_value(pParam, pStrValue);
          /* if the update type is 0 ( default) we send a refresh parameters, if the type is 1, we update the database only , the refresh will comes later */
          livetune_ac_wrapper_acAlgoSetConfig(pInstance->hAc.hAlgo, pStrName, pTranslated, (typeParam.byField.updateType == 0U) ? TRUE : FALSE);
          st_os_mem_free(pTranslated);
        }
        else
        {
          // Some parameters could not have factory : ST_TRACE_ERROR("Key Name not found");
        }
      }
      else
      {
        /* if we change a Bus parameter, we need to stop the graph and rebuild it */
        livetune_pipe_stop(pInstance->pPipe);
      }
    }
  }
}


/**
*   @brief Init the header state
* * @param pBuilder builder instance
*/

livetune_builder_result livetune_helper_builder_pipe_start_default(livetune_helper_builder *pBuilder)
{
  /* assume the algo is valid by default */
  memset(&pBuilder->pInstance->hAc, 0, sizeof(pBuilder->pInstance->hAc));
  livetune_helper_set_default_attrb(pBuilder->pInstance);
  pBuilder->pInstance->hAc.bValid = TRUE;
  return ST_BUILDER_OK;
}

/**
* @brief free the builder instance
*
* @param pBuilder Instance handle
* @return Error code
*/

ST_Result livetune_helper_builder_delete(st_class_element_instance *pCls)
{
  ST_Result                result    = ST_OK;
  if (pCls)
  {
    livetune_db_instance *pInstance = pCls->hBuilder.pInstance;
    st_os_mem_free(pCls);
    pInstance->pUserData = NULL;
  }

  return result;
}


/**
* @brief Class construction, allocate local memory etc...
*       This function is called before the pipeline start
* * @param pHandle the instance handle
*/

void livetune_helper_processing_constructor_default(livetune_db_instance *pInstance)
{
  pInstance->pUserData = livetune_ac_factory_builder_create(pInstance, 0);
  ST_ASSERT(pInstance->pUserData != NULL);
}


/**
* @brief Class destruction , free all resources allocated in the construction
*       This function is called before the pipeline stop
* * @param pHandle the instance handle
*/


void livetune_helper_processing_destructor_default(livetune_db_instance *pInstance)
{
  /* a destructor could be null, if a pipeline has been created and doesn't go to the end */
  if (pInstance->pUserData)
  {
    ST_VERIFY(livetune_helper_builder_delete(pInstance->pUserData) == ST_OK);
  }
}


