/**
******************************************************************************
* @file         audio_chain_json_from_factory.c
* @author       MCD Application Team
* @brief        manage the conversion of factory in json, no dependency on st designer
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
#include "audio_chain.h"
#include "audio_chain_json_from_factory.h"
#include "st_json.h"
#include "st_os_mem.h"
#include "stdio.h"
#include "stdbool.h"

/* Includes ------------------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#define ST_PINS_MAX             0xFFUL

#ifndef ST_FACT_ASSERT
  #define ST_FACT_ASSERT(a)
#endif

#ifndef ST_FACT_VERIFY
  #define ST_FACT_VERIFY(a)     ((void)(a))
#endif

#ifndef ST_FACT_PRINTF
  #define ST_FACT_PRINT(a)      printf(a);printf("\n");
#endif


/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/



static const char *tListIcon[] =
{
  "icon-elem-ac.png",
  "icon-elem-default.png",
  "icon-elem-sink.png",
  "icon-elem-src.png",
  NULL
};



/**
* @brief Fix element name and remove '_' not accepted by the javascript tool
*
*/

static void AudioChainJson_fix_name(json_instance_t *pJsonInst, jsonID rootID)
{
  const char *pTmpString;
  char        tScratch[40], c;
  uint32_t      id       = 0;
  int8_t       bFixName = false;

  json_object_get_string(pJsonInst, rootID, "", "Name", &pTmpString);
  if (pTmpString == NULL)
  {
    pTmpString = "Error: No-name";
  }

  /* The '_' is used as a separator by the designer, so this char is forbidden in the element name => replace it with '-' */
  for (const char *pScratch = pTmpString; *pScratch != '\0'; pScratch++)
  {
    c = *pScratch;
    if (c == '_')
    {
      /* replace '_' with '-' */
      tScratch[id] = '-';
      bFixName     = true;
    }
    else
    {
      tScratch[id] = c;
    }
    id++;
    if (id == ((uint32_t)sizeof(tScratch) - 1U))
    {
      /* pTmpString size bigger than tScratch size */
      break;
    }
  }
  tScratch[id] = '\0';

  pTmpString = tScratch;
  if (bFixName)
  {
    json_object_set_string(pJsonInst, rootID, "", "Name", pTmpString);
    ST_FACT_PRINT("Error: The Element name has a char '_'");
  }
}



/**
* @brief Convert an ac num pindef definition ( by flag) by a real number
*
*/

static uint32_t AudioChainJson_get_nbPinMax(uint32_t flags)
{
  uint32_t nbPindef;
  if ((flags & (uint32_t)AUDIO_CAPABILITY_CHUNK_MULTIPLE) != 0UL)
  {
    nbPindef = ST_PINS_MAX;
  }
  else
  {
    uint32_t pin = flags;

    nbPindef = 0;
    pin >>= 1; /* because of CHUNK_NONE */
    while (pin)
    {
      nbPindef++;
      pin >>= 1;
    }
  }
  return nbPindef;
}




/**
* @brief return the icon name coming from the capabilities
*
*/

static const char *AudioChainJson_get_icon(uint8_t icon)
{
  const char *pIcon = NULL;
  for (uint8_t indexIcon = 0; tListIcon[indexIcon] != 0; indexIcon++)
  {
    if (indexIcon == icon)
    {
      pIcon = tListIcon[indexIcon];
      break;
    }
  }
  return pIcon;
}



/**
* @brief return the pindef name
*
*/

static const char *AudioChainJson_get_pin_name(const char *pStrings, uint32_t pinDef, char *pDefault)
{
  const char *pName = pDefault;
  if (pStrings)
  {
    const char *pCurName  = pStrings;
    uint32_t      indexName = 0;
    while (*pCurName)
    {
      if (indexName == pinDef)
      {
        pName = pCurName;
        break;
      }
      pCurName = strchr(pCurName, 0) + 1;
      indexName++;
    }
  }
  return pName;
}



/**
* @brief return the default string  after the filter, if the preferred is not found, return the first filtered
*
*/

static uint32_t AudioChainJson_get_default_index(const audio_descriptor_key_value_t *pKeyValue, uint32_t iFilterSel, uint32_t preferred)
{
  uint32_t iFilter       = 1UL;
  uint32_t defaultVal    = 0UL;
  bool     firstFiltered = false;

  while (pKeyValue->pKey)
  {
    if (iFilterSel & iFilter)
    {
      if (!firstFiltered)
      {
        defaultVal    = pKeyValue->iValue;
        firstFiltered = true;
      }
      if (preferred == pKeyValue->iValue)
      {
        defaultVal = preferred;
        break;
      }
    }
    pKeyValue++;
    iFilter <<= 1;
  }

  return defaultVal;
}



/**
* @brief Generate a drop list from a attrib key value
*
*/

static uint32_t AudioChainJson_generate_droplist_index(const audio_descriptor_key_value_t *pKeyValue, char *pScratch, uint32_t szScratch, uint32_t iFilterSel, int32_t bAddNoChange, uint32_t *pLastValue)
{
  uint32_t iFilter = 1;
  uint32_t index   = 0;
  uint32_t i       = 0;
  ST_FACT_ASSERT((i + 1U) < szScratch);
  pScratch[i++] = '[';
  while (pKeyValue->pKey)
  {
    if (iFilterSel & iFilter)
    {
      if (index)
      {
        ST_FACT_ASSERT((i + 1U) < szScratch);
        pScratch[i++] = ',';
      }
      if (pLastValue)
      {
        *pLastValue = pKeyValue->iValue;
      }
      i += (uint32_t)snprintf(&pScratch[i], szScratch - i, "{'Name':'%s','Value':\"%d\"}", pKeyValue->pKey, pKeyValue->iValue);
      ST_FACT_ASSERT(i < szScratch);
      index++;
    }
    else if (iFilter > iFilterSel)
    {
      /* no need to go further */
      break;
    }
    pKeyValue++;
    iFilter <<= 1;
  }

  if (bAddNoChange)
  {
    if (index)
    {
      ST_FACT_ASSERT((i + 1U) < szScratch);
      pScratch[i++] = ',';
    }
    i += (uint32_t)snprintf(&pScratch[i], szScratch - i, "{'Name':'NO_CHANGE','Value':'NO_CHANGE'}");
    ST_FACT_ASSERT(i < szScratch);
  }
  ST_FACT_ASSERT((i + 2U) < szScratch);
  pScratch[i++] = ']';
  pScratch[i]   = '\0'; /* end of string */

  return index;
}

/**
* @brief Convert a pseudo json in json
         it is pity, be we need to convert a pseudo json in real json coming from the default,
         to offload the need of escaping \" we have removed '"' from the array definition to satisfy Tuning model requirement
         but, now we need to do the opposite to satisfy the livetune model

* @param pPseudoJson  pseudo json instance
* @return  string real json
*/

static const char *AudioChainJson_convert_pseudo_json(const char *pPseudoJson)
{
  jsonID        hObj;
  char        tScratch[200];
  int32_t       error       = -1;
  const char *pJsonString = NULL;

  json_instance_t jsonInst = {0};
  json_create_object(&jsonInst, &hObj);
  ST_FACT_ASSERT(hObj != JSON_ID_NULL);
  const char *pStream = pPseudoJson;
  if ((*pStream == '{') && (pStream[(uint32_t)strlen(pStream) - 1U] == '}'))
  {
    /* if we have {..} and the size is 2, the struct is empty */
    if (strlen(pStream) == 2U)
    {
      error = 0;
    }
    else
    {
      pStream++; // skip '{'
      /* the array is form Index ':[' data,data...] */
      while (*pStream)
      {
        char   *pEnd;
        int32_t index = strtol(pStream, &pEnd, 10); /*cstat !MISRAC2012-Rule-22.8 !MISRAC2012-Rule-22.9 errno not supported by this OS */
        if (pEnd != pStream)
        {
          /* the attribute is a Table entry */
          jsonID hArray;
          ST_FACT_VERIFY(json_create_array(&jsonInst, &hArray) == JSON_OK);
          snprintf(tScratch, sizeof(tScratch), "%d", index);
          ST_FACT_VERIFY(json_object_set_new(&jsonInst, hObj, tScratch, hArray) == JSON_OK);
          if ((pEnd[0] != ':') || (pEnd[1] != '['))
          {
            break;
          }
          pEnd++; // skip ':'
          pEnd++; // skip ['

          char *pStartStruct = pEnd;
          char *pEndStruct   = pEnd;
          while ((*pEndStruct != '\0') && (*pEndStruct != ']'))
          {
            pEndStruct++;
          }
          if (*pEndStruct != ']')
          {
            error = -1;
            break;
          }
          pEndStruct++; //skip  ']'
          memmove(tScratch, pStartStruct, (pEndStruct - pStartStruct) - 1UL);
          tScratch[(pEndStruct - pStartStruct) - 1] = '\0';
          jsonID hString;
          ST_FACT_VERIFY(json_create_string(&jsonInst, tScratch, &hString) == JSON_OK);
          ST_FACT_VERIFY(json_array_append_new(&jsonInst, hArray, hString) == JSON_OK);
          pStream = pEndStruct;
        }
        else
        {
          /* the attribute is a key value */
          char *pKey = tScratch;
          while ((*pStream != '\0') && (*pStream != ':'))
          {
            *pKey++ = *pStream++;
          }
          *pKey++ = '\0';
          if (*pStream != ':')
          {
            error = -1;
            break;
          }
          pStream++; // skip ':'
          char *pValueStart = pKey;
          char *pValue      = pKey;

          while ((*pStream != '\0') && (*pStream != ',') && (*pStream != '}'))
          {
            *pValue++ = *pStream++;
          }
          *pValue = '\0';
          ST_FACT_VERIFY(json_object_set_string(&jsonInst, hObj, "", tScratch, pValueStart) == JSON_OK);
        }
        /* Check end of series, in this cas exit with OK */
        if (*pStream == '}')
        {
          error = 0;
          break;
        }
        /* Check new series, continue */
        if (*pStream == ',')
        {
          pStream++;
        }
        else
        {
          error = -1;
          break;
        }
      }
    }
  }
  if (error == 0)
  {
    pJsonString = json_dumps(&jsonInst, hObj, 0);
  }
  json_shutdown(&jsonInst);
  return pJsonString;
}


/**
* @brief we need to translate a value according to its description, in this case we have to convert
*        the primary value to its acSdk value
         the return string must be freed by the caller
*
*/

char_t *AudioChainJson_factory_translate_key_value(const audio_descriptor_param_t *pParam, const char_t *pStringValue)
{
  int32_t bTranslateDone = FALSE;
  char_t *pAcValue       = NULL;
  if (pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_ALWAYS_DEFAULT)
  {
    /*  This flag allows overwriting the stored value in the LiveTune database with the default value stored in the algorithm.
        It is a workaround to fix a side effect when a project is done with an algorithm using pointer referencing data in the memory flash.
        This pointer is presented in LiveTune as grayed information that is not modifiable, but the pointer could change according to the board and cause a start failure if we load a project for a board different from the board running LiveTune.
        The pointer referenced in the LiveTune project becomes wrong, and the algorithm cannot verify the signature.
    */
    pStringValue = pParam->pDefault;
  }


  if (((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_TRANSLATE_KEY_VALUE) != 0U) && (pParam->pKeyValue != NULL))
  {
    const char_t *pDefine = AudioDescriptor_getKeyFromKeyId(pParam->pKeyValue, (uint32_t)atoi((char_t *)pStringValue));
    if (pDefine)
    {
      pAcValue = st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, (uint32_t)strlen(pDefine) + 1U);
      strcpy(pAcValue, pDefine);
      bTranslateDone = TRUE;
    }
    else
    {
      char errorString[100];
      snprintf(errorString, sizeof(errorString), "Key value %s fails for param %s", pStringValue, pParam->pName);
      ST_FACT_PRINT(errorString);
    }
  }
  if (((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_AS_ARRAY) != 0U) && (pParam->pDefArray != NULL))
  {
    /* We start with the size of string json, the result size will always be smaller because we remove chars */
    pAcValue = st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, (uint32_t)strlen(pStringValue) + 1U);
    if (pAcValue)
    {
      char_t *pDest = pAcValue;
      /* to simplify the coding we remove useless quote */
      const char_t *pSrc = pStringValue;
      while (*pSrc)
      {
        if (*pSrc != '\"')
        {
          *pDest++ = *pSrc++;
        }
        else
        {
          pSrc++;
        }
      }
      *pDest = '\0';
      /* reduce to the exec mem used, normally same pointer because we resize */
      pAcValue       = st_os_mem_realloc(ST_Mem_Type_ANY_SLOW, pAcValue, (uint32_t)strlen(pAcValue) + 1U);
      bTranslateDone = TRUE;
    }
  }

  if (bTranslateDone == FALSE)
  {
    /* We need to allocated the string even if we do nothing because the caller will free it */
    pAcValue = st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, (uint32_t)strlen(pStringValue) + 1U); /*cstat !MISRAC2012-Rule-22.1_a false positif, the API allocated a memory and return the block to the client */
    if (pAcValue)
    {
      strcpy(pAcValue, pStringValue);
    }
  }
  ST_FACT_ASSERT(pAcValue != NULL);
  return pAcValue;
}


/**
* @brief Fill json fields coming from ac parameters
*
* @param pJsonInst  json instance
* @param hObj     root key
* @param  pParam  ac desc param
* @param  type   ac desc type

*/


void AudioChainJson_update_fill_instance_param(json_instance_t *pJsonInst, jsonID hObj, const audio_descriptor_param_t *pParam, int32_t type)
{
  double        min;
  double        max;
  const char *pDescription;

  AudioDescriptor_getMinMax(pParam, &min, &max);

  json_object_set_string(pJsonInst, hObj, "", "Name", pParam->pName);
  json_object_set_number(pJsonInst, hObj, "", "Min", min);
  json_object_set_number(pJsonInst, hObj, "", "Max", max);
  json_object_set_integer(pJsonInst, hObj, "", "Type", type);
  if (pParam->pCustom)
  {
    json_object_set_string(pJsonInst, hObj, "", "Custom", pParam->pCustom);
  }

  if (pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_DISABLED)
  {
    json_object_set_boolean(pJsonInst, hObj, "", "Disabled", true);
  }
  if (((pParam->iParamFlag & (AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_VALUE | AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_STRING)) == 0UL))
  {
    if (
      (pParam->paramType == AUDIO_DESC_PARAM_TYPE_INT8) ||
      (pParam->paramType == AUDIO_DESC_PARAM_TYPE_UINT8) ||
      (pParam->paramType == AUDIO_DESC_PARAM_TYPE_INT16) ||
      (pParam->paramType == AUDIO_DESC_PARAM_TYPE_UINT16) ||
      (pParam->paramType == AUDIO_DESC_PARAM_TYPE_INT32) ||
      (pParam->paramType == AUDIO_DESC_PARAM_TYPE_UINT32) ||
      (pParam->paramType == AUDIO_DESC_PARAM_TYPE_ADDRESS))
    {
      json_object_set_boolean(pJsonInst, hObj, "", "Int", true);
    }
  }

  if (pParam->pDefault)
  {
    json_object_set_string(pJsonInst, hObj, "", "Default", pParam->pDefault);
  }
  else
  {
    json_object_set_number(pJsonInst, hObj, "", "Default", ((max + min) / 2.0F));
  }
  pDescription = pParam->pName;
  if (pParam->pDescription)
  {
    pDescription = pParam->pDescription;
  }
  json_object_set_string(pJsonInst, hObj, "", "Description", pDescription);
  if (pParam->pControl)
  {
    json_object_set_string(pJsonInst, hObj, "", "Control", pParam->pControl);
  }
  if (pParam->pGroup)
  {
    json_object_set_string(pJsonInst, hObj, "", "GroupName", pParam->pGroup);
  }


  if ((pParam->pKeyValue != NULL) && ((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_VALUE) != 0UL))
  {
    const audio_descriptor_key_value_t *pKeyValue = pParam->pKeyValue;
    json_instance_t                     jsonInst;
    memset(&jsonInst, 0, sizeof(jsonInst));
    jsonID hArray;
    json_create_array(&jsonInst, &hArray);
    ST_FACT_ASSERT(hArray != JSON_ID_NULL);
    if (hArray)
    {
      while (pKeyValue->pKey)
      {
        jsonID hDropObj;
        json_create_object(&jsonInst, &hDropObj);
        ST_FACT_ASSERT(hDropObj != JSON_ID_NULL);
        if (hDropObj)
        {
          json_object_set_string(&jsonInst, hDropObj, "", "Name", pKeyValue->pKey);
          json_object_set_integer(&jsonInst, hDropObj, "", "Value", (int64_t)pKeyValue->iValue);
          json_array_append_new(&jsonInst, hArray, hDropObj);
        }
        pKeyValue++;
      }
      char *pJson = (char *)json_dumps(&jsonInst, hArray, 0);
      ST_FACT_ASSERT(pJson != NULL);
      if (pJson)
      {
        /* convert the quotes */
        char *pString = pJson;
        while (*pString)
        {
          if (*pString == '\"')
          {
            *pString = '\'';
          }
          pString++;
        }
        json_object_set_string(pJsonInst, hObj, "", "ControlParams", pJson);
        st_os_mem_free(pJson);
      }
    }
    json_shutdown(&jsonInst);
  }

  if ((pParam->pKeyValue != NULL) && ((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_STRING) != 0UL))
  {
    const audio_descriptor_key_string_t *pKeyString = pParam->pKeyString;
    json_instance_t                     jsonInst;
    memset(&jsonInst, 0, sizeof(jsonInst));
    jsonID hArray;
    json_create_array(&jsonInst, &hArray);
    ST_FACT_ASSERT(hArray != JSON_ID_NULL);
    if (hArray)
    {
      while (pKeyString->pKey)
      {
        jsonID hDropObj;
        json_create_object(&jsonInst, &hDropObj);
        ST_FACT_ASSERT(hDropObj != JSON_ID_NULL);
        if (hDropObj)
        {
          char *pDecoded = st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, strlen(pKeyString->pValue) + 1U);
          ST_FACT_ASSERT(pDecoded != NULL);
          if (pDecoded)
          {
            strcpy(pDecoded, pKeyString->pValue);
            /* convert the quotes */
            char *pString = pDecoded;
            while (*pString)
            {
              if (*pString == '\'')
              {
                *pString = '"';
              }
              pString++;
            }
            json_object_set_string(&jsonInst, hDropObj, "", "Name", pKeyString->pKey);
            json_object_set_string(&jsonInst, hDropObj, "", "Value", pDecoded);
            json_array_append_new(&jsonInst, hArray, hDropObj);
            st_os_mem_free(pDecoded);
          }
        }
        pKeyString++;
      }
      char *pJson = (char *)json_dumps(&jsonInst, hArray, 0);
      if (pJson)
      {
        json_object_set_string(pJsonInst, hObj, "", "ControlParams", pJson);
        st_os_mem_free(pJson);
      }
    }
    json_shutdown(&jsonInst);
  }

  if ((pParam->pDefArray != NULL) && ((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_AS_ARRAY) != 0UL))
  {
    json_instance_t jsonInst = {0};
    if (json_create_object(&jsonInst, &jsonInst.pack_root) == JSON_OK)
    {
      jsonID arrayID;
      ST_FACT_VERIFY(json_object_set_integer(&jsonInst, jsonInst.pack_root, "", "MaxEntries", (int64_t)pParam->structDescr.structNbMaxInstances) == JSON_OK);
      uint8_t applyMsk = (uint8_t)((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_WANT_APPLY) != 0UL);
      ST_FACT_VERIFY(json_object_set_integer(&jsonInst, jsonInst.pack_root, "", "WantApply", (int64_t)applyMsk) == JSON_OK);

      const char *pDefault = "{}";
      if (pParam->pDefault)
      {
        pDefault = pParam->pDefault;
      }
      const char *pPseudoJson = AudioChainJson_convert_pseudo_json(pDefault);
      ST_FACT_ASSERT(pPseudoJson != NULL);
      json_object_set_string(pJsonInst, hObj, "", "Default", pPseudoJson);
      json_free((void *)pPseudoJson);


      ST_FACT_VERIFY(json_create_array(&jsonInst, &arrayID) == JSON_OK);
      ST_FACT_VERIFY(json_object_set_new(&jsonInst, jsonInst.pack_root, "Records", arrayID) == JSON_OK);


      const audio_descriptor_param_t *pParamIterator = pParam->pDefArray;
      while (pParamIterator->pName)
      {
        jsonID hElement;
        if (json_create_object(&jsonInst, &hElement) == JSON_OK)
        {
          AudioChainJson_update_fill_instance_param(&jsonInst, hElement, pParamIterator, type);
          ST_FACT_VERIFY(json_array_append_new(&jsonInst, arrayID, hElement) == JSON_OK);
        }
        pParamIterator++;
      }
    }
    const char *pDesc = json_dumps(&jsonInst, jsonInst.pack_root, 0);
    json_shutdown(&jsonInst);
    if (pDesc)
    {
      json_object_set_string(pJsonInst, hObj, "", "ControlParams", pDesc);
      json_free((void *)pDesc);
    }
  }
}





void AudioChainJson_add_attrib(json_instance_t *pJsonInst, jsonID rootID, audio_algo_common_t const *pCap)
{
  int32_t  error     = AUDIO_ERR_MGNT_NONE;
  int32_t  paramPos  = 0;
  uint32_t nbItem    = 0;
  uint32_t iDefault  = 0;
  uint32_t lastValue = 0U;
  bool  bGrayed   = false;
  char     tString[20];
  char  *pScratch  = st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, LIVETUNE_CHUNK_SCRATCH_SIZE);
  ST_FACT_ASSERT(pScratch != NULL);

  const audio_descriptor_params_t *pChunkDescriptor = AudioChainFactory_getChunkTemplate();


  jsonID hParams;
  ST_FACT_VERIFY(json_create_array(pJsonInst, &hParams) == JSON_OK);
  ST_FACT_VERIFY(json_object_set_new(pJsonInst, rootID, "Params", hParams) == JSON_OK);

  /* if the element is a generator, the no_change as no meaning */
  bool bAddNoChange = (((uint32_t)pCap->misc.flags & AUDIO_ALGO_FLAGS_MISC_INIT_DEFAULT) != 0U) ? false : true;

  /* if there is a single attrib, we assume a propagation from in to out */

  struct
  {
    uint32_t      iFilterSel;
    const char *pName;
  } chunkAttrib[7] =
  {
    {
      .iFilterSel = pCap->iosOut.type,
      .pName      = "bufferType"
    },
    {
      .iFilterSel = pCap->iosOut.time_freq,
      .pName      = "timeFreq"
    },
    {
      .iFilterSel = UINT32_MAX,
      .pName      = "nbElements"
    },
    {
      .iFilterSel = pCap->iosOut.fs,
      .pName      = "fs"
    },
    {
      .iFilterSel = pCap->iosOut.nbChan,
      .pName      = "nbChannels"
    },
    {
      .iFilterSel = pCap->iosOut.interleaving,
      .pName      = "interleaved"
    },
    {
      .iFilterSel = UINT32_MAX,
      .pName      = "nbFrames"
    }
  };

  for (int32_t i = 0; i < (int32_t)(sizeof(chunkAttrib) / sizeof(chunkAttrib[0])); i++)
  {
    bGrayed = false;
    const audio_descriptor_param_t *pParam;
    error = AudioDescriptor_getParam(pChunkDescriptor, chunkAttrib[i].pName, &pParam, NULL);
    if (error == 0)
    {
      /* Check if the parameter has a keyvalue list */
      if (pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_VALUE)
      {
        /* first build the key value list json */
        nbItem = AudioChainJson_generate_droplist_index(pParam->pKeyValue, pScratch, LIVETUNE_CHUNK_SCRATCH_SIZE, chunkAttrib[i].iFilterSel, (int32_t)bAddNoChange, &lastValue);
        if (nbItem == 1U)
        {
          /* if there is only one value, we set it as default and the control will be be grayed to prevent change */
          iDefault = lastValue;
          bGrayed  = true;
        }
        else
        {
          /* if there are several values, we need to select a valid entry as the default */
          iDefault = AudioChainJson_get_default_index(pParam->pKeyValue, chunkAttrib[i].iFilterSel, (uint32_t)atoi((char *)pParam->pDefault));
        }
      }
      else
      {
        /* if just a value, put it as default */
        iDefault = (uint32_t)atoi((char *)pParam->pDefault);
      }


      /* convert as string */
      snprintf(tString, sizeof(tString) - 1U, "%d", iDefault);
      const char *pDefault = tString;
      /* if bAddNoChange is true, the control must be initialized with "NO_CHANGE" by default */
      if ((bAddNoChange != 0) && (nbItem != 1U))
      {
        pDefault = CNX_PARAM_SKIP_MARKER;
      }
      double min, max;
      AudioDescriptor_getMinMax(pParam, &min, &max);
      AudioChainJson_json_add_param(pJsonInst,
                                    hParams,
                                    paramPos++,
                                    pParam->pDescription,
                                    0,
                                    pParam->pName,
                                    NULL,
                                    pParam->pControl,
                                    pScratch,
                                    pDefault,
                                    min,
                                    max,
                                    (uint8_t)bGrayed);
    }
  }

  if (pScratch)
  {
    st_os_mem_free(pScratch);
  }
}




/**
* @brief add a json parameters
*
* @param pJsonInst  json instance
* @param rootID     hCnx
*/



void AudioChainJson_json_add_param(json_instance_t *pJsonInst, jsonID hCnx, int32_t paramPos, const char *pDescription, uint32_t type, const char *pName, const char *pValue, const char *pControl, const char *pControlParam, const char *pDefault, double min, double max, uint8_t bDisabled)
{
  jsonID hObj;
  json_create_object(pJsonInst, &hObj);
  ST_FACT_ASSERT(hObj != JSON_ID_NULL);
  if (pDescription)
  {
    ST_FACT_VERIFY(json_object_set_string(pJsonInst, hObj, "", "Description", pDescription) == JSON_OK);
  }
  ST_FACT_VERIFY(json_object_set_integer(pJsonInst, hObj, "", "Type", (int64_t)type) == JSON_OK);
  if (pName)
  {
    ST_FACT_VERIFY(json_object_set_string(pJsonInst, hObj, "", "Name", pName) == JSON_OK);
  }
  if (pDefault)
  {
    ST_FACT_VERIFY(json_object_set_string(pJsonInst, hObj, "", "Default", pDefault) == JSON_OK);
  }
  if (pControl)
  {
    ST_FACT_VERIFY(json_object_set_string(pJsonInst, hObj, "", "Control", pControl) == JSON_OK);
  }
  if (pControlParam)
  {
    ST_FACT_VERIFY(json_object_set_string(pJsonInst, hObj, "", "ControlParams", pControlParam) == JSON_OK);
  }
  if (bDisabled)
  {
    ST_FACT_VERIFY(json_object_set_boolean(pJsonInst, hObj, "", "Disabled", 1) == JSON_OK);
  }

  ST_FACT_VERIFY(json_object_set_number(pJsonInst, hObj, "", "Min", min) == JSON_OK);
  ST_FACT_VERIFY(json_object_set_number(pJsonInst, hObj, "", "Max", max) == JSON_OK);

  ST_FACT_VERIFY(json_array_insert_new(pJsonInst, hCnx, paramPos, hObj) == JSON_OK);
}






/**
* @brief Add a pindef to the json
  Notice the number of pindef is given by the capabilities and pin description
*
*/
void AudioChainJson_add_pindef(json_instance_t *pJsonInst, jsonID rootID, audio_algo_common_t const *pCap, const audio_capabilities_ios_descr_t *pAcPin, uint32_t indexPinDef, uint32_t cnxLimit, char *pPinType)
{
  jsonID      objID;
  const char *pName = AudioChainJson_get_pin_name(pAcPin->pPinNames, indexPinDef, pPinType);
  ST_FACT_VERIFY(json_create_object(pJsonInst, &objID) == JSON_OK);
  ST_FACT_VERIFY(json_object_set_string(pJsonInst, objID, "", "Name", pName) == JSON_OK);
  ST_FACT_VERIFY(json_object_set_string(pJsonInst, objID, "", "Type", "acChunk") == JSON_OK);
  ST_FACT_VERIFY(json_object_set_integer(pJsonInst, objID, "", "ConnectionMax", (int64_t)cnxLimit) == JSON_OK);
  if (pAcPin->pDescs)
  {
    const char *pDescription = AudioChainJson_get_pin_name(pAcPin->pDescs, indexPinDef, "");
    ST_FACT_VERIFY(json_object_set_string(pJsonInst, objID, "", "Description", pDescription) == JSON_OK);
  }
  else
  {
    char tString[20];
    snprintf(tString, sizeof(tString) - 1U, "Chunk %s", pPinType);
    ST_FACT_VERIFY(json_object_set_string(pJsonInst, objID, "", "Description", tString) == JSON_OK);
  }
  if (pCap)
  {
    AudioChainJson_add_attrib(pJsonInst, objID, pCap);
  }
  ST_FACT_VERIFY(json_array_append_new(pJsonInst, rootID, objID) == JSON_OK);
}



/**
* @brief load pindef capabilities coming from the algo template
*
*/

void AudioChainJson_update_json_element_capabilities(json_instance_t *pJsonInst, jsonID rootID, const audio_algo_factory_t *pFactory)
{
  if ((pFactory != NULL) && (pFactory->pCapabilities != NULL))
  {
    audio_algo_common_t const *pCap = pFactory->pCapabilities;
    /* First update the basic header info */
    if (pCap->pName)
    {
      ST_FACT_VERIFY(json_object_set_string(pJsonInst, rootID, "", "Name", pCap->pName) == JSON_OK);
    }
    if (pCap->misc.pAlgoHelp)
    {

      ST_FACT_VERIFY(json_object_set_string(pJsonInst, rootID, "", "httpHelp", pCap->misc.pAlgoHelp) == JSON_OK);
    }
    const char *pIcon = AudioChainJson_get_icon(pCap->misc.icon);
    if (pIcon)
    {
      ST_FACT_VERIFY(json_object_set_string(pJsonInst, rootID, "", "Icon", pIcon) == JSON_OK);
    }

    if (pCap->misc.pAlgoDesc)
    {
      ST_FACT_VERIFY(json_object_set_string(pJsonInst, rootID, "", "Description", pCap->misc.pAlgoDesc) == JSON_OK);
    }

    if (pCap->misc.flags & AUDIO_ALGO_FLAGS_MISC_SINGLE_INSTANCE)
    {
      ST_FACT_VERIFY(json_object_set_string(pJsonInst, rootID, "", "InstanceMax", "1") == JSON_OK);
    }

    /* CreatePin def param according to its capabilities*/

    const audio_capabilities_ios_descr_t *pAcPin = &pCap->iosOut;
    uint32_t                              nbPin  = AudioChainJson_get_nbPinMax((uint32_t)pAcPin->nb);
    if (nbPin)
    {
      /* we have at least 1 pindef exposed */

      jsonID hPinDefOut;
      json_object_get_id_from_tree(pJsonInst, rootID, "PinDefOut", &hPinDefOut);
      if (hPinDefOut == JSON_ID_NULL)
      {
        /* the array doesn't exist, let's create it */
        ST_FACT_VERIFY(json_create_array(pJsonInst, &hPinDefOut) == JSON_OK);
        ST_FACT_VERIFY(json_object_set_new(pJsonInst, rootID, "PinDefOut", hPinDefOut) == JSON_OK);
      }

      /*
      A algo could have various links , 1 to 5 or multiple,
      Audio chain mixup the concept of bus ( pindef) and connection to a bus
      At the end we must be able to connect the bus in the right order because the link ID is the order of attachment

      Audio chain defines the number of link ID accepted for its pin,
      The basic rule is to expose a single pindef and set the maximum connection ( the reasonable count is 255)
      It is possible to create several pins to sort by link IDS ( order of connection to an algo)

      But for multiple link, we cannot create xx pins on the box ( spacing and graphic overload).
      In this case, we create a single pin with an unlimited number of connection
      for the time being, only split and router  use this capability

      Description
      IE :
      AUDIO_CAPABILITY_CHUNK_ONE = ConnectionMax=1 by pindef , 1 pindef
      AUDIO_CAPABILITY_CHUNK_TWO = ConnectionMax=1 by pindef , 2 pindef
      AUDIO_CAPABILITY_CHUNK_THREE = ConnectionMax=1 by pindef , 3 pindef
      AUDIO_CAPABILITY_CHUNK_FOUR = ConnectionMax=1 by pindef , 4 pindef
      AUDIO_CAPABILITY_CHUNK_FIVE = ConnectionMax= 1 by pindef , 5 pindef
      AUDIO_CAPABILITY_CHUNK_MULTIPLE = ConnectionMax=255 ( or more), 1 pindef

      */


      if ((((uint32_t)pAcPin->nb & (uint32_t)AUDIO_CAPABILITY_CHUNK_MULTIPLE) != 0U) && (nbPin != 0U))
      {
        AudioChainJson_add_pindef(pJsonInst, hPinDefOut, pCap, pAcPin, 0, ST_PINS_MAX, "Out");
      }
      else
      {
        for (uint32_t indexPinDef = 0; indexPinDef < nbPin; indexPinDef++)
        {
          AudioChainJson_add_pindef(pJsonInst, hPinDefOut, pCap, pAcPin, indexPinDef, 1, "Out");
        }
      }
    }

    pAcPin = &pCap->iosIn;
    nbPin  = AudioChainJson_get_nbPinMax((uint32_t)pAcPin->nb);
    if (nbPin)
    {
      jsonID hPinDefIn;
      json_object_get_id_from_tree(pJsonInst, rootID, "PinDefIn", &hPinDefIn);
      if (hPinDefIn == JSON_ID_NULL)
      {
        /* the array doesn't exist, let's create it */
        ST_FACT_VERIFY(json_create_array(pJsonInst, &hPinDefIn) == JSON_OK);
        ST_FACT_VERIFY(json_object_set_new(pJsonInst, rootID, "PinDefIn", hPinDefIn) == JSON_OK);
      }
      if ((((uint32_t)pAcPin->nb & (uint32_t)AUDIO_CAPABILITY_CHUNK_MULTIPLE) != 0U) && (nbPin != 0U))
      {
        /* remove the cap because in ac pin in as no parameters */
        AudioChainJson_add_pindef(pJsonInst, hPinDefIn, NULL, pAcPin, 0, ST_PINS_MAX, "In");
      }
      else
      {
        for (uint32_t indexPinDef = 0; indexPinDef < nbPin; indexPinDef++)
        {
          /* remove the cap because in ac pin in as no parameters */
          AudioChainJson_add_pindef(pJsonInst, hPinDefIn, NULL, pAcPin, indexPinDef, 1, "In");
        }
      }
    }
  }
}




/**
* @brief add json instance entry if the key is not found
*
* @param pJsonInst  json instance
* @param rootID     root key
* @return pTemplate ac tuning template
*/


void AudioChainJson_update_instance_params(json_instance_t *pJsonInst, jsonID hParam, const audio_descriptor_params_t *pTemplate, int32_t type, int32_t *pIndex)
{
  if (pTemplate)
  {
    for (uint8_t IndexParam = 0; IndexParam < pTemplate->nbParams; IndexParam++)
    {
      audio_descriptor_param_t *pParam = &pTemplate->pParam[IndexParam];
      jsonID                    hObj;
      json_create_object(pJsonInst, &hObj);
      ST_FACT_ASSERT(hObj != JSON_ID_NULL);
      AudioChainJson_update_fill_instance_param(pJsonInst, hObj, pParam, type);
      json_array_insert_new(pJsonInst, hParam, *pIndex, hObj);
      (*pIndex)++;
    }
  }
}

/*
* @brief add json instance entry if the key is not found
* @param pFactory   instance
* @param pJsonInst  json instance
* @param rootID     root key
* @return error code
*/

int32_t  AudioChainJson_update_element(const audio_algo_factory_t *pFactory, json_instance_t *pJsonInst, jsonID rootID)
{
  int32_t error = 0;
  if (pFactory)
  {

    if (pFactory->pFactoryConstructorCb)
    {
      pFactory->pFactoryConstructorCb(pFactory);
    }
    jsonID hParam;
    json_object_get_id_from_tree(pJsonInst, rootID, "Params", &hParam);
    if (hParam == JSON_ID_NULL)
    {
      /* the array doesn't exist, let's create it */
      ST_FACT_VERIFY(json_create_array(pJsonInst, &hParam) == JSON_OK);
      ST_FACT_VERIFY(json_object_set_new(pJsonInst, rootID, "Params", hParam) == JSON_OK);
    }

    if (hParam)
    {
      audio_algo_common_t const *pCap = pFactory->pCapabilities;
      /* the params already present in the json are stronger than the algo exposed to (to allow devel overload)
      so we load them in a list */
      if ((pCap->misc.flags & AUDIO_ALGO_FLAGS_MISC_IGNORE_ALGO_PARAMS) == 0U)
      {
        int32_t indexControl = 0;

        AudioChainJson_update_instance_params(pJsonInst, hParam, pFactory->pStaticParamTemplate, 0, &indexControl);
        AudioChainJson_update_instance_params(pJsonInst, hParam, pFactory->pDynamicParamTemplate, 1, &indexControl);
      }
    }

    AudioChainJson_update_json_element_capabilities(pJsonInst, rootID, pFactory);
  }
  AudioChainJson_fix_name(pJsonInst, rootID);
  return error ;
}