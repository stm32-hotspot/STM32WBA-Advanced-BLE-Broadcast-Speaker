/**
******************************************************************************
* @file          livetune_ac_wrapper.c
* @author        MCD Application Team
* @brief         implements a conditional call to the acSDK
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
#include "st_os_monitor_cpu.h"
#include "audio_descriptor.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* Log ac SDK sequence */
#define ST_TRACE_AC_SDK(...) // ST_TRACE_INFO(__VA_ARGS__)
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
#include "audio_chain.h"



/**
* @brief  callback delete function called when the post message is dispatched, we have to free the string context
*
* @param pPost post message handle
*/

static void livetune_ac_wrapper_msg_free(st_message_post_item *pPost)
{
  char_t *pCopy = (char_t *)(uint32_t)pPost->lparam2;
  st_os_mem_update_string(&pCopy, NULL);
}



static void livetune_ac_wrapper_msg_cb(uint32_t const evt, const char *const pMsg)
{
  char_t *pCopy = NULL;
  st_os_mem_update_string(&pCopy, pMsg);
  /* dispatch in the message  Queue */
  livetune_post_delete_cb(evt, 0, (st_message_param)ST_MAKE_INT64(0UL, pCopy), livetune_ac_wrapper_msg_free);
}

/**
* @brief  callback fired when an issue accours during the pipe start
*
* @param pMsg issue message
*/

static void livetune_ac_wrapper_start_issue_msg_cb(const char *const pMsg)
{
  livetune_ac_wrapper_msg_cb((uint32_t)ST_EVT_START_ISSUE_MSG, pMsg);
}


/**
* @brief  callback fired when algo's param update needs an update on livetune side
*
* @param pMsg param update message
*/

static void livetune_ac_wrapper_update_config_msg_cb(const char *const pMsg)
{
  livetune_ac_wrapper_msg_cb((uint32_t)ST_EVT_UPDATE_CONFIG_MSG, pMsg);
}


/**
* @brief cleanup pipe instance
*
* @param pPipe the pipe instance handle
*/

static int32_t livetune_ac_wrapper_cleanup_pipe(livetune_pipe *pPipe)
{
  int32_t result = 0;
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acPipeDelete(%x);", pPipe->hPipe);
    result = acPipeDelete(pPipe->hPipe);

    if (result != 0)
    {
      ST_TRACE_ERROR("acPipeDelete(%x)", pPipe->hPipe);
    }
    result = acTerminate();
    ST_TRACE_AC_SDK("acTerminate()");
    if (result != 0)
    {
      ST_TRACE_ERROR("acTerminate()", pPipe->hPipe);
    }
  }
  return result;
}



/**
* @brief create pipe instance
*
* @param pHandle the pipe instance handle
*/

void livetune_ac_wrapper_acPipeCreate(livetune_pipe *pPipe)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    /* init the build pipe error count, if non null, the pipe won't start */
    pPipe->buildErrorCount = 0;

    ST_TRACE_AC_SDK("acInitialize()");
    acInitialize();

    ST_VERIFY(acEnvSetConfig("bDefaultCyclesMgntCb", AC_TRUE) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"bDefaultCyclesMgntCb\", AC_TRUE)");
    ST_VERIFY(acEnvSetConfig("cyclesMgntCbTimeout", 5000UL) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"cyclesMgntCbTimeout\", 5000UL)");
    ST_VERIFY(acEnvSetConfig("cyclesMgntMeasureTimeout", ST_OS_MONITOR_CALCULATION_PERIOD_MS) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"cyclesMgntMeasureTimeout\", %d)", ST_OS_MONITOR_CALCULATION_PERIOD_MS);

    #ifdef ST_AC_DEBUG_INSTRUMENTATION
    /* enable ac real time verbose flags */
    ST_VERIFY(acEnvSetConfig("bLogInit", (((livetune_state_get_flags() & ST_STATE_FLG_LOG_VERBOSE_INIT) != 0UL) ? AC_TRUE : AC_FALSE)) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"%s\", %d)", "bLogInit", (((livetune_state_get_flags() & ST_STATE_FLG_LOG_VERBOSE_INIT) != 0UL) ? AC_TRUE : AC_FALSE));

    ST_VERIFY(acEnvSetConfig("bLogMalloc", (((livetune_state_get_flags() & ST_STATE_FLG_LOG_VERBOSE_MALLOCS) != 0UL) ? AC_TRUE : AC_FALSE)) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"%s\", %d)", "bLogMalloc", (((livetune_state_get_flags() & ST_STATE_FLG_LOG_VERBOSE_MALLOCS) != 0UL) ? AC_TRUE : AC_FALSE));

    ST_VERIFY(acEnvSetConfig("bLogCmsisOs", (((livetune_state_get_flags() & ST_STATE_FLG_LOG_VERBOSE_OS) != 0UL) ? AC_TRUE : AC_FALSE)) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"%s\", %d)", "bLogCmsisOs", (((livetune_state_get_flags() & ST_STATE_FLG_LOG_VERBOSE_OS) != 0UL) ? AC_TRUE : AC_FALSE));

    ST_VERIFY(acEnvSetConfig("bLogCycles", (((livetune_state_get_flags() & ST_STATE_FLG_LOG_VERBOSE_CYCLES) != 0UL) ? AC_TRUE : AC_FALSE)) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"%s\", %d)", "bLogCycles", (((livetune_state_get_flags() & ST_STATE_FLG_LOG_VERBOSE_CYCLES) != 0UL) ? AC_TRUE : AC_FALSE));

    ST_VERIFY(acEnvSetConfig("setTrace", "debug") == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"setTrace\", \"debug\");");
    #endif

    ST_VERIFY(acEnvSetConfig("chunkMemPool", livetune_state_get_chunk_pool()) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"chunkMemPool\", %d);", livetune_state_get_chunk_pool());

    ST_VERIFY(acEnvSetConfig("algoMemPool", livetune_state_get_algo_pool()) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"algoMemPool\", %d);", livetune_state_get_algo_pool());

    ST_VERIFY(acEnvSetConfig("initIssueMsgCb", livetune_ac_wrapper_start_issue_msg_cb) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"initIssueMsgCb\", %p);", livetune_ac_wrapper_start_issue_msg_cb);

    ST_VERIFY(acEnvSetConfig("updateCfgMsgCb", livetune_ac_wrapper_update_config_msg_cb) == 0);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"updateCfgMsgCb\", %p);", livetune_ac_wrapper_update_config_msg_cb);

    /* tuning is mandatory for the designer */
    #ifdef AUDIO_CHAIN_ACSDK_USED
    acEnvSetConfig("bTuning", AC_TRUE);
    ST_TRACE_AC_SDK("acEnvSetConfig(\"bTuning\",AC_TRUE);");
    #else
    ST_TRACE_ERROR("The designer needs the define AUDIO_CHAIN_ACSDK_USED");
    #endif


    ST_TRACE_AC_SDK("acPipeCreate(%x);");
    if (acPipeCreate(&pPipe->hPipe) != 0)
    {
      ST_TRACE_ERROR("Create pipe error");
      pPipe->buildErrorCount++;
    }
  }
}


/**
* @brief create chunk configs
*
* @param pHandle the pipe instance handle
*/
static void livetune_ac_wrapper_create_config_chunk(livetune_helper_builder *pBuilder, livetune_db_instance_cnx *pCnx)
{
  const audio_descriptor_params_t *pDescriptor = livetune_ac_factory_get_chunk_descriptor();
  ST_ASSERT(pDescriptor != NULL);
  for (int32_t indexParam = 0; indexParam < (int32_t)pDescriptor->nbParams; indexParam++)
  {
    audio_descriptor_param_t *pParam = &pDescriptor->pParam[indexParam];
    void                     *pData  = (uint8_t *)&pCnx->hAc.hConf + pParam->iOffset;
    char_t                    tValue[20];
    livetune_ac_factory_get_string_value(pParam, pData, tValue, sizeof(tValue));
    if ((pParam->iParamFlag & (uint32_t)AUDIO_DESC_PARAM_TYPE_FLAG_DISABLED) == 0U)
    {
      /* check if the new parameter is the default value, in this case no need to generate a code */

      if (livetune_ac_factory_is_default_value_string(pParam, tValue) == FALSE)
      {
        char_t *pTranslated = AudioChainJson_factory_translate_key_value(pParam, tValue);
        ST_TRACE_AC_SDK("acChunkSetConfig(%x,\"%s\",\"%s\");", pCnx->hAc.hChunk, pParam->pName, pTranslated);
        int32_t error = acChunkSetConfig(pCnx->hAc.hChunk, pParam->pName, pTranslated);
        if (error != 0)
        {
          pBuilder->pPipe->buildErrorCount++;
          ST_TRACE_ERROR("acChunkSetConfig reports an error %s %s,%s", pCnx->hAc.tVarName, pParam->pName, pTranslated);
        }
        st_os_mem_free(pTranslated);
      }
    }
  }
}




/**
* @brief create chunk instance
*
* @param pHandle the pipe instance handle
* @param pPinDef the pindefinstance handle
* @param pinDef   the out pindef
*/
void livetune_ac_wrapper_acChunkCreate(livetune_helper_builder *pBuilder, livetune_db_instance_pins_def **pPinDef, uint32_t pinDef)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    uint32_t nbCnx = livetune_helper_cnx_nb(pPinDef, (uint32_t)pinDef);
    for (uint32_t index = 0; index < nbCnx; index++)
    {
      livetune_db_instance_cnx *pCnx = livetune_helper_cnx_get(pPinDef, (uint32_t)pinDef, index);
      ST_ASSERT(pCnx != NULL);
      if ((pCnx->hAc.bValid != 0U) && (pCnx->pInstance->hAc.bValid != 0U))
      {
        /* the chunk is not an IO, create the chunk in the custom area */
        pCnx->hAc.bValid   = TRUE;
        char_t *pChunkName = pCnx->hAc.tVarName;
        if ((pCnx->hAc.pSysIoName != NULL) && (pBuilder->pInstance->hAc.bSysIO != 0U))
        {
          pChunkName = pCnx->hAc.pSysIoName;
        }
        int32_t error = acChunkCreate(pBuilder->pPipe->hPipe, pChunkName, &pCnx->hAc.hChunk);
        ST_TRACE_AC_SDK("acChunkCreate(%x,\"%s\",%x);", pBuilder->pPipe->hPipe, pChunkName, pCnx->hAc.hChunk);
        if (error != 0)
        {
          pCnx->hAc.bValid = FALSE;
          ST_TRACE_ERROR("acChunkCreate reports an error on the instance %s", pCnx->pName);
          pBuilder->pPipe->buildErrorCount++;
        }
        else
        {
          /* if sysio, we cannot change the chunk config */
          if ((pCnx->hAc.bValid != 0U) && (pBuilder->pInstance->hAc.bSysIO == 0U))
          {
            livetune_ac_wrapper_create_config_chunk(pBuilder, pCnx);
          }
        }
      }
    }
  }
}

/**
* @brief  set an environmental value
* @param  pKey:   key string
* @param  value:   value
* @retval None
*/
void livetune_ac_wrapper_acEnvSetConfig(const char_t *pKey, uint32_t value)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acEnvSetConfig(%s,%x);", pKey, value);
    int32_t error = acEnvSetConfig(pKey, value);
    if (error != 0)
    {
      ST_TRACE_ERROR("acEnvSetConfig");
    }
  }
}


/**
* @brief  inserts a new chunk at the end of the list of input chunks
* @param  pAlgo:   pointer to algo
* @param  pChunk: pointer to the chunk to be added
* @retval None
*/
void livetune_ac_wrapper_acPipeConnectPinIn(livetune_helper_builder *pBuilder, uint32_t pinID, acChunk hChunk)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acPipeConnectPinIn(%x,%x,%x,%x);", pBuilder->pPipe->hPipe, pBuilder->pInstance->hAc.hAlgo, pinID, hChunk);
    int32_t error = acPipeConnectPinIn(pBuilder->pPipe->hPipe, pBuilder->pInstance->hAc.hAlgo, pinID, hChunk);
    if (error != 0)
    {
      ST_TRACE_ERROR("acPipeConnectPinIn reports an error on the algo %s", pBuilder->pInstance->hAc.tVarName);
      pBuilder->pPipe->buildErrorCount++;
    }
  }
}


/**
* @brief  Connect algos
* @param  pAlgoSrc:   pointer to algo src
* @param  pChunk: pointer to the chunk to be added
* @param  pAlgoDst:   pointer to algo dst
* @retval None
*/
void livetune_ac_wrapper_acPipeConnect(livetune_helper_builder *pBuilder, uint32_t srcPinID, acChunk hChunk, acAlgo hAlgoDst, uint32_t dstPinID)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acPipeConnect(%x,%x,%x,%x,%x,%x);", pBuilder->pPipe->hPipe, pBuilder->pInstance->hAc.hAlgo, srcPinID, hChunk, hAlgoDst, dstPinID);

    int32_t error = acPipeConnect(pBuilder->pPipe->hPipe, pBuilder->pInstance->hAc.hAlgo, srcPinID, hChunk, hAlgoDst, dstPinID);
    if (error != 0)
    {
      ST_TRACE_ERROR("acPipeConnect reports an error on the algo %s", pBuilder->pInstance->hAc.tVarName);
      pBuilder->pPipe->buildErrorCount++;
    }
  }
}




/**
* @brief  inserts a new chunk at the end of the list of output chunks
* @param  headRef:   pointer to algo
* @param  pChunk: pointer to the chunk to be added
* @retval None
*/
void livetune_ac_wrapper_acPipeConnectPinOut(livetune_helper_builder *pBuilder, uint32_t pinID, acChunk hChunk)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acPipeConnectPinOut(%x,%x,%x,%x);", pBuilder->pPipe->hPipe, pBuilder->pInstance->hAc.hAlgo, pinID, hChunk);
    int32_t error = acPipeConnectPinOut(pBuilder->pPipe->hPipe, pBuilder->pInstance->hAc.hAlgo, pinID, hChunk);
    if (error != 0)
    {
      pBuilder->pPipe->buildErrorCount++;
      ST_TRACE_ERROR("acPipeConnectPinOut reports an error on the algo %s", pBuilder->pInstance->hAc.tVarName);
    }
  }
}



/**
* @brief  create Algo config
* @param  pBuilder:            instance handle
* @retval None
*/
void livetune_ac_wrapper_acAlgoCreate(livetune_helper_builder *pBuilder)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acAlgoCreate(%x,\"%s\",\"%s\",%x,%s,\"%s\");", pBuilder->pPipe->hPipe, pBuilder->pBuilderDef->pAlgoName, pBuilder->pInstance->pInstanceName, &pBuilder->pInstance->hAc.hAlgo, "",  pBuilder->pInstance->pDescription);

    int32_t error = acAlgoCreate(pBuilder->pPipe->hPipe, pBuilder->pBuilderDef->pAlgoName, pBuilder->pInstance->pInstanceName, &pBuilder->pInstance->hAc.hAlgo, 0, pBuilder->pInstance->pDescription);
    if (error != 0)
    {
      pBuilder->pPipe->buildErrorCount++;
      ST_TRACE_ERROR("acAlgoCreate reports an error on the algo %s", pBuilder->pInstance->hAc.tVarName);
      error = acAlgoCreate(pBuilder->pPipe->hPipe, pBuilder->pBuilderDef->pAlgoName, pBuilder->pInstance->pInstanceName, &pBuilder->pInstance->hAc.hAlgo, 0, pBuilder->pInstance->pDescription);
    }
  }
}

/**
* @brief  Set Algo common config
* @param  pBuilder:            instance handle
* @param  pKey:                Key string
* @param  pOpaque:             Value pointer
* @retval None
*/

void livetune_ac_wrapper_acAlgoSetCommonConfig(livetune_helper_builder *pBuilder, const char_t *pKey, void *pOpaque)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acAlgoSetCommonConfig(%x,\"%s\",%x);", pBuilder->pInstance->hAc.hAlgo, pKey, pOpaque);

    int32_t error = acAlgoSetCommonConfig(pBuilder->pInstance->hAc.hAlgo, pKey, pOpaque);
    if (error != 0)
    {
      pBuilder->pPipe->buildErrorCount++;
      ST_TRACE_ERROR("acAlgoSetCommonConfig reports an error on the algo %s", pBuilder->pInstance->hAc.tVarName);
    }
  }
}

/**
* @brief  return true if the graph is playing
* @retval Error; AUDIO_ERR_MGNT_NONE if no issue
*/


int32_t livetune_ac_wrapper_acPipeIsPlaying(livetune_pipe *pPipe)
{
  int32_t bState = FALSE;
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acPipeIsPlaying(%x);", pPipe->hPipe);
    if (acPipeIsPlaying(pPipe->hPipe) == 0)
    {
      bState = TRUE;
    }
  }
  return bState;
}



/**
* @brief  Initialize an audio chain.
* @retval Error; AUDIO_ERR_MGNT_NONE if no issue
*/
int32_t livetune_ac_wrapper_acPipePlay_start(livetune_pipe *pPipe)
{
  int32_t result = 0;
  if (ST_HELPER_GET_RT_STATE())
  {
    if (pPipe->buildErrorCount == 0U)
    {
      ST_TRACE_AC_SDK("acPipePlay(%x,%s);", pPipe->hPipe, "AC_START");
      result = acPipePlay(pPipe->hPipe, AC_START);
      if (result != 0)
      {
        /* an acPipePlay is not really an error to signal, it could be a bad graph formation   */
        ST_TRACE_WARNING("acPipePlay(AC_PLAY)");
        livetune_ac_wrapper_cleanup_pipe(pPipe);
      }
    }
    else
    {
      ST_TRACE_AC_SDK("acPipePlay(%x,%s);", pPipe->hPipe, "AC_CLEANUP");
      ST_TRACE_ERROR("The pipe construction reports %d errors, Check the log", pPipe->buildErrorCount);
      result = acPipePlay(pPipe->hPipe, AC_CLEANUP);
      if (result != 0)
      {
        /* error will be signaled in the log, it is not an implementation error but graph error  */
        ST_TRACE_ERROR("acPipePlay(AC_CLEANUP)");
      }
      livetune_ac_wrapper_cleanup_pipe(pPipe);
      ST_TRACE_ERROR("The start fails due to some SetConfig(...)  errors");
      result = TRUE;
    }
  }
  return result;
}


/**
* @brief  Deinitialize an audio chain.
* @retval Error; AUDIO_ERR_MGNT_NONE if no issue
*/
int32_t livetune_ac_wrapper_acPipePlay_stop(livetune_pipe *pPipe)
{
  int32_t result = 0;
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acPipePlay(%x,%s);", pPipe->hPipe, "AC_STOP");

    result = acPipePlay(pPipe->hPipe, AC_STOP);
    if (result != 0)
    {
      ST_TRACE_ERROR("acPipePlay(AC_STOP)");
    }
    livetune_ac_wrapper_cleanup_pipe(pPipe);
  }
  return result;
}



/**
* @brief Set an audio chain param from its name using string value
*
*/
void livetune_ac_wrapper_acAlgoSetConfig(acAlgo hAlgo, const char_t *pName, const char_t *pValue, int8_t bForceUpdate)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    const audio_descriptor_param_t *pParam = livetune_ac_factory_get_algo_param(hAlgo, pName);
    if ((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE) == 0U) /* private cannot be set */
    {
      ST_TRACE_AC_SDK("acAlgoSetConfig(%x,\"%s\",\"%s\")", hAlgo, pName, pValue);
      if (acAlgoSetConfig(hAlgo, pName, pValue) == 0)
      {
        if (pParam)
        {
          if ((pParam != NULL) && ((pParam->iParamFlag & (uint32_t)AUDIO_DESC_PARAM_TYPE_FLAG_STOP_GRAPH) != 0UL))
          {
            livetune_pipe *pPipe = &livetune_get_instance()->hAudioPipe;
            if (livetune_pipe_is_started(pPipe) == TRUE)
            {
              /* We cannot stop the pipe immdiatly, bec ause we could be in pipe construction, this cause a crash, so we post a message with a low priority in order to get the stop after the pipe construction */
              livetune_post(ST_EVT_PIPE_STOP_ERROR, 0, 0);
            }
          }
        }
        else
        {
          ST_TRACE_ERROR("Param not found");
        }
        if (bForceUpdate)
        {
          if (acAlgoRequestUpdate(hAlgo) != 0)
          {
            ST_TRACE_ERROR("acAlgoRequestUpdate");
          }
        }
      }
      else
      {
        ST_TRACE_ERROR("acAlgoSetConfig %s,%s", pName, pValue);
        livetune_get_instance()->hAudioPipe.buildErrorCount++;
      }
    }
  }
}




/**
* @brief Set an audio chain param from its name using ptr value
*
*/
void livetune_ac_wrapper_acAlgoSetConfig_ptr(acAlgo hAlgo, const char_t *pName, const void *pValue, int8_t bForceUpdate)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acAlgoSetConfigPtr(%x,\"%s\",%x)", hAlgo, pName, pValue);
    if (acAlgoSetConfigPtr(hAlgo, pName, pValue) == 0)
    {
      const audio_descriptor_param_t *pParam = livetune_ac_factory_get_algo_param(hAlgo, pName);
      if (pParam)
      {
        if ((pParam != NULL) && ((pParam->iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_STOP_GRAPH) != 0U))
        {
          livetune_pipe *pPipe = &livetune_get_instance()->hAudioPipe;
          if (livetune_pipe_is_started(pPipe) == TRUE)
          {
            /* We cannot stop the pipe immdiatly, bec ause we could be in pipe construction, this cause a crash, so we post a message with a low priority in order to get the stop after the pipe construction */
            livetune_post(ST_EVT_PIPE_STOP_ERROR, 0, 0);
          }

        }
      }
      else
      {
        ST_TRACE_ERROR("Param not found");
        livetune_get_instance()->hAudioPipe.buildErrorCount++;
      }
      if (bForceUpdate)
      {
        if (acAlgoRequestUpdate(hAlgo) != 0)
        {
          ST_TRACE_ERROR("acAlgoRequestUpdate");
        }
      }
    }
    else
    {
      livetune_get_instance()->hAudioPipe.buildErrorCount++;
      ST_TRACE_ERROR("acAlgoSetConfig");
    }
  }
}



/**
* @brief Dumps algo cycle counts
*
*/
void livetune_ac_wrapper_acCyclesDump(void)
{
  if (ST_HELPER_GET_RT_STATE())
  {
    ST_TRACE_AC_SDK("acCyclesDump(true, false, NULL, NULL)");
    acCyclesDump(true, false, NULL, NULL);  // display on terminal and compute interrupts and tasks cpu load
  }
}

/**
* @brief Check running pipe error
*
*/

void    livetune_ac_check_system_error(uint32_t level)
{
  livetune_pipe *pPipe = &livetune_get_instance()->hAudioPipe;

  if (livetune_get_instance() != NULL)
  {
    if (livetune_get_instance()->bSystemReady)
    {
      if ((level & (TRACE_LVL_ERROR | TRACE_LVL_ERROR_FATAL)) != 0U)
      {
        if (livetune_pipe_is_started(pPipe) == TRUE)
        {
          livetune_post(ST_EVT_PIPE_STOP_ERROR, (st_message_param)ST_MAKE_INT64(0UL, level), 0U);
        }
      }
    }
  }
}




