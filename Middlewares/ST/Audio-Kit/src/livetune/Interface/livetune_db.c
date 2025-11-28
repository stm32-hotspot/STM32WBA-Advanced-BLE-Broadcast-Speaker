/**
******************************************************************************
* @file          livetune_db.c
* @author        MCD Application Team
* @brief         manage the db
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
#include <stdio.h>
#include "livetune_db.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#define LIVETUNE_CHECK_PTR(ptr)                                   \
    if (st_os_mem_check_ptr(ST_Mem_Type_Designer, pInstance) != 0UL) \
    {                                                                \
        ST_ASSERT(0);                                                \
    }

/* Private typedef -----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */

__weak void livetune_db_cmd(livetune_db_pipe_cmd cmd, livetune_db_pipe_reason reason)
{
}


/**
* @brief  add an object to the Q
*
* @param pList the instance
* @param pObject the object , the object must be allocated
* @return the new list created
*/

livetune_db_list *livetune_db_list_create(livetune_db_list **pList, void *pObject)
{
  livetune_db_list *pNewItem = st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(livetune_db_list));
  if (pNewItem)
  {
    memset(pNewItem, 0, sizeof(*pNewItem));
    pNewItem->pObject = pObject;
    if (*pList == NULL)
    {
      pNewItem->pPrev = NULL;
      *pList          = pNewItem;
    }
    else
    {
      /* move to the end */
      livetune_db_list *pLast = *pList;
      while (pLast->pNext != NULL)
      {
        pLast = pLast->pNext;
      }
      pLast->pNext    = pNewItem; /* link the predecessor on the item */
      pNewItem->pPrev = pLast;    /* item prev point on the predecessor */
      pNewItem->pNext = NULL;     /* it the last item, so no link  */
    }
  }
  return pNewItem;
}



/**
* @brief  return the size list
*
* @param pList the instance
* @return the size
*/

uint32_t livetune_db_list_nb(livetune_db_list **pList)
{
  uint32_t size = 0;
  if (*pList)
  {
    livetune_db_list *pCur = *pList;
    while (pCur != NULL)
    {
      pCur = pCur->pNext;
      size++;
    }
  }
  return size;
}


/**
* @brief  return element list from an index
*
* @param pList the instance
* @param index the index

* @return the list pointer
*/

livetune_db_list *livetune_db_list_get(livetune_db_list **pList, uint32_t index)
{
  uint32_t             count = 0;
  livetune_db_list *pCur  = NULL;
  if (*pList)
  {
    pCur = *pList;
    while ((count != index) && (pCur != NULL))
    {
      pCur = pCur->pNext;
      count++;
    }
  }
  return pCur;
}

/**
* @brief  return object from an index
*
* @param pList the instance
* @param index the index

* @return the list pointer
*/
void *livetune_db_list_get_object(livetune_db_list **pList, uint32_t index)
{
  void                *pObject = NULL;
  livetune_db_list *pCur    = livetune_db_list_get(pList, index);
  if (pCur)
  {
    pObject = pCur->pObject;
  }
  return pObject;
}

/**
* @brief  find an object
*
* @param pList pointer on the instance
* @param void * pointer on the object
* @return the index or -1 for error
*/

int32_t livetune_db_list_find(livetune_db_list **pList, void *pObject)
{
  int32_t              index = -1;
  livetune_db_list *pCur  = *pList;
  if (pCur)
  {
    int32_t              count = 0;
    livetune_db_list *pCur  = *pList;
    while ((count != index) && (pCur != NULL))
    {
      if (pCur->pObject == pObject)
      {
        index = count;
        break;
      }
      pCur = pCur->pNext;
      count++;
    }
  }
  return index;
}


/**
* @brief  delete an object in the list
*
* @param pList the instance
* @param pCur pointer on an item
* @return error code
*/

ST_Result livetune_db_list_delete_item(livetune_db_list **pList, livetune_db_list *pCur, uint32_t bFreeObject)
{
  if (pCur == *pList)
  {
    /* first item is the current and remove the predecessor */
    *pList = pCur->pNext;
    if (pCur->pNext)
    {
      pCur->pNext->pPrev = NULL; /* no predecessor*/
    }
  }
  else
  {
    livetune_db_list *pPrev = pCur->pPrev;
    livetune_db_list *pNext = pCur->pNext;
    /* the predecessor, if any, points on the next or null */
    if (pPrev)
    {
      pPrev->pNext = pNext;
    }
    /* the successor, if any, points on the previous or null */
    if (pNext)
    {
      pNext->pPrev = pPrev;
    }
  }

  if (bFreeObject)
  {
    st_os_mem_free(pCur->pObject);
  }
  st_os_mem_free(pCur);
  return ST_OK;
}



/**
* @brief  delete an object in the list
*
* @param pList the instance
* @param index list index
* @return error code
*/

ST_Result livetune_db_list_delete(livetune_db_list **pList, uint32_t index, uint32_t bFreeObject)
{
  livetune_db_list *pCur = livetune_db_list_get(pList, (uint32_t)index);
  ST_ASSERT(pCur != NULL);
  return livetune_db_list_delete_item(pList, pCur, bFreeObject);
}


/**
* @brief  clear an object list
*
* @param pList the instance
* @param bFreeObject free object if true
* @return error code
*/

void livetune_db_list_clear(livetune_db_list **pList, uint32_t bFreeObject)
{
  while (livetune_db_list_nb(pList) != 0U)
  {
    livetune_db_list_delete(pList, 0, bFreeObject);
  }
}



/**
* @brief  install a callback to overload  register file
*
* @param pInstance the instance
* @param cb  the callback
*/
void livetune_db_set_register_update_cb(livetune_db *pHandle, LIVETUNE_DB_REGISTER_UPDATE_CB cb)
{
  pHandle->cbRegisterUpdate = cb;
}




/**
* @brief Free all instance parameters fields
*
* @param pInstance the instance
*/
static void livetune_db_free_instance_parameters(livetune_db_instance *pInstance)
{
  ST_ASSERT(pInstance != NULL);
  for (livetune_db_list *pParam = pInstance->pParams; pParam != NULL; pParam = pParam->pNext)
  {
    st_os_mem_update_string(&pParam->pInstParam->pParamValue, NULL);
  }

  livetune_db_list_clear(&pInstance->pParams, TRUE);
}

/**
* @brief Free A connection instance
*
* @param pCnxthe instance
*/
static void livetune_db_clear_cnx(livetune_db_instance_cnx *pCnx)
{
  st_os_mem_update_string(&pCnx->pName, NULL);
  uint32_t nbParams = livetune_db_list_nb(&pCnx->pParams);
  for (uint32_t indexParam = 0; indexParam < nbParams; indexParam++)
  {
    livetune_db_instance_param *pParam = livetune_db_list_get_object(&pCnx->pParams, indexParam);
    st_os_mem_update_string(&pParam->pParamValue, NULL);
    /* the pParam->pName is a direct link on the element const pname, we don't have to free it */
    pParam->pName = NULL;
  }
  livetune_db_list_clear(&pCnx->pParams, TRUE);
}


///**
//* @brief Free all an instance cnxList
//*
//* @param pCnxList the instance
//*/
//
//static void livetune_db_clear_cnx_list(livetune_db_instance_cnx_list *pCnxList)
//{
//  for (livetune_db_instance_cnx_list *pCnx = pCnxList; pCnx != NULL; pCnx = pCnx->pNext)
//  {
//    st_os_mem_update_string(&pCnx->pCnxInstance->pName, NULL);
//    livetune_db_clear_cnx(pCnx->pCnxInstance);
//  }
//}

/**
* @brief Free a pin connection
*
*/

static void livetune_db_free_cnx(livetune_db_instance_cnx_list *pPinList)
{
  if (pPinList)
  {
    if (pPinList->pCnxInstance)
    {
      //      livetune_db_clear_cnx_list(pPinList);
      livetune_db_clear_cnx(pPinList->pCnxInstance);
      st_os_mem_free(pPinList->pCnxInstance);
    }
    st_os_mem_free(pPinList);
  }
}




/**
* @brief delete a connection from a pin
*
*/
static ST_Result livetune_db_cnx_pin_delete(livetune_db_instance_pins_def **pPinDef, uint32_t defNum, int32_t iCnx)
{
  ST_ASSERT(pPinDef != NULL);
  ST_Result result = ST_ERROR;
  /* get the definition for the pin defNum */
  /* in this point, there is a connection list */
  /* we have to get the connection for the attached pin */
  livetune_db_instance_pins_def *pDef     = livetune_db_instance_pin_def_get(pPinDef, defNum);
  livetune_db_instance_cnx_list *pPrev    = NULL;
  livetune_db_instance_cnx_list *pList    = pDef->pObject;
  int32_t                           indexCnx = 0;
  /* look for the connection */

  while (pList)
  {
    if (indexCnx == iCnx)
    {
      /* we found the pint src, next pin dst */
      break;
    }
    indexCnx++;
    pPrev = pList;
    pList = pList->pNext;
  }
  if (pList)
  {
    /* we must force the disconnection */
    /* if pdef is non null, we found the connection */

    if (pPrev)
    {
      /* if pPrev is non null the cnx is not root */
      pPrev->pNext = pList->pNext;
    }
    else
    {
      /* the cnx is root, we have to update the first item */
      pDef->pObject = pList->pNext;
    }
    /* we can now free the connection */
    livetune_db_free_cnx(pList);
    result = ST_OK;
  }
  return result;
}

/**
* @brief delete a connection from a pin
*
*/
static ST_Result livetune_db_cnx_from_pin_delete(livetune_db_instance_pins_def **pPinDef, uint32_t defNum, livetune_db_instance *pInstanceAttached, uint32_t pinAttached)
{
  ST_ASSERT(pPinDef != NULL);
  ST_ASSERT(pInstanceAttached != NULL);
  ST_Result result = ST_ERROR;
  /* get the definition for the pin defNum */
  /* in this point, there is a connection list */
  /* we have to get the connection for the attached pin */
  livetune_db_instance_pins_def *pDef = livetune_db_instance_pin_def_get(pPinDef, defNum);
  /* first find the pin */
  uint32_t indexCnx = 0;
  if (pDef)
  {
    livetune_db_instance_cnx_list *pList = pDef->pObject;
    while (pList)
    {
      if ((pList->pCnxInstance->pInstance == pInstanceAttached) && (pList->pCnxInstance->iPinDef == pinAttached))
      {
        /* we found the pint src, next pin dst */
        break;
      }
      pList = pList->pNext;
      indexCnx++;
    }
    if (pList)
    {
      result = livetune_db_cnx_pin_delete(pPinDef, (uint32_t)defNum, (int32_t)indexCnx);
    }
  }
  return result;
}




/**
* @brief Delete a connection out
*
* @param pHandle the instance handle
* @param pInstanceName the instance name
* @param pRefInstance the reference instance name
* @param pin           the pin
* @return error code
*/

ST_Result livetune_db_cnx_delete(livetune_db *pHandle, const char_t *pInstanceNameOut, uint32_t pinOut, const char_t *pInstanceNameIn, uint32_t pinIn)
{
  ST_Result result = ST_OK;
  ST_ASSERT(pHandle != NULL);
  livetune_db_instance *pInstanceOut = livetune_db_instance_find_from_name(pHandle, pInstanceNameOut);
  livetune_db_instance *pInstanceIn  = livetune_db_instance_find_from_name(pHandle, pInstanceNameIn);
  if ((pInstanceOut != NULL) && (pInstanceIn != NULL))
  {
    if (livetune_db_cnx_from_pin_delete(&pInstanceOut->pPinOut, (uint32_t)pinOut, pInstanceIn, (uint32_t)pinIn) != ST_OK)
    {
      result = ST_ERROR;
    }
    if (livetune_db_cnx_from_pin_delete(&pInstanceIn->pPinIn, (uint32_t)pinIn, pInstanceOut, (uint32_t)pinOut) != ST_OK)
    {
      result = ST_ERROR;
    }
  }
  else
  {
    result = ST_ERROR;
  }
  return result;
}



/**
* @brief delete a connection from a pin
*
*/
livetune_db_instance_cnx_list *livetune_db_cnx_attach(livetune_db_instance_cnx_list **pPinList, livetune_db_instance *pHandleAttached, uint32_t pinAttached, livetune_db_element_pin *pPinElement)
{
  ST_Result                         result       = ST_OK;
  livetune_db_instance_cnx_list *pList        = NULL;
  livetune_db_instance_cnx      *pInstanceCnx = st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(livetune_db_instance_cnx));
  if (pInstanceCnx)
  {
    memset(pInstanceCnx, 0, sizeof(livetune_db_instance_cnx));
    pList = livetune_db_list_create(pPinList, pInstanceCnx);
    if (pList == NULL)
    {
      result = ST_ERROR;
    }
    else
    {
      pInstanceCnx->iPinDef   = pinAttached;
      pInstanceCnx->pInstance = pHandleAttached;
      /* now init params with default values */

      int32_t nbParams = (int32_t)livetune_db_list_nb(&pPinElement->pParams);
      /* init the instance cnx with its default value */
      for (int32_t indexParam = 0; indexParam < nbParams; indexParam++)
      {
        livetune_db_instance_param *pInstanceParam = st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(livetune_db_instance_param));

        if (pInstanceParam == NULL)
        {
          result = ST_ERROR;
          break;
        }
        memset(pInstanceParam, 0, sizeof(*pInstanceParam));
        livetune_db_element_params *pPinDefParam = livetune_db_list_get_object(&pPinElement->pParams, (uint32_t)indexParam);
        /* Direct link to the const element pName */
        pInstanceParam->pName = pPinDefParam->pName;
        /* Allocate because modifiable */
        st_os_mem_update_string(&pInstanceParam->pParamValue, pPinDefParam->pParamDefault);
        /* add the parameter to the list */

        if (livetune_db_list_create(&pInstanceCnx->pParams, pInstanceParam) == NULL)
        {
          result = ST_ERROR;
          break;
        }
      }
    }
  }
  if (result != ST_OK)
  {
    /* role back */
    if (pInstanceCnx)
    {
      if (pList)
      {
        for (livetune_db_list *pParam = pInstanceCnx->pParams; pParam != NULL; pParam = pParam->pNext)
        {
          st_os_mem_update_string(&pParam->pInstParam->pParamValue, NULL);
        }
        livetune_db_list_clear(&pInstanceCnx->pParams, TRUE);
        livetune_db_list_delete_item(pPinList, pList, TRUE);
      }
      pList = NULL;
    }
  }

  return pList; /*cstat !MISRAC2012-Dir-4.13_e !MISRAC2012-Rule-1.3_p  false positif This API returns a pointer that will be freed by the caller */
}

/**
* @brief Create a pindef
*
*/
static ST_Result livetune_db_instance_pin_def_create(livetune_db_instance_pins_def **pPinDef)
{
  ST_Result result = ST_ERROR;
  if (livetune_db_list_create(pPinDef, NULL))
  {
    result = ST_OK;
  }
  return result;
}

/**
* @brief Change a parameter as string
*
*/

ST_Result livetune_db_param_change(livetune_db_instance_param *pParam, char_t *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);

  char_t tParamValue[10];
  vsnprintf(tParamValue, sizeof(tParamValue), pFormat, args);
  st_os_mem_update_string(&pParam->pParamValue, tParamValue);
  va_end(args);
  return (pParam->pParamValue == NULL) ? ST_ERROR : ST_OK;
}


/**
* @brief Free instance
*
* @param pInstance the instance
*/
static void livetune_db_free_instance(livetune_db_instance *pInstance)
{
  ST_ASSERT(pInstance != NULL);
  if (pInstance->pScratchBuffer)
  {
    st_os_mem_free(pInstance->pScratchBuffer);
    pInstance->pScratchBuffer = NULL;
  }
  st_os_mem_update_string(&pInstance->pDescription, NULL);
  st_os_mem_update_string(&pInstance->pMeta, NULL);
  st_os_mem_update_string(&pInstance->pInstanceName, NULL);
}


/**
* @brief return the number of pins connected
*
* @param pList the instance list
@return the number of pins
*/


uint32_t livetune_db_cnx_nb(livetune_db_instance_cnx_list **pList)
{
  uint32_t                          nbPins = 0U;
  livetune_db_instance_cnx_list *pl     = *pList;
  while (pl)
  {
    nbPins++;
    pl = pl->pNext;
  }

  return nbPins;
}

/**
* @brief return the pinlist struct from an index position
*
*/
livetune_db_instance_cnx *livetune_db_cnx_get(livetune_db_instance_cnx_list **pCnxList, uint32_t index)
{
  uint32_t                          curIndex = 0;
  livetune_db_instance_cnx      *pItem    = NULL;
  livetune_db_instance_cnx_list *pList    = *pCnxList;

  while (pList)
  {
    if (curIndex == index)
    {
      pItem = pList->pCnxInstance;
      break;
    }
    pList = pList->pNext;
    curIndex++;
  }
  return pItem;
}
/**
 * @brief return the number of pindef
 *
 * @param pPinDef the list
 * @return uint32_t number of pindef
 */

uint32_t livetune_db_instance_pin_def_nb(livetune_db_instance_pins_def **pPinDef)
{
  uint32_t                          nbPins = 0U;
  livetune_db_instance_pins_def *pD     = *pPinDef;
  while (pD)
  {
    nbPins++;
    pD = pD->pNext;
  }

  return nbPins;
}

/**
 * @brief return the pindef pointer from an index
 *
 * @param pPinDef pindef list
 * @param index  the index
 * @return livetune_db_instance_pins_def*
 */

livetune_db_instance_pins_def *livetune_db_instance_pin_def_get(livetune_db_instance_pins_def **pPinDef, uint32_t index)
{
  uint32_t                          curIndex = 0U;
  livetune_db_instance_pins_def *pItem    = NULL;
  livetune_db_instance_pins_def *pList    = *pPinDef;

  while (pList)
  {
    if (curIndex == index)
    {
      pItem = pList;
      break;
    }
    pList = pList->pNext;
    curIndex++;
  }
  return pItem;
}




/**
* @brief set an instance parameter
*
* @param pHandle the instance handle
* @param index  parameter index
* @param value parameter value

@return Error code
*/



ST_Result livetune_db_param_set(livetune_db *pHandle, livetune_db_instance *pInstance, livetune_info_param paramType, livetune_db_instance_param *pParams, const char_t *pValue)
{
  ST_Result result = ST_OK;
  ST_ASSERT(pParams != NULL);
  ST_ASSERT(pValue != NULL);

  st_os_mem_update_string(&pParams->pParamValue, pValue);
  if (pInstance->pElements->hInterface.parameter_change_cb)
  {
    pInstance->pElements->hInterface.parameter_change_cb(pInstance, paramType);
  }
  /*cstat -MISRAC2012-Rule-10.8 -MISRAC2012-Rule-12.2 for HW reasons,we assume that a pointer could be casted in an offset 32 bits */
  livetune_db_send(pHandle, (uint32_t)ST_EVT_UPDATE_PARAM, (st_message_param)ST_MAKE_INT64(paramType.byValue, pInstance));
  /*cstat +MISRAC2012-Rule-10.8 +MISRAC2012-Rule-12.2*/

  return result;
}



/**
* @brief returns an index param from parameter name
*
* @param pElem the instance handle
* @param pName the ref name
* @return an index or -1
*/

int32_t livetune_db_param_find_from_name(livetune_db_list **pParams, const char_t *pName)
{
  int32_t index  = -1;
  int32_t iParam = 0;

  for (livetune_db_list *pList = *pParams; pList != NULL; pList = pList->pNext, iParam++)
  {
    if (strcmp(pList->pElemParam->pName, pName) == 0)
    {
      index = iParam;
      break;
    }
  }
  return index;
}

/**
* @brief returns an index Connection from parameter name
*
* @param pHandle the instance handle
* @param pName the connection ref name
* @param pin  the connection pin
* @return an index or -1
*/

int32_t livetune_db_cnx_find_from_name(livetune_db_instance_pins_def **pHandle, uint32_t pinRef, const char_t *pInstanceNameDst, uint32_t pinDst)
{
  int32_t                           index = -1;
  livetune_db_instance_pins_def *pDef  = livetune_db_instance_pin_def_get(pHandle, pinRef);
  if (pDef)
  {
    if (pDef->pObject)
    {
      livetune_db_instance_cnx_list *pItem    = pDef->pObject;
      int32_t                           indexCnx = 0;
      while (pItem)
      {
        if ((pItem->pCnxInstance->pInstance != NULL) && (strcmp(pItem->pCnxInstance->pInstance->pInstanceName, pInstanceNameDst) == 0) && (pinDst == pItem->pCnxInstance->iPinDef))
        {
          index = indexCnx;
          break;
        }
        pItem = pItem->pNext;
        indexCnx++;
      }
    }
  }
  return index;
}


/**
* @brief returns an instance  Connection from parameter name
*
* @param pHandle the instance handle
* @param pName the connection ref name , if null returns the first connection
* @param pin  the connection pin
*/

livetune_db_instance_cnx *livetune_db_cnx_name_search(livetune_db_instance_pins_def **ppDef, const char_t *pCnxName, uint32_t pinDef)
{
  livetune_db_instance_cnx      *pRefCnx = NULL;
  livetune_db_instance_pins_def *pDef    = livetune_db_instance_pin_def_get(ppDef, pinDef);
  if (pDef)
  {
    if (pDef->pObject)
    {
      livetune_db_instance_cnx_list *pItem = pDef->pObject;
      while (pItem)
      {
        if ((pCnxName == NULL) || (strcmp(pItem->pCnxInstance->pName, pCnxName) == 0))
        {
          pRefCnx = pItem->pCnxInstance;
          break;
        }
        pItem = pItem->pNext;
      }
    }
  }
  return pRefCnx;
}


/**
* @brief returns an index Connection from parameter name
*
* @param pHandle the instance handle
* @param pName the connection ref name , if null returns the first connection
* @param pin  the connection pin
*/

int32_t livetune_db_cnx_name_search_index(livetune_db_instance_pins_def **ppDef, const char_t *pCnxName, uint32_t pinDef)
{
  int32_t                           index = -1;
  livetune_db_instance_pins_def *pDef  = livetune_db_instance_pin_def_get(ppDef, pinDef);
  if (pDef)
  {
    if (pDef->pObject)
    {
      int32_t                           count = 0;
      livetune_db_instance_cnx_list *pItem = pDef->pObject;
      while (pItem)
      {
        if ((pCnxName == NULL) || (strcmp(pItem->pCnxInstance->pName, pCnxName) == 0))
        {
          index = count;
          break;
        }
        pItem = pItem->pNext;
        count++;
      }
    }
  }
  return index;
}




/**
* @brief returns a pointer instance from the name
*
* @param pHandle the instance handle
* @param pName the instance  name
* @return an index or -1
*/

livetune_db_instance_cnx *livetune_db_cnx_find(livetune_db_instance_pins_def **pHandle, uint32_t pinRef, const char_t *pInstanceNameDst, uint32_t pinDst)
{
  livetune_db_instance_cnx *pCnx  = NULL;
  int32_t                      index = livetune_db_cnx_find_from_name(pHandle, pinRef, pInstanceNameDst, pinDst);
  if (index != -1)
  {
    livetune_db_instance_pins_def *pPinRef = livetune_db_instance_pin_def_get(pHandle, pinRef);
    if (pPinRef)
    {
      livetune_db_instance_cnx_list *pList = pPinRef->pObject;
      pCnx                                    = livetune_db_cnx_get(&pList, (uint32_t)index);
    }
  }
  return pCnx;
}



/**
* @brief returns a pointer instance from the name
*
* @param pHandle the instance handle
* @param pName the instance  name
* @return an index or -1
*/

livetune_db_instance *livetune_db_instance_find_from_name(livetune_db *pHandle, const char_t *pName)
{
  livetune_db_instance *pInstance  = NULL;
  uint32_t                 nbInstance = livetune_db_list_nb(&pHandle->pInstances);

  for (uint32_t iInst = 0; iInst < nbInstance; iInst++)
  {
    livetune_db_instance *pCurInstance = livetune_db_list_get_object(&pHandle->pInstances, iInst);
    ST_ASSERT(pCurInstance != NULL); /* must exist */
    if (strcmp(pCurInstance->pInstanceName, pName) == 0)
    {
      pInstance = pCurInstance;
      break;
    }
  }
  return pInstance;
}


/**
* @brief returns an index instance from the name
*
* @param pHandle the instance handle
* @param pName the instance  name
* @return an index or -1
*/

static int32_t livetune_db_find_instance_index_from_name(livetune_db *pHandle, const char_t *pName)
{
  int32_t  index      = -1;
  uint32_t nbInstance = livetune_db_list_nb(&pHandle->pInstances);

  for (uint32_t iInst = 0; iInst < nbInstance; iInst++)
  {
    livetune_db_instance *pCurInstance = livetune_db_list_get_object(&pHandle->pInstances, iInst);

    if (strcmp(pCurInstance->pInstanceName, pName) == 0)
    {
      index = (int32_t)iInst;
      break;
    }
  }
  return index;
}




/**
* @brief returns a pointer from a reference element name
*
* @param pHandle the instance handle
* @param pRef    the ref name
* @return pointer on the element or NULL
*/
livetune_db_element *livetune_db_element_find(livetune_db *pHandle, const char_t *pRef)
{
  livetune_db_element *pElem      = NULL;
  uint32_t                nbElements = livetune_db_list_nb(&pHandle->pElements);

  for (uint32_t index = 0; index < nbElements; index++)
  {
    livetune_db_element *pElement = livetune_db_list_get_object(&pHandle->pElements, index);
    if (pElement->pName)
    {
      if (strcmp(pElement->pName, pRef) == 0)
      {
        pElem = pElement;
        break;
      }
    }
  }

  return pElem;
}


/**
* @brief delete all instances

*
* @param pHandle      the instance handle
* @return error code
*/

void livetune_db_instance_cleanup(livetune_db *pHandle)
{
  livetune_db_cmd(ST_PIPE_PUSH_STOP, ST_PIPE_REASON_CLEANUP);
  st_os_mem_update_string(&pHandle->pMeta, NULL);
  while (livetune_db_list_nb(&pHandle->pInstances))
  {
    livetune_db_instance *pCurInstance = livetune_db_list_get_object(&pHandle->pInstances, 0);
    livetune_db_instance_delete_from_name(pHandle, pCurInstance->pInstanceName);
  }
  livetune_db_cmd(ST_PIPE_POP, ST_PIPE_REASON_CLEANUP);
}


/**
* @brief Disconnect the instance from other instances
*
* @param pHandle the instance handle
* @return error code
*/


static ST_Result livetune_db_json_disconnect(livetune_db_instance *pInstance)
{
  ST_Result result   = ST_OK;
  uint32_t  nbDefOut = livetune_db_instance_pin_def_nb(&pInstance->pPinOut);
  for (uint32_t indexDef = 0; indexDef < nbDefOut; indexDef++)
  {
    /* iterate on all pin definition output */

    livetune_db_instance_pins_def *pDef  = livetune_db_instance_pin_def_get(&pInstance->pPinOut, indexDef);
    uint32_t                          nbCnx = (uint32_t)livetune_db_cnx_nb(&pDef->pCnxList);
    /* We clear only the content of CNX */
    for (uint32_t indexCnx = 0; indexCnx < nbCnx; indexCnx++)
    {
      livetune_db_instance_cnx *pCnxInstance = livetune_db_list_get_object(&pDef->pCnxList, indexCnx);
      if (pCnxInstance)
      {
        /* first disconnect the peer from this instance */
        livetune_db_instance *pPeerInstance = pCnxInstance->pInstance;
        if (pPeerInstance)
        {
          /* pin out connected to pinin peer */
          livetune_db_instance_pins_def *pPeerDef = livetune_db_instance_pin_def_get(&pPeerInstance->pPinIn, pCnxInstance->iPinDef);
          if (pPeerDef)
          {
            /* Look for the CNX peer connected to the instance out pin */
            int32_t index = livetune_db_cnx_name_search_index(&pPeerInstance->pPinIn, pCnxInstance->pName, pCnxInstance->iPinDef);
            if (index != -1)
            {
              /* delete the peer connection */
              livetune_db_instance_cnx_list *pPeerCnxList = livetune_db_list_get(&pPeerDef->pCnxList, (uint32_t)index);
              livetune_db_clear_cnx(pPeerCnxList->pCnxInstance);
              livetune_db_list_delete_item(&pPeerDef->pCnxList, pPeerCnxList, TRUE);
            }
          }
          livetune_db_clear_cnx(pCnxInstance);
        }
      }
    }
    /* Now we can delete the list */
    livetune_db_list_clear(&pDef->pCnxList, TRUE);
  }

  /* we do the same with pin definition  input */
  uint32_t nbDefIn = livetune_db_instance_pin_def_nb(&pInstance->pPinIn);
  /* We clear only the content of CNX */
  for (uint32_t indexDef = 0; indexDef < nbDefIn; indexDef++)
  {
    livetune_db_instance_pins_def *pDef  = livetune_db_instance_pin_def_get(&pInstance->pPinIn, indexDef);
    uint32_t                       nbCnx = livetune_db_cnx_nb(&pDef->pCnxList);
    for (uint32_t indexCnx = 0; indexCnx < nbCnx; indexCnx++)
    {
      livetune_db_instance_cnx *pCnxInstance = livetune_db_list_get_object(&pDef->pCnxList, (uint32_t)indexCnx);
      if (pCnxInstance)
      {
        livetune_db_instance *pPeerInstance = pCnxInstance->pInstance;
        if (pPeerInstance)
        {
          /* pin in connected to pinout peer */
          livetune_db_instance_pins_def *pPeerDef = livetune_db_instance_pin_def_get(&pPeerInstance->pPinOut, pCnxInstance->iPinDef);
          if (pPeerDef)
          {
            /* first disconnect the peer from this instance */
            int32_t index = livetune_db_cnx_name_search_index(&pPeerInstance->pPinOut, pCnxInstance->pName, pCnxInstance->iPinDef);
            if (index != -1)
            {
              /* delete the peer connection */

              livetune_db_instance_cnx_list *pPeerCnxList = livetune_db_list_get(&pPeerDef->pCnxList, (uint32_t)index);
              livetune_db_clear_cnx(pPeerCnxList->pCnxInstance);
              livetune_db_list_delete_item(&pPeerDef->pCnxList, pPeerCnxList, TRUE);
            }
          }
          livetune_db_clear_cnx(pCnxInstance);
        }
      }
    }
    /* Now we can delete the list */
    livetune_db_list_clear(&pDef->pCnxList, TRUE);
  }
  return result;
}



/**
* @brief Delete an instance
*
* @param pHandle the instance handle
* @param pInstanceName the instance name
* @return error code
*/

ST_Result livetune_db_instance_delete(livetune_db *pHandle, livetune_db_instance *pInstance)
{
  ST_Result result = ST_ERROR;
  ST_ASSERT(pHandle != NULL);
  if (pInstance)
  {
    int32_t indexInst = livetune_db_find_instance_index_from_name(pHandle, pInstance->pInstanceName);
    ST_ASSERT(indexInst != -1); /* must exist */
    /* free parameters instances */
    livetune_db_free_instance_parameters(pInstance);
    /* Disconnect the instance form other instances*/

    livetune_db_json_disconnect(pInstance);
    livetune_db_list_clear(&pInstance->pPinOut, FALSE);
    livetune_db_list_clear(&pInstance->pPinIn, FALSE);
    livetune_db_free_instance(pInstance);
    pInstance->pElements = NULL;
    livetune_db_list_delete(&pHandle->pInstances, (uint32_t)indexInst, TRUE);
    result = ST_OK;
  }
  else
  {
    ST_TRACE_ERROR("instance not found");
  }

  return result;
}




/**
* @brief Delete an instance
*
* @param pHandle the instance handle
* @param pInstanceName the instance name
* @return error code
*/

ST_Result livetune_db_instance_delete_from_name(livetune_db *pHandle, const char_t *pInstanceName)
{
  ST_Result result    = ST_ERROR;
  int32_t   indexInst = livetune_db_find_instance_index_from_name(pHandle, pInstanceName);
  if (indexInst != -1)
  {
    /* free parameters instances */
    livetune_db_instance *pInstance = livetune_db_list_get_object(&pHandle->pInstances, (uint32_t)indexInst);
    if (pInstance)
    {
      result = livetune_db_instance_delete(pHandle, pInstance);
    }
  }

  return result;
}


/**
* @brief Create a connection out
*
* @param pHandle the instance handle
* @param pInstanceName the instance name
* @param pRefInstance the reference instance name
* @param pin           the pin
* @return error code
*/

ST_Result livetune_db_cnx_create(livetune_db *pHandle, const char_t *pInstanceNameOut, uint32_t pinOut, const char_t *pInstanceNameIn, uint32_t pinIn)
{
  ST_Result result = ST_ERROR;
  ST_ASSERT(pHandle != NULL);
  /* Lock for the out element */
  livetune_db_instance *pInstanceOut = livetune_db_instance_find_from_name(pHandle, pInstanceNameOut);
  livetune_db_instance *pInstanceIn  = livetune_db_instance_find_from_name(pHandle, pInstanceNameIn);
  if ((pInstanceOut != NULL) && (pInstanceIn != NULL))
  {
    /* Get the pin def for the target instance */
    livetune_db_instance_pins_def *pDefOut = livetune_db_instance_pin_def_get(&pInstanceOut->pPinOut, pinOut);
    ST_ASSERT(pDefOut != NULL);
    if (pDefOut)
    {
      /* Create a connection and attach the out with the pin in  */
      livetune_db_element_pin *pRefPinElement = livetune_db_list_get_object(&pInstanceOut->pElements->pPinOut, (uint32_t)pinOut);
      ST_VERIFY(livetune_db_cnx_attach((livetune_db_list **)&pDefOut->pCnxList, pInstanceIn, pinIn, pRefPinElement) != NULL);
    }

    /* Get the pin def for the source instance */
    livetune_db_instance_pins_def *pDefIn = livetune_db_instance_pin_def_get(&pInstanceIn->pPinIn, pinIn);
    ST_ASSERT(pDefIn != NULL);
    if (pDefIn != NULL)
    {
      /* Create a connection and attach the in  with the pin out  */
      livetune_db_element_pin *pRefPinElement = livetune_db_list_get_object(&pInstanceIn->pElements->pPinIn, (uint32_t)pinIn);
      ST_VERIFY(livetune_db_cnx_attach((livetune_db_list **)&pDefIn->pCnxList, pInstanceOut, (uint32_t)pinOut, pRefPinElement) != NULL);
    }
    result = ST_OK;
  }
  return result;
}



/**
* @brief Create an instance
*
* @param pHandle the instance handle
* @param pInstanceName the instance name
* @param pRefElement the ref element name
* @return error code
*/
livetune_db_instance *livetune_db_instance_create(livetune_db *pHandle, const char_t *pInstanceName, const char_t *pRefElement)
{
  ST_Result result = ST_ERROR;
  ST_ASSERT(pHandle != NULL);
  livetune_db_instance *pInstance = NULL;
  livetune_db_element  *pElement  = livetune_db_element_find(pHandle, pRefElement);
  if (pElement)
  {
    pInstance = st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(livetune_db_instance));
    if (pInstance)
    {
      result = ST_OK;
      /* assume all 0 */
      memset(pInstance, 0, sizeof(*pInstance));
      if (livetune_db_list_create(&pHandle->pInstances, pInstance))
      {
        /* the name and the ref element */
        st_os_mem_update_string(&pInstance->pInstanceName, pInstanceName);
        pInstance->pElements = pElement;
        /* now create pins definition in and out */

        int32_t nbPinIn = (int32_t)livetune_db_list_nb(&pInstance->pElements->pPinIn);
        for (int32_t indexIn = 0; (indexIn < nbPinIn) && (result == ST_OK); indexIn++)
        {
          result = livetune_db_instance_pin_def_create(&pInstance->pPinIn);
        }
        int32_t nbPinOut = (int32_t)livetune_db_list_nb(&pInstance->pElements->pPinOut);
        for (int32_t indexOut = 0; (indexOut < nbPinOut) && (result == ST_OK); indexOut++)
        {
          result = livetune_db_instance_pin_def_create(&pInstance->pPinOut);
        }
        if (result == ST_OK)
        {
          int32_t nbParams = (int32_t)livetune_db_list_nb(&pInstance->pElements->pParams);
          for (int32_t indexParam = 0; (indexParam < nbParams) && (result == ST_OK); indexParam++)
          {
            livetune_db_instance_param *pInstParam = st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(livetune_db_instance_param));
            if (pInstParam == NULL)
            {
              result = ST_ERROR;
              break;
            }
            memset(pInstParam, 0, sizeof(*pInstParam));
            if (livetune_db_list_create(&pInstance->pParams, pInstParam) == NULL)
            {
              result = ST_ERROR;
              break;
            }
            livetune_db_element_params *pElemParam = livetune_db_list_get_object(&pInstance->pElements->pParams, (uint32_t)indexParam);
            if (pElemParam == NULL)
            {
              result = ST_ERROR;
              break;
            }

            st_os_mem_update_string(&pInstParam->pParamValue, pElemParam->pParamDefault);
            pInstParam->pName = pElemParam->pName;
          }
        }
      }
    }
  }
  if (result != ST_OK)
  {
    if (pInstance)
    {
      livetune_db_instance_delete(pHandle, pInstance);
      pInstance = NULL;
    }
  }

  return pInstance;
}

/*
  @brief create an element

* @param pJson  the json description or null
* @param pRefBuilder  the reference builder or null
* @param interface The process callback

* @return the pointer on the element created

*/
livetune_db_element *livetune_db_element_create(livetune_db *pHandle, const char_t *pJsonDesc, const void *pRefBuilder, livetune_interface interface)
{
  livetune_db_element *pElement = st_os_mem_alloc(ST_Mem_Type_Designer, sizeof(livetune_db_element));
  if (pElement)
  {
    if (livetune_db_list_create(&pHandle->pElements, pElement))
    {
      memset(pElement, 0, sizeof(*pElement));
      pElement->hInterface      = interface;
      pElement->pJsonDefinition = pJsonDesc;
      pElement->pRefBuilder     = pRefBuilder;
    }
    else
    {
      st_os_mem_free(pElement);
      pElement = NULL;
    }
  }
  return pElement;
}



/**
* @brief Free all element parameters fields
*
* @param pInstance pointer on the element
*/
static void livetune_db_free_element_parameters(livetune_db_element *pElement)
{
  ST_ASSERT(pElement != NULL);
  for (livetune_db_list *pParam = pElement->pParams; pParam != NULL; pParam = pParam->pNext)
  {
    st_os_mem_update_string(&pParam->pElemParam->pName, NULL);
    st_os_mem_update_string(&pParam->pElemParam->pParamType, NULL);
    st_os_mem_update_string(&pParam->pElemParam->pParamDefault, NULL);
  }
  livetune_db_list_clear(&pElement->pParams, TRUE);
}
/*
  @brief delete  an element

* @param pHandle  pointer to the instance
* @param pElement  pointer to the element
* @return error code

*/
ST_Result livetune_db_element_delete(livetune_db *pHandle, livetune_db_element *pElement)
{
  ST_Result result = ST_ERROR;
  if (pElement)
  {
    int32_t indexElem = livetune_db_list_find(&pHandle->pElements, pElement);
    ST_ASSERT(indexElem != -1); /* must exist */

    pElement->pJsonDefinition = NULL;
    st_os_mem_update_string(&pElement->pName, NULL);
    pElement->pRefBuilder = NULL;

    livetune_db_free_element_parameters(pElement);
    //    livetune_db_free_element_pin_defs(&pElement->hPinOut);
    //    livetune_db_free_element_pin_defs(&pElement->hPinIn);
    livetune_db_list_delete(&pHandle->pElements, (uint32_t)indexElem, TRUE);
  }
  return result;
}



/*
  @brief return the number of element registered

  @param pElements       the instance
  @return nb elements

*/
uint32_t livetune_db_element_nb_get(livetune_db *pHandle)
{
  ST_ASSERT(pHandle != NULL);
  return livetune_db_list_nb(&pHandle->pElements);
}



/*
  @brief return the element registered

  @param hHandle         the instance
  @param index           element index
  @return the element json definition or null

*/
livetune_db_element *livetune_db_element_get(livetune_db *pHandle, uint32_t index)
{
  ST_ASSERT(pHandle != NULL);
  return livetune_db_list_get_object(&pHandle->pElements, index);
}


/*
  @brief return the number of element registered

  @param pHandle         the instance
  @param index           element index
  @return the element json definition or null

*/
const char_t *livetune_db_element_definition_get(livetune_db *pHandle, uint32_t index)
{
  ST_ASSERT(pHandle != NULL);
  const char_t           *pJson    = NULL;
  livetune_db_element *pElement = livetune_db_list_get_object(&pHandle->pElements, index);
  if (pElement)
  {
    pJson = pElement->pJsonDefinition;
  }
  return pJson;
}



/*
  @brief Create the data base

*/


ST_Result livetune_db_create(livetune_db *pHandle)
{
  ST_ASSERT(pHandle != NULL);
  memset(pHandle, 0, sizeof(*pHandle));

  return ST_OK;
}

/**
 * @brief reset  the DB
 *
 * @param pHandle the instance
 * @return ST_Result
 */

ST_Result livetune_db_new(livetune_db *pHandle)
{
  livetune_db_cmd(ST_PIPE_PUSH_STOP, ST_PIPE_REASON_CLEANUP);
  livetune_db_instance_cleanup(pHandle);
  livetune_db_send(pHandle, (uint32_t)ST_EVT_NEW, (st_message_param)0);
  livetune_db_cmd(ST_PIPE_POP, ST_PIPE_REASON_CLEANUP);
  return ST_OK;
}


/*
  @brief delete the data base

*/

ST_Result livetune_db_delete(livetune_db *pHandle)
{
  livetune_db_instance_cleanup(pHandle);

  return ST_OK;
}


/*
  @brief post a message

*/


void livetune_db_send(livetune_db *pHandle, uint32_t evt, st_message_param lparam)
{
  ST_ASSERT(pHandle != NULL);
  if (pHandle->cbMsg)
  {
    pHandle->cbMsg(pHandle, evt, lparam);
  }
}


/*
  @brief set the CB message

*/

void livetune_db_msg_cb_set(livetune_db *pHandle, LIVETUNE_DB_MSG_CB cb)
{
  ST_ASSERT(pHandle != NULL);
  pHandle->cbMsg = cb;
}


void livetune_db_instance_set_update_type(livetune_db_instance *pInstance, uint8_t type)
{
  ST_ASSERT(pInstance != NULL);
  pInstance->iUpdateType = type;
}


void livetune_db_coherency(livetune_db *pHandle)
{
  int32_t nbInstance = (int32_t)livetune_db_list_nb(&pHandle->pInstances);

  for (int32_t indexInst = 0; indexInst < nbInstance; indexInst++)
  {
    livetune_db_list     *pInstanceList = livetune_db_list_get(&pHandle->pInstances, (uint32_t)indexInst);
    livetune_db_instance *pInstance     = pInstanceList->pInstance;
    ST_TRACE_INFO("%03d:%s", indexInst, pInstance->pInstanceName);

    if ((pInstanceList != NULL) && (pInstanceList->pNext != NULL))
    {
      ST_ASSERT(pInstanceList->pNext->pPrev == pInstanceList);
    }

    LIVETUNE_CHECK_PTR(pInstance);
    LIVETUNE_CHECK_PTR(pInstance->pMeta);
    LIVETUNE_CHECK_PTR(pInstance->pScratchBuffer);
    LIVETUNE_CHECK_PTR(pInstance->pInstanceName);
    LIVETUNE_CHECK_PTR(pInstance->pDescription);
    LIVETUNE_CHECK_PTR(pInstance->pElements);
    LIVETUNE_CHECK_PTR(pInstance->pPipe);
    uint32_t nbParams = livetune_db_list_nb(&pInstance->pParams);

    for (uint32_t indexParam = 0; indexParam < nbParams; indexParam++)
    {
      livetune_db_instance_param *pParam = livetune_db_list_get_object(&pInstance->pParams, (uint32_t)indexParam);
      LIVETUNE_CHECK_PTR(pParam->pName);
      LIVETUNE_CHECK_PTR(pParam->pParamValue);
    }
    livetune_db_instance_pins_def *pDef = pInstance->pPinOut;
    while (pDef)
    {
      livetune_db_instance_cnx_list *pList = pDef->pCnxList;
      if ((pList != NULL) && (pList->pNext != NULL))
      {
        ST_ASSERT(pList->pNext->pPrev == pList);
      }

      if (pList)
      {
        livetune_db_instance_cnx_list *pCnx = pList;
        while (pCnx)
        {
          if ((pCnx != NULL) && (pCnx->pNext != NULL))
          {
            ST_ASSERT(pCnx->pNext->pPrev == pCnx);
          }


          LIVETUNE_CHECK_PTR(pCnx);
          livetune_db_instance_cnx *pCnxInst = pCnx->pCnxInstance;
          LIVETUNE_CHECK_PTR(pCnxInst);
          LIVETUNE_CHECK_PTR(pCnxInst->pInstance);
          LIVETUNE_CHECK_PTR(pCnxInst->pName);
          uint32_t nbParams = livetune_db_list_nb(&pCnxInst->pParams);
          for (uint32_t indexCnx = 0; indexCnx < nbParams; indexCnx++)
          {
            livetune_db_instance_param *pParam = livetune_db_list_get_object(&pCnxInst->pParams, indexCnx);

            LIVETUNE_CHECK_PTR(pParam->pName);
            LIVETUNE_CHECK_PTR(pParam->pParamValue);
          }
          pCnx = pCnx->pNext;
        }
      }
      pDef = pDef->pNext;
    }


    pDef = pInstance->pPinIn;
    while (pDef)
    {
      livetune_db_instance_cnx_list *pList = pDef->pObject;
      if ((pList != NULL) && (pList->pNext != NULL))
      {
        ST_ASSERT(pList->pNext->pPrev == pList);
      }

      if (pList)
      {
        livetune_db_instance_cnx_list *pCnx = pList;
        while (pCnx)
        {
          if ((pCnx != NULL) && (pCnx->pNext != NULL))
          {
            ST_ASSERT(pCnx->pNext->pPrev == pCnx);
          }

          livetune_db_instance_cnx *pCnxInst = pCnx->pCnxInstance;


          LIVETUNE_CHECK_PTR(pCnxInst->pInstance);
          LIVETUNE_CHECK_PTR(pCnxInst->pName);
          uint32_t nbParams = livetune_db_list_nb(&pCnxInst->pParams);

          for (uint32_t indexCnx = 0; indexCnx < nbParams; indexCnx++)
          {
            livetune_db_instance_param *pParam = livetune_db_list_get_object(&pCnxInst->pParams, indexCnx);

            LIVETUNE_CHECK_PTR(pParam->pName);
            LIVETUNE_CHECK_PTR(pParam->pParamValue);
          }
          pCnx = pCnx->pNext;
        }
      }
      pDef = pDef->pNext;
    }
  }
}





