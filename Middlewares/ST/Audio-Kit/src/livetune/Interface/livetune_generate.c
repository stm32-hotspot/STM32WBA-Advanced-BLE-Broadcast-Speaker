/**
******************************************************************************
* @file          livetune_generate.c
* @author        MCD Application Team
* @brief         generate source code
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
#include "livetune_helper.h"
#include "audio_chain_factory.h"
#include "audio_descriptor.h"
#include "audio_chain_sysIOs.h"
#include "audio_chain_sysIOs_conf.h"


#include <ctype.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define GENERATE_LONG_STRING_THRESHOLD 200U

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private routines ----------------------------------------------------------*/
static void trim_trailing_spaces_and_tabs(char *const pString);



/**
* @brief  Generate the source code c for chunk  instances
*
* @param pHandle the instance handle
* @param pPinDef   instance pin def
* @param pin  index pin
*/

void livetune_generate_chunk_handles(livetune_pipe *pHandle, livetune_db_instance_pins_def **pPinDef, uint32_t pin)
{
  if (pHandle->bGenerateCode)
  {
    livetune_db_instance_pins_def *pDef  = livetune_db_instance_pin_def_get(pPinDef, pin);
    livetune_db_instance_cnx_list *pList = pDef->pObject;
    uint32_t                          nbCnx = (uint32_t)livetune_db_cnx_nb(&pList);

    /* copy the pin in on each cnx out */
    for (uint32_t indexCnx = 0; indexCnx < nbCnx; indexCnx++)
    {
      livetune_db_instance_cnx *pCnx = livetune_db_cnx_get(&pList, indexCnx);
      if (pCnx->pInstance->hAc.bSysIO == 0U)
      {
        livetune_generate_global(pHandle, "static acChunk", pCnx->hAc.tVarName, livetune_helper_format("// %s handle ", pCnx->hAc.tVarName));
      }
    }
  }
}


/**
* @brief Generate list of algo used for this graph
*
* @param pHandle the instance handle
*/

static ST_Result livetune_generate_code_add_algo_list(livetune_pipe *pHandle)
{
  ST_Result result = ST_OK;
  livetune_db_list *pElementUsed = NULL;
  livetune_db *pDb = pHandle->pDataBase;


  /* select only algo used by the graph */
  uint32_t nbInstance = livetune_db_list_nb(&pDb->pInstances);
  for (uint32_t indexInstance = 0; indexInstance < nbInstance; indexInstance++)
  {
    livetune_db_instance  *pInstance = livetune_db_list_get_object(&pDb->pInstances, indexInstance);
    uint8_t                found     = FALSE;
    for (livetune_db_list *pList     = pElementUsed; pList != NULL; pList = pList->pNext)
    {
      if (strcmp(pList->pObject, pInstance->pElements->pName) == 0)
      {
        found = 1;
        break;
      }
    }
    if (found == 0U)
    {
      livetune_helper_builder_def *pRef = (livetune_helper_builder_def *)pInstance->pElements->pRefBuilder;

      /* check if it is an algo factory */
      if ((pRef != NULL) && (pRef ->pAlgoName != NULL))
      {
        if (livetune_ac_is_factory_algo(pRef ->pAlgoName))
        {
          livetune_db_list_create(&pElementUsed, (void *)pRef ->pAlgoName);
        }
      }
    }
  }
  livetune_pipe_add_log(pHandle, LIVETUNE_LOG_CODE, "#ifdef ALGO_USE_LIST\n");
  livetune_pipe_add_log(pHandle, LIVETUNE_LOG_CODE, "  // This code allows to reduce the flash footprint for the release firmware\n");
  livetune_pipe_add_log(pHandle, LIVETUNE_LOG_CODE, "  // Only algos mandatory for the audio pipeline will be linked\n");

  livetune_pipe_add_log(pHandle, LIVETUNE_LOG_CODE, "  #include \"audio_chain_factory.h\"\n");

  int32_t indexElement = 0;
  for (livetune_db_list *pList = pElementUsed; pList != NULL; pList = pList->pNext)
  {
    const char_t *pAlgoName            = pList->pObject;
    const char_t *pFactoryInstanceName = livetune_ac_factory_get_factory_instance_name(pAlgoName);
    if (pFactoryInstanceName)
    {
      livetune_pipe_add_printf(pHandle, LIVETUNE_LOG_CODE, "  /* %02d %s */\n", indexElement, pAlgoName);
      livetune_pipe_add_printf(pHandle, LIVETUNE_LOG_CODE, "  extern audio_algo_factory_t %s;\n", pFactoryInstanceName);
      livetune_pipe_add_printf(pHandle, LIVETUNE_LOG_CODE, "  ALGO_FACTORY_DECLARE_FORCE(%s);\n", pFactoryInstanceName);
    }
    indexElement++;
  }
  livetune_pipe_add_printf(pHandle, LIVETUNE_LOG_CODE, "#endif\n");
  livetune_db_list_clear(&pElementUsed, FALSE);
  return result;
}


/**
* @brief Generate the source code c that reproduce the graph
*
* @param pHandle the instance handle
*/

char_t *livetune_generate_code(livetune_pipe *pHandle)
{
  ST_ASSERT(pHandle != NULL);
  char_t   *pCode  = NULL;
  char_t    tScratch[50];
  ST_Result result = ST_ERROR;
  livetune_pipe_stop(pHandle);
  int8_t oldRtState = livetune_pipe_set_rt_graph_construction(pHandle, FALSE);
  livetune_pipe_enable_generate_code(pHandle, TRUE);

  /*
  to generate the source code we going to simulate a pipe start without start audio, and each instance must write its code
  using livetune_pipe_log
  */
  if (livetune_pipe_build_order(pHandle))
  {
    const audio_persist_config *const pGenerated_code_audio_persist_config = audio_persist_get_config_cur();
    uint32_t buildId      = (uint32_t)st_os_sys_time() & 0xFFFFUL;

    pHandle->szHeaderList = 0;
    livetune_generate_header_add(pHandle, "stdio.h");
    livetune_generate_header_add(pHandle, "string.h");
    livetune_generate_header_add(pHandle, "assert.h");
    livetune_generate_header_add(pHandle, "acSdk.h");


    livetune_pipe_notify(pHandle, LIVETUNE_PIPE_CONSTRUCTOR);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "// Code generated by LiveTune: refId: %d\n\n", buildId);

    livetune_generate_header(pHandle);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n");


    snprintf(tScratch, sizeof(tScratch), "\"%s\"", AC_VERSION);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "%s %-25s %-25s  %s\n", "#define", "CODE_GEN_SDKVERSION", tScratch, "// The code has been generated with this Firmware version");
    snprintf(tScratch, sizeof(tScratch), "\"%s\"", STM32_NAME_STRING);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "%s %-25s %-25s  %s\n", "#define", "CODE_GEN_BOARD", tScratch, "// The code has been generated with this board");
    audio_persist_get_config_id(pGenerated_code_audio_persist_config, tScratch + 1, sizeof(tScratch) - 1U);
    tScratch[0] = '\"';
    strcat(tScratch, "\"");
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "%s %-25s %-25s  %s\n", "#define", "CODE_GEN_AUDIOCONFIG", tScratch, "// The code has been generated with this audio configuration");

    livetune_pipe_notify(pHandle, LIVETUNE_PIPE_START);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n\n");

    livetune_generate_global(pHandle, "static acPipe", "hPipe", "// Pipe Handle");

    livetune_pipe_notify(pHandle, LIVETUNE_PIPE_PROLOG);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n\n"); // new line

    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "// This function is called at the very beginning of the system initialization.\n");
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "// It gives the opportunity to replace the default audio config by the configuration used during the code generation.\n");
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "void AudioChainInstance_initSystem(void)\n");
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "{\n");
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "  assert(acAudioConfigSet(CODE_GEN_AUDIOCONFIG) == 0);\n");
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "}\n\n");


    livetune_pipe_notify(pHandle, LIVETUNE_PIPE_PRE_INIT_GRAPH);

    livetune_generate_fn_declare(pHandle, "void", "AudioChainInstance_initGraph(void)");
    livetune_generate_fn_declare(pHandle, "{", NULL);

    livetune_generate_fn_body(pHandle, "int32_t error = 0;\n", NULL);
    livetune_generate_fn_body(pHandle, "// Check code generated audio alignment", NULL);
    livetune_generate_fn_body(pHandle, "if (strcmp(acSdkVersion(), CODE_GEN_SDKVERSION) != 0)", NULL);
    livetune_generate_fn_body(pHandle, "{", NULL);
    livetune_generate_fn_body(pHandle, "  acTrace(\"The firmware version of the code generated (%s) is misaligned with the current firmware version and could cause failures.\\n\", CODE_GEN_SDKVERSION);", NULL);
    livetune_generate_fn_body(pHandle, "  acTrace(\"Please consider to regenerate the code from your LiveTune project using this a firmware %s.\\n\", acSdkVersion());", NULL);
    livetune_generate_fn_body(pHandle, "  acTrace(\"You can also remove this check, if the SDK version doesn't impact the generated code.\\n\");", NULL);
    livetune_generate_fn_body(pHandle, "}\n", NULL);


    livetune_generate_fn_body(pHandle, " // Check code generated board alignment", NULL);
    livetune_generate_fn_body(pHandle, "if (strcmp(acConfigBoard(), CODE_GEN_BOARD) != 0)", NULL);
    livetune_generate_fn_body(pHandle, "{", NULL);
    livetune_generate_fn_body(pHandle, "  acTrace(\"The board of the code generated (%s) is misaligned with the current firmware version and could cause failures.\\n\", CODE_GEN_BOARD);", NULL);
    livetune_generate_fn_body(pHandle, "  acTrace(\"Please consider to regenerate the code from your LiveTune project using a board %s.\\n\", acConfigBoard());", NULL);
    livetune_generate_fn_body(pHandle, "  acTrace(\"You can also remove this check, if the SDK version doesn't impact the generated code.\\n\");", NULL);
    livetune_generate_fn_body(pHandle, "}\n", NULL);



    livetune_generate_fn_body(pHandle, "// Initialize acSDK", NULL);
    livetune_generate_fn_body(pHandle, "error = acInitialize();", NULL);
    livetune_generate_fn_body(pHandle, "assert(error == 0);", NULL);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n");


    livetune_generate_fn_body(pHandle, "// Enable the background cycle count", NULL);
    livetune_generate_fn_body(pHandle, "error = acEnvSetConfig(\"bLogCycles\", AC_FALSE);", NULL);
    livetune_generate_fn_body(pHandle, "assert(error == 0);", NULL);
    livetune_generate_fn_body(pHandle, "// Enable the default cycleCount handler, the default handler prints cycleCount in Mhz in the console", NULL);
    livetune_generate_fn_body(pHandle, "error = acEnvSetConfig(\"bDefaultCyclesMgntCb\", AC_TRUE);", NULL);
    livetune_generate_fn_body(pHandle, "assert(error == 0);", NULL);
    livetune_generate_fn_body(pHandle, "// Set the cycleCount call-back delay to 5 secs", NULL);
    livetune_generate_fn_body(pHandle, "error = acEnvSetConfig(\"cyclesMgntCbTimeout\", 5000UL);", NULL);
    livetune_generate_fn_body(pHandle, "assert(error == 0);", NULL);
    livetune_generate_fn_body(pHandle, "// Set the cycleCount measure delay to 500 msecs", NULL);
    livetune_generate_fn_body(pHandle, "error = acEnvSetConfig(\"cyclesMgntMeasureTimeout\", 500UL);", NULL);
    livetune_generate_fn_body(pHandle, "assert(error == 0);", NULL);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n");

    livetune_generate_fn_body(pHandle, "// Set the chunks' memory pool", NULL);
    snprintf(tScratch, sizeof(tScratch), "error = acEnvSetConfig(\"chunkMemPool\", %dUL);", livetune_state_get_chunk_pool());
    livetune_generate_fn_body(pHandle, tScratch, NULL);
    livetune_generate_fn_body(pHandle, "assert(error == 0);", NULL);
    livetune_generate_fn_body(pHandle, "// Set the algos' handle & config memory pool", NULL);
    snprintf(tScratch, sizeof(tScratch), "error = acEnvSetConfig(\"algoMemPool\", %dUL);", livetune_state_get_algo_pool());
    livetune_generate_fn_body(pHandle, tScratch, NULL);
    livetune_generate_fn_body(pHandle, "assert(error == 0);", NULL);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n");

    livetune_helper_builder_pipe_init(pHandle);

    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n");


    livetune_pipe_notify(pHandle, LIVETUNE_PIPE_POST_INIT_GRAPH);
    livetune_pipe_notify(pHandle, LIVETUNE_PIPE_INIT_GRAPH_ELEMENT);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n");
    livetune_pipe_notify(pHandle, LIVETUNE_PIPE_INIT_GRAPH_ATTACH_CNX);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n");

    livetune_generate_fn_body(pHandle, "// Start the pipe", NULL);
    livetune_generate_fn_body(pHandle, "error = acPipePlay(hPipe, AC_START);", NULL);
    livetune_generate_fn_body(pHandle, "if (error != 0)", NULL);
    livetune_generate_fn_body(pHandle, "{", NULL);

    livetune_generate_fn_body(pHandle, "  acTrace(\"Ac play error\\n\");", NULL);

    livetune_generate_fn_body(pHandle, "}", NULL);
    livetune_generate_fn_body(pHandle, "else", NULL);
    livetune_generate_fn_body(pHandle, "{", NULL);
    livetune_generate_fn_body(pHandle, "  acTrace(\"Ac playing\\n\");", NULL);
    livetune_generate_fn_body(pHandle, "}", NULL);

    livetune_generate_fn_declare(pHandle, "}", NULL);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "\n\n");
    result = ST_OK;
  }
  if (result == ST_OK)
  {
    result = livetune_generate_code_add_algo_list(pHandle);
  }
  if (result == ST_OK)
  {
    // call again trim_trailing_spaces_and_tabs in case some useless spaces and tabs are still present (they should have already been filtered in livetune_generate_aligned)
    trim_trailing_spaces_and_tabs(pHandle->pGenerateCode);

    /* detach the buffer, must be freed by the caller */
    pCode                  = pHandle->pGenerateCode;
    pHandle->pGenerateCode = NULL;
    livetune_pipe_enable_generate_code(pHandle, FALSE);
  }
  livetune_pipe_enable_generate_code(pHandle, FALSE);
  livetune_pipe_set_rt_graph_construction(pHandle, oldRtState);



  return pCode;
}

/**
* @brief Generate a configuration
*
* @param pBuilder the instance handle builder
* @param pParamConf the instance handle generator
*/

static void livetune_generate_config(livetune_helper_builder *pBuilder, livetune_generate_conf *pParamConf, int8_t bForceGenerateAllConfigs)
{
  audio_descriptor_param_t *pParam = pParamConf->pParamDesc;

  if ((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE) == 0UL)
  {
    /* check if the new parameter is the default value, in this case no need to generate a code */
    if ((livetune_ac_factory_is_default_value_string(pParam, pParamConf->pValue) == FALSE) || (bForceGenerateAllConfigs == TRUE))
    {
      livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("// %s ", pParam->pDescription), NULL);

      char_t *pTranslated = AudioChainJson_factory_translate_key_value(pParam, pParamConf->pValue);
      if (strlen(pTranslated) > GENERATE_LONG_STRING_THRESHOLD)
      {
        // if the string is too long, we going to cut it by in order to improve the presentation
        #ifdef ST_GENERATE_SPLIT_LONG_STRING
        livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("const char *%s_%s =", pBuilder->pInstance->hAc.tVarName, pParam->pName), NULL);
        uint32_t      countText   = strlen(pTranslated);
        const char_t *pCharStream = pTranslated;
        char_t        tScratch[130];
        while (countText)
        {
          uint32_t blk = 128;
          if (countText < blk)
          {
            blk = countText;
          }
          memcpy(tScratch, pCharStream, blk);
          tScratch[blk] = 0;
          pCharStream += blk;
          countText -= blk;
          livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("                    \"%s\"%s", tScratch, blk == 128 ? "" : ";"), NULL);
        }
        #else
        livetune_pipe_add_log(pBuilder->pPipe, LIVETUNE_LOG_CODE, livetune_helper_format("  const char *%s_%s =\"", pBuilder->pInstance->hAc.tVarName, pParam->pName));
        livetune_pipe_add_log(pBuilder->pPipe, LIVETUNE_LOG_CODE, pTranslated);
        livetune_pipe_add_log(pBuilder->pPipe, LIVETUNE_LOG_CODE, "\";\n");
        #endif
        livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = %s(%s, \"%s\", %s_%s);", pParamConf->pFunction, pParamConf->pVarName, pParam->pName, pBuilder->pInstance->hAc.tVarName, pParam->pName), NULL);
      }
      else
      {
        livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = %s(%s, \"%s\", \"%s\");", pParamConf->pFunction, pParamConf->pVarName, pParam->pName, pTranslated), NULL);
      }

      livetune_generate_fn_body(pBuilder->pPipe, "assert(error == 0);", NULL);
      st_os_mem_free(pTranslated);
    }
  }
}
/**
 * @brief Generate a config chunk
 *
 * @param pBuilder the builder
 * @param pCnx the connection
 */
static void livetune_generate_create_config_chunk(livetune_helper_builder *pBuilder, livetune_db_instance_cnx *pCnx)
{
  const audio_descriptor_params_t *pDescriptor = livetune_ac_factory_get_chunk_descriptor();
  ST_ASSERT(pDescriptor != NULL);
  for (uint32_t indexParam = 0; indexParam < pDescriptor->nbParams; indexParam++)
  {
    audio_descriptor_param_t *pParam = &pDescriptor->pParam[indexParam];
    void                     *pData  = (uint8_t *)&pCnx->hAc.hConf + pParam->iOffset;

    char_t tValue[20];
    livetune_ac_factory_get_string_value(pParam, pData, tValue, sizeof(tValue));
    livetune_generate_conf genConf;
    genConf.pFunction  = "acChunkSetConfig";
    genConf.pParamDesc = pParam;
    genConf.pValue     = tValue;
    genConf.pVarName   = pCnx->hAc.tVarName;
    livetune_generate_config(pBuilder, &genConf, TRUE);
  }
}


/**
* @brief return the ac define name from the chunk name
*
* @param pName chunk name
* @param define name
*/
static const char_t *livetune_generate_get_acChunk_define(char_t *pName)
{
  const char_t        *pDefine = pName;
  int32_t index = 0;
  while (tLiveTuneDefineConv[index][0])
  {
    if (strcmp(tLiveTuneDefineConv[index][1], pName) == 0)
    {
      pDefine = tLiveTuneDefineConv[index][0];
      break;
    }
    index++;
  }
  return pDefine;
}



/**
* @brief Generate the source code c that init a chunk
*
* @param pHandle the instance handle
* @param pChunk  the chunk instance
* @param pVariable the chunk variable
*/
static void livetune_generate_create_chunk(livetune_helper_builder *pBuilder, livetune_db_instance_cnx *pCnx)
{
  if (pBuilder->pPipe->bGenerateCode)
  {
    livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("// Create the chunk %s", pCnx->hAc.tVarName), NULL);
    /* Manage the sysio name */
    const char_t *pChunkName = pCnx->hAc.tVarName;
    if (pCnx->hAc.pSysIoName)
    {
      pChunkName = livetune_generate_get_acChunk_define(pCnx->hAc.pSysIoName);
      livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = acChunkCreate(hPipe, %s, &%s);", pChunkName, pCnx->hAc.tVarName), NULL);
    }
    else
    {
      livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = acChunkCreate(hPipe, \"%s\", &%s);", pChunkName, pCnx->hAc.tVarName), NULL);
    }
    livetune_generate_fn_body(pBuilder->pPipe, "assert(error == 0);", NULL);
    /* if sysio, we cannot change chunk config */
    if (pBuilder->pInstance->hAc.bSysIO == 0U)
    {
      livetune_generate_create_config_chunk(pBuilder, pCnx);
    }
  }
}



/**
* @brief Generate the source code c for the chunk append
*
* @param pHandle the instance handle
* @param ...
*/

void livetune_generate_connect_pins(livetune_pipe *pHandle, char_t *pAlgoName, uint32_t pinId, char_t *pChunkName, uint32_t pinType)
{
  if (pHandle->bGenerateCode)
  {
    if (pinType == (uint32_t)ST_GENERATOR_PIN_TYPE_IN)
    {
      livetune_generate_fn_body(pHandle, livetune_helper_format("error = acPipeConnectPinIn(hPipe, %s, %d, %s);", pAlgoName, pinId, pChunkName), NULL);
      livetune_generate_fn_body(pHandle, "assert(error == 0);", NULL);
    }
    if (pinType == (uint32_t)ST_GENERATOR_PIN_TYPE_OUT)
    {
      livetune_generate_fn_body(pHandle, livetune_helper_format("error = acPipeConnectPinOut(hPipe, %s, %d, %s);", pAlgoName, pinId, pChunkName), NULL);
      livetune_generate_fn_body(pHandle, "assert(error == 0);", NULL);
    }
  }
}



/**
* @brief  generate a connect algo to algo
*
* @param pHandle the instance handle
* @param ...
*/

void livetune_generate_connect(livetune_helper_builder *pBuilder, char_t *pChunkName, char_t *pAlgoDst)
{
  if (pBuilder->pPipe->bGenerateCode)
  {
    livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = acPipeConnect(hPipe,%s, %s,%s);", pBuilder->pInstance->hAc.tVarName, pChunkName, pAlgoDst), NULL);
    livetune_generate_fn_body(pBuilder->pPipe, "assert(error == 0);", NULL);
  }
}




/**
* @brief Generate the source code aligned on tabs
*
* @param pHandle the instance handle
* @param ...
*/

void livetune_generate_aligned(livetune_pipe *pHandle, int32_t wPos1, char_t *pPos1, int32_t wPos2, char_t *pPos2, int32_t wPos3, char_t *pPos3, int32_t wPos4, char_t *pPos4)
{
  char_t tlocalScratch[2 * LIVETUNE_VAR_NAME_MAX];
  if (pHandle->bGenerateCode)
  {
    livetune_helper_format_clear(pHandle->tScratch);
    if (pPos1 == NULL)
    {
      pPos1 = "";
    }
    snprintf(tlocalScratch, sizeof(tlocalScratch), "%-*s", wPos1, pPos1);
    strcat(pHandle->tScratch, tlocalScratch);
    if (pPos2 == NULL)
    {
      pPos2 = "";
    }
    snprintf(tlocalScratch, sizeof(tlocalScratch), "%-*s", wPos2, pPos2);
    strcat(pHandle->tScratch, tlocalScratch);
    if (pPos3 == NULL)
    {
      pPos3 = "";
    }
    snprintf(tlocalScratch, sizeof(tlocalScratch), "%-*s", wPos3, pPos3);
    strcat(pHandle->tScratch, tlocalScratch);

    if (pPos4 == NULL)
    {
      pPos4 = "";
    }
    snprintf(tlocalScratch, sizeof(tlocalScratch), "%-*s", wPos4, pPos4);
    strcat(pHandle->tScratch, tlocalScratch);
    trim_trailing_spaces_and_tabs(pHandle->tScratch);
    livetune_pipe_add_log(pHandle, LIVETUNE_LOG_CODE, pHandle->tScratch);
    livetune_pipe_add_log(pHandle, LIVETUNE_LOG_CODE, "\n");
  }
}



/**
* @brief Generate the source code c a global
*
* @param pHandle the instance handle
* @param ...
*/

#define GLOBAL_ALIGN_TYPE    20
#define GLOBAL_ALIGN_VAR     30
#define GLOBAL_ALIGN_COMMENT 30

void livetune_generate_global(livetune_pipe *pHandle, char_t *pType, char_t *pVarName, char_t *pComment)
{
  if (pHandle->bGenerateCode)
  {
    livetune_generate_aligned(pHandle, GLOBAL_ALIGN_TYPE, pType, GLOBAL_ALIGN_VAR, livetune_helper_format("%s;", pVarName), GLOBAL_ALIGN_COMMENT, pComment, 0, NULL);
  }
}

#define FN_DECLARE_ALIGN_TYPE 3
#define FN_DECLARE_ALIGN_BODY 50

void livetune_generate_fn_declare(livetune_pipe *pHandle, char_t *pType, char_t *pBody)
{
  if (pHandle->bGenerateCode)
  {
    livetune_generate_aligned(pHandle, FN_DECLARE_ALIGN_TYPE, livetune_helper_format("%s ", pType), FN_DECLARE_ALIGN_BODY, pBody, 0, NULL, 0, NULL);
  }
}


/**
 * @brief Generate a comment
 *
 * @param pHandle  the instance
 * @param pText    the text comment
 */

void livetune_generate_comment(livetune_pipe *pHandle, char_t *pText)
{
  if (pHandle->bGenerateCode)
  {
    livetune_pipe_add_log(pHandle, LIVETUNE_LOG_CODE, pText);
    livetune_pipe_add_log(pHandle, LIVETUNE_LOG_CODE, "\n");
  }
}

#define FN_BODY_ALIGN_INDENT  2
#define FN_BODY_ALIGN_BODY    100
#define FN_BODY_ALIGN_COMMENT 1

/**
 * @brief Generate a function body
 *
 * @param pHandle the instance
 * @param pBody   the body text
 * @param pComment the comment
 */

void livetune_generate_fn_body(livetune_pipe *pHandle, char_t *pBody, char_t *pComment)
{
  if (pHandle->bGenerateCode)
  {
    livetune_generate_aligned(pHandle, FN_BODY_ALIGN_INDENT, "", FN_BODY_ALIGN_BODY, pBody, FN_BODY_ALIGN_COMMENT, pComment, 0, NULL);
  }
}

/**
* @brief Generate the source code c a local
*
* @param pHandle the instance handle
* @param ...
*/

void livetune_helper_generate_local(livetune_pipe *pHandle, char_t *pType, char_t *pVarName, char_t *pComment)
{
  if (pHandle->bGenerateCode)
  {
    livetune_helper_format_clear(pHandle->tScratch);
    livetune_helper_format_tab(pHandle->tScratch, ST_GENERATOR_TAB_1);
    livetune_helper_format_add(pHandle->tScratch, ST_GENERATOR_SZ_CODE_1, "%s", pType);
    livetune_helper_format_add(pHandle->tScratch, ST_GENERATOR_SZ_CODE_1, "%s", pVarName);
    livetune_helper_format_add(pHandle->tScratch, ST_GENERATOR_SZ_CODE_1, "// %s", (pComment != NULL) ? pComment : "");
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "%s\n", pHandle->tScratch);
  }
}

/**
* @brief Generate the source code c raw text
*
* @param pHandle the instance handle
* @param ...
*/

void livetune_generate_text(livetune_pipe *pHandle, char_t *pFormat, ...)
{
  if (pHandle->bGenerateCode)
  {
    va_list args;
    va_start(args, pFormat);
    vsnprintf(pHandle->tScratch, sizeof(pHandle->tScratch), pFormat, args);
    livetune_pipe_log(pHandle, LIVETUNE_LOG_CODE, "%s\n", pHandle->tScratch);
    va_end(args);
  }
}



/**
* @brief Generate the source code c add to the header list
*
* @param pHandle the instance handle
* @param ...
*/

void livetune_generate_header_add(livetune_pipe *pHandle, const char_t *pHeader)
{
  if ((pHandle->bGenerateCode != 0) && (pHandle->szHeaderList < ST_PIPE_HEADER_LIST_MAX))
  {
    for (uint32_t indexHeader = 0; indexHeader < pHandle->szHeaderList; indexHeader++)
    {
      if (strcmp(pHandle->tHeaderList[indexHeader], pHeader) == 0)
      {
        return;
      }
    }
    pHandle->tHeaderList[pHandle->szHeaderList++] = pHeader;
  }
}


/**
* @brief Generate the source code c header list
*
* @param pHandle the instance handle
* @param ...
*/

void livetune_generate_header(livetune_pipe *pHandle)
{
  if (pHandle->bGenerateCode)
  {
    for (uint32_t indexHeader = 0; indexHeader < pHandle->szHeaderList; indexHeader++)
    {
      livetune_generate_text(pHandle, "#include \"%s\"", pHandle->tHeaderList[indexHeader]);
    }
  }
}



/**
* @brief Generate an auto initialized array
*
* @param pArray  the opaque array
* @param pScracth the write buffer
* @param nbRow    nb row
* @param nbColumn nb Column
* @param  type    nb bits 8,32
*/


void livetune_generate_initalized_array(void *pArray, char_t *pScratch, int32_t nbRow, int32_t nbColumn, int32_t type)
{
  ST_ASSERT(pArray != NULL);
  char_t *pDim = (char_t *)pArray;
  pScratch[0]  = '\0';
  strcat(pScratch, "{");
  for (int32_t row = 0; row < nbRow; row++)
  {
    if (row)
    {
      strcat(pScratch, ",");
    }
    strcat(pScratch, "{");
    for (int32_t col = 0; col < nbColumn; col++)
    {
      uint32_t val = 0;
      if (type == 8)
      {
        val = (uint32_t)(*pDim++);
      }
      if (type == 32)
      {
        val = *((uint32_t *)pDim);
        pDim += sizeof(uint32_t);
      }
      if (col)
      {
        strcat(pScratch, ",");
      }
      sprintf(strchr(pScratch, 0), "%ld", val);
    }
    strcat(pScratch, "}");
  }
  strcat(pScratch, "}");
}


/**
* @brief generate chunk instance
*
* @param pHandle the pipe instance handle
* @param pPinDef the pindef instance handle
* @param pinDef   the out pindef
*/
void livetune_generate_create_chunks(struct livetune_helper_builder *pBuilder, livetune_db_instance_pins_def **pPinDef, uint32_t pinDef)
{
  uint32_t nbCnx = (uint32_t)livetune_helper_cnx_nb(pPinDef, pinDef);
  for (uint32_t index = 0; index < nbCnx; index++)
  {
    livetune_db_instance_cnx *pCnx = livetune_helper_cnx_get(pPinDef, pinDef, index);
    ST_ASSERT(pCnx != NULL);
    if ((pCnx->hAc.bValid != 0U) && (pCnx->pInstance->hAc.bValid != 0U))
    {
      livetune_generate_create_chunk(pBuilder, pCnx);
    }
  }
}



/**
* @brief instantiate or generate code
* * @param pHandle the instance handle
*/

const char_t *livetune_generate_get_type_str(livetune_helper_builder *pBuilder, audio_descriptor_param_type_t type)
{
  const char_t *pVarType = "unknown";
  switch (type)
  {
    case AUDIO_DESC_PARAM_TYPE_UINT8:
    {
      pVarType = "uint8_t";
      break;
    }
    case AUDIO_DESC_PARAM_TYPE_INT8:
    {
      pVarType = "int8_t";
      break;
    }
    case AUDIO_DESC_PARAM_TYPE_UINT16:
    {
      pVarType = "uint16_t";
      break;
    }
    case AUDIO_DESC_PARAM_TYPE_INT16:
    {
      pVarType = "int16_t";
      break;
    }
    case AUDIO_DESC_PARAM_TYPE_UINT32:
    {
      pVarType = "uint32_t";
      break;
    }
    case AUDIO_DESC_PARAM_TYPE_OBJECT:
    case AUDIO_DESC_PARAM_TYPE_ADDRESS:
    {
      pVarType = "void *";

      break;
    }


    case AUDIO_DESC_PARAM_TYPE_FLOAT:
    {
      pVarType = "float";
      break;
    }
    break;
    case AUDIO_DESC_PARAM_TYPE_INT32:
    default:
    {
      pVarType = "int32_t";
      break;
    }
  }
  return pVarType;
}

/**
* @brief generate a control callback
*
* @param pBuilder builder instance
* @param fnName   callback name
*/
void livetune_generate_control_callback(livetune_helper_builder *pBuilder, char_t *fnName, livetune_generate_control_cb generateCB)
{
  if (pBuilder->pPipe->bGenerateCode)
  {
    livetune_generate_comment(pBuilder->pPipe, livetune_helper_format("\n// Implement the control callback for the algo %s", pBuilder->pInstance->pInstanceName));
    livetune_generate_fn_declare(pBuilder->pPipe, "static int32_t", livetune_helper_format("%s(acAlgo hAlgo)", fnName));
    livetune_generate_fn_declare(pBuilder->pPipe, "{", NULL);
    livetune_generate_fn_body(pBuilder->pPipe, "int32_t error = 0;", NULL);
    livetune_generate_fn_body(pBuilder->pPipe, "void   *pUserData = NULL;\n", NULL);
    livetune_generate_fn_body(pBuilder->pPipe, "error = acAlgoGetCommonConfig(hAlgo, \"userData\", &pUserData);", NULL);
    livetune_generate_fn_body(pBuilder->pPipe, "assert(error == 0);\n", NULL);
    const audio_algo_factory_t *pFactory = livetune_ac_factory_get_algo_factory_name(pBuilder->pBuilderDef->pAlgoName);
    if (pFactory)
    {
      for (uint32_t indexCtrl = 0; indexCtrl < pFactory->pControlTemplate->nbParams; indexCtrl++)
      {
        const audio_algo_control_param_t *pParam = &pFactory->pControlTemplate->pParams[indexCtrl];
        livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("// %s", pParam->pDescription), NULL);
        livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("%s %s;", livetune_generate_get_type_str(pBuilder, pParam->paramType), pParam->pName), NULL);
        livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = acAlgoGetControl(hAlgo, \"%s\", &%s);", pParam->pName, pParam->pName), NULL);
        livetune_generate_fn_body(pBuilder->pPipe, "assert(error == 0);\n", NULL);
      }
    }
    if (generateCB == NULL)
    {
      livetune_generate_fn_body(pBuilder->pPipe, "// your code here...", NULL);
      livetune_generate_fn_body(pBuilder->pPipe, "// see acAlgoGetControlPtr for optimization\n", NULL);
    }
    else
    {
      generateCB(pBuilder);
    }

    livetune_generate_fn_body(pBuilder->pPipe, "return 0;", NULL);
    livetune_generate_fn_declare(pBuilder->pPipe, "}\n\n", NULL);
  }
}


/**
* @brief   Generate the parameter comment
*/

static void livetune_generate_algo_common_config_comment(livetune_helper_builder *pBuilder, const char_t *pKey)
{
  const audio_algo_descriptor_t *pParam = livetune_ac_factory_get_algo_common_config_param_descriptor(pKey);
  if (pParam)
  {
    livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("// %s ", pParam->pDescription), NULL);
  }
}




/**
* @brief   Generate the parameter comment
*/

void livetune_generate_algo_set_config_value_comment(livetune_helper_builder *pBuilder, const char_t *pKey)
{
  const audio_descriptor_param_t *pParam = livetune_ac_factory_get_algo_config_param_descriptor(pBuilder, pKey);
  if (pParam)
  {
    livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("// %s ", pParam->pDescription), NULL);
  }
}



/**
* @brief   Generate the parameter comment
*/

void livetune_generate_chunk_set_config_value_comment(livetune_pipe *pPipe, const char_t *pKey)
{
  int32_t                         error = 0;
  const audio_descriptor_param_t *pParam;
  error = livetune_ac_factory_get_chunk_config_param_descriptor(pKey, &pParam);
  if (error == 0)
  {
    livetune_generate_fn_body(pPipe, livetune_helper_format("// %s ", pParam->pDescription), NULL);
  }
}




/**
* @brief   Generate the parameter config init
*/

void livetune_generate_algo_set_common_config_value(livetune_helper_builder *pBuilder, const char_t *pKey, const char_t *pValue)
{
  livetune_generate_algo_common_config_comment(pBuilder, pKey);
  livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = acAlgoSetCommonConfig(%s, \"%s\", %s);", pBuilder->pInstance->hAc.tVarName, pKey, pValue), NULL);
  livetune_generate_fn_body(pBuilder->pPipe, "assert(error == 0);", NULL);
}



/**
* @brief   Generate the parameter config init
*/

void livetune_generate_algo_set_config_value_ptr(livetune_helper_builder *pBuilder, const char_t *pKey, const char_t *pValue)
{
  livetune_generate_algo_set_config_value_comment(pBuilder, pKey);
  livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = acAlgoSetConfigPtr(%s,\"%s\",%s);", pBuilder->pInstance->hAc.tVarName, pKey, pValue), NULL);
  livetune_generate_fn_body(pBuilder->pPipe, "assert(error == 0);", NULL);
}




/**
* @brief   Generate the parameter config init
*/

void livetune_generate_algo_set_config_value(livetune_helper_builder *pBuilder, const char_t *pKey, const char_t *pValue)
{
  livetune_generate_algo_set_config_value_comment(pBuilder, pKey);
  livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = acAlgoSetConfig(%s,\"%s\",\"%s\");", pBuilder->pInstance->hAc.tVarName, pKey, pValue), NULL);
  livetune_generate_fn_body(pBuilder->pPipe, "assert(error == 0);", NULL);
}


/**
* @brief   Generate the parameter config init
*/

void livetune_generate_algo_set_config_nb(livetune_helper_builder *pBuilder, const audio_descriptor_params_t *pParamDescr, uint32_t *pControIndex)
{
  if (pParamDescr)
  {
    for (uint8_t paramID = 0; paramID < pParamDescr->nbParams; paramID++)
    {
      livetune_generate_conf genConf;
      genConf.pParamDesc = &pParamDescr->pParam[paramID];
      if ((genConf.pParamDesc->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE) == 0U)
      {
        genConf.pFunction = "acAlgoSetConfig";
        genConf.pValue    = LIVETUNE_STRING(pBuilder->pInstance, (*pControIndex));
        genConf.pVarName  = pBuilder->pInstance->hAc.tVarName;
        livetune_generate_config(pBuilder, &genConf, FALSE);
      }
      (*pControIndex)++;
    }
  }
}

/**
* @brief   Generate the parameter config init
*/

void livetune_generate_algo_config(livetune_helper_builder *pBuilder)
{
  uint32_t indexControl = 0;
  if (pBuilder->pFactory->pStaticParamTemplate)
  {
    livetune_generate_algo_set_config_nb(pBuilder, pBuilder->pFactory->pStaticParamTemplate, &indexControl);
  }
  if (pBuilder->pFactory->pDynamicParamTemplate)
  {
    livetune_generate_algo_set_config_nb(pBuilder, pBuilder->pFactory->pDynamicParamTemplate, &indexControl);
  }
}

/**
* @brief   Generate the parameter config init
*/
void livetune_generate_algo_create(livetune_helper_builder *pBuilder, char_t *pOptCbsName)
{
  /* C code to Configure the algo */
  livetune_pipe_log(pBuilder->pPipe, LIVETUNE_LOG_CODE, "\n");
  const char_t               *pAlgoName = livetune_factory_get_algo_name(pBuilder->pFactory);
  const char_t               *pDesc     = livetune_factory_get_algo_desc(pBuilder->pFactory);
  const audio_algo_factory_t *pFactory  = livetune_ac_factory_get_algo_factory_name(pAlgoName);
  if ((pFactory != NULL) && (pDesc != NULL))
  {
    livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("// %s", pDesc), NULL);
  }
  livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("// Create the instance %s", pBuilder->pInstance->pInstanceName), NULL);
  const char *pDescription = pBuilder->pInstance->pDescription;
  if (strcmp(pDescription, pBuilder->pInstance->pInstanceName) == 0)
  {
    pDescription = NULL;
  }

  char tScratchDescription[100];
  tScratchDescription[0] = '\0';
  if (strcmp(pBuilder->pInstance->pDescription, pBuilder->pInstance->pInstanceName) != 0)
  {
    snprintf(tScratchDescription, sizeof(tScratchDescription), "// Instance description : %s", pBuilder->pInstance->pDescription);
  }
  livetune_generate_fn_body(pBuilder->pPipe, livetune_helper_format("error = acAlgoCreate(hPipe, \"%s\", \"%s\", &%s, 0, \"%s\");", pAlgoName, pBuilder->pInstance->pInstanceName, pBuilder->pInstance->hAc.tVarName, pBuilder->pInstance->pDescription), tScratchDescription);
  livetune_generate_fn_body(pBuilder->pPipe, "assert(error == 0);", NULL);
}



static void trim_trailing_spaces_and_tabs(char *const pString)
{
  if (pString != NULL)
  {
    char *pStringLocal = pString;

    while (true)
    {
      char *pEol = strchr(pStringLocal, (int)'\n');

      if (pEol == NULL)
      {
        // there is no 'end of line' in the string => remove trailing spaces and tabs
        int eol = 0;

        for (int i = 0; pStringLocal[i] != '\0'; i++)
        {
          if ((pStringLocal[i] != ' ') && (pStringLocal[i] != '\t'))
          {
            eol = i + 1;
          }
        }
        pStringLocal[eol] = '\0';
        break;
      }
      else
      {
        // there is an 'end of line' in the string => remove spaces and tabs before this 'end of line'
        for (char *pChar = pEol - 1; pChar >= pStringLocal; pChar--)
        {
          if ((*pChar != ' ') && (*pChar != '\t'))
          {
            pStringLocal = pChar + 1; // position of first space or tab before 'end of line' => will be after copy position of new 'end of line'
            break;
          }
        }
        if (pStringLocal != pEol)
        {
          // there is at least 1 space or tab before 'end of line'
          memmove(pStringLocal, pEol, strlen(pEol) + 1UL);  // +1 for end of string
        }
        pStringLocal++; // position just after new 'end of line'
      }
    }
  }
}
