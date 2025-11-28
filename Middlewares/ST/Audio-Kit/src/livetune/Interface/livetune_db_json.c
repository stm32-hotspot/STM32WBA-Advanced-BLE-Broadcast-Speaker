/**
******************************************************************************
* @file          livetune_db_json.c
* @author        MCD Application Team
* @brief         manage the db as json
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

#include <string.h>
#include <math.h>
#include <limits.h>
#include "livetune_db_json.h"
#include "st_flash_storage.h"

/* Global variables ----------------------------------------------------------*/
st_flash_storage_type gCurrent_json = st_flash_storage_SAVE_JSON1; /* global Json ID to be used */
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */


/**
* @brief Parse parameters
         parameters could be placed on the element our on the connection
*
* @return error code
*/
static void livetune_db_json_parameters_parse(livetune_db *pHandle, json_instance_t *pJsonInst, jsonID hParams, livetune_db_instance *pInstance, livetune_info_param paramType, livetune_db_list **pParams, livetune_db_list **pElemParam)
{
  uint16_t count;
  /* get the parameter count */
  json_list_get_count(pJsonInst, hParams, &count);
  for (int32_t indexParam = 0; (indexParam < (int32_t)count) && (pHandle->bParsingError == 0U); indexParam++)
  {
    jsonID valueParamID;
    json_list_pair(pJsonInst, hParams, (uint16_t)indexParam, NULL, &valueParamID);
    if (valueParamID)
    {
      const char_t *pParamValue = "0.0";
      const char_t *pTmpString  = NULL;
      const char_t *pParamID    = NULL;
      int32_t       iParam      = indexParam;

      /* we need to manage the fact that parameters can change its position */

      json_object_get_string(pJsonInst, valueParamID, "", "Name", &pTmpString);
      if (pTmpString != NULL)
      {
        int32_t iPos = livetune_db_param_find_from_name(pElemParam, pTmpString);
        if (iPos != -1)
        {
          iParam = iPos;
        }
      }
      /* a parameter is composed by a Name and a Value*/

      livetune_db_element_params *pElmParam = livetune_db_list_get_object(pElemParam, (uint32_t)iParam);
      livetune_db_instance_param *pInsParam = livetune_db_list_get_object(pParams, (uint32_t)iParam);
      if ((pInsParam != NULL) && (pElmParam != NULL))
      {
        /* both must exist */
        ST_ASSERT(pInsParam != NULL);
        ST_ASSERT(pElmParam != NULL);

        pInsParam->pName = pElmParam->pName;
        json_object_get_string(pJsonInst, valueParamID, "", "Value", &pParamValue);
        if (pParamValue == NULL)
        {
          ST_TRACE_ERROR("Parameter %s not found, replace by default", pParamID);
          pParamValue = pElmParam->pParamDefault;
        }
        paramType.byField.indexParam = (uint8_t)iParam;
        paramType.byField.updateType = pInstance->iUpdateType;
        livetune_db_param_set(pHandle, pInstance, paramType, pInsParam, pParamValue);
      }
    }
  }
}

/**
* @brief Dump parameters
         parameters could be placed on the element our on the connection
*
* @return error code
*/
static int32_t livetune_db_json_parameter_dump(livetune_db_instance *pInstance, json_instance_t *pJsonInstance, jsonID hParams, livetune_db_list **pParams, livetune_db_list **pElemParam)
{
  int32_t result   = TRUE;
  int32_t nbParams = (int32_t)livetune_db_list_nb(pParams);

  for (int32_t indexParam = 0; indexParam < nbParams; indexParam++)
  {
    jsonID hObjParam;
    ST_VERIFY(json_create_object(pJsonInstance, &hObjParam) == JSON_OK);
    ST_VERIFY(json_array_append_new(pJsonInstance, hParams, hObjParam) == JSON_OK);
    livetune_db_instance_param *pParam = livetune_db_list_get_object(pParams, (uint32_t)indexParam);

    char_t *pValue = pParam->pParamValue;
    /* generate a Name and a value attribute */
    const char_t *pName = pParam->pName;
    if (pName)
    {
      ST_VERIFY(json_object_set_string(pJsonInstance, hObjParam, "", "Name", pName) == JSON_OK);
    }

    if (pValue)
    {
      ST_VERIFY(json_object_set_string(pJsonInstance, hObjParam, "", "Value", pValue) == JSON_OK);
    }
  }
  return result;
}



/**
* @brief Designer Update for removing

*
* @param pHandle      the instance handle
* @param pJsonInst    the json instance and root
* @param nodeID       the root node

* @return error code
*/
static ST_Result livetune_db_json_update_metas(livetune_db *pHandle, json_instance_t *pJsonInst, jsonID nodeID)
{
  ST_Result     result = ST_OK;
  jsonID        hInstance;
  const char_t *pTmpString;

  /* Meta save extra parameters used only by the PC tool */

  json_object_get_string(pJsonInst, nodeID, "", "Meta", &pTmpString);
  if (pTmpString)
  {
    st_os_mem_update_string(&pHandle->pMeta, pTmpString);
  }
  /* list of instances */
  json_object_get_id_from_tree(pJsonInst, nodeID, "Instances", &hInstance);
  if (hInstance)
  {
    ST_ASSERT(json_is_array(pJsonInst, hInstance) == JSON_TRUE);
    uint16_t nbInstance;
    json_list_get_count(pJsonInst, hInstance, &nbInstance);
    if (nbInstance)
    {
      for (uint32_t indexInst = 0; indexInst < nbInstance; indexInst++)
      {
        /* parse the instance */
        jsonID keyID;
        ST_VERIFY(json_list_pair(pJsonInst, hInstance, (uint16_t)indexInst, NULL, &keyID) == JSON_OK);
        json_object_get_string(pJsonInst, keyID, "", "InstanceName", &pTmpString);
        if (pTmpString)
        {
          livetune_db_instance *pInstance = livetune_db_instance_find_from_name(pHandle, pTmpString);
          if (pInstance)
          {
            /* Meta saves parameters used by the PC tool */
            json_object_get_string(pJsonInst, keyID, "", "Meta", &pTmpString);
            if (pTmpString)
            {
              st_os_mem_update_string(&pInstance->pMeta, pTmpString);
            }
          }
          else
          {
            /*no created yet */
          }
        }
      }
    }
  }
  return result;
}



/**
* @brief Designer Update for removing
         this message removes instance and  connection
*
* @param pHandle      the instance handle
* @param pJsonInst    the json instance and root
* @param nodeID       the root node

* @return error code
*/
static ST_Result livetune_db_json_update_delete(livetune_db *pHandle, json_instance_t *pJsonInst, jsonID nodeID)
{
  ST_Result result = ST_OK;
  jsonID    hInstance;
  json_object_get_id_from_tree(pJsonInst, nodeID, "Instances", &hInstance);
  if (hInstance)
  {
    ST_ASSERT(json_is_array(pJsonInst, hInstance) == JSON_TRUE);
    uint16_t nbInstance;
    json_list_get_count(pJsonInst, hInstance, &nbInstance);
    if (nbInstance)
    {
      for (uint32_t indexInst = 0; indexInst < nbInstance; indexInst++)
      {
        jsonID keyID;
        ST_VERIFY(json_list_pair(pJsonInst, hInstance, (uint16_t)indexInst, NULL, &keyID) == JSON_OK);
        const char_t *pInstanceName;
        json_object_get_string(pJsonInst, keyID, "", "InstanceName", &pInstanceName);
        if (pInstanceName)
        {
          if (livetune_db_instance_delete_from_name(pHandle, pInstanceName) != ST_OK)
          {
            ST_TRACE_ERROR("Delete instance %s", pInstanceName);
            result = ST_ERROR;
          }
        }
      }
    }
  }

  jsonID hConnections;
  json_object_get_id_from_tree(pJsonInst, nodeID, "Connections", &hConnections);
  if (hConnections)
  {
    ST_ASSERT(json_is_array(pJsonInst, hConnections) == JSON_TRUE);
    uint16_t nbItem;
    json_list_get_count(pJsonInst, hConnections, &nbItem);
    if (nbItem)
    {
      for (uint32_t indexItem = 0; indexItem < nbItem; indexItem++)
      {
        jsonID keyID;
        ST_VERIFY(json_list_pair(pJsonInst, hConnections, (uint16_t)indexItem, NULL, &keyID) == JSON_OK);
        const char_t *pInstanceNameOut;
        const char_t *pInstanceNameIn;
        uint64_t      pinIn;
        uint64_t      pinOut;
        json_object_get_string(pJsonInst, keyID, "", "InstanceNameOut", &pInstanceNameOut);
        json_object_get_string(pJsonInst, keyID, "", "InstanceNameIn",  &pInstanceNameIn);

        if (json_object_get_unsigned_integer(pJsonInst, keyID, "", "PinIn", &pinIn) != JSON_OK)
        {
          pinIn = LLONG_MAX;
        }

        if (json_object_get_unsigned_integer(pJsonInst, keyID, "", "PinOut", &pinOut) != JSON_OK)
        {
          pinOut = LLONG_MAX;
        }
        if ((pInstanceNameOut != NULL) && (pInstanceNameIn != NULL) && (pinIn != ULLONG_MAX) && (pinOut != ULLONG_MAX))
        {
          if (livetune_db_cnx_delete(pHandle, pInstanceNameOut, (uint32_t)pinOut, pInstanceNameIn, (uint32_t)pinIn) != ST_OK)
          {
            ST_TRACE_DEBUG("connection already delete %s: %s:%lld", pinIn, pInstanceNameIn, pinIn);
            result = ST_ERROR;
          }
        }
      }
    }
  }
  return result;
}

/**
* @brief Designer Update for Creation
*
* @param pHandle      the instance handle
* @param pJsonInst    the json instance and root
* @param nodeID       the root node

* @return error code
*/
static ST_Result livetune_db_json_update_create(livetune_db *pHandle, json_instance_t *pJsonInst, jsonID nodeID)
{
  ST_Result result = ST_OK;
  jsonID    hInstance;
  json_object_get_id_from_tree(pJsonInst, nodeID, "Instances", &hInstance);
  jsonID hConnections;
  json_object_get_id_from_tree(pJsonInst, nodeID, "Connections", &hConnections);
  if (hInstance)
  {
    ST_ASSERT(json_is_array(pJsonInst, hInstance) == JSON_TRUE);
    uint16_t nbInstance;
    json_list_get_count(pJsonInst, hInstance, &nbInstance);
    if (nbInstance)
    {
      for (uint32_t indexInst = 0; indexInst < nbInstance; indexInst++)
      {
        jsonID keyID;
        ST_VERIFY(json_list_pair(pJsonInst, hInstance, (uint16_t)indexInst, NULL, &keyID) == JSON_OK);
        const char_t *pInstanceName;
        const char_t *pRefElement;
        const char_t *pDescription;

        json_object_get_string(pJsonInst, keyID, "", "InstanceName", &pInstanceName);
        json_object_get_string(pJsonInst, keyID, "", "RefElement", &pRefElement);
        json_object_get_string(pJsonInst, keyID, "", "Description", &pDescription);
        if ((pInstanceName != NULL) && (pRefElement != NULL))
        {
          livetune_db_instance *pInstance = livetune_db_instance_create(pHandle, pInstanceName, pRefElement); /*cstat !MISRAC2012-Rule-22.1_a false positif, the API allocated a memory and return the block to the client */
          if (pInstance == NULL)
          {
            ST_TRACE_ERROR("Create instance %s:%s", pInstanceName, pRefElement);
            result = ST_ERROR;
          }
          if ((pInstance != NULL) && (pDescription != NULL))
          {
            st_os_mem_update_string(&pInstance->pDescription, pDescription);
          }
        }
      }
    }
  }
  if (hConnections)
  {
    ST_ASSERT(json_is_array(pJsonInst, hConnections) == JSON_TRUE);
    uint16_t nbItem;
    json_list_get_count(pJsonInst, hConnections, &nbItem);
    if (nbItem)
    {
      for (uint16_t indexItem = 0; indexItem < nbItem; indexItem++)
      {
        jsonID keyID;
        ST_VERIFY(json_list_pair(pJsonInst, hConnections, indexItem, NULL, &keyID) == JSON_OK);
        const char_t *pInstanceNameOut;
        const char_t *pInstanceNameIn;
        const char_t *pCnxName;


        uint64_t pinIn;
        uint64_t pinOut;
        jsonID   hIn;
        jsonID   hOut;

        ST_VERIFY(json_object_get_id_from_tree(pJsonInst, keyID, "In", &hIn) == JSON_OK);
        ST_VERIFY(json_object_get_id_from_tree(pJsonInst, keyID, "Out", &hOut) == JSON_OK);

        json_object_get_string(pJsonInst, hOut, "", "InstanceName", &pInstanceNameOut);
        json_object_get_string(pJsonInst, keyID, "", "Name", &pCnxName);
        json_object_get_string(pJsonInst, hIn, "", "InstanceName", &pInstanceNameIn);
        if (json_object_get_unsigned_integer(pJsonInst, hIn, "", "PinDef", &pinIn) != JSON_OK)
        {
          pinIn = LLONG_MAX;
        }
        if (json_object_get_unsigned_integer(pJsonInst, hOut, "", "PinDef", &pinOut) != JSON_OK)
        {
          pinOut = LLONG_MAX;
        }

        if ((pInstanceNameOut != NULL) && (pInstanceNameIn != NULL) && (pinIn != ULLONG_MAX))
        {
          if (livetune_db_cnx_create(pHandle, pInstanceNameOut, (uint32_t)pinOut, pInstanceNameIn, (uint32_t)pinIn) != ST_OK)
          {
            ST_TRACE_ERROR("Create connection %s %s:%lld", pInstanceNameOut, pInstanceNameIn, pinIn);
            result = ST_ERROR;
          }
          else
          {
            livetune_db_instance *pInstanceOut = livetune_db_instance_find_from_name(pHandle, pInstanceNameOut);
            ST_ASSERT(pInstanceOut != NULL);
            livetune_db_instance_cnx *pCnxOut = livetune_db_cnx_find(&pInstanceOut->pPinOut, (uint32_t)pinOut, pInstanceNameIn, (uint32_t)pinIn);
            if (pCnxOut)
            {
              st_os_mem_update_string(&pCnxOut->pName, pCnxName);
            }

            livetune_db_instance *pInstanceIn = livetune_db_instance_find_from_name(pHandle, pInstanceNameIn);
            ST_ASSERT(pInstanceIn != NULL);
            livetune_db_instance_cnx *pCnxIn = livetune_db_cnx_find(&pInstanceIn->pPinIn, (uint32_t)pinIn, pInstanceNameOut, (uint32_t)pinOut);
            if (pCnxIn)
            {
              st_os_mem_update_string(&pCnxIn->pName, pCnxName);
            }
            jsonID hParam;
            json_object_get_id_from_tree(pJsonInst, hIn, "Params", &hParam);
            if (hParam)
            {
              livetune_db_instance *pInstance = livetune_db_instance_find_from_name(pHandle, pInstanceNameIn);
              if (pInstance)
              {
                livetune_db_instance_cnx *pCnx = livetune_db_cnx_name_search(&pInstance->pPinIn, pCnxName, (uint32_t)pinIn);
                ST_ASSERT(pCnx != NULL); // must exist  else corruption
                livetune_db_element_pin *pElemPinDef = livetune_db_list_get_object(&pInstance->pElements->pPinIn, (uint32_t)pinIn);
                ST_ASSERT(pElemPinDef != NULL);

                livetune_info_param paramType;
                paramType.byField.indexCnx    = (uint8_t)livetune_db_cnx_name_search_index(&pInstance->pPinIn, pCnxName, (uint32_t)pinIn);
                paramType.byField.indexPinDef = (uint8_t)pinIn;
                paramType.byField.infoType    = ST_INFO_TYPE_PININ;
                livetune_db_json_parameters_parse(pHandle, pJsonInst, hParam, pInstance, paramType, &pCnx->pParams, &pElemPinDef->pParams);
              }
            }
            json_object_get_id_from_tree(pJsonInst, hOut, "Params", &hParam);
            if (hParam)
            {
              livetune_db_instance *pInstance = livetune_db_instance_find_from_name(pHandle, pInstanceNameOut);
              if (pInstance)
              {
                livetune_db_instance_cnx *pCnx = livetune_db_cnx_name_search(&pInstance->pPinOut, pCnxName, (uint32_t)pinOut);
                ST_ASSERT(pCnx != NULL); // must exist  else corruption
                livetune_db_element_pin *pElemPinDef = livetune_db_list_get_object(&pInstance->pElements->pPinOut, (uint32_t)pinOut);
                ST_ASSERT(pElemPinDef != NULL);
                livetune_info_param paramType;
                paramType.byField.indexCnx    = (uint8_t)livetune_db_cnx_name_search_index(&pInstance->pPinOut, pCnxName, (uint32_t)pinOut);
                paramType.byField.indexPinDef = (uint8_t)pinOut;
                paramType.byField.infoType    = ST_INFO_TYPE_PINOUT;
                livetune_db_json_parameters_parse(pHandle, pJsonInst, hParam, pInstance, paramType, &pCnx->pParams, &pElemPinDef->pParams);
              }
            }
          }
        }
      }
    }
  }

  return result;
}


/**
* @brief Designer Update for Parameters

*
* @param pHandle      the instance handle
* @param pJsonInst    the json instance and root
* @param nodeID       the root node

* @return error code
*/

static ST_Result livetune_db_json_update_instance_params(livetune_db *pHandle, json_instance_t *pJsonInst, jsonID nodeID)
{
  ST_Result     result = ST_OK;
  const char_t *pTmpString;

  json_object_get_string(pJsonInst, nodeID, "", "Meta", &pTmpString);
  if (pTmpString)
  {
    st_os_mem_update_string(&pHandle->pMeta, pTmpString);
  }
  jsonID hInstance;
  json_object_get_id_from_tree(pJsonInst, nodeID, "Instances", &hInstance);
  if (hInstance)
  {
    uint16_t count;
    json_list_get_count(pJsonInst, hInstance, &count);
    for (uint16_t indexInst = 0; (indexInst < count) && (pHandle->bParsingError == 0U); indexInst++)
    {
      jsonID valueID;
      json_list_pair(pJsonInst, hInstance, indexInst, NULL, &valueID);
      ST_ASSERT(valueID != JSON_ID_NULL);
      /* type is optional */
      int64_t type = 0;
      json_object_get_integer(pJsonInst, valueID, "", "Type", &type);
      /* record some attributes, some can be omitted  */
      json_object_get_string(pJsonInst, valueID, "", "InstanceName", &pTmpString);
      if (pTmpString == NULL)
      {
        pHandle->bParsingError = TRUE;
        continue;
      }
      livetune_db_instance *pInstance = livetune_db_instance_find_from_name(pHandle, pTmpString);
      if (pInstance == NULL)
      {
        pHandle->bParsingError = TRUE;
        continue;
      }
      livetune_db_instance_set_update_type(pInstance, (uint8_t)type);
      json_object_get_string(pJsonInst, hInstance, "", "Meta", &pTmpString);
      if (pTmpString)
      {
        st_os_mem_update_string(&pHandle->pMeta, pTmpString);
      }
      json_object_get_string(pJsonInst, valueID, "", "Description", &pTmpString);
      if (pTmpString)
      {
        st_os_mem_update_string(&pInstance->pDescription, pTmpString);
      }
      /* Init the parameters value, if omitted we can take the default one */
      jsonID hParams;
      json_array_get(pJsonInst, valueID, "Params", &hParams);
      if (hParams)
      {
        livetune_info_param paramType;
        paramType.byValue          = 0;
        paramType.byField.infoType = ST_INFO_TYPE_ELEMENT;
        /* will set pHandle->bParsingError */
        livetune_db_json_parameters_parse(pHandle, pJsonInst, hParams, pInstance, paramType, &pInstance->pParams, &pInstance->pElements->pParams);
      }
    }
  }
  if (pHandle->bParsingError)
  {
    result = ST_ERROR;
  }

  return result;
}



/**
* @brief Designer Update connection for Parameters

*
* @param pHandle      the instance handle
* @param pJsonInst    the json instance and root
* @param nodeID       the root node

* @return error code
*/

static ST_Result livetune_db_json_update_cnx_params(livetune_db *pHandle, json_instance_t *pJsonInst, jsonID nodeID)
{
  ST_Result result = ST_OK;
  jsonID    hInstance;
  json_object_get_id_from_tree(pJsonInst, nodeID, "Connections", &hInstance);
  if (hInstance)
  {
    uint16_t count;
    json_list_get_count(pJsonInst, hInstance, &count);
    for (uint16_t indexInst = 0; (indexInst < count) && (pHandle->bParsingError == 0U); indexInst++)
    {
      jsonID        valueID;
      const char_t *pCnxName      = NULL;
      const char_t *pCnxType      = NULL;
      const char_t *pInstanceName = NULL;
      int64_t       iPinDef       = 0;

      json_list_pair(pJsonInst, hInstance, indexInst, NULL, &valueID);
      ST_ASSERT(valueID != JSON_ID_NULL);

      json_object_get_string(pJsonInst, valueID, "", "InstanceName", &pInstanceName);
      if (pInstanceName == NULL)
      {
        pHandle->bParsingError = TRUE;
        continue;
      }
      if (json_object_get_integer(pJsonInst, valueID, "", "PinDef", &iPinDef) != JSON_OK)
      {
        pHandle->bParsingError = TRUE;
        continue;
      }
      json_object_get_string(pJsonInst, valueID, "", "CnxName", &pCnxName);
      if (pCnxName == NULL)
      {
        pHandle->bParsingError = TRUE;
        continue;
      }
      json_object_get_string(pJsonInst, valueID, "", "Type", &pCnxType);
      if (pCnxType == NULL)
      {
        pHandle->bParsingError = TRUE;
        continue;
      }
      livetune_db_instance_cnx *pInstCnx = NULL;
      livetune_db_element_pin  *pElemCnx = NULL;
      livetune_info_param       paramType;
      paramType.byField.indexPinDef      = (uint8_t)iPinDef;
      livetune_db_instance *pInstance = livetune_db_instance_find_from_name(pHandle, pInstanceName);
      if (pInstance == NULL)
      {
        pHandle->bParsingError = TRUE;
        continue;
      }
      if (strcmp(pCnxType, "Out") == 0)
      {
        pInstCnx                   = livetune_db_cnx_name_search(&pInstance->pPinOut, pCnxName, (uint32_t)iPinDef);
        paramType.byField.indexCnx = (uint8_t)livetune_db_cnx_name_search_index(&pInstance->pPinOut, pCnxName, (uint32_t)iPinDef);
        pElemCnx                   = livetune_db_list_get_object(&pInstance->pElements->pPinOut, (uint32_t)iPinDef);
        paramType.byField.infoType = ST_INFO_TYPE_PINOUT;
      }
      if (strcmp(pCnxType, "In") == 0)
      {
        pInstCnx                   = livetune_db_cnx_name_search(&pInstance->pPinIn, pCnxName, (uint32_t)iPinDef);
        paramType.byField.indexCnx = (uint8_t)livetune_db_cnx_name_search_index(&pInstance->pPinIn, pCnxName, (uint32_t)iPinDef);
        pElemCnx                   = livetune_db_list_get_object(&pInstance->pElements->pPinIn, (uint32_t)iPinDef);
        paramType.byField.infoType = ST_INFO_TYPE_PININ;
      }

      /* Init the parameters value, if omitted we can take the default one */
      jsonID hParams;
      json_array_get(pJsonInst, valueID, "Params", &hParams);
      if (hParams)
      {
        livetune_db_json_parameters_parse(pHandle, pJsonInst, hParams, pInstance, paramType, &pInstCnx->pParams, &pElemCnx->pParams);
      }
    }
  }
  if (pHandle->bParsingError)
  {
    result = ST_ERROR;
  }

  return result;
}

/**
* @brief update instances
the json string is formatted to update different  graph elements
the syntax is:
{"delete":{"Instances":[{"InstanceName":"usb-0"}],"Connections":[{"InstanceNameOut":"speaker","InstanceNameIn":"usb-0","PinDefIn":0}],"create":{"Instances":[{"InstanceName":"usb-0","RefElement":"micro"}],"Connections":[{"InstanceNameOut":"speaker","InstanceNameIn":"merge","PinDefIn":0}],"update":{"Instances":[{"InstanceName":"usb-0","Params":[{" Name ":"Type of capture"," Value ":1}]}]}}}}
*
* @param pHandle      the instance handle
* @param pJsonInst    the jsion instance and root
* @return error code
*/

static ST_Result livetune_db_json_update_instances(livetune_db *pHandle, json_instance_t *pJsonInst)
{
  ST_Result result = ST_OK;
  jsonID    hCreate;
  json_object_get_id_from_tree(pJsonInst, pJsonInst->pack_root, "create", &hCreate);
  if (hCreate)
  {
    livetune_db_cmd(ST_PIPE_PUSH_STOP, ST_PIPE_REASON_CREATE);
    result = livetune_db_json_update_create(pHandle, pJsonInst, hCreate);
    livetune_db_cmd(ST_PIPE_POP, ST_PIPE_REASON_CREATE);
    if (result != ST_OK)
    {
      ST_TRACE_ERROR("Update Create");
    }
  }
  jsonID hDelete;
  json_object_get_id_from_tree(pJsonInst, pJsonInst->pack_root, "delete", &hDelete);
  if (hDelete)
  {
    livetune_db_cmd(ST_PIPE_PUSH_STOP, ST_PIPE_REASON_DELETE);
    result = livetune_db_json_update_delete(pHandle, pJsonInst, hDelete);
    livetune_db_cmd(ST_PIPE_POP, ST_PIPE_REASON_DELETE);
    if (result != ST_OK)
    {
      ST_TRACE_ERROR("Update Delete");
    }
  }
  jsonID hUpdate;
  json_object_get_id_from_tree(pJsonInst, pJsonInst->pack_root, "update", &hUpdate);
  if (hUpdate)
  {
    livetune_db_cmd(ST_PIPE_PUSH_STOP, ST_PIPE_REASON_UPDATE);

    /* the is a marker sent by the host to manage update specificities, this parameter is optional the default is 0 */
    result = livetune_db_json_update_instance_params(pHandle, pJsonInst, hUpdate);
    if (result != ST_OK)
    {
      ST_TRACE_ERROR("Update instance params");
    }
    result = livetune_db_json_update_cnx_params(pHandle, pJsonInst, hUpdate);
    if (result != ST_OK)
    {
      ST_TRACE_ERROR("Update cnx params");
    }
    livetune_db_cmd(ST_PIPE_POP, ST_PIPE_REASON_UPDATE);
  }
  jsonID hMeta;
  json_object_get_id_from_tree(pJsonInst, pJsonInst->pack_root, "metas", &hMeta);
  if (hMeta)
  {
    result = livetune_db_json_update_metas(pHandle, pJsonInst, hMeta);
    if (result != ST_OK)
    {
      ST_TRACE_ERROR("Update metas");
    }
  }
  return result;
}

/**
* @brief Create all instances without connections from  json
*
* @param pHandle      the instance handle
* @param pJsonInst    the jsion instance and root
* @return error code
*/

static ST_Result livetune_db_json_parse_json_instances(livetune_db *pHandle, json_instance_t *pJsonInst)
{
  ST_Result     result = ST_OK;
  const char_t *pTmpString;
  /* Parse the common infos */
  /* it is not an error to omit the name */
  json_object_get_string(pJsonInst, pJsonInst->pack_root, "", "Meta", &pTmpString);
  if (pTmpString)
  {
    st_os_mem_update_string(&pHandle->pMeta, pTmpString);
  }
  /* Parse the Instances array */
  jsonID hInstance;
  json_array_get(pJsonInst, pJsonInst->pack_root, "Instances", &hInstance);
  if (hInstance)
  {
    if (json_is_array(pJsonInst, hInstance) == JSON_FALSE)
    {
      pHandle->bParsingError = TRUE;
    }
    else
    {
      uint16_t count;
      json_list_get_count(pJsonInst, hInstance, &count);
      for (uint16_t indexInst = 0; (indexInst < count) && (pHandle->bParsingError == 0U); indexInst++)
      {
        const char_t *pRefName;
        const char_t *pInstanceName;

        jsonID valueID;
        json_list_pair(pJsonInst, hInstance, indexInst, NULL, &valueID);
        ST_ASSERT(valueID != JSON_ID_NULL);

        /* record some attributes, some can be omitted  */

        json_object_get_string(pJsonInst, valueID, "", "InstanceName", &pInstanceName);
        if (pInstanceName == NULL)
        {
          pHandle->bParsingError = TRUE;
          continue;
        }
        ST_TRACE_DEBUG("%02d:Create Instance %s", indexInst, pInstanceName);
        json_object_get_string(pJsonInst, valueID, "", "RefElement", &pRefName);
        if (pRefName == NULL)
        {
          pHandle->bParsingError = TRUE;
          continue;
        }

        livetune_db_instance *pInstance = livetune_db_instance_create(pHandle, pInstanceName, pRefName); /*cstat !MISRAC2012-Rule-22.1_a false positif, the API allocated a memory and return the block to the client */
        if (pInstance == NULL)
        {
          /* instance is not valid, probably the reference element is missing  */
          continue;
        }
        /* reset instance */

        json_object_get_string(pJsonInst, valueID, "", "Description", &pTmpString);
        st_os_mem_update_string(&pInstance->pDescription, pTmpString);

        json_object_get_string(pJsonInst, valueID, "", "Meta", &pTmpString);
        st_os_mem_update_string(&pInstance->pMeta, pTmpString);
        jsonID hParams;
        json_array_get(pJsonInst, valueID, "Params", &hParams);
        if (hParams)
        {
          livetune_info_param paramType;
          paramType.byValue          = 0;
          paramType.byField.infoType = ST_INFO_TYPE_ELEMENT;
          /* will set !pHandle->bParsingError */
          livetune_db_json_parameters_parse(pHandle, pJsonInst, hParams, pInstance, paramType, &pInstance->pParams, &pInstance->pElements->pParams);
        }
      }
    }
  }
  if (pHandle->bParsingError)
  {
    livetune_db_instance_cleanup(pHandle);
    result = ST_ERROR;
  }
  return result;
}

/**
* @brief  Create a link between pins from json

*
* @param pHandle      the instance handle
* @param pJsonInst    the jsion instance
* @param hInstance    the jsion root
* @param pArray        the array name
* @return instance count
*/

static void livetune_db_json_parse_json_connected(livetune_db *pHandle, livetune_db_instance *pSrcInstance, uint8_t bIsCnxOut, json_instance_t *pJsonInst, jsonID hInstance, const char_t *pArray)
{
  if ((pSrcInstance != NULL) && (pHandle->bParsingError == 0U))
  {
    while (pHandle->bParsingError == 0U)
    {
      /* check if there are some connections */
      jsonID hPinDefID;
      json_array_get(pJsonInst, hInstance, pArray, &hPinDefID);
      if (hPinDefID)
      {
        if (json_is_array(pJsonInst, hPinDefID) == JSON_FALSE)
        {
          pHandle->bParsingError = TRUE;
          continue;
        }

        /* Iterate in the Pin Definition*/
        uint16_t count;
        json_list_get_count(pJsonInst, hPinDefID, &count);
        for (uint16_t indexPinDef = 0; (indexPinDef < count) && (pHandle->bParsingError == 0U); indexPinDef++)
        {
          const char_t *pInstanceName = NULL;
          uint64_t      pinDst        = 0;
          uint64_t      pinSrc        = 0;
          jsonID        pinID;
          json_list_pair(pJsonInst, hPinDefID, indexPinDef, NULL, &pinID);

          /* each PinDef has an array Cnx */
          jsonID idCnx;
          json_array_get(pJsonInst, pinID, "Cnx", &idCnx);
          if (idCnx == JSON_ID_NULL)
          {
            ST_TRACE_ERROR("PinDef has no Cnx");
            pHandle->bParsingError = TRUE;
            continue;
          }

          uint16_t cnxCount;
          json_list_get_count(pJsonInst, idCnx, &cnxCount);
          for (uint16_t indexCnx = 0; (indexCnx < cnxCount) && (pHandle->bParsingError == 0U); indexCnx++)
          {
            const char_t *pCnxName = NULL;
            jsonID        cnxObjID;
            json_list_pair(pJsonInst, idCnx, indexCnx, NULL, &cnxObjID);

            /* extract the destination instance name */
            json_object_get_string(pJsonInst, cnxObjID, "", "InstanceNameDst", &pInstanceName);
            if (pInstanceName == NULL)
            {
              /* the cnx is empty */
              continue;
            }
            /* convert instance name as instance pointer */
            livetune_db_instance *pInstanceDst = livetune_db_instance_find_from_name(pHandle, pInstanceName);
            if (pInstanceDst == NULL)
            {
              ST_TRACE_ERROR("Instance not found: %s", pInstanceName);
              pHandle->bParsingError = TRUE;
              continue;
            }
            json_object_get_string(pJsonInst, cnxObjID, "", "Name", &pCnxName);

            /* extract the destination pin */
            if (json_object_get_unsigned_integer(pJsonInst, cnxObjID, "", "PinDst", &pinDst) != JSON_OK)
            {
              ST_TRACE_ERROR("Pin error Cnx:%s", pCnxName);
              pHandle->bParsingError = TRUE;
              continue;
            }

            livetune_db_instance_pins_def  *pDef     = NULL;
            livetune_db_instance_pins_def **ppDef    = NULL;
            livetune_db_element_pin        *pElemDef = NULL;
            livetune_info_param             paramType;
            paramType.byField.indexCnx    = (uint8_t)indexCnx;
            paramType.byField.indexPinDef = (uint8_t)indexPinDef;
            if (bIsCnxOut)
            {
              /* if the connection is an  out ,the destination is an connected to a pin in*/
              ppDef                      = &pSrcInstance->pPinOut;
              pElemDef                   = livetune_db_list_get_object(&pSrcInstance->pElements->pPinOut, indexPinDef);
              paramType.byField.infoType = ST_INFO_TYPE_PINOUT;
            }
            else
            {
              /* if the connection is an In ,the destination is an connected to a pin out*/
              ppDef                      = &pSrcInstance->pPinIn;
              pElemDef                   = livetune_db_list_get_object(&pSrcInstance->pElements->pPinIn, indexPinDef);
              paramType.byField.infoType = ST_INFO_TYPE_PININ;
            }
            if (pElemDef == NULL)
            {
              ST_TRACE_ERROR("pElemDef not found ");
              pHandle->bParsingError = TRUE;
              continue;
            }

            pDef = livetune_db_instance_pin_def_get(ppDef, indexPinDef);
            if (pDef == NULL)
            {
              ST_TRACE_ERROR("Pin def");
              pHandle->bParsingError = TRUE;
              continue;
            }
            livetune_db_instance_cnx_list **pListCnx      = (livetune_db_instance_cnx_list **)&pDef->pCnxList;
            livetune_db_instance_cnx_list  *pListAttached = livetune_db_cnx_attach(pListCnx, pInstanceDst, (uint32_t)pinDst, pElemDef); /*cstat !MISRAC2012-Rule-22.1_a false positif, the API allocated a memory and return the block to the client */
            if (pListAttached == NULL)
            {
              ST_TRACE_ERROR("Create connection %s:%lld %s:%lld", pSrcInstance->pInstanceName, pinSrc, pInstanceDst->pInstanceName, pinDst);
              pHandle->bParsingError = TRUE;
              continue;
            }
            st_os_mem_update_string(&pListAttached->pCnxInstance->pName, pCnxName);
            /* update param connection */
            jsonID hParams;
            json_array_get(pJsonInst, cnxObjID, "Params", &hParams);
            if (hParams)
            {
              livetune_db_instance_cnx_list *pList = pDef->pObject;
              int32_t                           index = livetune_db_cnx_find_from_name(ppDef, indexPinDef, (const char_t *)pInstanceName, (uint32_t)pinDst);
              if (index >= 0)
              {
                livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, (uint32_t)index);
                if (pCnx)
                {
                  /* will set !pHandle->bParsingError */
                  livetune_db_json_parameters_parse(pHandle, pJsonInst, hParams, pSrcInstance, paramType, &pCnx->pParams, &pElemDef->pParams);
                }
              }
            }
          }
        }
      }
      break;
    }
  }
}



/**
* @brief  Re-Create all connections

*
* @param pHandle      the instance handle
* @param pJsonInst    the jsion instance and root
* @return error code
*/

static ST_Result livetune_db_json_parse_json_connections(livetune_db *pHandle, json_instance_t *pJsonInst)
{
  ST_Result result = ST_ERROR;
  jsonID    hInstance;
  /* Parse the Instances array */
  json_array_get(pJsonInst, pJsonInst->pack_root, "Instances", &hInstance);
  if (hInstance)
  {
    uint16_t count;
    json_list_get_count(pJsonInst, hInstance, &count);
    /* scan all instances in the json */
    for (uint16_t indexInst = 0; (indexInst < count) && (pHandle->bParsingError == 0U); indexInst++)
    {
      livetune_db_instance *pInstance = livetune_db_list_get_object(&pHandle->pInstances, indexInst);
      jsonID                   valueInstanceID;
      json_list_pair(pJsonInst, hInstance, indexInst, NULL, &valueInstanceID);
      if (valueInstanceID == JSON_ID_NULL)
      {
        pHandle->bParsingError = TRUE;
        continue;
      }
      /* the instance has the same pins number that the element */
      /* now load connection in and out in the instance */
      livetune_db_json_parse_json_connected(pHandle, pInstance, FALSE, pJsonInst, valueInstanceID, "PinDefIn");
      livetune_db_json_parse_json_connected(pHandle, pInstance, TRUE, pJsonInst, valueInstanceID, "PinDefOut");
    }
  }
  if (pHandle->bParsingError == 0U)
  {
    result = ST_OK;
  }
  else
  {
    ST_TRACE_ERROR("Connection link invalid");
  }
  return result;
}

/**
* @brief  parse the json parameters and register them in the class
*
* @param pJsonInst    the json handle
* @param hRoot        the json root
* @param pParams      param instance
* @param pNbParams    nb param read
* @return error code
*/

static ST_Result livetune_db_json_register_params(json_instance_t *pJsonInst, jsonID hRoot, livetune_db_list **pParams)
{
  const char_t *pTmpString;
  ST_Result     error = ST_OK;


  uint16_t count;
  json_list_get_count(pJsonInst, hRoot, &count);
  for (uint16_t a = 0; (a < count) && (error == ST_OK); a++)
  {
    jsonID valueID;
    json_list_pair(pJsonInst, hRoot, a, NULL, &valueID);
    ST_ASSERT(valueID != JSON_ID_NULL);
    livetune_db_element_params *pParam = st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(livetune_db_element_params));
    if (pParam == NULL)
    {
      ST_TRACE_ERROR("Param Alloc");
      error = ST_ERROR;
      break;
    }
    memset(pParam, 0, sizeof(*pParam));
    uint64_t tmpVal;
    if (json_object_get_unsigned_integer(pJsonInst, valueID, "", "Type", &tmpVal) == JSON_OK)
    {
      pParam->iType = (uint32_t)tmpVal;
    }
    json_object_get_string(pJsonInst, valueID, "", "Name", &pTmpString);
    if (pTmpString)
    {
      st_os_mem_update_string(&pParam->pName, pTmpString);
    }

    /* check if the param name is unique, other wise the system doesn't work */
    int32_t index = livetune_db_param_find_from_name(pParams, pTmpString);
    if (index != -1)
    {
      ST_TRACE_ERROR("Parameter name not unique %s", pTmpString);
      ST_ASSERT(0);
    }
    json_object_get_string(pJsonInst, valueID, "", "Default", &pTmpString);
    if (pTmpString != NULL)
    {
      st_os_mem_update_string(&pParam->pParamDefault, pTmpString);
    }
    pParam->iIndex = a;

    if (livetune_db_list_create(pParams, pParam) == NULL)
    {
      st_os_mem_update_string(&pParam->pParamDefault, NULL);
      st_os_mem_free(pParam);
      ST_TRACE_ERROR("Param list Alloc");
      error = ST_ERROR;
      break;
    }
  }
  return error;
}


/**
* @brief  parse the json and register a pindef
*
* @param pHandle      the db instance
* @param pElement     the element instance
* @param pJsonInst    the json instance
* @param root         the json root id
* @param bType        the type
* @return error code
*/


static ST_Result livetune_db_json_register_pindef(livetune_db *pHandle, livetune_db_element *pElement, json_instance_t *pJsonInst, jsonID root, uint8_t bType)
{
  const char_t *pTmpString;
  ST_Result     result = ST_OK;
  uint16_t      count;
  json_list_get_count(pJsonInst, root, &count);
  for (uint16_t a = 0; (a < count) && (result == ST_OK); a++)
  {
    jsonID valueID;
    json_list_pair(pJsonInst, root, a, NULL, &valueID);
    ST_ASSERT(valueID != JSON_ID_NULL);
    if (json_is_object(pJsonInst, valueID))
    {
      livetune_db_list        *pElemCreated = NULL;
      livetune_db_element_pin *pPins        = st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(livetune_db_element_pin));

      if (bType)
      {
        if (pPins)
        {
          pElemCreated = livetune_db_list_create(&pElement->pPinOut, pPins); /*cstat !MISRAC2012-Rule-22.1_a false positif, the API allocated and add it to a list the block will be freed later */
        }
        if ((pPins == NULL) || (pElemCreated == NULL))
        {
          if (pPins)
          {
            st_os_mem_free(pPins);
            result = ST_ERROR;
            continue;
          }
        }
      }
      else
      {
        if (pPins)
        {
          pElemCreated = livetune_db_list_create(&pElement->pPinIn, pPins); /*cstat !MISRAC2012-Rule-22.1_a false positif, the API allocated and add it to a list the block will be freed later */
        }
        if ((pPins == NULL) || (pElemCreated == NULL))
        {
          if (pPins)
          {
            st_os_mem_free(pPins);
            result = ST_ERROR;
            continue;
          }
        }
      }
      memset(pPins, 0, sizeof(*pPins));
      /* default is wav */
      json_object_get_string(pJsonInst, valueID, "", "Type", &pTmpString);
      ST_ASSERT(pTmpString != NULL);
      if (pTmpString)
      {
        st_os_mem_update_string(&pPins->pType, pTmpString);
      }

      /* optional */
      json_object_get_string(pJsonInst, valueID, "", "Name", &pTmpString);
      if (pTmpString)
      {
        st_os_mem_update_string(&pPins->pPinName, pTmpString);
      }

      jsonID hParams = 0;
      json_array_get(pJsonInst, valueID, "Params", &hParams);
      if (hParams)
      {
        result = livetune_db_json_register_params(pJsonInst, hParams, &pPins->pParams);
      }
    }
  }
  return result;
}


/**
* @brief register an audio rendering element
*
* @param pHandle the instance handle
* @param pJson  the json description or null
* @param pRefBuilder  the reference builder or null
* @param interface The process callback
* @param ppElement pointer to created element pointer
* @return error code
*/

ST_Result livetune_db_element_json_register(livetune_db *pHandle, const char_t *pJsonDesc, const void *pRefBuilder, livetune_interface interface, livetune_db_element **ppElement)
{
  ST_Result result = ST_ERROR;
  if (pJsonDesc == NULL)
  {
    pJsonDesc = "{}";
  }

  livetune_db_element *pElementCreated = livetune_db_element_create(pHandle, pJsonDesc, pRefBuilder, interface); /*cstat !MISRAC2012-Rule-22.1_a false positif, the API allocated and add it to a list the block will be freed later */
  livetune_db_element *pElement = pElementCreated;
  if (pElement)
  {
    json_instance_t jsonInst = {0};
    jsonID          rootID;
    const char_t   *pTmpString;
    json_load(&jsonInst, pJsonDesc, &rootID);
    ST_ASSERT(rootID != JSON_ID_NULL);
    if (rootID)
    {
      /* Parse the common infos */
      if (pHandle->cbRegisterUpdate)
      {
        pHandle->cbRegisterUpdate(&jsonInst, rootID, pElement);
      }
      json_object_get_string(&jsonInst, rootID, "", "Name", &pTmpString);
      if (pTmpString)
      {
        ST_ASSERT((strchr(pTmpString, (int)'_') == NULL)); /* The '_' is used as a separator by the designer, so this char is forbidden in the element name */
        st_os_mem_update_string(&pElement->pName, pTmpString);
        result = ST_OK;
      }
      else
      {
        /* element not found */
        result = ST_NOT_IMPL;
      }
      if (result == ST_OK)
      {
        /* Parse the PinOut array */
        jsonID hPinOut;
        json_array_get(&jsonInst, rootID, "PinDefOut", &hPinOut);

        if (hPinOut)
        {
          result = livetune_db_json_register_pindef(pHandle, pElement, &jsonInst, hPinOut, TRUE);
        }
        if (result == ST_OK)
        {
          /* Parse the PinIn array */
          jsonID PinIn;
          json_array_get(&jsonInst, rootID, "PinDefIn", &PinIn);

          if (PinIn)
          {
            result = livetune_db_json_register_pindef(pHandle, pElement, &jsonInst, PinIn, FALSE);
          }
          /* Parse the param descriptor */
          if (result == ST_OK)
          {
            jsonID hParams;
            json_array_get(&jsonInst, rootID, "Params", &hParams);

            if (hParams)
            {
              result = livetune_db_json_register_params(&jsonInst, hParams, &pElement->pParams);
            }
          }
        }
      }
    }

    json_shutdown(&jsonInst);
    if ((pElement != NULL) && (result == ST_OK))
    {
      livetune_db_send(pHandle, (uint32_t)ST_EVT_REGISTERED_ELEMENTS, (st_message_param)ST_MAKE_INT64(0UL, pElement));
    }
    else
    {
      /* the element is not valid, delete it */
      livetune_db_element_delete(pHandle, pElementCreated);
      pElement = NULL;
    }
  }
  *ppElement = pElement;

  return result;
}




/**
* @brief load an audio render pipe from a json string
*
* @param pHandle the instance handle
* @param pJson  the json description
* @return Error code
*/

ST_Result livetune_db_json_update(livetune_db *pHandle, char_t *pJsonDesc)
{
  ST_Result result = ST_ERROR;
  ST_ASSERT(pHandle != NULL);
  ST_ASSERT(pJsonDesc != NULL);
  pHandle->bParsingError   = FALSE;
  json_instance_t jsonInst = {0};
  json_load(&jsonInst, pJsonDesc, &jsonInst.pack_root);
  if (jsonInst.pack_root)
  {
    ST_TRACE_JSON("Update:%s", pJsonDesc);


    if (livetune_db_json_update_instances(pHandle, &jsonInst))
    {
      result = ST_OK;
    }
  }
  json_shutdown(&jsonInst);
  return result;
}

/**
* @brief parse an audio render pipe from a json string
*
* @param pHandle the instance handle
* @param pJson  the json description
* @return Error code
*/

ST_Result livetune_db_json_parse(livetune_db *pHandle, char_t *pJsonDesc)
{
  ST_Result result = ST_ERROR;
  ST_ASSERT(pHandle != NULL);
  ST_ASSERT(pJsonDesc != NULL);
  livetune_db_instance_cleanup(pHandle);
  pHandle->bParsingError = FALSE;
  ST_TRACE_JSON("Parse instances:%s", pJsonDesc);
  livetune_db_cmd(ST_PIPE_PUSH_STOP, ST_PIPE_REASON_CLEANUP);


  json_instance_t jsonInst = {0};
  json_load(&jsonInst, pJsonDesc, &jsonInst.pack_root);
  if (jsonInst.pack_root)
  {
    if (livetune_db_json_parse_json_instances(pHandle, &jsonInst))
    {
      if (livetune_db_json_parse_json_connections(pHandle, &jsonInst))
      {
        result = ST_OK;
      }
    }
  }
  json_shutdown(&jsonInst);
  livetune_db_cmd(ST_PIPE_POP, ST_PIPE_REASON_CLEANUP);
  return result;
}



/**
* @brief add  the json audio render pipe connection
*
* @param pHandle the instance handle
* @param pJson  the json description
* @return the pointer on the json created
*/

static ST_Result livetune_db_json_dump_connections(livetune_db_instance *pInstance, json_instance_t *pJsonInst, jsonID hObject, char_t *pPinDefName, uint8_t bType)
{
  ST_Result                          result = ST_OK;
  livetune_db_instance_pins_def **pInstPinDef;
  livetune_db_element_pin_def   **pElemPinDef;
  if (bType == 0U)
  {
    pInstPinDef = &pInstance->pPinIn;
    pElemPinDef = &pInstance->pElements->pPinIn;
  }
  else
  {
    pInstPinDef = &pInstance->pPinOut;
    pElemPinDef = &pInstance->pElements->pPinOut;
  }
  /* create the entry only if a pin exists*/
  uint32_t nbPind = livetune_db_instance_pin_def_nb(pInstPinDef);
  if ((pInstPinDef != NULL) && (nbPind != 0UL))
  {
    /* create the pindef array */
    jsonID hPinDef;
    json_create_array(pJsonInst, &hPinDef);
    if (hPinDef)
    {
      /* attach the array to the root object */
      ST_VERIFY(json_object_set_new(pJsonInst, hObject, pPinDefName, hPinDef) == JSON_OK);
      /* iterator on the number of pins exposed by the instance */
      uint32_t nbDef = livetune_db_instance_pin_def_nb(pInstPinDef);
      for (uint32_t indexDef = 0; indexDef < nbDef; indexDef++)
      {
        /* each pin entry may have a series of connections */
        /* get the pointer on the pin entry */
        livetune_db_instance_pins_def *pDef = livetune_db_instance_pin_def_get(pInstPinDef, indexDef);
        if (pDef)
        {
          /* a pindef has a series of object define each pindef itself, the order is the index */
          jsonID hDefObj;
          ST_VERIFY(json_create_object(pJsonInst, &hDefObj) == JSON_OK);
          ST_VERIFY(json_array_append_new(pJsonInst, hPinDef, hDefObj) == JSON_OK);

          /* each pindef object has a array of connection Cnx*/
          jsonID hPinCnx;
          ST_VERIFY(json_create_array(pJsonInst, &hPinCnx) == JSON_OK);
          if (hPinCnx)
          {
            livetune_db_instance_cnx_list *pList = pDef->pObject;
            ST_VERIFY(json_object_set_new(pJsonInst, hDefObj, "Cnx", hPinCnx) == JSON_OK);
            uint16_t nbConnected = (uint16_t)livetune_db_cnx_nb(&pList);
            for (uint16_t indeCnx = 0; indeCnx < nbConnected; indeCnx++)
            {
              /* Each Connection has a series of object, one by cnx */
              jsonID hCnxObj;
              ST_VERIFY(json_create_object(pJsonInst, &hCnxObj) == JSON_OK);
              ST_VERIFY(json_array_append_new(pJsonInst, hPinCnx, hCnxObj) == JSON_OK);
              livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, indeCnx);
              /* check the pin type src sink or element */
              /* the object is empty of the pin is not connected */
              if ((pCnx->pInstance != NULL) && (pCnx->pInstance->pInstanceName != NULL))
              {
                json_object_set_string(pJsonInst, hCnxObj, "", "Name", pCnx->pName);
                json_object_set_string(pJsonInst, hCnxObj, "", "InstanceNameDst", pCnx->pInstance->pInstanceName);
                json_object_set_integer(pJsonInst, hCnxObj, "", "PinDst", (int64_t)pCnx->iPinDef);
                livetune_db_element_pin *pPin       = livetune_db_list_get_object(pElemPinDef, indexDef);
                uint32_t                    nbPinParam = livetune_db_list_nb(&pPin->pParams);
                if (nbPinParam)
                {
                  jsonID hParam;
                  json_create_array(pJsonInst, &hParam);
                  ST_ASSERT(hParam != JSON_ID_NULL);
                  /* attach the array to the root object */
                  ST_VERIFY(json_object_set_new(pJsonInst, hCnxObj, "Params", hParam) == JSON_OK);
                  if (hParam)
                  {
                    livetune_db_json_parameter_dump(pInstance, pJsonInst, hParam, &pCnx->pParams, &pPin->pParams);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return result;
}

/**
* @brief build an instance render pipe in a json instance
*
* @param pHandle the instance handle
* @param pJson  the json description
* @param pJsonInstance  json  instance
* @return error code
*/

ST_Result livetune_db_json_dump(livetune_db *pHandle, json_instance_t *pJsonInstance)
{
  ST_Result result = ST_ERROR;
  ST_ASSERT(pHandle != NULL);
  ST_ASSERT(pJsonInstance != NULL);
  if (pJsonInstance->pack_root)
  {
    /* save the project name */
    if (pHandle->pMeta)
    {
      json_object_set_string(pJsonInstance, pJsonInstance->pack_root, "", "Meta", pHandle->pMeta);
    }

    /* then, save instances , instances are in an array */

    jsonID hInstances;
    json_create_array(pJsonInstance, &hInstances);
    if (hInstances)
    {
      ST_VERIFY(json_object_set_new(pJsonInstance, pJsonInstance->pack_root, "Instances", hInstances) == JSON_OK);
      uint32_t nbInstance = livetune_db_list_nb(&pHandle->pInstances);
      for (uint16_t indexInst = 0; indexInst < nbInstance; indexInst++)
      {
        livetune_db_instance *pInstance = livetune_db_list_get_object(&pHandle->pInstances, indexInst);
        if (pInstance->pElements)
        {
          jsonID hObj;
          ST_VERIFY(json_create_object(pJsonInstance, &hObj) == JSON_OK);
          ST_VERIFY(json_array_append_new(pJsonInstance, hInstances, hObj) == JSON_OK);
          /* save the reference element */
          if (pInstance->pElements->pName)
          {
            json_object_set_string(pJsonInstance, hObj, "", "RefElement", pInstance->pElements->pName);
          }

          /* save the instance name, must be unique */
          if (pInstance->pInstanceName)
          {
            json_object_set_string(pJsonInstance, hObj, "", "InstanceName", pInstance->pInstanceName);
          }

          /* save the optional description */
          if (pInstance->pDescription)
          {
            json_object_set_string(pJsonInstance, hObj, "", "Description", pInstance->pDescription);
          }

          /* save the optional description */
          if (pInstance->pMeta)
          {
            json_object_set_string(pJsonInstance, hObj, "", "Meta", pInstance->pMeta);
          }
          uint32_t nbParams = livetune_db_list_nb(&pInstance->pElements->pParams);
          if (nbParams)
          {
            /* save parameters */
            /* parameters are in an array */

            jsonID hParams;
            json_create_array(pJsonInstance, &hParams);
            ST_VERIFY(json_object_set_new(pJsonInstance, hObj, "Params", hParams) == JSON_OK);
            livetune_db_json_parameter_dump(pInstance, pJsonInstance, hParams, &pInstance->pParams, &pInstance->pElements->pParams);
          }
          livetune_db_json_dump_connections(pInstance, pJsonInstance, hObj, "PinDefIn", FALSE);
          livetune_db_json_dump_connections(pInstance, pJsonInstance, hObj, "PinDefOut", TRUE);
        }
      }
      result = ST_OK;
    }
  }
  return result;
}

/**
* @brief Load the project from the persistent storage
*
* @param pHandle the instance handle
* @return error code
*/
ST_Result livetune_db_json_load(livetune_db *pHandle)
{
  ST_Result result = ST_ERROR;

  livetune_db_instance_cleanup(pHandle);
  uint32_t offsetStorage;
  /* notice, we assume that the flash is mapped in case of external flash */

  ST_VERIFY(st_flash_storage_get_partition(gCurrent_json, &offsetStorage, NULL, NULL, NULL));
  st_project_storage_save_json *pStorage = (st_project_storage_save_json *)offsetStorage;
  if (pStorage->magic == ST_INSTANCE_MAGIC)
  {
    if (livetune_db_json_parse(pHandle, (char_t *)&pStorage[1]) == ST_OK)
    {
      result = ST_OK;
    }
  }
  if (result != ST_OK)
  {
    livetune_db_instance_cleanup(pHandle);
  }
  livetune_db_send(pHandle, (uint32_t)ST_EVT_LOAD, (st_message_param)ST_MAKE_INT64(0UL, result));

  return result;
}

/**
* @brief save the project from the persistent storage
*
* @param pHandle the instance handle
* @return error code
*/
ST_Result livetune_db_json_save(livetune_db *pHandle)
{
  ST_Result       result = ST_ERROR;
  const char_t   *pJson = NULL;
  json_instance_t jsonInst;
  memset(&jsonInst, 0, sizeof(jsonInst));
  json_create_object(&jsonInst, &jsonInst.pack_root);
  if (jsonInst.pack_root)
  {
    if (livetune_db_json_dump(pHandle, &jsonInst) == ST_OK)
    {
      pJson = json_dumps(&jsonInst, jsonInst.pack_root, 0);
      if (pJson != NULL)
      {
        st_project_storage_save_json header_storage;
        memset(&header_storage, 0, sizeof(header_storage));
        header_storage.magic = ST_INSTANCE_MAGIC;
        ST_Storage_Result err = st_flash_storage_erase(gCurrent_json);
        if (err == ST_STORAGE_OK)
        {
          /* write the header at 0 */
          err  = st_flash_storage_write(gCurrent_json, 0, &header_storage, sizeof(header_storage));
        }

        if (err  == ST_STORAGE_OK)
        {
          /* write the body at the first prog page available */
          err  = st_flash_storage_write(gCurrent_json, sizeof(header_storage), (void *)pJson, (uint32_t)strlen(pJson) + 1U);
        }
        if (err == ST_STORAGE_OK)
        {
          result = ST_OK;
        }

        if (result  != ST_OK)
        {
          ST_TRACE_ERROR("Save json file");
        }

      }
      if (pJson)
      {
        json_free((void *)pJson);
        pJson = NULL;
      }
    }
  }
  json_shutdown(&jsonInst);

  livetune_db_send(pHandle, (uint32_t)ST_EVT_SAVE, (st_message_param)ST_MAKE_INT64(0UL, result));
  return result;
}





