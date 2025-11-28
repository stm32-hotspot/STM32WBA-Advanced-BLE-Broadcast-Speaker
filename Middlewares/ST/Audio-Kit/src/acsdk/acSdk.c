/**
******************************************************************************
* @file    acSdk.c
* @author  MCD Application Team
* @brief   Audio Chain User SDK
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

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "st_base.h"
#include "audio_chain_instance.h"
#include "audio_chain_factory.h"
#include "acSdk.h"
//#include "st_os_mem.h"
#include "cycles_measure.h"
#include "algos_memory_usage.h"
#include "audio_persist_config.h"
#include "stm32xxx_voice_bsp.h"


/* Private defines -----------------------------------------------------------*/
#define AC_DEFAULT_CHUNK_MEM_POOL AUDIO_MEM_TCM     // chunk's handler and buffer
#define AC_DEFAULT_ALGO_MEM_POOL  AUDIO_MEM_RAMINT  // algo's handler and config structures

/* Private typedef -----------------------------------------------------------*/

typedef struct acGlobal_t
{
  uint8_t        initRef;
  uint8_t        pipeRef;
  audio_chain_t *pAcHdle;
} acGlobal_t;


/* Private function prototypes -----------------------------------------------*/
static bool    s_acCheckPipeHandle(acPipe hPipe);
static bool    s_acCheckChunkHandle(acChunk hChunk);
static bool    s_acCheckAlgoHandle(acAlgo hAlgo);
static int32_t s_acSetHotConfig(acAlgo hAlgo, const char *pKey, const char *pValue, char **ppErrorString);
static int32_t s_acSetColdConfig(acAlgo hAlgo, const char *pKey, const char *pValue, char **ppErrorString);


/* Global variables ----------------------------------------------------------*/

static acGlobal_t hAcInstance; /* global SDK instance */


/* Functions Definition ------------------------------------------------------*/

/**
* @brief  Init the system as the standard APIs, for the time being we do nothing but probably useful in the future
*         Documented in the header
*
*/

int32_t acInitialize(void)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (hAcInstance.initRef == 0U)
  {
    hAcInstance.initRef = 1U;
    error               = AUDIO_ERR_MGNT_NONE;
  }
  return error;
}



/**
*
* @brief return the current SDK version
*
**/


const char *acSdkVersion(void)
{
  return AC_VERSION;
}



/**
*
* @brief return the current SDK version
*
**/


const char *acConfigBoard(void)
{
  return STM32_NAME_STRING;
}


/**
*
* @brief set the audio config from its name
*
**/


int32_t acAudioConfigSet(const char *pConfigName)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  int32_t indexAudio = acGetPersistConfigIndexFromString(pConfigName);
  if (indexAudio  != -1)
  {
    acSetPersistSetConfigIndex(indexAudio);
    error = AUDIO_ERR_MGNT_NONE;
  }

  return error;
}

/**
* @brief  hook to allow acSDK to manage audio persistent configuration thru acAudioConfigSet
*         Documented in the header
*
*/
void acSetPersistSetConfigIndex(int32_t index)
{
  audio_persist_set_config_index(index);
}


/**
* @brief  hook to allow acSDK to manage audio persistent configuration thru acAudioConfigSet
*         Documented in the header
*
*/
int32_t acGetPersistConfigIndexFromString(const char *pId)
{
  return audio_persist_get_config_index_from_string(pId);
}


/**
* @brief  Terminate the system as the standard APIs, for the time being we do nothing but probably useful in the future
*         Documented in the header
*
*/

int32_t acTerminate(void)
{
  hAcInstance.initRef = 0;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Create pipe as the standard APIs, for the time being ac supports only one pipe, so a second call must fails
*         Documented in the header
*
*/

int32_t acPipeCreate(acPipe *ppPipe)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if ((hAcInstance.pipeRef == 0U) && (ppPipe != NULL))
  {
    acPipe   hPipe       = &AudioChainInstance;
    uint32_t bLogInit    = 0UL;
    uint32_t bLogCmsisOs = 0UL;

    *ppPipe             = hPipe;
    hAcInstance.pAcHdle = hPipe;
    hAcInstance.pipeRef = 1;

    error = acEnvGetConfig("bLogInit", &bLogInit);
    if (acErrorIsOk(error))
    {
      error = acEnvGetConfig("bLogCmsisOs", &bLogCmsisOs);
    }
    if (acErrorIsOk(error))
    {
      AudioChain_setLogInit(hPipe, (bLogInit    != 0UL));
      AudioChain_setLogCmsisOs(hPipe, (bLogCmsisOs != 0UL));
    }
  }
  return error;
}


/**
* @brief  delete  the pipe as the standard APIs, for the time being we do nothing but probably useful in the future
*         Documented in the header
*
*/
int32_t acPipeDelete(acPipe hPipe)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckPipeHandle(hPipe))
  {
    hAcInstance.pipeRef = 0;
    error               = AUDIO_ERR_MGNT_NONE;
  }
  return error;
}


/**
* @brief  Dump all algo available in the pipe system
*         Documented in the header
*
*/
int32_t acAlgoListDump(void)
{
  return AudioChain_dumpFactoryEntry(hAcInstance.pAcHdle);
}


/**
* @brief  Dump all algo parameters from its name
*         Documented in the header
*
*/
int32_t acAlgoTemplateDump(char *pName)
{
  return AudioChain_dumpAlgoDescriptor(hAcInstance.pAcHdle, pName);
}


/**
* @brief  Reset default algo parameters
*         Documented in the header
*
*/
int32_t acResetDefault(acAlgo hAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_NOT_FOUND;
  if (s_acCheckAlgoHandle(hAlgo))
  {
    error = AudioAlgo_resetConfig(hAlgo);
  }
  return error;
}

/**
* @brief  create an algo
*         Documented in the header
*/


int32_t acAlgoCreate(acPipe hPipe, const char *pAlgoName, const char *pInstanceName, acAlgo *ppAlgo, uint32_t iAlgoFlags, const char *pDesc)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if (s_acCheckPipeHandle(hPipe))
  {
    audio_algo_t      *pAlgo                    = NULL;
    audio_algo_state_t algoState                = AUDIO_ALGO_STATE_ENABLED;
    CycleStatsCb_t    *pCycleCount              = NULL;
    uint32_t           cyclesMgntMeasureTimeout = 1000UL;
    uint32_t           cyclesMgntCbTimeout      = 1000UL;
    int32_t            defaultCycleCountCB      = FALSE;
    int32_t            bLogCycles               = FALSE;
    int32_t            algoMemPool              = (int32_t)AC_DEFAULT_ALGO_MEM_POOL;

    /* Disable the algo */
    if (iAlgoFlags & AUDIO_CHAIN_DISABLED)
    {
      algoState = AUDIO_ALGO_STATE_DISABLED;
    }

    error = acEnvGetConfig("bLogCycles", &bLogCycles);
    if (acErrorIsOk(error))
    {
      if (bLogCycles)
      {
        /* if bLogCycles, check if the default cycleCount if true, else in any case don't use defaultCycleCountCB */
        error = acEnvGetConfig("bDefaultCyclesMgntCb", &defaultCycleCountCB);
      }
    }

    if (acErrorIsOk(error))
    {
      error = acEnvGetConfig("cyclesMgntMeasureTimeout", &cyclesMgntMeasureTimeout);
    }

    if (acErrorIsOk(error))
    {
      error = acEnvGetConfig("cyclesMgntCbTimeout", &cyclesMgntCbTimeout);
    }

    if (acErrorIsOk(error))
    {
      error = acEnvGetConfig("algoMemPool", &algoMemPool);
    }

    if (acErrorIsOk(error))
    {
      if (defaultCycleCountCB != FALSE)
      {
        pCycleCount = cycleMeasure_displayCpuLoadOnUart;
      }

      /* add the default cycle count callback */
      if (iAlgoFlags & AUDIO_CHAIN_DEFAULT_CYCLECOUNT)
      {
        pCycleCount = cycleMeasure_displayCpuLoadOnUart;
      }
      error = AudioAlgo_create((audio_algo_t **)ppAlgo,
                               pAlgoName,
                               algoState,
                               (pDesc == NULL) ? pInstanceName : pDesc,
                               cyclesMgntMeasureTimeout,
                               cyclesMgntCbTimeout,
                               pCycleCount,
                               (memPool_t)algoMemPool);
    }
    if (acErrorIsOk(error))
    {
      pAlgo = *ppAlgo;
      error = AudioAlgo_setUtilsHdle(pAlgo, AudioChain_getUtilsHdle(hPipe));
    }
    if (acErrorIsOk(error))
    {
      /*
      configure the algo with default parameters
      some other parameters may be modified using acSetAlgoxxxx or acSetCommonxxxx
      */

      error = AudioChain_registerAlgos(hPipe, pAlgo, NULL);
    }
  }
  else
  {
    error = AUDIO_ERR_MGNT_ERROR;
  }
  return error;
}


/**
* @brief  Get an algo instance from its name
*         Documented in the header
*/


acAlgo acAlgoGetInstance(acPipe hPipe, const char *pInstanceName)
{
  acAlgo pInstance = NULL;
  if (s_acCheckPipeHandle(hPipe))
  {
    if (acPipeIsPlaying(hPipe) == 0)
    {
      if (AudioChain_getAlgosList(&AudioChainInstance) !=  NULL)
      {
        for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); pCurrent != NULL; pCurrent = pCurrent->next)
        {
          audio_algo_t *pAlgo = pCurrent->pAlgo;

          if (strcmp(pInstanceName, AudioAlgo_getInstanceName(pAlgo)) == 0)
          {
            pInstance = pAlgo;
            break;
          }
        }
      }
    }
  }
  return pInstance;
}



/**
* @brief  Delete an algo
*         Documented in the header
*/
int32_t acAlgoDelete(acAlgo hAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckAlgoHandle(hAlgo))
  {
    error = AUDIO_ERR_MGNT_NONE;
  }
  return error;
}


/**
* @brief  Set an algo parameter and check if the system is started or not
*         Documented in the header
*/

int32_t acAlgoSetConfig(acAlgo hAlgo, const char *pKey, const char *pValue)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckAlgoHandle(hAlgo) && (pKey != NULL) && (pValue != NULL))
  {
    char *pErrorString = NULL;
    if (AudioChain_isStarted(hAcInstance.pAcHdle))
    {
      /* the graph is started, we can must use tuning api */
      error = s_acSetHotConfig(hAlgo, pKey, pValue, &pErrorString);
    }
    else
    {
      /* the graph not started we can modify directly the config*/
      error = s_acSetColdConfig(hAlgo, pKey, pValue, &pErrorString);
    }

    if (acErrorIsWarning(error))
    {
      error = AUDIO_ERR_MGNT_NONE; // ignore warnings (especially parameters clamped)
    }
    if (error == AUDIO_ERR_MGNT_NOT_FOUND)
    {
      AudioAlgo_trace(hAlgo, TRACE_LVL_ERROR, NULL, 0, "the parameter %s is not found, please check the algo documentation or use AudioAlgo_dumpAlgoTemplate(...) to print parameters descriptions", pKey);
    }
    else if (acErrorIsError(error))
    {
      if (pErrorString != NULL)
      {
        AudioAlgo_trace(hAlgo, TRACE_LVL_ERROR, NULL, 0, "error %d (%s) for parameter %s", error, pErrorString, pKey);
      }
      else
      {
        AudioAlgo_trace(hAlgo, TRACE_LVL_ERROR, NULL, 0, "error %d for parameter %s", error, pKey);
      }
    }
  }
  return error;
}



/**
* @brief  Get an algo parameter
*         Documented in the header
*/

int32_t acAlgoGetConfig(acAlgo hAlgo, const char *pKey, char *pValue, uint32_t szValue)
{
  int32_t error = AUDIO_ERR_MGNT_NOT_FOUND;
  const audio_descriptor_param_t *pParamDesc;
  if (s_acCheckAlgoHandle(hAlgo) && (pKey != NULL) && (pValue != NULL))
  {
    const audio_algo_factory_t *const pFactory = AudioAlgo_getFactory(hAlgo);
    if (pFactory)
    {
      if (pFactory->pDynamicParamTemplate)
      {
        error = AudioDescriptor_getParam(pFactory->pDynamicParamTemplate, pKey, &pParamDesc, NULL);
        if (AudioError_isOk(error))
        {
          if (pParamDesc)
          {
            AudioDescriptor_getParamValueString(pParamDesc, AudioAlgo_getDynamicConfig(hAlgo), pValue, szValue);
          }
        }
      }
      if ((error == AUDIO_ERR_MGNT_NOT_FOUND) && (pFactory->pStaticParamTemplate != NULL))
      {
        error = AudioDescriptor_getParam(pFactory->pStaticParamTemplate, pKey, &pParamDesc, NULL);
        if (AudioError_isOk(error))
        {
          if (pParamDesc)
          {
            AudioDescriptor_getParamValueString(pParamDesc, AudioAlgo_getStaticConfig(hAlgo), pValue, szValue);
          }
        }
      }
    }
  }
  return error;
}





/**
* @brief  Set an algo parameter and check if the system is started or not
*         Documented in the header
*/

int32_t acAlgoSetConfigPtr(acAlgo hAlgo, const char *pKey, const void *pValue)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckAlgoHandle(hAlgo) && (pKey != NULL))
  {
    if (!AudioChain_isStarted(hAcInstance.pAcHdle))
    {
      /* the graph not started we can modify directly the config*/
      char *pErrorString = NULL;
      error = AudioAlgo_setColdConfigPtr(hAlgo, pKey, pValue, &pErrorString);
      if (acErrorIsWarning(error))
      {
        error = AUDIO_ERR_MGNT_NONE; // ignore warnings (especially parameters clamped)
      }
      if (error == AUDIO_ERR_MGNT_NOT_FOUND)
      {
        AudioAlgo_trace(hAlgo, TRACE_LVL_ERROR, NULL, 0, "the parameter %s is not found, please check the algo documentation or use AudioAlgo_dumpAlgoTemplate(...) to print parameters descriptions", pKey);
      }
      else if (acErrorIsError(error))
      {
        if (pErrorString != NULL)
        {
          AudioAlgo_trace(hAlgo, TRACE_LVL_ERROR, NULL, 0, "error %d (%s) for parameter %s", error, pErrorString, pKey);
        }
        else
        {
          AudioAlgo_trace(hAlgo, TRACE_LVL_ERROR, NULL, 0, "error %d for parameter %s", error, pKey);
        }
      }
    }
  }
  return error;
}



/**
* @brief  Dump the algo properties
*         Documented in the header
*/

int32_t acAlgoDump(acAlgo hAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckAlgoHandle(hAlgo))
  {
    const audio_algo_factory_t *const pFactory = AudioAlgo_getFactory(hAlgo);
    if (pFactory)
    {
      AudioChain_trace(hAcInstance.pAcHdle, "Algo  ", TRACE_LVL_INFO, NULL, 0, "Dump algo %s", pFactory->pCapabilities->pName, pFactory->pCapabilities->misc.pAlgoDesc);
      AudioChain_dumpParamDescriptor(hAcInstance.pAcHdle, pFactory->pStaticParamTemplate, AudioAlgo_getStaticConfig(hAlgo), "Algo  ");
      AudioChain_dumpParamDescriptor(hAcInstance.pAcHdle, pFactory->pDynamicParamTemplate, AudioAlgo_getDynamicConfig(hAlgo), "Algo  ");
      AudioChainFactory_dumpAlgoCommonValue(hAlgo);
      error = AUDIO_ERR_MGNT_NONE;
    }
  }
  return error;
}



/**
* @brief  Set an common algorithm  parameter
*         Documented in the header
*/

int32_t acAlgoSetCommonConfig(acAlgo hAlgo, const char *pKey, const void *pParam)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckAlgoHandle(hAlgo) && (pKey != NULL))
  {
    error = AudioChainFactory_setAlgoCommonConfig(hAlgo, pKey, (void *)pParam);
  }
  return error;
}


/**
* @brief  Get an common algorithm  parameter
*         Documented in the header
*/

int32_t acAlgoGetCommonConfig(acAlgo hAlgo, const char *pKey, void *pParam)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckAlgoHandle(hAlgo) && (pKey != NULL) && (pParam != NULL))
  {
    error = AudioChainFactory_getAlgoCommonConfig(hAlgo, pKey, pParam);
  }
  return error;
}




/**
* @brief  Set a chunk parameter configuration
*         Documented in the header
*/

int32_t acAlgoGetControl(acAlgo hAlgo, const char *pKey, void *pParam)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;

  if (s_acCheckAlgoHandle(hAlgo))
  {
    error = AudioAlgo_getControlParam(hAlgo, pKey, pParam);
  }
  return error;
}


/**
* @brief  Get a pointer on the alogithm's control structure
*         Documented in the header
*/

void *acAlgoGetControlPtr(acAlgo hAlgo)
{
  return AudioAlgo_getCtrlData(hAlgo);
}


/**
* @brief  Set a chunk parameter configuration
*         Documented in the header
*/

int32_t acAlgoSetControl(acAlgo hAlgo, const char *pKey, ...)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  va_list args;
  va_start(args, pKey);

  if (s_acCheckAlgoHandle(hAlgo))
  {
    error = AudioAlgo_setControlParam(hAlgo, pKey, args);
  }
  va_end(args);

  return error;
}

/**
* @brief  Set a chunk point parameter configuration
*         Documented in the header
*/

int32_t acAlgoGetControlVarPtr(acAlgo hAlgo, const char *pKey, void **ppOutParam)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;

  if (s_acCheckAlgoHandle(hAlgo) && (pKey != NULL) && (ppOutParam != NULL))
  {
    error = AudioAlgo_getControlVarPtr(hAlgo, pKey, ppOutParam);
  }
  return error;
}


/**
* @brief  Request an parameter update
*         Documented in the header
*/

int32_t acAlgoRequestUpdate(acAlgo hAlgo)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckAlgoHandle(hAlgo))
  {
    if (AudioChain_isStarted(hAcInstance.pAcHdle))
    {
      error = AudioAlgo_requestTuningUpdate(hAlgo);
      if (acErrorIsWarning(error))
      {
        error = AUDIO_ERR_MGNT_NONE; // ignore warnings
      }
    }
  }
  return error;
}


/**
* @brief  Create a chunk instance
*         Documented in the header
*/

int32_t acChunkCreate(acPipe hPipe, char *pChunkName, acChunk *pphChunk)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckPipeHandle(hPipe) && (pChunkName != NULL) && (pphChunk != NULL))
  {
    audio_chunk_t **ppChunk = (audio_chunk_t **)pphChunk;

    error = AudioChainSysIOs_create(pChunkName, ppChunk);
    if (error == AUDIO_ERR_MGNT_NOT_FOUND)
    {
      /* create a custom chunk */
      int32_t chunkMemPool = (int32_t)AC_DEFAULT_CHUNK_MEM_POOL;
      error = acEnvGetConfig("chunkMemPool", &chunkMemPool);
      error = AudioError_update(error, AudioChunk_create(ppChunk, pChunkName, AudioChain_getUtilsHdle(hPipe), (memPool_t)chunkMemPool));
    }
  }
  return error;
}


/**
* @brief  Dump all algorithm properties in the console log
*         Documented in the header
*/
int32_t acChunkDump(acChunk hChunk)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckChunkHandle(hChunk))
  {
    if (!AudioChunk_isSystem(hChunk))
    {
      audio_chunk_conf_t *const pConfChunk = AudioChunk_getConf(hChunk);

      AudioChain_trace(hAcInstance.pAcHdle, "Chunk ", TRACE_LVL_INFO, NULL, 0, "Dump chunk %s", pConfChunk->pName);
      AudioChain_dumpParamDescriptor(hAcInstance.pAcHdle, AudioChainFactory_getChunkTemplate(), pConfChunk, "Chunk  ");
      error = AUDIO_ERR_MGNT_NONE;
    }
  }
  return error;
}


/**
* @brief  Delete a chunk instance
*         Documented in the header
*/

int32_t acChunkDelete(acChunk hChunk)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckChunkHandle(hChunk))
  {
    error = AUDIO_ERR_MGNT_NONE;
  }
  return error;
}


/**
* @brief  Set a chunk parameter configuration
*         Documented in the header
*/
int32_t acChunkSetConfig(acChunk hChunk, const char *pKey, const char *pValue)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckChunkHandle(hChunk) && (pKey != NULL) && (pValue != NULL))
  {
    if (!AudioChain_isStarted(hAcInstance.pAcHdle))
    {
      error = AudioChunk_setParamConfig(hChunk, pKey, pValue);
    }
  }
  return error;
}

/**
* @brief  Connect an algo pin out to a chunk
*         Documented in the header
*/
int32_t acPipeConnectPinOut(acPipe hPipe, acAlgo hAlgo, uint32_t pinID, acChunk hChunk)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  /* notice, hPipe is not used, but present for futur evolutions */
  if (s_acCheckPipeHandle(hPipe))
  {
    if (s_acCheckAlgoHandle(hAlgo) && s_acCheckChunkHandle(hChunk))
    {
      /* the concept of pindef doesn't exist yet.
      so, for future use, acPipeConnectPinOut passes a pindef
      but ac doesn't use it and connect always the pin 0 */

      error = AudioAlgo_addOutput(hAlgo, hChunk);
    }
  }
  return error;
}

/**
* @brief  Connect an algo pin in to a chunk
*         Documented in the header
*/

int32_t acPipeConnectPinIn(acPipe hPipe, acAlgo hAlgo, uint32_t pinID, acChunk hChunk)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  /* notice, hPipe is not used, but present for futur evolutions */
  if (s_acCheckPipeHandle(hPipe))
  {
    if (s_acCheckAlgoHandle(hAlgo) && s_acCheckChunkHandle(hChunk))
    {
      error = AudioAlgo_setInput(hAlgo, hChunk, (int32_t)pinID);
    }
  }
  return error;
}


/**
* @brief  Connect two algos  to a chunk
*         Documented in the header
*/
int32_t acPipeConnect(acPipe hPipe, acAlgo hAlgoSrc, uint32_t srcPinID, acChunk hChunk, acAlgo hAlgoDst, uint32_t dstPinID)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  /* notice, hPipe is not used, but present for futur evolutions */
  if (s_acCheckPipeHandle(hPipe))
  {
    if (s_acCheckAlgoHandle(hAlgoSrc) && s_acCheckChunkHandle(hChunk) && s_acCheckAlgoHandle(hAlgoDst))
    {
      error = acPipeConnectPinOut(hPipe, hAlgoSrc, srcPinID, hChunk);
      if (acErrorIsOk(error))
      {
        error = acPipeConnectPinIn(hPipe, hAlgoDst, dstPinID, hChunk);
      }
    }
  }
  return error;
}


/**
* @brief  Start and stop the pipe
*         Documented in the header
*/
int32_t acPipePlay(acPipe hPipe, acState_t state)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckPipeHandle(hPipe))
  {
    uint32_t        bLogMalloc               = 0UL;
    uint32_t        bTuning                  = 0UL;
    uint32_t        bLogCycles               = 0UL;
    uint32_t        cyclesMgntMeasureTimeout = 0UL;
    uint32_t        cyclesMgntCbTimeout      = 0UL;
    uint32_t        bDefaultCyclesMgntCb     = 0UL;
    CycleStatsCb_t *pCycleCount              = NULL;

    error = acEnvGetConfig("bLogMalloc",                 &bLogMalloc);
    if (acErrorIsOk(error))
    {
      error = acEnvGetConfig("bTuning",                  &bTuning);
    }
    if (acErrorIsOk(error))
    {
      error = acEnvGetConfig("bLogCycles",               &bLogCycles);
    }
    if (acErrorIsOk(error))
    {
      error = acEnvGetConfig("bDefaultCyclesMgntCb",     &bDefaultCyclesMgntCb);
      if (bLogCycles)
      {
        /* if bLogCycles, check if the default cycleCount if true, else in any case don't use bDefanp ultCyclesMgntCb */
        if (bDefaultCyclesMgntCb)
        {
          pCycleCount = cycleMeasure_displayCpuLoadOnUart;
        }
      }
    }
    if (acErrorIsOk(error))
    {
      error = acEnvGetConfig("cyclesMgntMeasureTimeout", &cyclesMgntMeasureTimeout);
    }
    if (acErrorIsOk(error))
    {
      error = acEnvGetConfig("cyclesMgntCbTimeout",      &cyclesMgntCbTimeout);
    }
    if (acErrorIsOk(error))
    {
      AudioChain_setDataInOutTaskCycleMgntCbTimeout(hPipe,       pCycleCount, cyclesMgntMeasureTimeout, cyclesMgntCbTimeout);
      AudioChain_setProcessTaskCycleMgntCbTimeout(hPipe,         pCycleCount, cyclesMgntMeasureTimeout, cyclesMgntCbTimeout);
      AudioChain_setProcessLowLevelTaskCycleMgntCbTimeout(hPipe, pCycleCount, cyclesMgntMeasureTimeout, cyclesMgntCbTimeout);
      AudioChain_setControlTaskCycleMgntCbTimeout(hPipe,         pCycleCount, cyclesMgntMeasureTimeout, cyclesMgntCbTimeout);

      switch (state)
      {
        case AC_START:
        {
          error = AudioChain_configPendingChunks(hPipe);

          if (acErrorIsOk(error))
          {
            error = acEnvSetConfig("bTraceAsyncronous", AC_FALSE);
          }
          if (acErrorIsOk(error))
          {
            error = AudioChain_initGraph(hPipe); // may return a warning which doesn't prevent the graph to run
            if (acErrorIsWarning(error))
            {
              error = AUDIO_ERR_MGNT_NONE; // ignore warnings
            }
          }
          if (acErrorIsOk(error))
          {
            if (bTuning)
            {
              error = AudioChain_initTuning(hPipe);
            }
          }
          if (acErrorIsOk(error))
          {
            error = AudioError_update(error, acEnvSetConfig("bTraceAsyncronous", AC_TRUE));
          }
          else
          {
            error = AudioError_update(error, AudioChain_deinitTuning(hPipe));
            error = AudioError_update(error, AudioChain_deinitGraph(hPipe));
          }
          if (bLogMalloc)
          {
            AudioMallocDisplayRemainingMalloc(1);
          }
          break;
        }
        case AC_CLEANUP:
        /* for the time being, cleanup is equivalent to stop until we have tested the use case, cleanup must be called if errors have been detected during the pipe construction */
        case AC_STOP:
        {
          error = acEnvSetConfig("bTraceAsyncronous", AC_FALSE);
          error = AudioError_update(error, AudioChain_deinitTuning(hPipe));
          error = AudioError_update(error, AudioChain_deinitGraph(hPipe));

          if (bLogMalloc)
          {
            AudioMallocDisplayRemainingMalloc(1);
          }
          UTIL_AUDIO_RENDER_clearBuffer();
          break;
        }

        default:
        {
          break;
        }
      }
    }
  }
  return error;
}


/**
* @brief  return check if the pipe is playing
*         Documented in the header
*/
int32_t acPipeIsPlaying(acPipe hPipe)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (s_acCheckPipeHandle(hPipe))
  {
    if (AudioChain_isStarted(hAcInstance.pAcHdle))
    {
      error = AUDIO_ERR_MGNT_NONE;
    }
  }
  return error;
}


/**
* @brief  Dump the cycle count status
*         Documented in the header
*/

int32_t acCyclesDump(bool traceLog, bool interruptsOnly, float *pInterruptsPcent, float *pInterruptsMHz)
{
  cyclesMeasure_values_t cyclesMeasure_values;

  displayDetailedCycleCount(traceLog, interruptsOnly, &cyclesMeasure_values);
  if (pInterruptsPcent != NULL)
  {
    *pInterruptsPcent = cyclesMeasure_values.audioInterrupt.pcent + cyclesMeasure_values.uartInterrupt.pcent;
  }
  if (pInterruptsMHz != NULL)
  {
    *pInterruptsMHz = cyclesMeasure_values.audioInterrupt.MHz + cyclesMeasure_values.uartInterrupt.MHz;
  }

  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Dump the mem status
*         Documented in the header
*/

int32_t acAlgosMemDump(bool const verbose)
{
  displayDetailedAlgosMemoryUsage(verbose);

  return AUDIO_ERR_MGNT_NONE;
}


__weak void displayDetailedCycleCount(bool const traceLog, bool const interruptsOnly, cyclesMeasure_values_t *const pCyclesMeasure_values)
{
}


/**
* @brief  print a trace formatted
*         Documented in the header
*/

void acTrace(const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);
  trace_print_args(TRACE_OUTPUT_UART, TRACE_LVL_INFO, pFormat, args);
  va_end(args);
}


/**
* @brief  Set an environment configuration
*         Documented in the header
*/
int32_t acEnvSetConfig(const char *pKey, ...)
{
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (pKey)
  {
    va_list args;
    va_start(args, pKey);
    uint32_t value = va_arg(args, uint32_t);
    error          = AudioChainInstance_setEnvDescriptors(pKey, value);
  }
  return error;
}


/**
* @brief  Get an environment configuration
*         Documented in the header
*/
int32_t acEnvGetConfig(const char *pKey, void *pParam)
{
  return AudioChainInstance_getEnvConfig(pKey, pParam);
}



/**
* @brief  Get an environment descriptor with all env data
*         Documented in the header
*/
void *acEnvGetDescriptor(void)
{
  return (void *)AudioChainInstance_getEnvDescriptors();
}


/**
* @brief  Dump the env config
*         Documented in the header
*/
int32_t acEnvDump(void)
{
  return AudioChainInstance_dumpEnvDescriptors();
}


/**
* @brief  Check status from error code
*         Documented in the header
*/
int32_t acErrorIsOk(int32_t error)
{
  return (int32_t)AudioError_isOk(error);
}


/**
* @brief  Check status from error code
*         Documented in the header
*/
int32_t acErrorIsNok(int32_t error)
{
  return (int32_t)AudioError_isNok(error);
}


/**
* @brief  Check status from error code
*         Documented in the header
*/
int32_t acErrorIsWarning(int32_t error)
{
  return (int32_t)AudioError_isWarning(error);
}


/**
* @brief  Check status from error code
*         Documented in the header
*/
int32_t acErrorIsError(int32_t error)
{
  return (int32_t)AudioError_isError(error);
}


/**
* @brief  Check status from error code
*         Documented in the header
*/
int32_t acErrorIsNoError(int32_t error)
{
  return (int32_t)AudioError_isNoError(error);
}


/* Private Functions Definition ----------------------------------------------*/

/**
* @brief  Set a config parameter when the pipe is not started
*
*/
static int32_t s_acSetColdConfig(acAlgo hAlgo, const char *pKey, const char *pValue, char **ppErrorString)
{
  return AudioAlgo_setColdConfig(hAlgo, pKey, pValue, ppErrorString);
}


/**
* @brief  Set a config parameter when the pipe is  started
*
*/
static int32_t s_acSetHotConfig(acAlgo hAlgo, const char *pKey, const char *pValue, char **ppErrorString)
{
  return AudioAlgo_setHotConfig(hAlgo, pKey, pValue, ppErrorString);
}


/**
* @brief  return true if the opaque handle is an algorithm handle
*
*/
static bool s_acCheckAlgoHandle(acAlgo hAlgo)
{
  audio_algo_t *pAlgo = (audio_algo_t *)hAlgo;
  return (pAlgo != NULL) && (pAlgo->signature == AUDIO_ALGO_SIGNATURE);
}


/**
* @brief  return true if the opaque handle is a chunk handle
*
*/
static bool s_acCheckChunkHandle(acChunk hChunk)
{
  audio_chunk_t *pChunk = (audio_chunk_t *)hChunk;
  return (pChunk != NULL) && (pChunk->signature == AUDIO_CHUNK_SIGNATURE);
}


/**
* @brief  return true if the opaque handle is an instance handle
*
*/
static bool s_acCheckPipeHandle(acPipe hPipe)
{
  audio_chain_t *pChain = (audio_chain_t *)hPipe;
  return (pChain != NULL) && (pChain->signature == AUDIO_INSTANCE_SIGNATURE);
}
