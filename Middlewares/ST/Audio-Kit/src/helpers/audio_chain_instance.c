/**
******************************************************************************
* @file    audio_chain_instance.c
* @author  MCD Application Team
* @brief   Helper file that creates instance able to cope with up to three
*          parallel audio path
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
#include <stdlib.h>
#include <stdio.h>

#include "audio_chain_instance.h"
#include "audio_chain_sysIOs.h"
#include "audio_chain_tasks.h"
#include "cycles.h"
#include "irq_utils.h"


/* Global variables ----------------------------------------------------------*/
audio_chain_t AudioChainInstance;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct audio_chain_instance_env_data_t
{
  uint8_t      iChunkMemoryPool;
  uint8_t      iAlgoMemoryPool;
  uint8_t      bTuning;
  uint8_t      bLogInit;
  uint8_t      bLogMalloc;
  uint8_t      bLogCmsisOs;
  uint8_t      bLogCycles;
  uint8_t      bDefaultCycleCountMngtCb;
  uint32_t     iCycleCountCbTimeout;
  uint32_t     iCycleCountMeasureTimeout;
} audio_chain_instance_env_data_t;

/* Private function prototypes -----------------------------------------------*/
static void    s_error(void);
static void    s_enable_CRC_peripheral(void);
static int32_t s_envCb_setTraceLevel(audio_algo_t               *const pNull, void  *const arg);
static int32_t s_envCb_clearTraceLevel(audio_algo_t             *const pNull, void  *const arg);
static int32_t s_envCb_traceAsyncronous(audio_algo_t            *const pNull, void  *const arg);
static int32_t s_envCb_setLogInit(audio_algo_t                  *const pNull, void  *const arg);
static int32_t s_envCb_getLogInit(audio_algo_t                  *const pNull, void **const pData);
static int32_t s_envCb_setLogMalloc(audio_algo_t                *const pNull, void  *const arg);
static int32_t s_envCb_getLogMalloc(audio_algo_t                *const pNull, void **const pData);
static int32_t s_envCb_setLogCmsisOs(audio_algo_t               *const pNull, void  *const arg);
static int32_t s_envCb_getLogCmsisOs(audio_algo_t               *const pNull, void **const pData);
static int32_t s_envCb_setLogCycles(audio_algo_t                *const pNull, void  *const arg);
static int32_t s_envCb_getLogCycles(audio_algo_t                *const pNull, void **const pData);
static int32_t s_envCb_setDefaultCyclesMngtCb(audio_algo_t      *const pNull, void  *const args);
static int32_t s_envCb_getDefaultCyclesMngtCb(audio_algo_t      *const pNull, void **const pData);
static int32_t s_envCb_setCyclesMngtCbTimeout(audio_algo_t      *const pNull, void  *const arg);
static int32_t s_envCb_getCyclesMngtCbTimeout(audio_algo_t      *const pNull, void **const pData);
static int32_t s_envCb_setCyclesMngtMeasureTimeout(audio_algo_t *const pNull, void  *const arg);
static int32_t s_envCb_getCyclesMngtMeasureTimeout(audio_algo_t *const pNull, void **const pData);
static int32_t s_envCb_setChunkMemPool(audio_algo_t             *const pNull, void *const arg);
static int32_t s_envCb_getChunkMemPool(audio_algo_t             *const pNull, void **const pData);
static int32_t s_envCb_setAlgoMemPool(audio_algo_t              *const pNull, void *const arg);
static int32_t s_envCb_getAlgoMemPool(audio_algo_t              *const pNull, void **const pData);
static int32_t s_envCb_setTuning(audio_algo_t                   *const pNull, void  *const arg);
static int32_t s_envCb_getTuning(audio_algo_t                   *const pNull, void **const pData);
static int32_t s_envCb_initIssueMsgCb(audio_algo_t              *const pNull, void  *const arg);
static int32_t s_envCb_updateCfgMsgCb(audio_algo_t              *const pNull, void  *const arg);
static void    s_trace(const char                               *pFormat, ...);


/* Private variables ---------------------------------------------------------*/

static audio_chain_instance_env_data_t gEnvData =
{
  .iChunkMemoryPool          = AUDIO_MEM_TCM,
  .iAlgoMemoryPool           = AUDIO_MEM_RAMINT,
  .bTuning                   = 0U,
  .bLogInit                  = 0U,
  .bLogMalloc                = 0U,
  .bLogCmsisOs               = 0U,
  .bLogCycles                = 0U,
  .bDefaultCycleCountMngtCb  = 0U,
  .iCycleCountCbTimeout      = 5000UL,
  .iCycleCountMeasureTimeout = 500UL
};

static const audio_descriptor_key_value_t tTraceLevelType[] =
{
  {"debug",   TRACE_LVL_DEBUG},
  {"log",     TRACE_LVL_LOG},
  {"info",    TRACE_LVL_INFO},
  {"info2",   TRACE_LVL_INFO2},
  {"warning", TRACE_LVL_WARNING},
  {"error",   TRACE_LVL_ERROR},
  {0}
};


/*
List of environment commands
*/
static const audio_algo_descriptor_t tEnvParams[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Set the trace mode, direct output(false) or asynchronous(true). The mode true allows to display traces in back-ground to avoid impacts on real-time processing, the side effect is that some traces could be dropped."),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("uint32_t 0 or 1"),
    .pName           = "bTraceAsyncronous",
    .paramType       = AUDIO_DESC_PARAM_TYPE_INT32,
    .set_cb          = s_envCb_traceAsyncronous,
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Enable a trace category"),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("A string with one of these categories 'debug log info info2 warning error'"),
    .pName           = "setTrace",
    .paramType       = AUDIO_DESC_PARAM_TYPE_INT32,
    .set_cb          = s_envCb_setTraceLevel,
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Disable a trace category"),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("A string with one of these categories 'debug log info info2 warning error'"),
    .pName           = "clearTrace",
    .paramType       = AUDIO_DESC_PARAM_TYPE_INT32,
    .set_cb          = s_envCb_clearTraceLevel,
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Enable the tuning API. Using this API you can update algo parameters in real time."),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("uint32_t : AC_TRUE or AC_FALSE, default is AC_FALSE"),
    .pName           = "bTuning",
    .paramType       = AUDIO_DESC_PARAM_TYPE_UINT32,
    .set_cb          = s_envCb_setTuning,
    .get_cb          = s_envCb_getTuning
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Enable the init log at the beginning of the pipe playback (graph structure: algos connections through chunks)"),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("uint32_t : AC_TRUE or AC_FALSE, default is AC_FALSE"),
    .pName           = "bLogInit",
    .paramType       = AUDIO_DESC_PARAM_TYPE_UINT32,
    .set_cb          = s_envCb_setLogInit,
    .get_cb          = s_envCb_getLogInit
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Enable the malloc log during the pipe playback, signal leaks when we stop the pipe."),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("uint32_t : AC_TRUE or AC_FALSE, default is AC_FALSE"),
    .pName           = "bLogMalloc",
    .paramType       = AUDIO_DESC_PARAM_TYPE_UINT32,
    .set_cb          = s_envCb_setLogMalloc,
    .get_cb          = s_envCb_getLogMalloc
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Enable the CMSIS OS audio tasks log during the pipe playback (stacks/queues levels increase)."),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("uint32_t : AC_TRUE or AC_FALSE, default is AC_FALSE"),
    .pName           = "bLogCmsisOs",
    .paramType       = AUDIO_DESC_PARAM_TYPE_UINT32,
    .set_cb          = s_envCb_setLogCmsisOs,
    .get_cb          = s_envCb_getLogCmsisOs
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("Enable The Algo CycleCount log."),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("uint32_t : AC_TRUE or AC_FALSE, default is AC_FALSE"),
    .pName           = "bLogCycles",
    .paramType       = AUDIO_DESC_PARAM_TYPE_UINT32,
    .set_cb          = s_envCb_setLogCycles,
    .get_cb          = s_envCb_getLogCycles
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("if true, all algos will be setup with a default Cycles count callback that prints the cpuload on the console."),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("uint8_t : AC_TRUE = enabled, AC_FALSE = disabled"),
    .pName           = "bDefaultCyclesMgntCb",
    .paramType       = AUDIO_DESC_PARAM_TYPE_UINT32,
    .set_cb          = s_envCb_setDefaultCyclesMngtCb,
    .get_cb          = s_envCb_getDefaultCyclesMngtCb
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("call-back period in ms for the cycle count management (for instance used for periodical display)."),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("uint32_t : time in ms, default is 5000"),
    .pName           = "cyclesMgntCbTimeout",
    .paramType       = AUDIO_DESC_PARAM_TYPE_UINT32,
    .set_cb          = s_envCb_setCyclesMngtCbTimeout,
    .get_cb          = s_envCb_getCyclesMngtCbTimeout
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("measure period in ms for the cycle count management."),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("uint32_t : time in ms, default is 500"),
    .pName           = "cyclesMgntMeasureTimeout",
    .paramType       = AUDIO_DESC_PARAM_TYPE_UINT32,
    .set_cb          = s_envCb_setCyclesMngtMeasureTimeout,
    .get_cb          = s_envCb_getCyclesMngtMeasureTimeout
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("This callback will be called during the pipe initialization, when an issue is detected"),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("A callback pointer typedef void (*)(const char *const pMsg), this string format is [errorType]:[instance]:[comment]"),
    .pName           = "initIssueMsgCb",
    .paramType       = AUDIO_DESC_PARAM_TYPE_ADDRESS,
    .set_cb          = s_envCb_initIssueMsgCb,
    .get_cb          = NULL
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("This callback will be called when algo's param update needs an update on livetune side"),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("A callback pointer typedef void (*)(const char *const pMsg)"),
    .pName           = "updateCfgMsgCb",
    .paramType       = AUDIO_DESC_PARAM_TYPE_ADDRESS,
    .set_cb          = s_envCb_updateCfgMsgCb,
    .get_cb          = NULL
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("chunk memory pool"),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("memPool_t enum from audio_mem_mgnt"),
    .pName           = "chunkMemPool",
    .paramType       = AUDIO_DESC_PARAM_TYPE_INT32,
    .set_cb          = s_envCb_setChunkMemPool,
    .get_cb          = s_envCb_getChunkMemPool
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("algo algo (handle & conf) memory pool"),
    .pExpectedValue  = AUDIO_ALGO_OPT_STR("memPool_t enum from audio_mem_mgnt"),
    .pName           = "algoMemPool",
    .paramType       = AUDIO_DESC_PARAM_TYPE_INT32,
    .set_cb          = s_envCb_setAlgoMemPool,
    .get_cb          = s_envCb_getAlgoMemPool
  },
  {0}
};


/* Functions Definition ------------------------------------------------------*/
const audio_algo_descriptor_t *AudioChainInstance_getEnvDescriptors(void)
{
  return tEnvParams;
}


void AudioChainInstance_init(audio_chain_instance_params_t *const pParams)
{
  audio_chain_utilities_t AudioChainUtils;
  audio_chain_tasks_t     AudioChainTasks;

  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG, "\tSdkVersion  : %s\n", AC_VERSION);

  /* All resets */
  memset(&AudioChainTasks,    0, sizeof(audio_chain_tasks_t));
  memset(&AudioChainUtils,    0, sizeof(audio_chain_utilities_t));
  memset(&AudioChainInstance, 0, sizeof(AudioChainInstance));

  /* Enable CRC peripheral to unlock the PDM library */
  s_enable_CRC_peripheral();

  /* Init AudioChain instance from system connections */
  if (AudioError_isError(AudioChain_init(&AudioChainInstance)))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_init error");
  }

  /* Reset error callback to a default local routine and other stuff about errors management */
  AudioChainUtils.error                      = s_error;
  AudioChainUtils.lockFwOnAssert             = true;
  AudioChainUtils.cyclesCbs.coreClock        = cycleMeasure_getSystemCoreClock;
  AudioChainUtils.cyclesCbs.currentCycles    = cycleMeasure_currentCycles;
  AudioChainUtils.cyclesCbs.init             = cycleMeasure_Init;
  AudioChainUtils.cyclesCbs.reset            = cycleMeasure_Reset;
  AudioChainUtils.cyclesCbs.fullReset        = cycleMeasure_FullReset;
  AudioChainUtils.cyclesCbs.start            = cycleMeasure_Start;
  AudioChainUtils.cyclesCbs.stop             = cycleMeasure_Stop;
  AudioChainUtils.cyclesCbs.isTimeoutExpired = cycleMeasure_isTimeoutExpired;
  AudioChainUtils.getNbMsFromStart           = AudioChainInstance_getNbMsFromStart;
  if (pParams->traceEnable)
  {
    AudioChainUtils.trace        = trace_log;
    AudioChainUtils.enableTraces = true;
  }
  AudioChain_registerUtils(&AudioChainInstance, &AudioChainUtils);

  // logInit & logCmsisOs must be set after AudioChain_registerUtils
  AudioChain_setLogInit(&AudioChainInstance,    pParams->logInit);
  AudioChain_setLogCmsisOs(&AudioChainInstance, pParams->logCmsisOs);

  // set frame duration and run duration
  AudioChain_setFrameAndRunDurations(&AudioChainInstance, (1000000UL / AC_N_MS_DIV) * AC_FRAME_MS, (1000000UL / AC_N_MS_DIV) * AC_N_MS_PER_RUN);

  // set max cpu load during reinit (must be called after AudioChain_setFrameAndRunDurations())
  AudioChain_setMaxReinitLoadPcent(&AudioChainInstance, 10U); // algo's reinit cpu load mustn't exceed 10%

  if (pParams->isDataInOutSpecificTask)
  {
    AudioChainTasks.dataInOut.cbs.init    = AudioChain_task_create_dataInOut;
    AudioChainTasks.dataInOut.cbs.deinit  = AudioChain_task_terminate_dataInOut;
    AudioChainTasks.dataInOut.cbs.trigger = AudioChain_task_trigger_dataInOut;
  }

  if (pParams->isProcessSpecificTask)
  {
    AudioChainTasks.process.cbs.init      = AudioChain_task_create_process;
    AudioChainTasks.process.cbs.deinit    = AudioChain_task_terminate_process;
    AudioChainTasks.process.cbs.trigger   = AudioChain_task_trigger_process;
  }

  if (pParams->isControlSpecificTask)
  {
    AudioChainTasks.control.cbs.init      = AudioChain_task_create_control;
    AudioChainTasks.control.cbs.deinit    = AudioChain_task_terminate_control;
    AudioChainTasks.control.cbs.trigger   = AudioChain_task_trigger_control;
  }

  AudioChain_registerTasks(&AudioChainInstance, &AudioChainTasks);
  AudioChain_registerMutexCbs(&AudioChainInstance, disable_irq_with_cnt, enable_irq_with_cnt);

  // initializations for memory stats
  AudioChain_setBuffersMallocStatsPtr(&AudioChainInstance);
  AudioChain_setChunksMallocStatsPtr(&AudioChainInstance);
  AudioAlgo_setAudioChainHdle(&AudioChainInstance);

  if (pParams->logInit)
  {
    AudioChain_trace(&AudioChainInstance, "AUDIO_CHAIN", TRACE_LVL_INFO, NULL, 0, "Audio Chain initialized");
  }
}


void AudioChainInstance_deinit(void)
{
  if (AudioError_isError(AudioChain_deinit(&AudioChainInstance)))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_deinit error");
  }
}


void AudioChainInstance_idle(void)
{
  if (AudioChainInstance_isDeinitGraphRequested())
  {
    AudioChainInstance_deinitTuning();
    AudioChainInstance_deinitGraph();
  }
}


void AudioChainInstance_deinitGraph(void)
{
  if (AudioError_isError(AudioChain_deinitGraph(&AudioChainInstance)))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_deinitGraph error");
  }
}

void AudioChainInstance_initTuning(void)
{
  #if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)
  if (AudioError_isError(AudioChain_initTuning(&AudioChainInstance)))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_initTuning error");
  }
  #endif
}


void AudioChainInstance_deinitTuning(void)
{
  #if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)
  if (AudioError_isError(AudioChain_deinitTuning(&AudioChainInstance)))
  {
    AudioChain_error(&AudioChainInstance, __FILE__, __LINE__, "AudioChain_deinitTuning error");
  }
  #endif
}


void AudioChainInstance_deinitGraphRequest(void)
{
  AudioChain_deinitGraphRequest(&AudioChainInstance);
}


bool AudioChainInstance_isInitialized(void)
{
  return AudioChain_isInitialized(&AudioChainInstance);
}


bool AudioChainInstance_isStarted(void)
{
  return AudioChain_isStarted(&AudioChainInstance);
}


bool AudioChainInstance_isDeinitGraphRequested(void)
{
  return AudioChain_isDeinitGraphRequested(&AudioChainInstance);
}


uint64_t AudioChainInstance_getNbMsFromStart(void)
{
  return (AudioChain_getInterruptNb(&AudioChainInstance) / AC_N_MS_DIV) * (uint64_t)AC_N_MS_PER_RUN;
}


void AudioChainInstance_run(void)
{
  AudioChain_run(&AudioChainInstance);
}


void AudioChainInstance_error(const char *pFile, int const line, const char *pErrorMsg)
{
  AudioChain_error(&AudioChainInstance, pFile, line, pErrorMsg);
}


int32_t AudioChainInstance_getChunkAlgoOut(audio_chunk_t *const pChunk, uint8_t *const pAlgoId, uint8_t *const pChunkId)
{
  audio_algo_t *pAlgo = NULL;
  int32_t       error = AudioChain_getChunkAlgoOut(&AudioChainInstance, pChunk, &pAlgo, pChunkId);

  if (AudioError_isOk(error))
  {
    *pAlgoId = AudioAlgo_getIndex(pAlgo);
  }

  return error;
}


int32_t AudioChainInstance_getChunkAlgoIn(audio_chunk_t *const pChunk, uint8_t *const pAlgoId, uint8_t *const pChunkId)
{
  audio_algo_t *pAlgo = NULL;
  int32_t       error = AudioChain_getChunkAlgoIn(&AudioChainInstance, pChunk, &pAlgo, pChunkId);

  if (AudioError_isOk(error))
  {
    *pAlgoId = AudioAlgo_getIndex(pAlgo);
  }

  return error;
}


bool AudioChainInstance_setEnableCyclesCnt(bool const enable)
{
  return AudioChain_setEnableCyclesCnt(&AudioChainInstance, enable);
}


bool AudioChainInstance_getCyclesCntStatus(void)
{
  return AudioChain_getCyclesCntStatus(&AudioChainInstance);
}


bool AudioChainInstance_setEnableTraces(bool const enable)
{
  return AudioChain_setEnableTraces(&AudioChainInstance, enable);
}


bool AudioChainInstance_getTracesStatus(void)
{
  return AudioChain_getTracesStatus(&AudioChainInstance);
}


uint32_t AudioChainInstance_getCycleCountMeasureTimeout(void)
{
  return gEnvData.iCycleCountMeasureTimeout;
}


uint32_t AudioChainInstance_getCycleCountCbTimeout(void)
{
  return gEnvData.iCycleCountCbTimeout;
}


int32_t AudioChainInstance_dumpFactoryEntry(void)
{
  return AudioChain_dumpFactoryEntry(&AudioChainInstance);
}


/**
* @brief  Set an environment configuration
*/
int32_t AudioChainInstance_setEnvDescriptors(const char *pKey, uint32_t value)
{
  int32_t  error = AUDIO_ERR_MGNT_ERROR;
  if (pKey != NULL)
  {
    error  = AudioAlgo_setCommonDescConfig(NULL, tEnvParams, pKey, (void *)value);  /*cstat !MISRAC2012-Rule-11.6 cast to pointer because it's the API*/
  }
  return error;
}



/**
* @brief  Set an environment configuration
*/
int32_t AudioChainInstance_dumpEnvDescriptors(void)
{
  const audio_algo_descriptor_t  *pCommonDesc = tEnvParams;
  while (pCommonDesc->pName)
  {
    s_trace("%-29s : %-20s\n", "Name",  pCommonDesc->pName);
    s_trace("\t%-21s : %-14s\n", "Description",  pCommonDesc->pDescription);
    s_trace("\t%-21s : %-14s\n", "Expected Value",  pCommonDesc->pExpectedValue);
    pCommonDesc++;
  }
  return AUDIO_ERR_MGNT_NONE;
}



/**
* @brief  Get an environment configuration
*         Documented in the header
*/
int32_t AudioChainInstance_getEnvConfig(const char *pKey, void *pParam)
{
  int32_t  error = AUDIO_ERR_MGNT_ERROR;
  if ((pKey != NULL) && (pParam != NULL))
  {
    error = AudioChainFactory_getAlgoCommonDescConfig(NULL, tEnvParams, pKey, pParam);
  }
  return error;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// static (local) routines definition ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void s_error(void)
{
  trace_setAsynchronous(false);
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_ERROR, "Audio Chain error => stop graph\n");
  AudioChainInstance_deinitGraphRequest();
}


static void s_enable_CRC_peripheral(void)
{
  __HAL_RCC_CRC_CLK_ENABLE();
  // reset is managed by the PDM lib
}


/**
* @brief  Set/Get default log cycles
*
*/
static int32_t s_envCb_setDefaultCyclesMngtCb(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  uint8_t value = (uint8_t)(uint32_t)arg;   /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.bDefaultCycleCountMngtCb = value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getDefaultCyclesMngtCb(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = (uint32_t)gEnvData.bDefaultCycleCountMngtCb;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set/Get call-back cycles timeout
*
*/
static int32_t s_envCb_setCyclesMngtCbTimeout(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  uint32_t value = (uint32_t)arg; /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.iCycleCountCbTimeout = value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getCyclesMngtCbTimeout(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = gEnvData.iCycleCountCbTimeout;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set/Get measure log cycles timeout
*
*/
static int32_t s_envCb_setCyclesMngtMeasureTimeout(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  uint32_t value = (uint32_t)arg; /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.iCycleCountMeasureTimeout = value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getCyclesMngtMeasureTimeout(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = gEnvData.iCycleCountMeasureTimeout;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set/Get chunk memory pool
*
*/
static int32_t s_envCb_setChunkMemPool(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  uint32_t value = (uint32_t)arg; /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.iChunkMemoryPool = (uint8_t)value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getChunkMemPool(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = (uint32_t)gEnvData.iChunkMemoryPool;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set/Get algo memory pool
*
*/
static int32_t s_envCb_setAlgoMemPool(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  uint32_t value = (uint32_t)arg; /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.iAlgoMemoryPool = (uint8_t)value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getAlgoMemPool(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = (uint32_t)gEnvData.iAlgoMemoryPool;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set/Get default log init
*
*/
static int32_t s_envCb_setLogInit(audio_algo_t *const pNull, void  *const arg)
{
  (void)pNull;  // unused parameter
  uint8_t value = (uint8_t)(uint32_t)arg; /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.bLogInit = value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getLogInit(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = (uint32_t)gEnvData.bLogInit;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set/Get default trace allocs
*
*/
static int32_t s_envCb_setLogMalloc(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  uint8_t value = (uint8_t)(uint32_t)arg; /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.bLogMalloc = value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getLogMalloc(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = (uint32_t)gEnvData.bLogMalloc;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set/Get default trace CMSIS OS
*
*/
static int32_t s_envCb_setLogCmsisOs(audio_algo_t *const pNull, void  *const arg)
{
  (void)pNull;  // unused parameter
  uint8_t value = (uint8_t)(uint32_t)arg; /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.bLogCmsisOs = value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getLogCmsisOs(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = (uint32_t)gEnvData.bLogCmsisOs;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set/Get default trace allocs
*
*/
static int32_t s_envCb_setLogCycles(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  uint8_t value = (uint8_t)(uint32_t)arg; /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.bLogCycles = value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getLogCycles(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = (uint32_t)gEnvData.bLogCycles;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set/Get default tuning enable
*
*/
static int32_t s_envCb_setTuning(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  uint8_t value = (uint8_t)(uint32_t)arg; /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  gEnvData.bTuning = value;
  return AUDIO_ERR_MGNT_NONE;
}


static int32_t s_envCb_getTuning(audio_algo_t *const pNull, void **const pData)
{
  (void)pNull;  // unused parameter
  *((uint32_t *)pData) = (uint32_t)gEnvData.bTuning;
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set the trace mode
*
*/
static int32_t s_envCb_traceAsyncronous(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  trace_setAsynchronous((bool)arg);   /*cstat !MISRAC2012-Rule-11.6 cast from pointer because it's the API*/
  return AUDIO_ERR_MGNT_NONE;
}


/**
* @brief  Set the trace flag
*
*/
static int32_t s_envCb_setTraceLevel(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (arg)
  {
    uint32_t value = 0UL;
    error = AudioDescriptor_getValueFromKey(tTraceLevelType, arg, &value, NULL);
    if (AudioError_isOk(error))
    {
      uint32_t curTrace = (uint32_t)trace_getLevel();
      curTrace |= value;
      trace_setLevel((traceLvl_t)curTrace);
    }
  }
  return error;
}


/**
* @brief  Clear the trace flag
*
*/
static int32_t s_envCb_clearTraceLevel(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  int32_t error = AUDIO_ERR_MGNT_ERROR;
  if (arg)
  {
    uint32_t value = 0UL;
    error = AudioDescriptor_getValueFromKey(tTraceLevelType, arg, &value, NULL);
    if (AudioError_isOk(error))
    {
      uint32_t curTrace = (uint32_t)trace_getLevel();
      curTrace &= ~value;
      trace_setLevel((traceLvl_t)curTrace);
    }
  }
  return error;
}


/**
* @brief  Set the callback for hot init error detection
*
*/

static int32_t s_envCb_initIssueMsgCb(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  typedef void (*const _warningMsgCb)(const char *const pMsg);
  int32_t error = AUDIO_ERR_MGNT_NONE;
  AudioChain_registerWarningMsgCb(&AudioChainInstance, (_warningMsgCb)arg); /*cstat !MISRAC2012-Rule-11.1 cast from pointer because it's the API*/
  return error;
}


/**
* @brief  Set the callback for update config
*
*/

static int32_t s_envCb_updateCfgMsgCb(audio_algo_t *const pNull, void *const arg)
{
  (void)pNull;  // unused parameter
  typedef void (*const _updateCfgMsgCb)(const char *const pMsg);
  int32_t error = AUDIO_ERR_MGNT_NONE;
  AudioChain_registerUpdateCfgMsgCb(&AudioChainInstance, (_updateCfgMsgCb)arg); /*cstat !MISRAC2012-Rule-11.1 cast from pointer because it's the API*/
  return error;
}


/**
* @brief  print a trace formatted
*         Documented in the header
*/

void s_trace(const char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);
  trace_print_args(TRACE_OUTPUT_UART, TRACE_LVL_INFO, pFormat, args);
  va_end(args);
}
