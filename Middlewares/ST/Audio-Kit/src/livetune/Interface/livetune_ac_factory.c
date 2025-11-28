/**
******************************************************************************
* @file          livetune_ac_factory.c
* @author        MCD Application Team
* @brief         manage the access to ac factory and descriptors
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
#include "livetune.h"
#include "livetune_ac_wrapper.h"
#include "audio_chain.h"
#include "audio_chain_factory.h"
#include "audio_chain_sysIOs.h"
#include "audio_chain_sysIOs_conf.h"

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/




/**
* @brief return the algo factory from the element pointer
*
* @param pElement the  instance handle
*/


const audio_algo_factory_t *livetune_ac_factory_get_factory_from_element(livetune_db_element *pElement)
{
  const audio_algo_factory_t *pFactory = NULL;

  const livetune_helper_builder_def *pBuilderDef = (const livetune_helper_builder_def *)pElement->pRefBuilder;
  if ((pBuilderDef != NULL) && (pBuilderDef->pAlgoName != NULL))
  {
    pFactory = livetune_ac_factory_get_algo_factory_name(pBuilderDef->pAlgoName);
  }
  return pFactory;
}


/**
* @brief  Init the sysio
* @param  type:     sysio type
* @todo   AudioChainSysIOs_initxxxx are not the clean, change for a better way to init sysio
*/
void livetune_ac_factory_init_sysio(const char_t *pSysName, livetune_db_instance_cnx *pCnx)
{
  if (strcmp(pSysName, AC_SYSOUT_BLE_LC3_NAME) == 0)
  {
    pCnx->hAc.pSysIoName = (char_t *)AC_SYSOUT_BLE_LC3_NAME;
    pCnx->hAc.bValid     = TRUE;
  }
  /*else if (strcmp(pSysName, AC_SYSIN_USB_NAME) == 0)
  {
    pCnx->hAc.pSysIoName = (char_t *)AC_SYSIN_USB_NAME;
    pCnx->hAc.bValid     = TRUE;
  }*/
  else if (strcmp(pSysName, AC_SYSIN_PDM_NAME) == 0)
  {
    pCnx->hAc.pSysIoName = (char_t *)AC_SYSIN_PDM_NAME;
    pCnx->hAc.bValid     = TRUE;
  }
  /*else if (strcmp(pSysName, AC_SYSOUT_USB_NAME) == 0)
  {
    pCnx->hAc.pSysIoName = (char_t *)AC_SYSOUT_USB_NAME;
    pCnx->hAc.bValid     = TRUE;
  }*/
  else if (strcmp(pSysName, AC_SYSOUT_CODEC_DAC_NAME) == 0)
  {
    pCnx->hAc.pSysIoName = (char_t *)AC_SYSOUT_CODEC_DAC_NAME;
    pCnx->hAc.bValid     = TRUE;
  }
  else if (strcmp(pSysName, AC_SYSIN_BLE_LC3_NAME) == 0)
  {
    pCnx->hAc.pSysIoName = (char_t *)AC_SYSIN_BLE_LC3_NAME;
    pCnx->hAc.bValid     = TRUE;
  }
  else if (strcmp(pSysName, AC_SYSIN_WAVFILE_NAME) == 0)
  {
    pCnx->hAc.pSysIoName = (char_t *)AC_SYSIN_WAVFILE_NAME;
    pCnx->hAc.bValid     = TRUE;
  }
  else
  {
    ST_TRACE_ERROR("Sys Io %s not initialized", pSysName);
  }
}

/**
 * @brief return the parameter algo descriptor
 *
 * @param hAlgo algo instance
 * @param pKey  the parameter name
 * @return const audio_descriptor_param_t*
 */
const audio_descriptor_param_t *livetune_ac_factory_get_algo_param(acAlgo hAlgo, const char_t *pKey)
{
  int32_t                         error    = AUDIO_ERR_MGNT_NONE;
  const audio_descriptor_param_t *pParam   = NULL;
  const audio_algo_factory_t     *pFactory = AudioAlgo_getFactory(hAlgo);

  if (pFactory == NULL)
  {
    ST_TRACE_ERROR("no algo factory");
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  if (acErrorIsOk(error))
  {
    error = AudioDescriptor_getParam(pFactory->pDynamicParamTemplate, pKey, &pParam, NULL);
    if (error == AUDIO_ERR_MGNT_NOT_FOUND)
    {
      error = AudioDescriptor_getParam(pFactory->pStaticParamTemplate, pKey, &pParam, NULL);
    }
    if (error == AUDIO_ERR_MGNT_NOT_FOUND)
    {
      ST_TRACE_ERROR("param called %s not found in factory of %s", pKey, pFactory->pCapabilities->pName);
    }
    else if (acErrorIsError(error))
    {
      ST_TRACE_ERROR("error in %s", __FUNCTION__);
    }
  }

  return pParam;
}


/**
* @brief Get the min max according to its type
*
*/

void livetune_ac_factory_get_min_max(const audio_descriptor_param_t *pParam, double *pMin, double *pMax)
{
  ST_ASSERT(pParam != NULL);
  ST_ASSERT(pMin != NULL);
  ST_ASSERT(pMax != NULL);
  AudioDescriptor_getMinMax(pParam, pMin, pMax);
}


/**
* @brief get algo config param descriptor
*/

const audio_algo_factory_t *livetune_ac_factory_get_algo_factory_name(const char_t *pName)
{
  return AudioChainFactory_getAlgoFactory(pName);
}



/**
* @brief get algo config param descriptor
*/

const audio_factory_entry_t *livetune_ac_factory_get_algo_factory_entry(void)
{
  return AudioChainFactory_getFactoryEntry();
}



/**
* @brief return the parameter descriptor from the builder and a key or NULL if the parameter is not found
         the function could return null, if the parameter is created manually
*/


const audio_descriptor_param_t *livetune_ac_factory_get_algo_config_param_descriptor(livetune_helper_builder *pBuilder, const char_t *pKey)
{
  int32_t                         error    = AUDIO_ERR_MGNT_NONE;
  const audio_descriptor_param_t *pDescr   = NULL;
  const audio_algo_factory_t     *pFactory = livetune_ac_factory_get_algo_factory_name(pBuilder->pBuilderDef->pAlgoName);

  if (pFactory == NULL)
  {
    ST_TRACE_ERROR("no algo factory");
  }
  if (acErrorIsOk(error))
  {
    error = AudioDescriptor_getParam(pFactory->pStaticParamTemplate, pKey, &pDescr, NULL);
    if (error == AUDIO_ERR_MGNT_NOT_FOUND)
    {
      error = AudioDescriptor_getParam(pFactory->pDynamicParamTemplate, pKey, &pDescr, NULL);
    }
  }
  if (pDescr == NULL)
  {
    ST_TRACE_WARNING("Description %s not found in factory of %s", pKey, pFactory->pCapabilities->pName);
  }

  return pDescr;
}

/**
* @brief get algo config common  descriptor
*/

const audio_algo_descriptor_t *livetune_ac_factory_get_algo_common_config_param_descriptor(const char_t *pKey)
{
  return AudioChainFactory_getAlgoCommonDescriptor(pKey);
}

/**
* @brief get chunk config descriptor
*/

int32_t livetune_ac_factory_get_chunk_config_param_descriptor(const char_t *pKey, const audio_descriptor_param_t **const ppParam)
{
  return AudioDescriptor_getParam(AudioChainFactory_getChunkTemplate(), pKey, ppParam, NULL);
}



/**
* @brief return TRUE if the default value is == to the value
*
*/

int8_t livetune_ac_factory_cmp_default_value(const audio_descriptor_param_t *pParam, void *pValue)
{
  ST_ASSERT(pParam != NULL);
  ST_ASSERT(pValue != NULL);
  int8_t result = FALSE;

  switch (pParam->paramType)
  {
    case AUDIO_DESC_PARAM_TYPE_UINT8:
      result = ((uint8_t)atoi(pParam->pDefault) == *((uint8_t *)pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_INT8:
      result = ((int8_t)atoi(pParam->pDefault) == *((int8_t *)pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_UINT16:
      result = ((uint16_t)atoi(pParam->pDefault) == *((uint16_t *)pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_INT16:
      result = ((int16_t)atoi(pParam->pDefault) == *((int16_t *)pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_UINT32:
    case AUDIO_DESC_PARAM_TYPE_ADDRESS:
      result = ((uint32_t)atoi(pParam->pDefault) == *((uint32_t *)pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_INT32:
      result = ((int32_t)atoi(pParam->pDefault) == *((int32_t *)pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_FLOAT:
      result = ((float)atof(pParam->pDefault) == *((float *)pValue)) ? 1 : 0;
      break;
    default:
      return FALSE;
      break;
  }
  return result;
}

/**
* @brief return TRUE if the default value is == to the value
*
*/

int8_t livetune_ac_factory_is_default_value_string(audio_descriptor_param_t *pParam, char_t *pValue)
{
  ST_ASSERT(pParam != NULL);
  ST_ASSERT(pValue != NULL);
  int8_t result = FALSE;

  switch (pParam->paramType)
  {
    case AUDIO_DESC_PARAM_TYPE_UINT8:
      result = (atoi(pParam->pDefault) == atoi(pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_INT8:
      result = ((int8_t)atoi(pParam->pDefault) == atoi(pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_UINT16:
      result = (atoi(pParam->pDefault) == atoi(pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_INT16:
      result = (atoi(pParam->pDefault) == atoi(pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_INT32:
      result = (atoi(pParam->pDefault) == atoi(pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_FLOAT:
      result = (atof(pParam->pDefault) == atof(pValue)) ? 1 : 0;
      break;
    case AUDIO_DESC_PARAM_TYPE_ADDRESS:
    case AUDIO_DESC_PARAM_TYPE_OBJECT:
      result = FALSE; /* for an address, always ignore the default because we don't know how to interpret the address*/
      break;

    case AUDIO_DESC_PARAM_TYPE_UINT32:
      result = (atoi(pParam->pDefault) == atoi(pValue)) ? 1 : 0;
      break;

    default:
      result = (atoi(pParam->pDefault) == atoi(pValue)) ? 1 : 0;
      break;
  }
  return result;
}
/**
 * @brief return the string value from a descriptor
 *
 * @param pParam  descriptor instance
 * @param pData   base data pointer
 * @param pValue  the string value
 * @param szValue  the size string value
 */
void livetune_ac_factory_get_string_value(audio_descriptor_param_t *pParam, void *pData, char_t *pValue, uint32_t szValue)
{
  ST_ASSERT(pParam != NULL);
  ST_ASSERT(pValue != NULL);

  switch (pParam->paramType)
  {
    case AUDIO_DESC_PARAM_TYPE_UINT8:
      snprintf(pValue, szValue, "%u", *((uint8_t *)pData));
      break;
    case AUDIO_DESC_PARAM_TYPE_INT8:
      snprintf(pValue, szValue, "%d", *((int8_t *)pData));
      break;
    case AUDIO_DESC_PARAM_TYPE_UINT16:
      snprintf(pValue, szValue, "%u", *((uint16_t *)pData));
      break;
    case AUDIO_DESC_PARAM_TYPE_INT16:
      snprintf(pValue, szValue, "%d", *((int16_t *)pData));
      break;
    case AUDIO_DESC_PARAM_TYPE_INT32:
      snprintf(pValue, szValue, "%d", *((int32_t *)pData));
      break;
    case AUDIO_DESC_PARAM_TYPE_FLOAT:
      snprintf(pValue, szValue, "%f", (double)(*((float *)pData)));
      {
        // remove useless trailing zeroes after decimal point
        char *pChar1 = strchr(pValue, (int)'.');
        if (pChar1 != NULL)
        {
          for (char *pChar2 = pChar1 + 1; *pChar2 != '\0'; pChar2++)
          {
            if (*pChar2 != '0')
            {
              pChar1 = pChar2 + 1;
            }
          }
          *pChar1 = '\0';
        }
      }
      break;
    case AUDIO_DESC_PARAM_TYPE_UINT32:
    case AUDIO_DESC_PARAM_TYPE_ADDRESS:
    case AUDIO_DESC_PARAM_TYPE_OBJECT:
    default:
      snprintf(pValue, szValue, "%u", *((uint32_t *)pData));
      break;
  }
}




/**
* @brief Apply a config parameter to  struct pointer
*
*/

void livetune_ac_factory_config_string_value(const audio_descriptor_params_t *const pParam, void *const pData, const char *const pKey, const char *const pValue)
{
  int32_t                         error = AUDIO_ERR_MGNT_NONE;
  const audio_descriptor_param_t *pDesc;

  error = AudioDescriptor_getParam(pParam, pKey, &pDesc, NULL);
  if (acErrorIsOk(error))
  {
    error = AudioDescriptor_applyParamConfigStr(pDesc, pData, pValue, NULL);
  }
  if (acErrorIsError(error))
  {
    ST_TRACE_ERROR("Apply config %s:%s", pKey, pValue);
  }
}



/**
* @brief return the chunk descriptor
*
*/

const audio_descriptor_params_t *livetune_ac_factory_get_chunk_descriptor(void)
{
  return AudioChainFactory_getChunkTemplate();
}



/**
* @brief if the pindef has parameters, we are supposed to override attribute after the propagation
*
* @param pHandle the instance handle
* @param pinIn  index pin in
* @param pinIn  index pin out
*/

void livetune_ac_factory_apply_changes(livetune_db_instance *pInstance, uint32_t pinOut)
{
  livetune_db_instance_pins_def *pDef     = livetune_db_instance_pin_def_get(&pInstance->pPinOut, pinOut);
  livetune_db_instance_cnx_list *pList    = pDef->pObject;
  uint32_t                          nbCnxOut = (uint32_t)livetune_db_cnx_nb(&pList);
  /* copy the pin in on each cnx out */
  for (uint32_t indexCnxOut = 0; indexCnxOut < nbCnxOut; indexCnxOut++)
  {
    livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, indexCnxOut);
    ST_ASSERT(pCnx != NULL); // must exist
    livetune_db_element_pin *pElemPinDef = livetune_db_list_get_object(&pInstance->pElements->pPinOut, pinOut);
    ST_ASSERT(pElemPinDef != NULL); // must exist
    const audio_descriptor_params_t *pDescriptor = livetune_ac_factory_get_chunk_descriptor();
    if (pCnx->pInstance->hAc.bSysIO == 0U)
    {
      /* iterate on the instance params located in the pindef out */

      uint32_t nbParams = livetune_db_list_nb(&pCnx->pParams);
      for (uint32_t indexParam = 0; indexParam < nbParams; indexParam++)
      {
        livetune_db_instance_param *pParam = livetune_db_list_get_object(&pCnx->pParams, indexParam);
        ST_ASSERT(pParam->pParamValue);
        ST_ASSERT(pParam->pName);

        if (strcmp(pParam->pParamValue, CNX_PARAM_SKIP_MARKER) != 0)
        {
          livetune_ac_factory_config_string_value(pDescriptor, &pCnx->hAc.hConf, pParam->pName, pParam->pParamValue);
        }
      }
    }
  }
}



/**
* @brief fill with check default values ( mainly used for generator)
*
*/
void livetune_ac_factory_init_default(livetune_db_instance *pInstance, livetune_chunk_conf_t *pConf)
{
  const audio_descriptor_params_t *pChunkTemplate = livetune_ac_factory_get_chunk_descriptor();
  for (int32_t index = 0; index < (int32_t)pChunkTemplate->nbParams; index++)
  {
    audio_descriptor_param_t *pParam = &pChunkTemplate->pParam[index];
    AudioDescriptor_applyParamConfigStr(pParam, pConf, pParam->pDefault, NULL);
  }
}




/**
* @brief return true if the value is the default value
*
*/

int8_t livetune_ac_factory_is_default_value(char_t *pKey, void *pData)
{
  int8_t                           bResult        = FALSE;
  int32_t                          error          = AUDIO_ERR_MGNT_NONE;
  const audio_descriptor_params_t *pChunkTemplate = livetune_ac_factory_get_chunk_descriptor();
  if (pChunkTemplate)
  {
    const audio_descriptor_param_t *pParam;
    error = AudioDescriptor_getParam(pChunkTemplate, pKey, &pParam, NULL);
    if (acErrorIsOk(error))
    {
      bResult = livetune_ac_factory_cmp_default_value(pParam, pData);
    }
  }
  return bResult;
}



/**
* @brief return true if the bus is an acChunk
*
*/

uint8_t livetune_ac_factory_is_acBus(livetune_db_instance *pInstance, int32_t bType, uint32_t pinDef)
{
  uint8_t bValid = FALSE;
  if (bType == 0) /* if pin in */
  {
    livetune_db_element_pin *pPinDef = livetune_db_list_get_object(&pInstance->pElements->pPinIn, pinDef);
    ST_ASSERT(pPinDef != NULL);
    if (strcmp(pPinDef->pType, LIVETUNE_AC_BUS) == 0)
    {
      bValid = TRUE;
    }
  }
  else
  {
    livetune_db_element_pin *pPinDef = livetune_db_list_get_object(&pInstance->pElements->pPinOut, pinDef);
    ST_ASSERT(pPinDef != NULL);
    if (strcmp(pPinDef->pType, LIVETUNE_AC_BUS) == 0)
    {
      bValid = TRUE;
    }
  }
  return bValid;
}


/**
* @brief return the number of sys io
*
*/


uint32_t livetune_ac_factory_get_sys_connection_nb(livetune_ac_factory_sys_type type)
{
  uint32_t                             nb        = 0;
  audio_chain_sys_connections_t const *pSysInCnx = NULL;


  switch ((uint32_t)type)
  {
    case AC_SYS_IN:
    {
      pSysInCnx = AudioChainSysIOs_getCnxIn();
      ST_ASSERT(pSysInCnx != NULL);
      nb = (uint32_t)pSysInCnx->nb;
      break;
    }

    case AC_SYS_OUT:
    {
      pSysInCnx = AudioChainSysIOs_getCnxOut();
      ST_ASSERT(pSysInCnx != NULL);
      nb = (uint32_t)pSysInCnx->nb;
      break;
    }
    default:
    {
      break;
    }
  }
  return nb;
}



/**
* @brief return the number of sys io configuration
*
*/



audio_chain_sys_connection_conf_t *livetune_ac_factory_get_sys_connection_conf(livetune_ac_factory_sys_type type, int32_t index)
{
  audio_chain_sys_connection_conf_t   *pConf     = NULL;
  audio_chain_sys_connections_t const *pSysInCnx = NULL;

  switch ((uint32_t)type)
  {
    case AC_SYS_IN:
    {
      pSysInCnx = AudioChainSysIOs_getCnxIn();
      ST_ASSERT(pSysInCnx != NULL);
      pConf = &pSysInCnx->pConf[index];
      break;
    }

    case AC_SYS_OUT:
    {
      pSysInCnx = AudioChainSysIOs_getCnxOut();
      ST_ASSERT(pSysInCnx != NULL);
      pConf = &pSysInCnx->pConf[index];
      break;
    }
    default:
    {
      break;
    }
  }
  return pConf;
}


/**
* @brief return the sysio name
*
*/


const char_t *livetune_ac_factory_get_sys_connection_name(const audio_chain_sys_connection_conf_t *pConf)
{
  ST_ASSERT(pConf != NULL);
  return pConf->pVarName;
}



/**
* @brief Clone a sysio chunk
*
*/


void livetune_ac_factory_clone_sys_connection_chunk(const audio_chain_sys_connection_conf_t *pConf, livetune_chunk_conf_t *pCloneChunk)
{
  audio_chunk_conf_t const *pChunConf = AudioChainSysIOs_getConf(pConf->pSysChunk);
  pCloneChunk->chunkType              = pChunConf->chunkType;
  pCloneChunk->nbChannels             = pChunConf->nbChannels;
  pCloneChunk->fs                     = pChunConf->fs;
  pCloneChunk->nbElements             = pChunConf->nbElements;
  pCloneChunk->nbFrames               = pChunConf->nbFrames;
  pCloneChunk->timeFreq               = pChunConf->timeFreq;
  pCloneChunk->bufferType             = pChunConf->bufferType;
  pCloneChunk->interleaved            = pChunConf->interleaved;
  pCloneChunk->pName                  = pConf->pVarName;
}


/**
* @brief return the factory instance name
*
*/


const char_t *livetune_ac_factory_get_factory_instance_name(const char_t *pAlgoName)
{
  const char_t                *pInstanceName = NULL;
  const audio_factory_entry_t *pFactoryEntry = AudioChainFactory_getAlgoFactoryEntry(pAlgoName);
  if (pFactoryEntry)
  {
    pInstanceName = pFactoryEntry->pFactoryName;
  }
  return pInstanceName;
}




/**
* @brief return if the element is a factory algo
*
*/


int8_t livetune_ac_is_factory_algo(const char_t *pAlgoName)
{
  int8_t bResult = FALSE;
  const audio_factory_entry_t *pFactoryEntry = AudioChainFactory_getAlgoFactoryEntry(pAlgoName);
  if (pFactoryEntry)
  {
    bResult = TRUE;
  }
  return bResult ;
}



/**
 * @brief get the API version string
 *
 * @return const char_t*
 */

const char_t *livetune_ac_factory_get_ac_version_string(void)
{
  return AC_VERSION;
}

/**
 * @brief Copy chunks
 *
 * @param pSrcConf src chunk
 * @param pDstConf dst chunk
 * @param typeOverWrite type
 */
void livetune_ac_factory_copy_chunk(const livetune_chunk_conf_t *pSrcConf, livetune_chunk_conf_t *pDstConf, int32_t typeOverWrite)
{
  /* copy the conf only */
  if (typeOverWrite == -1L)
  {
    typeOverWrite = (int32_t)pSrcConf->chunkType;
  }
  if (typeOverWrite == -2L)
  {
    typeOverWrite = (int32_t)AUDIO_CHUNK_TYPE_USER_INOUT;
  }


  pDstConf->chunkType   = (uint8_t)typeOverWrite;
  pDstConf->nbChannels  = pSrcConf->nbChannels;
  pDstConf->fs          = pSrcConf->fs;
  pDstConf->nbElements  = pSrcConf->nbElements;
  pDstConf->nbFrames    = pSrcConf->nbFrames;
  pDstConf->timeFreq    = pSrcConf->timeFreq;
  pDstConf->bufferType  = pSrcConf->bufferType;
  pDstConf->interleaved = pSrcConf->interleaved;
}




/**
*  @brief Init the instance param config
* * @param pBuilder the pBuilder instance handle
*/

void livetune_ac_factory_apply_algo_config_params(livetune_helper_builder *pBuilder)
{
  uint32_t indexControl = 0;

  if (pBuilder->pFactory->pStaticParamTemplate)
  {
    for (int32_t paramID = 0; paramID < (int32_t)pBuilder->pFactory->pStaticParamTemplate->nbParams; paramID++)
    {
      audio_descriptor_param_t *pParam = &pBuilder->pFactory->pStaticParamTemplate->pParam[paramID];
      /* if the param is private ( grayed) it is not modifiable by edition, so we skip it */
      if ((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE) == 0UL)
      {
        char_t *pTranslated = AudioChainJson_factory_translate_key_value(pParam, LIVETUNE_STRING(pBuilder->pInstance, indexControl++));
        livetune_ac_wrapper_acAlgoSetConfig(pBuilder->pInstance->hAc.hAlgo, pParam->pName, pTranslated, FALSE);
        st_os_mem_free(pTranslated);
      }
      else
      {
        indexControl++;
      }
    }
  }
  if (pBuilder->pFactory->pDynamicParamTemplate)
  {
    for (int32_t paramID = 0; paramID < (int32_t)pBuilder->pFactory->pDynamicParamTemplate->nbParams; paramID++)
    {
      audio_descriptor_param_t *pParam = &pBuilder->pFactory->pDynamicParamTemplate->pParam[paramID];
      /* if the param is private ( grayed) it is not modifiable by edition, so we skip it */
      if ((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE) == 0UL)
      {
        /* livetune_ac_wrapper_acAlgoSetConfig wants key instead of value, we need to convert the value as Key */

        char_t *pTranslated = AudioChainJson_factory_translate_key_value(pParam, LIVETUNE_STRING(pBuilder->pInstance, indexControl++));
        livetune_ac_wrapper_acAlgoSetConfig(pBuilder->pInstance->hAc.hAlgo, pParam->pName, pTranslated, FALSE);
        st_os_mem_free(pTranslated);
      }
      else
      {
        indexControl++;
      }
    }
  }
}



/**
* @brief Init the builder instance
*
* @param pBuilder Instance handle
* @return Error code
*/


st_class_element_instance *livetune_ac_factory_builder_create(livetune_db_instance *pInstance, uint32_t extSize)
{
  ST_Result result = ST_ERROR;
  uint32_t  szCls  = (uint32_t)sizeof(st_class_element_instance) + (uint32_t)extSize;

  st_class_element_instance *pCls = st_os_mem_alloc(ST_Mem_Type_Designer, szCls);
  if (pCls)
  {
    memset(pCls, 0, szCls);
    livetune_helper_builder *pBuilder = &pCls->hBuilder;
    /* the builder def is also in the element interface via the cookie, it is set at the element registration */
    pBuilder->pBuilderDef             = pInstance->pElements->pRefBuilder;
    pBuilder->pFactory                = livetune_ac_factory_get_algo_factory_name(pBuilder->pBuilderDef->pAlgoName);
    pBuilder->pInstance               = pInstance;
    pBuilder->pPipe                   = pInstance->pPipe;
    pBuilder->pInstance->pBuilderData = &pCls->hBuilder;
    result = ST_OK;
  }
  if ((pCls != NULL) && (result != ST_OK))
  {
    st_os_mem_free(pCls);
    pCls = NULL;
  }
  return pCls;
}



const char_t *livetune_factory_get_algo_name(const audio_algo_factory_t *pFactory)
{
  return pFactory->pCapabilities->pName;
}

const char_t *livetune_factory_get_algo_desc(const audio_algo_factory_t *pFactory)
{
  return pFactory->pCapabilities->misc.pAlgoDesc;
}


