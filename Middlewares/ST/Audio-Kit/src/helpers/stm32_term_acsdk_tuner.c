/**
******************************************************************************
* @file          stm32_term_acsdk_tuner.c
* @author        MCD Application Team
* @brief         general purpose commands
*******************************************************************************
*
*
This code supports a fine tuning using live tune, in this mode live tune is able  to modify parameters only.
There are some elements not displayed on the interface due to the live tuning configuration ( no database ).
Connections are shown  but not modifiable,  All non-audio chain elements such as viewer are not shown too.
The graph is always supposed running.

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

#include <assert.h>
#include <string.h>
#include "main_hooks.h"
#include "st_os.h"
#include "acSdk.h"
#include "stm32_term_cmd.h"
#include "st_os_monitor_cpu.h"
#include "audio_chain_instance.h"
#include "audio_chain_json_from_factory.h"
#include "audio_chain_json_transmission.h"
#include "audio_chain_sysIOs.h"
#include "st_json.h"
#include "audio_persist_config.h"

#define SZ_PROLOG_CHECK_SUM_SIZE        4U
#define SEND_TRANSMISION_BLOCK_SIZE     (70 * 1024)
#define acSDK_HANDLE(handle)            ((acAlgo)handle)
#define MAX_ELEMENT_DIFF                50
#define TYPE_PIN_IN                     0
#define TYPE_PIN_OUT                    1
#define TEMPLATE_SYS_OUT                "{\"Description\":\"Sink Connection point\",\"Icon\":\"icon-elem-sink.png\",\"Name\":\"SysOutChunk1\",\"InstanceMax\":1,\"PinDefIn\":[{\"Name\":\"sink\",\"Type\":\"acChunk\"}]}"
#define TEMPLATE_SYS_IN                 "{\"Description\":\"\",\"Icon\":\"icon-elem-src.png\",\"Name\":\"\",\"InstanceMax\":1,\"PinDefOut\":[{\"disabled\":true,\"ConnectionMax\":1,\"Type\":\"acChunk\"}]}"


static audio_json_transmission *hTransmision;

/**
* @brief   The connection name must be unique, so we compute an unique id
*/
static uint32_t s_compute_cnx_name(const char *pSrcName, const char *pDstName)
{
  uint32_t uname = 0UL;
  while (*pSrcName) { uname += (uint32_t)(*pSrcName++); }
  while (*pDstName) { uname += (uint32_t)(*pDstName++); }
  return uname;
}



/**
* @brief   Returns the chunk id index from its pointer ( probably add it to the SDK)
*/
static int32_t AudioChainSysIOs_getId(audio_chain_sys_connections_t const *const  pConnectionList, audio_chunk_t const *const pChunk)
{
  int32_t id = -1;
  for (int32_t index = 0; index < (int32_t)pConnectionList->nb ; index++)
  {
    if (strcmp(pConnectionList->pConf[index].pVarName, pChunk->pName) == 0)
    {
      id = index;
      break;
    }
  }
  return id;
}


/* Send a block using the raw serial api synchrone */
static bool s_acsdk_send(const void *pBlock, uint32_t szBlock)
{
  return (UTIL_UART_write((void *)pBlock, szBlock) == 0);
}

/* send a command acknowledge for the board */

static bool s_acsdk_cmd_ack(bool result, char *pSuccessCmdMsg, char *pErrorCmdMsg, char *pDesc)
{
  char body[50];

  if (!result)
  {
    snprintf(body, sizeof(body), "{\"Signature\":\"Ack\",\"Title\":\"Message\",\"Text\":\"%s\"}", (pErrorCmdMsg == NULL) ? "" : pErrorCmdMsg);
  }
  else
  {
    snprintf(body, sizeof(body), "{\"Signature\":\"Ack\",\"Title\":\"Message\",\"Text\":\"%s\"}", (pSuccessCmdMsg == NULL) ? "" : pSuccessCmdMsg);
  }
  return audio_json_transmission_send(hTransmision, body, strlen(body), false);
}


static bool s_is_algo_pin_multiple(audio_algo_t *const pAlgo)
{
  const audio_algo_factory_t *const pFactory       = AudioAlgo_getFactory(pAlgo);
  return (((uint32_t)pFactory->pCapabilities->iosOut.nb & (uint32_t)AUDIO_CAPABILITY_CHUNK_MULTIPLE) != 0U);;
}



/**
* @brief return always pin 0 for pinout and pin multiple
*
* @param
*/


static uint8_t s_get_apply_pin_multiple(audio_algo_t *const pAlgo, uint8_t pin)
{
  if (s_is_algo_pin_multiple(pAlgo))
  {
    pin = 0;
  }
  return pin;
}

/**
* @brief add a parameter to the algo instance
*
* @param
*/

static jsonErr s_acsdk_add_params(audio_algo_t *const pAlgo, audio_descriptor_params_t const *pParams, json_instance_t *pJsonInstance, jsonID hParams, char *pBufferValue, uint32_t szBufferValue)
{
  jsonErr error = JSON_OK;
  for (uint8_t index = 0U; index < pParams->nbParams; index++)
  {
    const audio_descriptor_param_t *pParam = &pParams->pParam[index];

    jsonID hObjParam;
    error = json_create_object(pJsonInstance, &hObjParam);
    if (error == JSON_OK)
    {
      error = json_array_append_new(pJsonInstance, hParams, hObjParam);
    }
    if (error == JSON_OK)
    {
      error = json_object_set_string(pJsonInstance, hObjParam, "", "Name", pParam->pName);
    }
    if (error == JSON_OK)
    {
      error = (acAlgoGetConfig(pAlgo, pParam->pName, pBufferValue, szBufferValue) == 0) ? JSON_OK : JSON_ERROR;
    }
    if (error == JSON_OK)
    {
      error = json_object_set_string(pJsonInstance, hObjParam, "", "Value", pBufferValue);
    }
  }
  return error;
}

/**
* @brief add a json connection
*
* @param
*/
static jsonErr s_acsdk_json_connectionAdd(json_instance_t *pJsonInstance, jsonID hCnxs, uint32_t cnxIndex, const char *pInstanceName, uint8_t pinName)
{
  jsonErr error = JSON_OK;
  jsonID  hAttachCnx;
  char    tScratch[30];
  snprintf(tScratch, sizeof(tScratch), "Cnx_%X", cnxIndex);

  error = json_create_object(pJsonInstance, &hAttachCnx);
  if (error == JSON_OK)
  {
    error = json_array_append_new(pJsonInstance, hCnxs, hAttachCnx);
    if (error == JSON_OK)
    {
      error = json_object_set_string(pJsonInstance, hAttachCnx, "", "Name", tScratch);
    }
    if (error == JSON_OK)
    {
      error = json_object_set_string(pJsonInstance, hAttachCnx, "", "InstanceNameDst", pInstanceName);
    }
    if (error == JSON_OK)
    {
      error = json_object_set_integer(pJsonInstance, hAttachCnx, "", "PinDst", (int64_t)pinName);
    }
  }
  return error;
}



/**
* @brief add cnx in to the instance
*
* @param
*/

static jsonErr s_add_instance_connection(audio_algo_list_t *pCurrent, json_instance_t *pJsonInstance, jsonID root, int32_t pinType)
{
  jsonErr error = JSON_OK;
  jsonID hCnxs;
  uint8_t indexPin;
  char tInstaneName[100];

  const char  *pPinDefName = NULL;
  audio_chunk_list_t *pPinList = NULL;
  /* if the pin definition name is different for out/in and the chunk list too */
  if (pinType == TYPE_PIN_IN)
  {
    pPinDefName = "PinDefIn";
    pPinList = AudioAlgo_getChunksIn(pCurrent->pAlgo);
  }
  else if (pinType == TYPE_PIN_OUT)
  {
    /* usually, algos have an unique connection for 1 pindef, but for the out like the split the entry Cnx[] could have several targets */
    pPinDefName = "PinDefOut";
    pPinList = AudioAlgo_getChunksOut(pCurrent->pAlgo);
  }
  else
  {
    error = JSON_ERROR;
  }
  /* Create the array PinDefOut/PinDefIn */
  jsonID hPinDef = JSON_ID_NULL;
  if (error == JSON_OK)
  {
    error = json_create_array(pJsonInstance, &hPinDef);
  }
  if (error == JSON_OK)
  {
    error = json_object_set_new(pJsonInstance, root, pPinDefName, hPinDef);
  }
  if (error == JSON_OK)
  {
    if (!s_is_algo_pin_multiple(pCurrent->pAlgo))
    {
      /* Generate a single connection for each pindef */
      for (indexPin = 0U; (pPinList != NULL) && (error == JSON_OK) ; pPinList = pPinList->next, indexPin++)
      {
        /* Create the PinDefXX[{}] entry and add it */
        jsonID hPindefObj;
        error = json_create_object(pJsonInstance, &hPindefObj);
        if (error == JSON_OK)
        {
          error = json_array_append_new(pJsonInstance, hPinDef, hPindefObj);
        }
        /* create the PinDefXX[{Cnx[]}}] entry and add it */
        error = json_create_array(pJsonInstance, &hCnxs);
        if (error == JSON_OK)
        {
          error = json_object_set_new(pJsonInstance, hPindefObj, "Cnx", hCnxs);
        }
        if (error == JSON_OK)
        {
          if (AudioChunk_getType(pPinList->pChunk) == AUDIO_CHUNK_TYPE_SYS_IN)
          {
            /* if the chunk is a sysin, let's create a connection with a sysin instance */
            audio_chain_sys_connections_t const   *pSysIo = AudioChainSysIOs_getCnxIn();
            audio_chain_sys_connection_conf_t const *pConf = &pSysIo->pConf[AudioChainSysIOs_getId(pSysIo, pPinList->pChunk)];
            snprintf(tInstaneName, sizeof(tInstaneName), "%s-1", pConf->pName);
            error = s_acsdk_json_connectionAdd(pJsonInstance, hCnxs, s_compute_cnx_name(pCurrent->pAlgo->pDesc, tInstaneName), tInstaneName, 0U);
          }
          else if (AudioChunk_getType(pPinList->pChunk) == AUDIO_CHUNK_TYPE_SYS_OUT)
          {
            /* if the chunk is a sysout, let's create a connection with a sysout instance */

            audio_chain_sys_connections_t const   *pSysIo = AudioChainSysIOs_getCnxOut();
            audio_chain_sys_connection_conf_t const *pConf = &pSysIo->pConf[AudioChainSysIOs_getId(pSysIo, pPinList->pChunk)];
            snprintf(tInstaneName, sizeof(tInstaneName), "%s-1", pConf->pName);
            error = s_acsdk_json_connectionAdd(pJsonInstance, hCnxs, s_compute_cnx_name(pCurrent->pAlgo->pDesc, tInstaneName), tInstaneName, 0U);
          }
          else
          {
            /* Otherwise, it is a connection algo to algo pins*/
            audio_algo_t *pAlgoDst;
            uint8_t       ialgoPin;
            /* find in all algo and pins in if the chunk is connected an return the algo found and the pin */
            if (pinType == TYPE_PIN_OUT)
            {
              AudioChain_getChunkAlgoIn(&AudioChainInstance, pPinList->pChunk, &pAlgoDst, &ialgoPin);
            }
            else
            {
              AudioChain_getChunkAlgoOut(&AudioChainInstance, pPinList->pChunk, &pAlgoDst, &ialgoPin);
              ialgoPin = s_get_apply_pin_multiple(pAlgoDst, ialgoPin);
            }

            assert(pAlgoDst != NULL);
            error = s_acsdk_json_connectionAdd(pJsonInstance, hCnxs, s_compute_cnx_name(pCurrent->pAlgo->pDesc, pAlgoDst->pDesc), AudioAlgo_getInstanceName(pAlgoDst), ialgoPin);
          }
        }
      }
    }
    else
    {
      /* Generate a single pindef all connections attached in a Cnx Array  */

      jsonID hPindefObj;
      error = json_create_object(pJsonInstance, &hPindefObj);
      if (error == JSON_OK)
      {
        error = json_array_append_new(pJsonInstance, hPinDef, hPindefObj);
      }
      /* create the PinDefXX[{Cnx[]}}] entry and add  it */

      error = json_create_array(pJsonInstance, &hCnxs);
      if (error == JSON_OK)
      {
        error = json_object_set_new(pJsonInstance, hPindefObj, "Cnx", hCnxs);
      }

      for (indexPin = 0U; (pPinList != NULL) && (error == JSON_OK) ; pPinList = pPinList->next)
      {
        if (AudioChunk_getType(pPinList->pChunk) == AUDIO_CHUNK_TYPE_SYS_IN)
        {
          /* if the chunk is a sysin, let's create a connection with a sysin instance */
          audio_chain_sys_connections_t const   *pSysIo = AudioChainSysIOs_getCnxIn();
          audio_chain_sys_connection_conf_t const *pConf = &pSysIo->pConf[AudioChainSysIOs_getId(pSysIo, pPinList->pChunk)];
          snprintf(tInstaneName, sizeof(tInstaneName), "%s-1", pConf->pName);
          error = s_acsdk_json_connectionAdd(pJsonInstance, hCnxs, s_compute_cnx_name(pCurrent->pAlgo->pDesc, tInstaneName), tInstaneName, 0U);
        }
        else if (AudioChunk_getType(pPinList->pChunk) == AUDIO_CHUNK_TYPE_SYS_OUT)
        {
          /* if the chunk is a sysout, let's create a connection with a sysout instance */

          audio_chain_sys_connections_t const   *pSysIo = AudioChainSysIOs_getCnxOut();
          audio_chain_sys_connection_conf_t const *pConf = &pSysIo->pConf[AudioChainSysIOs_getId(pSysIo, pPinList->pChunk)];
          snprintf(tInstaneName, sizeof(tInstaneName), "%s-1", pConf->pName);
          error = s_acsdk_json_connectionAdd(pJsonInstance, hCnxs, s_compute_cnx_name(pCurrent->pAlgo->pDesc, tInstaneName), tInstaneName, 0U);
        }
        else
        {
          /* otherwise, it a connection algo to algo pins out */
          audio_algo_t *pAlgoDst;
          uint8_t       ialgoPin;
          /* find in all algo and pins in if the chunk is connected an return the algo found and the pin */

          if (pinType == TYPE_PIN_OUT)
          {
            AudioChain_getChunkAlgoIn(&AudioChainInstance, pPinList->pChunk, &pAlgoDst, &ialgoPin);
          }
          else
          {
            AudioChain_getChunkAlgoOut(&AudioChainInstance, pPinList->pChunk, &pAlgoDst, &ialgoPin);
          }
          assert(pAlgoDst != NULL);
          error = s_acsdk_json_connectionAdd(pJsonInstance, hCnxs, s_compute_cnx_name(pCurrent->pAlgo->pDesc, pAlgoDst->pDesc), AudioAlgo_getInstanceName(pAlgoDst), indexPin);
        }
      }
    }
  }
  return error;
}


/**
* @brief generate project fake sysio instances
*
* @param
*/

static jsonErr s_generate_sysio_json(json_instance_t *pJsonInstance, jsonID hInstances, int32_t pinType)
{
  jsonErr error = JSON_OK;
  char tInstaneName[100];

  for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); (pCurrent != NULL) && (error == JSON_OK); pCurrent = pCurrent->next)
  {
    audio_algo_factory_t *pFactAlgo = (audio_algo_factory_t *)AudioAlgo_getFactory(pCurrent->pAlgo);
    const char  *pPinDefName = NULL;
    audio_chunk_list_t *pPinList = NULL;
    uint8_t     indexPin;
    audio_chain_sys_connections_t const *pSysIo = NULL;
    /* fields and  chunk list are different according to the pin direction */

    if (pinType == TYPE_PIN_IN)
    {
      pPinDefName = "PinDefOut";// the target is pin out
      pPinList = AudioAlgo_getChunksIn(pCurrent->pAlgo);
      pSysIo = AudioChainSysIOs_getCnxIn();

    }
    else if (pinType == TYPE_PIN_OUT)
    {
      pPinDefName = "PinDefIn"; // the target is pin in
      pPinList = AudioAlgo_getChunksOut(pCurrent->pAlgo);
      pSysIo = AudioChainSysIOs_getCnxOut();
    }
    else
    {
      error = JSON_ERROR;
    }
    /* iterate all pins */
    for (indexPin = 0U; (pPinList != NULL) && (error == JSON_OK); pPinList = pPinList->next)
    {
      /* we are iterested only for system chunk */
      uint8_t type = (uint8_t)AudioChunk_getType(pPinList->pChunk);
      if ((type & (uint8_t)AUDIO_CHUNK_TYPE_SYSTEM) != 0U)
      {
        /* get the check configuration */
        audio_chain_sys_connection_conf_t const *pConf = &pSysIo->pConf[AudioChainSysIOs_getId(pSysIo, pPinList->pChunk)];

        /* create an instance object representing the fake sysio */
        jsonID hObj;
        error = json_create_object(pJsonInstance, &hObj);
        if (error == JSON_OK)
        {
          error = json_array_append_new(pJsonInstance, hInstances, hObj);
          if (error == JSON_OK)
          {
            /* a sysio has only one instance so, we can hard code "-1" */
            snprintf(tInstaneName, sizeof(tInstaneName), "%s-1", pConf->pName);
            error = json_object_set_string(pJsonInstance, hObj, "", "InstanceName", tInstaneName);
            /* create the instance header */
            if (error == JSON_OK)
            {
              error = json_object_set_string(pJsonInstance, hObj, "", "RefElement", pConf->pName);
            }
            if (error == JSON_OK)
            {
              error = json_object_set_string(pJsonInstance, hObj, "", "Description", pConf->pDescription);
            }
            /* create an empty array pindef PindefXX[Cnx[]], there is at least one connection */
            jsonID hPinDef;
            error = json_create_array(pJsonInstance, &hPinDef);
            if (error == JSON_OK)
            {
              error = json_object_set_new(pJsonInstance, hObj, pPinDefName, hPinDef);
            }
            jsonID hPindefObj;
            error = json_create_object(pJsonInstance, &hPindefObj);
            if (error == JSON_OK)
            {
              error = json_array_append_new(pJsonInstance, hPinDef, hPindefObj);
            }
            jsonID hCnxs;

            error = json_create_array(pJsonInstance, &hCnxs);
            if (error == JSON_OK)
            {
              error = json_object_set_new(pJsonInstance, hPindefObj, "Cnx", hCnxs);
            }
            if ((type & (uint8_t)AUDIO_CHUNK_TYPE_SINK) != 0U)
            {
              /* a sink is connected to an algo and can be connect to a pin multiple */
              audio_algo_t *pAlgoDst = NULL;
              uint8_t       ialgoPin;
              AudioChain_getChunkAlgoOut(&AudioChainInstance, pPinList->pChunk, &pAlgoDst, &ialgoPin);
              ialgoPin = s_get_apply_pin_multiple(pAlgoDst, ialgoPin);
              assert(pAlgoDst != NULL);
              error = s_acsdk_json_connectionAdd(pJsonInstance, hCnxs, s_compute_cnx_name(tInstaneName, pAlgoDst->pDesc), AudioAlgo_getInstanceName(pAlgoDst), indexPin);
            }
            if ((type & (uint8_t)AUDIO_CHUNK_TYPE_SRC) != 0U)
            {
              audio_algo_t *pAlgoDst = NULL;
              uint8_t       ialgoPin;
              AudioChain_getChunkAlgoIn(&AudioChainInstance, pPinList->pChunk, &pAlgoDst, &ialgoPin);
              assert(pAlgoDst != NULL);
              error = s_acsdk_json_connectionAdd(pJsonInstance, hCnxs, s_compute_cnx_name(tInstaneName, pAlgoDst->pDesc), AudioAlgo_getInstanceName(pAlgoDst), indexPin);
            }
          }
        }
      }
    }
  }
  return error;
}




/**
* @brief Generate a project livetune json from the running graph
*
* @param
*/

static jsonErr s_generate_instances_json(json_instance_t *pJsonInstance, jsonID hInstances)
{
  jsonErr error = JSON_OK;
  uint32_t szBufferValue = 4UL * 1024UL; /* reasonable size for arrays */
  char *pBufferValue = st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, szBufferValue);
  if (pBufferValue)
  {
    if (error == JSON_OK)
    {
      /* iterate all instances used */
      for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); pCurrent != NULL; pCurrent = pCurrent->next)
      {
        audio_algo_factory_t *pFactAlgo = (audio_algo_factory_t *)AudioAlgo_getFactory(pCurrent->pAlgo);

        /* create an instance obj to add to the array */
        jsonID hObj;
        error = json_create_object(pJsonInstance, &hObj);
        if (error == JSON_OK)
        {
          error = json_array_append_new(pJsonInstance, hInstances, hObj);
          if (error == JSON_OK)
          {
            /* fill basic header infos */
            error = json_object_set_string(pJsonInstance, hObj, "", "InstanceName", AudioAlgo_getInstanceName(pCurrent->pAlgo));

            if (error == JSON_OK)
            {
              error = json_object_set_string(pJsonInstance, hObj, "", "RefElement", pFactAlgo->pCapabilities->pName);
            }
            if (error == JSON_OK)
            {
              error = json_object_set_string(pJsonInstance, hObj, "", "Description", pCurrent->pAlgo->pDesc); // Instance description
            }
            uint8_t nbParams = 0U;
            if (pFactAlgo->pStaticParamTemplate)
            {
              nbParams += pFactAlgo->pStaticParamTemplate->nbParams;
            }
            if (pFactAlgo->pDynamicParamTemplate)
            {
              nbParams += pFactAlgo->pDynamicParamTemplate->nbParams;
            }
            /* if the algo has parameters, create the param array */
            if (nbParams)
            {
              jsonID hParam;
              json_create_array(pJsonInstance, &hParam);
              assert(hParam != JSON_ID_NULL);
              /* attach the array to the root object */
              json_object_set_new(pJsonInstance, hObj, "Params", hParam);
              if (hParam)
              {
                /* fill static and dynamic params */
                if ((error == JSON_OK) && (pFactAlgo->pStaticParamTemplate != NULL))
                {
                  error = s_acsdk_add_params(pCurrent->pAlgo, pFactAlgo->pStaticParamTemplate, pJsonInstance, hParam, pBufferValue, szBufferValue);
                }
                if ((error == JSON_OK) && (pFactAlgo->pDynamicParamTemplate != NULL))
                {
                  error = s_acsdk_add_params(pCurrent->pAlgo, pFactAlgo->pDynamicParamTemplate, pJsonInstance, hParam, pBufferValue, szBufferValue);
                }
              }
            }
            /* Evaluate instance connection with sys algos/in/out */
            if (error  == JSON_OK)
            {
              error = s_add_instance_connection(pCurrent, pJsonInstance, hObj, TYPE_PIN_IN);
            }
            if (error  == JSON_OK)
            {
              error = s_add_instance_connection(pCurrent, pJsonInstance, hObj, TYPE_PIN_OUT);
            }
          }
        }

      }
    }
  }
  if (pBufferValue)
  {
    st_os_mem_free(pBufferValue);
  }
  return error;
}


/**
* @brief Expose platform params
* @param pInstance json instance
* @param root json root id
*/
static bool s_terminal_send_config(void)
{
  bool            bResult = false;
  json_instance_t hJson;
  const char_t   *pJson = NULL;

  memset(&hJson, 0, sizeof(hJson));
  json_create_object(&hJson, &hJson.pack_root);
  if (hJson.pack_root)
  {
    json_object_set_string(&hJson, hJson.pack_root, "", "Signature", "CnxConfig");
    char tScratch[50] = "ID_CUSTOM";
    audio_persist_get_config_id(audio_persist_get_config_cur(), tScratch, sizeof(tScratch));
    json_object_set_string(&hJson, hJson.pack_root, "", "AudioConfig", tScratch);
    snprintf(tScratch, sizeof(tScratch), "HW_%s", STM32_NAME_STRING);
    json_object_set_string(&hJson, hJson.pack_root, "", "HWConfig", tScratch);
    pJson = json_dumps(&hJson, hJson.pack_root, 0);
    json_shutdown(&hJson);
    if (pJson)
    {
      bResult = audio_json_transmission_send(hTransmision, (void *)pJson, strlen(pJson), false);
      json_free((void *)pJson);
    }
  }

  return bResult;
}

/**
* @brief  build the list of element used in the live graph
*/
static int32_t s_build_elements_list(audio_algo_factory_t **pFactoryUsed, int32_t maxFactory)
{
  int32_t nbFactory = 0;
  for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); (pCurrent != NULL) && (nbFactory < maxFactory); pCurrent = pCurrent->next)
  {
    audio_algo_factory_t *pFactAlgo = (audio_algo_factory_t *)AudioAlgo_getFactory(pCurrent->pAlgo);
    assert(pFactAlgo != NULL);

    bool found = false;
    for (int32_t indexUsed = 0; indexUsed < nbFactory; indexUsed++)
    {
      if (strcmp(pFactoryUsed[indexUsed]->pCapabilities->pName, pFactAlgo->pCapabilities->pName) == 0)
      {
        found = true;
        break;
      }
    }
    if (!found)
    {
      pFactoryUsed[nbFactory++] = pFactAlgo;
    }
  }

  return nbFactory;
}


/**
* @brief  build the element json
*/
static char *s_build_elements_string(audio_algo_factory_t *pFactory, uint32_t szHeaderDump)
{
  char           *pJson    = NULL;
  json_instance_t jsonInst = {0};
  jsonID          rootID;

  json_create_object(&jsonInst, &rootID);
  if (rootID)
  {
    AudioChainJson_update_element(pFactory, &jsonInst, rootID);
    json_dump_t dump =
    {
      .szDump  = 0,
      .curDump = 0,
      .pDump   = NULL
    };
    /* in case of space reservation before the json */
    if (szHeaderDump != 0U)
    {
      dump.pDump   = st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, szHeaderDump);
      dump.curDump = szHeaderDump;
      dump.szDump  = szHeaderDump;
    }

    pJson = (char *)json_stringify(&jsonInst, rootID, 0, &dump);
    json_shutdown(&jsonInst);
  }
  return pJson;
}

/**
* @brief  Create a json element common header
*/
static char_t *s_sysio_create_element_template(audio_chain_sys_connection_conf_t *pConf, const char_t *pDesc, uint32_t szHeaderDump)
{
  char_t *pDescription = NULL;

  json_instance_t jsonInst = {0};
  jsonID          rootID;
  json_load(&jsonInst, pDesc, &rootID);
  ST_ASSERT(rootID != JSON_ID_NULL);
  if (rootID)
  {
    audio_chunk_conf_t const *pChunConf = AudioChainSysIOs_getConf(pConf->pSysChunk);

    ST_VERIFY(json_object_set_string(&jsonInst, rootID, "", "Description", pConf->pDescription) == JSON_OK);
    ST_VERIFY(json_object_set_string(&jsonInst, rootID, "", "Name", pConf->pName) == JSON_OK);
    ST_VERIFY(json_object_set_string(&jsonInst, rootID, "", "SysIo", "1") == JSON_OK);
    jsonID hParam;
    ST_VERIFY(json_create_array(&jsonInst, &hParam) == JSON_OK);
    ST_VERIFY(json_object_set_new(&jsonInst, rootID, "Params", hParam) == JSON_OK);
    AudioChainJson_json_add_param(&jsonInst, hParam, -1, NULL, 0, "variable", NULL, "sysio", NULL, pConf->pVarName, 0.0F, 0.0F, TRUE);
    /* in case of space reservation before the json */
    json_dump_t dump =
    {
      .szDump  = 0,
      .curDump = 0,
      .pDump   = NULL
    };

    if (szHeaderDump != 0U)
    {
      dump.pDump   = st_os_mem_alloc(ST_Mem_Type_ANY_SLOW, szHeaderDump);
      dump.curDump = szHeaderDump;
      dump.szDump  = szHeaderDump;
    }
    pDescription = (char_t *)json_stringify(&jsonInst, rootID, 0, &dump);
    json_shutdown(&jsonInst);
  }

  return pDescription;
}



/**
* @brief  Create a json sysio element
*/
static bool  s_sysio_create_element(const audio_chain_sys_connections_t *pSysCnx, const char *pTemplate, int32_t *pIndexElem)
{
  bool bResult  = false;
  uint32_t nbSysIo = pSysCnx->nb;
  char headerJson[50];


  for (uint32_t indexSysCnx = 0; indexSysCnx < nbSysIo ; indexSysCnx++)
  {
    snprintf(headerJson, sizeof(headerJson), "{\"Signature\":\"Element\",\"Num\":%d,\"Element\":", *pIndexElem);
    (*pIndexElem)++;
    uint32_t szHeaderDump = strlen(headerJson);

    char_t *pDescription = s_sysio_create_element_template(&pSysCnx->pConf[indexSysCnx], pTemplate, szHeaderDump);
    bResult  = false;
    if (pDescription)
    {
      /* Copy the header in the provisioned size */
      memcpy(pDescription, headerJson, szHeaderDump);
      /* grow the buffer to add the terminator */
      pDescription = st_os_mem_realloc(ST_Mem_Type_ANY_SLOW, pDescription, strlen(pDescription) + 2U);

      if (pDescription)
      {
        /* add the terminator */
        strcat(pDescription, "}");
        UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_OFF);
        bResult = audio_json_transmission_send(hTransmision, pDescription, strlen(pDescription), false);
        UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);
      }
    }
    if (!bResult)
    {
      break;
    }
    if (pDescription)
    {
      json_free(pDescription);
    }
    st_os_task_delay(1);
  }
  return bResult;
}




static bool s_create_elements(audio_algo_factory_t **pFactoryUsed, int32_t nbElements, int32_t *pIndexElem)
{
  bool bResult  = false;

  char headerJson[50];
  for (int32_t indexElem = 0; indexElem < nbElements; indexElem++)
  {
    audio_algo_factory_t  *pFactory = pFactoryUsed[indexElem];
    snprintf(headerJson, sizeof(headerJson), "{\"Signature\":\"Element\",\"Num\":%d,\"Element\":", *pIndexElem);
    (*pIndexElem)++;
    assert(strlen(headerJson) < (sizeof(headerJson) - 1U));
    char *pJsonDesc = s_build_elements_string(pFactory, strlen(headerJson));
    if (pJsonDesc)
    {
      /* Copy the header in the provisioned size */
      memcpy(pJsonDesc, headerJson, strlen(headerJson));
      /* grow the buffer to add the terminator */
      char *pJsonFinal = st_os_mem_realloc(ST_Mem_Type_ANY_SLOW, pJsonDesc, strlen(pJsonDesc) + 2U);
      if (pJsonFinal)
      {
        /* add the terminator */
        strcat(pJsonFinal, "}");
        /* send the payload */
        UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_OFF);
        bResult = audio_json_transmission_send(hTransmision, pJsonFinal, strlen(pJsonFinal), false);
        UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);
      }
      else
      {
        /* if a realloc return null, the pointer to grow is not freed */
        st_os_mem_free((void *)pJsonDesc);
      }
      st_os_mem_free((void *)pJsonFinal);
    }
    else
    {
      bResult = false;
    }
    st_os_task_delay(1);
  }
  return bResult;
}





/**
* @brief  Export the json elements
*
* @param argc  num args
* @param argv  args list
*/
static bool s_build_elements(audio_algo_factory_t **pFactoryUsed, int32_t nbElements)
{
  bool bResult = true;
  int32_t indexElem = 0;
  /* send first fake element sysin and sysout */
  if (!s_sysio_create_element(AudioChainSysIOs_getCnxIn(), TEMPLATE_SYS_IN, &indexElem))
  {
    bResult = false;
  }
  if (!s_sysio_create_element(AudioChainSysIOs_getCnxOut(), TEMPLATE_SYS_OUT, &indexElem))
  {
    bResult = false;
  }

  if (!s_create_elements(pFactoryUsed, nbElements, &indexElem))
  {
    bResult = false;
  }

  if (!bResult)
  {
    UTIL_TERM_printf_cr("Error: Send elements");
  }
  return bResult;
}





/**
* @brief  export the json instances ( .livetune file )
*
* @param argc  num args
* @param argv  args list
*/
static void stm32_term_acsdk_cmd_get_instances(int argc, char *argv[])
{
  bool     bOk      = false;
  uint16_t dumpFlag = 0U;
  bool     bResult  = false;

  /* compact produces a json without indentation, it is faster to download */
  if (argc == 1)
  {
    bOk = true;
  }
  else if (argc == 2)
  {
    if (strcmp(argv[1], "formatted") == 0)
    {
      bOk      = true;
      dumpFlag = JSON_INDENT;
    }
  }
  else
  {
    UTIL_TERM_printf_cr("%s command: wrong number of arguments", argv[0]);
  }
  if (bOk)
  {
    bResult = s_terminal_send_config();
    if (bResult)
    {
      jsonErr error = JSON_OK;
      const char     *pJson = NULL;
      json_instance_t jsonInst;
      memset(&jsonInst, 0, sizeof(jsonInst));
      json_create_object(&jsonInst, &jsonInst.pack_root);
      if (jsonInst.pack_root)
      {
        /* format the header instances */
        if (error == JSON_OK)
        {
          error = json_object_set_string(&jsonInst,  jsonInst.pack_root, "", "Signature", "Instance");
        }
        if (error == JSON_OK)
        {
          error = json_object_set_string(&jsonInst,  jsonInst.pack_root, "", "Version", AC_VERSION);
        }
        if (error == JSON_OK)
        {
          error = json_object_set_boolean(&jsonInst, jsonInst.pack_root, "", "Editable", 0);
        }
        if (error == JSON_OK)
        {
          error = json_object_set_string(&jsonInst, jsonInst.pack_root, "", "ProjectType", "AudioKit");
        }

        if (error == JSON_OK)
        {
          error = json_object_set_integer(&jsonInst, jsonInst.pack_root, "", "InstanceMax", 100);
        }
        if (error == JSON_OK)
        {
          error = json_object_set_integer(&jsonInst, jsonInst.pack_root, "", "ConnectionOutMax", 100);
        }
        if (error == JSON_OK)
        {
          error = json_object_set_integer(&jsonInst, jsonInst.pack_root, "", "ConnectionInMax", 100);
        }

        jsonID hInstances;
        error =  json_create_array(&jsonInst, &hInstances);
        if (error == JSON_OK)
        {
          error =  json_object_set_new(&jsonInst, jsonInst.pack_root, "Instances", hInstances);
        }
        /* add instance realy used in this graph */
        if (error == JSON_OK)
        {
          error = s_generate_instances_json(&jsonInst, hInstances) ;
        }
        /* add fake sysio */
        if (error == JSON_OK)
        {
          error = s_generate_sysio_json(&jsonInst, hInstances, TYPE_PIN_IN);
        }
        if (error == JSON_OK)
        {
          error = s_generate_sysio_json(&jsonInst, hInstances, TYPE_PIN_OUT);
        }
        if (error == JSON_OK)
        {
          pJson = json_dumps(&jsonInst, jsonInst.pack_root, dumpFlag);
          if (pJson != NULL)
          {
            bResult = true;
          }
        }
      }
      json_shutdown(&jsonInst);
      if (pJson)
      {
        bResult = audio_json_transmission_send(hTransmision, (void *)pJson, strlen(pJson), false);
        json_free((char *)pJson);
      }
    }
  }
  bResult = s_acsdk_cmd_ack(bResult, NULL, "Error: Load Instance", "get_instances");
  assert(bResult);
}


/**
* @brief  Export the json elements
*
* @param argc  num args
* @param argv  args list
*/
static void stm32_term_acsdk_cmd_get_elements(int argc, char *argv[])
{
  bool                  bResult = false;
  audio_algo_factory_t *tFactory[MAX_ELEMENT_DIFF];
  int32_t               nbElements = s_build_elements_list(tFactory, MAX_ELEMENT_DIFF);

  if (nbElements)
  {
    bResult = s_build_elements(tFactory, nbElements);
  }

  bResult = s_acsdk_cmd_ack(bResult, NULL, "Error: Get element", "get_elements");
  assert(bResult);
}


/**
* @brief  Apply the parameter
*/

static void stm32_term_acsdk_apply(const char *pInstanceName, const char *pName, const char *pValue, int64_t type)
{
  int32_t error = 0;
  acAlgo hAlgo = acAlgoGetInstance((acPipe)&AudioChainInstance, pInstanceName);
  if (hAlgo)
  {
    const audio_algo_factory_t *pFactory = AudioAlgo_getFactory(hAlgo);

    if (pFactory == NULL)
    {
      UTIL_TERM_printf_cr("no algo factory");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
    else
    {
      const audio_descriptor_param_t *pParam = NULL;

      error = AudioDescriptor_getParam(pFactory->pDynamicParamTemplate, pName, &pParam, NULL);
      if (error == AUDIO_ERR_MGNT_NOT_FOUND)
      {
        error = AudioDescriptor_getParam(pFactory->pStaticParamTemplate, pName, &pParam, NULL);
      }
      if (error == AUDIO_ERR_MGNT_NOT_FOUND)
      {
        UTIL_TERM_printf_cr("param called %s not found in factory of %s", pName, pFactory->pCapabilities->pName);
      }
      else if (error == 0)
      {
        char_t *pTranslated = AudioChainJson_factory_translate_key_value(pParam, pValue);
        error = acAlgoSetConfig(hAlgo, pName, pTranslated);
        st_os_mem_free(pTranslated);
      }
    }

    if (error == 0)
    {
      if (type == 0)
      {
        error = acAlgoRequestUpdate(hAlgo);
      }
    }
  }
  if (error != 0)
  {
    UTIL_TERM_printf_cr("Apply Algo %s:%s fails", pInstanceName, pName);
  }
}




/**
* @brief  the board connect livetune
*
* @param argc  num args
* @param argv  args list
*/

static void stm32_term_acsdk_connect(int argc, char *argv[])
{
  /* send a sync message in order to purge the serial queue */
  bool bResult = audio_json_transmission_send_sync(hTransmision);
  assert(bResult);

  /* send the ack to prevent no response message  */
  bResult = s_acsdk_cmd_ack(true, NULL, "Error: Connect", "Connect");
  assert(bResult);

  stm32_term_acsdk_cmd_get_elements(argc, argv);
  /* send the project */
  stm32_term_acsdk_cmd_get_instances(argc, argv);
}


/*Example message : Update:{"update":{"Instances":[{"InstanceName":"faust-reverb-dattorro-1","Params":[{"Name":"diffusion_wall4","Value":0}]}]}}*/
static jsonErr stm32_term_acsdk_apply_update(const char *pJson)
{
  UTIL_TERM_printf_cr("%s", pJson);

  jsonErr error = JSON_ERROR;
  json_instance_t jsonInst = {0};
  /* load the json */
  json_load(&jsonInst, pJson, &jsonInst.pack_root);
  if (jsonInst.pack_root)
  {
    /*verify the array update*/
    jsonID hUpdate;
    json_object_get_id_from_tree(&jsonInst, jsonInst.pack_root, "update", &hUpdate);
    if (hUpdate)
    {

      /* verify array instances */
      const char   *pInstanceName = "";
      const char   *pName = "";
      const char   *pValue = "";
      int64_t       type = 0;
      error = JSON_OK;

      jsonID hInstance;
      json_object_get_id_from_tree(&jsonInst, hUpdate, "Instances", &hInstance);
      if (hInstance)
      {
        uint16_t count;
        json_list_get_count(&jsonInst, hInstance, &count);
        for (uint16_t indexInst = 0; (indexInst < count) && (error == JSON_OK); indexInst++)
        {
          /* read the object instance */
          jsonID valueID;
          error = json_list_pair(&jsonInst, hInstance, indexInst, NULL, &valueID);
          if (error  == JSON_OK)
          {
            type = 0;

            /* type is optional, (0) by default apply immediately, (1) don't apply, so return an error but we ignore it  */
            json_object_get_integer(&jsonInst, valueID, "", "Type", &type);
          }
          /* instance name   */
          if (error  == JSON_OK)
          {
            error = json_object_get_string(&jsonInst, valueID, "", "InstanceName", &pInstanceName);
          }
          jsonID hParams;
          error = json_array_get(&jsonInst, valueID, "Params", &hParams);
          if (error  == JSON_OK)
          {
            uint16_t countParam;
            json_list_get_count(&jsonInst, hParams, &countParam);
            for (uint16_t indexParam = 0; (indexParam < countParam) && (error == JSON_OK); indexParam++)
            {
              /* read the object instance */
              jsonID objectID;
              error = json_list_pair(&jsonInst, hParams, indexInst, NULL, &objectID);
              if (error == JSON_OK)
              {
                error = json_object_get_string(&jsonInst, objectID, "", "Name", &pName);
              }
              if (error == JSON_OK)
              {
                error = json_object_get_string(&jsonInst, objectID, "", "Value", &pValue);
              }
              if (error == JSON_OK)
              {
                stm32_term_acsdk_apply(pInstanceName, pName, pValue, type);
              }
            }
          }
        }
      }
    }
  }
  return error;
}


/**
* @brief  Update the json instances
*
* @param argc  num args
* @param argv  args list
*/
static void stm32_term_acsdk_cmd_update_instances(int argc, char *argv[])
{
  bool     bOk     = false;
  bool     bResult = false;
  uint32_t syncTmo = 60UL;
  char    *pJson   = NULL;
  uint32_t szRead;

  if (argc == 1)
  {
    bOk = true;
  }
  if (bOk)
  {
    int32_t  error = UTIL_TERM_receive_file(&pJson, &szRead, syncTmo * 1000UL);
    UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);
    if ((szRead > 0UL) && (error == UTIL_ERROR_NONE))
    {
      bResult = (stm32_term_acsdk_apply_update(pJson) == JSON_OK);
    }
    else
    {
      UTIL_TERM_printf_cr("Aborted or error");
    }
  }
  if (pJson)
  {
    st_os_mem_free(pJson);
  }
  bResult = s_acsdk_cmd_ack(bResult, NULL, "Error: Update", "update_instances");
  assert(bResult);
}


/* hooks the terminal to init the transmission */

void UTIL_TERM_Initialized(UTIL_TERM_t *gContext);
void UTIL_TERM_Initialized(UTIL_TERM_t *gContext)
{
  /* make sure json uses st_os_mem */
  json_set_alloc_funcs(st_os_mem_generic_realloc_slow, st_os_mem_generic_free);
  hTransmision = audio_json_transmission_init(SEND_TRANSMISION_BLOCK_SIZE);
  assert(hTransmision != NULL);
  audio_json_transmission_set_cb(hTransmision, s_acsdk_send, NULL);
}



TERM_CMD_DECLARE("get_elements",     NULL,          "Export the json elements", stm32_term_acsdk_cmd_get_elements);
TERM_CMD_DECLARE("connect",          NULL,          "Connect the target",       stm32_term_acsdk_connect);
TERM_CMD_DECLARE("get_instances",    "[formatted]", "Export the json instance", stm32_term_acsdk_cmd_get_instances);
TERM_CMD_DECLARE("update_instances", NULL,          "Update the graph",         stm32_term_acsdk_cmd_update_instances);
