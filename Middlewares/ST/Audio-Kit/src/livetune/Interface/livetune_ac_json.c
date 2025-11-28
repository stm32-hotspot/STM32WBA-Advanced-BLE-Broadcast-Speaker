/**
******************************************************************************
* @file          livetune_ac_json.c
* @author        MCD Application Team
* @brief         manage the build or read of json for audio chain
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
#include "livetune_ac_factory.h"
#include "livetune_ac_json.h"
#include "audio_chain_json_from_factory.h"
#include "audio_chain_sysIOs.h"
#include "audio_chain_factory.h"
#include "audio_chain.h"



/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/


/**
* @brief fill the json  parameters with values coming from a pre-initialized sys-io
*
*/
void livetune_ac_json_fill_fake_parameters(json_instance_t *pJsonInst, jsonID hParam, livetune_chunk_conf_t *pChunConf)
{
  const audio_descriptor_params_t *pDescriptor = livetune_ac_factory_get_chunk_descriptor();
  for (int32_t indexParam = 0; indexParam < (int32_t)pDescriptor->nbParams; indexParam++)
  {
    audio_descriptor_param_t *pParam = &pDescriptor->pParam[indexParam];
    char_t                    tNum[20];
    /* comput the variable offset in the struct */
    void *pData = (uint8_t *)pChunConf + pParam->iOffset;
    /* convert it as string because AudioChainJson_factory_translate_key_value needs string */
    livetune_ac_factory_get_string_value(pParam, pData, tNum, sizeof(tNum));
    /* Get its config representation ( key value or numeric ) */
    char_t *pTranslated = AudioChainJson_factory_translate_key_value(pParam, tNum);
    /* produce the config as static no modifiable */
    AudioChainJson_json_add_param(pJsonInst, hParam, -1, NULL, 0, pParam->pName, NULL, "sysio", NULL, pTranslated, 0.0F, 0.0F, TRUE);
    st_os_mem_free(pTranslated);
  }
}


/**
* @brief load params coming from the algo template
*
*/

void livetune_ac_json_update_element_description(json_instance_t *pJsonInst, jsonID rootID, livetune_db_element *pElement)
{
  if (rootID)
  {


    livetune_helper_create_element_instance(pElement);
    const audio_algo_factory_t *pFactory = livetune_ac_factory_get_factory_from_element(pElement);
    AudioChainJson_update_element(pFactory, pJsonInst, rootID);
  }
}




/**
* @brief Produce a json string update with ac definitions
*
* @param pHandle the db instance
* @param pElement the element instance
* @return the string or NULL, the caller must free the pointer
*/


char_t *livetune_ac_json_update_element(livetune_db *pHandle, livetune_db_element *pElement, uint32_t szHeaderDump)
{
  char_t *pJson = NULL;

  ST_ASSERT(pHandle != NULL);
  ST_ASSERT(pElement != NULL);

  json_instance_t jsonInst = {0};
  jsonID          rootID;
  json_load(&jsonInst, pElement->pJsonDefinition, &rootID);
  ST_ASSERT(rootID != JSON_ID_NULL);
  if (rootID)
  {
    livetune_ac_json_update_element_description(&jsonInst, rootID, pElement);
    json_dump_t dump =
    {
      .szDump  = 0,
      .curDump = 0,
      .pDump   = NULL
    };
    /* in case of space reservation before the json */
    if (szHeaderDump != 0U)
    {
      dump.pDump = st_os_mem_generic_realloc_fast(NULL, szHeaderDump);
      dump.curDump = szHeaderDump;
      dump.szDump = szHeaderDump;
    }

    pJson = (char_t *)json_stringify(&jsonInst, rootID, 0, &dump);
    json_shutdown(&jsonInst);
  }
  return pJson;
}



/**
* @brief Create the json sysio element definition from the template
*/


char_t *livetune_ac_json_sysio_create_def(audio_chain_sys_connection_conf_t *pConf, const char_t *pDesc)
{
  char_t *pDescription = NULL;

  json_instance_t jsonInst = {0};
  jsonID          rootID;
  json_load(&jsonInst, pDesc, &rootID);
  ST_ASSERT(rootID != JSON_ID_NULL);
  if (rootID)
  {
    audio_chunk_conf_t const *pChunConf = AudioChainSysIOs_getConf(pConf->pSysChunk);

    /*
    AC uses 2 structs to describe a chunk audio_chunk_conf_t and audio_chunk_conf_t with almost the same parameters
    in order to use API using audio_chunk_conf_t and implement a specific one for audio_chunk_conf_t, we create
    a temporary derivative audio_chunk_conf_t*/

    livetune_chunk_conf_t fakeConf;
    fakeConf.bufferType  = pChunConf->bufferType;
    fakeConf.chunkType   = pChunConf->chunkType;
    fakeConf.fs          = pChunConf->fs;
    fakeConf.interleaved = pChunConf->interleaved;
    fakeConf.nbChannels  = pChunConf->nbChannels;
    fakeConf.nbElements  = pChunConf->nbElements;
    fakeConf.nbFrames    = pChunConf->nbFrames;
    fakeConf.timeFreq    = pChunConf->timeFreq;


    ST_VERIFY(json_object_set_string(&jsonInst, rootID, "", "Description", pConf->pDescription) == JSON_OK);
    ST_VERIFY(json_object_set_string(&jsonInst, rootID, "", "Name", pConf->pName) == JSON_OK);
    ST_VERIFY(json_object_set_string(&jsonInst, rootID, "", "SysIo", "1") == JSON_OK);
    jsonID hParam;
    ST_VERIFY(json_create_array(&jsonInst, &hParam) == JSON_OK);
    ST_VERIFY(json_object_set_new(&jsonInst, rootID, "Params", hParam) == JSON_OK);

    livetune_ac_json_fill_fake_parameters(&jsonInst, hParam, &fakeConf);
    AudioChainJson_json_add_param(&jsonInst, hParam, -1, NULL, 0, "variable", NULL, "sysio", NULL, pConf->pVarName, 0.0F, 0.0F, TRUE);
    pDescription = (char_t *)json_dumps(&jsonInst, rootID, 0);
    json_shutdown(&jsonInst);
  }

  return pDescription;
}



